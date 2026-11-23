import org.basaltmc.proxy.config.configStr
import org.basaltmc.proxy.core.Ctx
import org.basaltmc.proxy.core.Priority
import org.basaltmc.proxy.events.eventPlayer
import org.basaltmc.proxy.events.on
import org.basaltmc.proxy.log.logError
import org.basaltmc.proxy.log.logInfo
import org.basaltmc.proxy.network.antibotBan
import org.basaltmc.proxy.network.antibotEventGet
import org.basaltmc.proxy.network.antibotIsVerified
import org.basaltmc.proxy.network.antibotOn
import org.basaltmc.proxy.network.antibotUnban
import org.basaltmc.proxy.network.antibotUnderAttack
import org.basaltmc.proxy.network.antibotUnverify
import org.basaltmc.proxy.network.antibotVerify
import org.basaltmc.proxy.network.channelRegister
import org.basaltmc.proxy.network.channelUnregister
import org.basaltmc.proxy.network.messageGet
import org.basaltmc.proxy.network.messageOn
import org.basaltmc.proxy.network.messageSend
import org.basaltmc.proxy.network.packetBody
import org.basaltmc.proxy.network.packetCancel
import org.basaltmc.proxy.network.packetGet
import org.basaltmc.proxy.network.packetNames
import org.basaltmc.proxy.network.packetOn
import org.basaltmc.proxy.network.packetReceive
import org.basaltmc.proxy.network.packetRemove
import org.basaltmc.proxy.network.packetResolve
import org.basaltmc.proxy.network.packetSend
import org.basaltmc.proxy.network.packetSetBody
import org.basaltmc.proxy.network.packetWatch
import org.basaltmc.proxy.values.Value
import org.basaltmc.proxy.values.ValueWriter

var rewriteMarker = ""
var cancelMarker = ""
var channel = ""
var refusedOnce = false

fun find(hay: ByteArray, needle: String): Int {
    val n = needle.encodeToByteArray()
    if (n.isEmpty()) return -1
    var at = 0
    while (at + n.size <= hay.size) {
        var i = 0
        while (i < n.size && hay[at + i] == n[i]) i++
        if (i == n.size) return at
        at++
    }
    return -1
}

fun text(map: Value, key: String): String = map.find(key)?.asString() ?: ""

fun chatBody(s: String): ByteArray {
    val t = s.encodeToByteArray()
    return byteArrayOf(0x08, (t.size shr 8).toByte(), t.size.toByte()) + t + byteArrayOf(0)
}

fun stringPrefixed(first: String, rest: String): ByteArray {
    val a = first.encodeToByteArray()
    return byteArrayOf(a.size.toByte()) + a + rest.encodeToByteArray()
}

fun onMessage(ctx: Ctx) {
    val map = messageGet(ctx)?.let { Value.open(it) }
    if (map == null) {
        logError("message read failed")
        return
    }
    val data = map.find("data")?.asBytes()?.decodeToString() ?: ""
    logInfo("net: message ${text(map, "source")} ${text(map, "channel")} $data")
    val player = eventPlayer(ctx)
    if (player > 0 && messageSend(player, "basalt:demo", "pong".encodeToByteArray(), true) == 0) {
        logInfo("net: reply sent")
    }
}

fun onDecider(ctx: Ctx) {
    val map = packetGet(ctx)?.let { Value.open(it) }
    val body = packetBody(ctx)
    if (map == null || body == null) {
        logError("packet read failed")
        return
    }
    val at = find(body, rewriteMarker)
    if (find(body, cancelMarker) >= 0) {
        if (packetCancel(ctx) == 0) logInfo("net: cancel for ${text(map, "player_name")}")
    } else if (at >= 0) {
        for (k in 0 until rewriteMarker.encodeToByteArray().size) {
            if (body[at + k] in 'a'.code.toByte()..'z'.code.toByte()) body[at + k] = (body[at + k] - 32).toByte()
        }
        if (packetSetBody(ctx, body) == 0) logInfo("net: rewrite for ${text(map, "player_name")}")
    }
}

fun onWatch(ctx: Ctx) {
    val map = packetGet(ctx)?.let { Value.open(it) } ?: return
    if (!refusedOnce && packetCancel(ctx) < 0) {
        refusedOnce = true
        logInfo("net: observer cancel refused")
    }
    if (map.find("cancelled")?.asBool() == true) logInfo("net: watch saw a cancelled packet")
}

fun onAntibot(ctx: Ctx) {
    val map = antibotEventGet(ctx)?.let { Value.open(it) } ?: return
    logInfo("net: antibot ${text(map, "kind")} ${text(map, "action")}")
}

fun idle(ctx: Ctx) {}

fun onJoin(ctx: Ctx) {
    val player = eventPlayer(ctx)
    if (player <= 0) return
    val body = chatBody("net welcome")
    if (packetSend(player, "clientbound.system_chat", body) == 0) logInfo("net: sent welcome")
    if (packetSend(player, "serverbound.play.chat_command", body) < 0) {
        logInfo("net: wrong direction refused")
    }
    if (messageSend(player, channel, "hello".encodeToByteArray(), true) == 0) {
        logInfo("net: sent hello message")
    }
    if (packetReceive(player, "serverbound.play.custom_payload", stringPrefixed(channel, "ping")) == 0) {
        logInfo("net: injected custom payload")
    }
    if (packetReceive(player, "serverbound.play.chat_command", stringPrefixed("list", "")) == 0) {
        logInfo("net: injected command")
    }
}

fun onEnable(ctx: Ctx) {
    val names = packetNames()?.let { Value.open(it) }
    if (names != null && names.count() > 100) logInfo("net: packets known")
    val resolved = packetResolve("clientbound.system_chat")?.let { Value.open(it) }
    if (resolved != null && resolved.find("serverbound")?.asBool() == false) {
        logInfo("net: resolved a clientbound play packet")
    }
    if (packetResolve("nope.nothing") == null) logInfo("net: unknown packet refused")
    if (channelRegister(channel) == 0) logInfo("net: channel registered")
    if (channelRegister("BungeeCord") < 0) logInfo("net: bungee channel refused")
    if (channelRegister("basalt:temp") == 0 && channelUnregister("basalt:temp") == 0 &&
        channelUnregister("basalt:temp") < 0
    ) {
        logInfo("net: temp channel unregistered")
    }
    messageOn("basalt:demo", ::onMessage)
    val opts = ValueWriter().beginMap().key("priority").string("late").key("fail").string("closed").end().finish()
        ?: ByteArray(0)
    if (packetOn("clientbound.system_chat", opts, ::onDecider) > 0) logInfo("net: decider listening")
    packetWatch("clientbound.system_chat", ByteArray(0), ::onWatch)
    val listener = packetOn("serverbound.play.*", ByteArray(0), ::idle)
    if (listener > 0 && packetRemove(listener) == 0 && packetRemove(listener) < 0) {
        logInfo("net: listener removed")
    }
    if (packetOn("nope.nothing", ByteArray(0), ::idle) < 0) logInfo("net: bad listener name refused")
    if (packetGet(9999) == null) logInfo("net: bad context refused")
    antibotOn(::onAntibot)
    antibotVerify("NetBot")
    logInfo(if (antibotIsVerified("NetBot") == 1) "net: verified yes" else "net: verified no")
    antibotUnverify("NetBot")
    logInfo(if (antibotIsVerified("NetBot") == 0) "net: unverified" else "net: still verified")
    if (antibotBan("203.0.113.9") == 0) logInfo("net: ban ok")
    if (antibotUnban("203.0.113.9") == 0) logInfo("net: unban ok")
    if (antibotUnban("203.0.113.9") < 0) logInfo("net: unban absent")
    if (antibotBan("not an address") < 0) logInfo("net: bad address refused")
    logInfo(if (antibotUnderAttack() == 0) "net: no attack" else "net: under attack")
}

fun init() {
    rewriteMarker = configStr("rewrite-marker") ?: ""
    cancelMarker = configStr("cancel-marker") ?: ""
    channel = configStr("channel") ?: ""
    on("plugin.enable", Priority.NORMAL, ::onEnable)
    on("player.join", Priority.NORMAL, ::onJoin)
}
