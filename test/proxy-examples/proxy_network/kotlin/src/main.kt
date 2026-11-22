import org.basaltmc.proxy.core.Ctx
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
    val map = org.basaltmc.proxy.network.messageGet(ctx)?.let { Value.open(it) }
    if (map == null) {
        org.basaltmc.proxy.log.logError("message read failed")
        return
    }
    val data = map.find("data")?.asBytes()?.decodeToString() ?: ""
    org.basaltmc.proxy.log.logInfo("net: message ${text(map, "source")} ${text(map, "channel")} $data")
    val player = org.basaltmc.proxy.events.eventPlayer(ctx)
    if (player > 0 && org.basaltmc.proxy.network.messageSend(player, "basalt:demo", "pong".encodeToByteArray(), true) == 0) {
        org.basaltmc.proxy.log.logInfo("net: reply sent")
    }
}

fun onDecider(ctx: Ctx) {
    val map = org.basaltmc.proxy.network.packetGet(ctx)?.let { Value.open(it) }
    val body = org.basaltmc.proxy.network.packetBody(ctx)
    if (map == null || body == null) {
        org.basaltmc.proxy.log.logError("packet read failed")
        return
    }
    val at = find(body, rewriteMarker)
    if (find(body, cancelMarker) >= 0) {
        if (org.basaltmc.proxy.network.packetCancel(ctx) == 0) org.basaltmc.proxy.log.logInfo("net: cancel for ${text(map, "player_name")}")
    } else if (at >= 0) {
        for (k in 0 until rewriteMarker.encodeToByteArray().size) {
            if (body[at + k] in 'a'.code.toByte()..'z'.code.toByte()) body[at + k] = (body[at + k] - 32).toByte()
        }
        if (org.basaltmc.proxy.network.packetSetBody(ctx, body) == 0) org.basaltmc.proxy.log.logInfo("net: rewrite for ${text(map, "player_name")}")
    }
}

fun onWatch(ctx: Ctx) {
    val map = org.basaltmc.proxy.network.packetGet(ctx)?.let { Value.open(it) } ?: return
    if (!refusedOnce && org.basaltmc.proxy.network.packetCancel(ctx) < 0) {
        refusedOnce = true
        org.basaltmc.proxy.log.logInfo("net: observer cancel refused")
    }
    if (map.find("cancelled")?.asBool() == true) org.basaltmc.proxy.log.logInfo("net: watch saw a cancelled packet")
}

fun onAntibot(ctx: Ctx) {
    val map = org.basaltmc.proxy.network.antibotEventGet(ctx)?.let { Value.open(it) } ?: return
    org.basaltmc.proxy.log.logInfo("net: antibot ${text(map, "kind")} ${text(map, "action")}")
}

fun idle(ctx: Ctx) {}

fun onJoin(ctx: Ctx) {
    val player = org.basaltmc.proxy.events.eventPlayer(ctx)
    if (player <= 0) return
    val body = chatBody("net welcome")
    if (org.basaltmc.proxy.network.packetSend(player, "clientbound.system_chat", body) == 0) org.basaltmc.proxy.log.logInfo("net: sent welcome")
    if (org.basaltmc.proxy.network.packetSend(player, "serverbound.play.chat_command", body) < 0) {
        org.basaltmc.proxy.log.logInfo("net: wrong direction refused")
    }
    if (org.basaltmc.proxy.network.messageSend(player, channel, "hello".encodeToByteArray(), true) == 0) {
        org.basaltmc.proxy.log.logInfo("net: sent hello message")
    }
    if (org.basaltmc.proxy.network.packetReceive(player, "serverbound.play.custom_payload", stringPrefixed(channel, "ping")) == 0) {
        org.basaltmc.proxy.log.logInfo("net: injected custom payload")
    }
    if (org.basaltmc.proxy.network.packetReceive(player, "serverbound.play.chat_command", stringPrefixed("list", "")) == 0) {
        org.basaltmc.proxy.log.logInfo("net: injected command")
    }
}

fun onEnable(ctx: Ctx) {
    val names = org.basaltmc.proxy.network.packetNames()?.let { Value.open(it) }
    if (names != null && names.count() > 100) org.basaltmc.proxy.log.logInfo("net: packets known")
    val resolved = org.basaltmc.proxy.network.packetResolve("clientbound.system_chat")?.let { Value.open(it) }
    if (resolved != null && resolved.find("serverbound")?.asBool() == false) {
        org.basaltmc.proxy.log.logInfo("net: resolved a clientbound play packet")
    }
    if (org.basaltmc.proxy.network.packetResolve("nope.nothing") == null) org.basaltmc.proxy.log.logInfo("net: unknown packet refused")
    if (org.basaltmc.proxy.network.channelRegister(channel) == 0) org.basaltmc.proxy.log.logInfo("net: channel registered")
    if (org.basaltmc.proxy.network.channelRegister("BungeeCord") < 0) org.basaltmc.proxy.log.logInfo("net: bungee channel refused")
    if (org.basaltmc.proxy.network.channelRegister("basalt:temp") == 0 && org.basaltmc.proxy.network.channelUnregister("basalt:temp") == 0 &&
        org.basaltmc.proxy.network.channelUnregister("basalt:temp") < 0
    ) {
        org.basaltmc.proxy.log.logInfo("net: temp channel unregistered")
    }
    org.basaltmc.proxy.network.messageOn("basalt:demo", ::onMessage)
    val opts = ValueWriter().beginMap().key("priority").string("late").key("fail").string("closed").end().finish()
        ?: ByteArray(0)
    if (org.basaltmc.proxy.network.packetOn("clientbound.system_chat", opts, ::onDecider) > 0) org.basaltmc.proxy.log.logInfo("net: decider listening")
    org.basaltmc.proxy.network.packetWatch("clientbound.system_chat", ByteArray(0), ::onWatch)
    val listener = org.basaltmc.proxy.network.packetOn("serverbound.play.*", ByteArray(0), ::idle)
    if (listener > 0 && org.basaltmc.proxy.network.packetRemove(listener) == 0 && org.basaltmc.proxy.network.packetRemove(listener) < 0) {
        org.basaltmc.proxy.log.logInfo("net: listener removed")
    }
    if (org.basaltmc.proxy.network.packetOn("nope.nothing", ByteArray(0), ::idle) < 0) org.basaltmc.proxy.log.logInfo("net: bad listener name refused")
    if (org.basaltmc.proxy.network.packetGet(9999) == null) org.basaltmc.proxy.log.logInfo("net: bad context refused")
    org.basaltmc.proxy.network.antibotOn(::onAntibot)
    org.basaltmc.proxy.network.antibotVerify("NetBot")
    org.basaltmc.proxy.log.logInfo(if (org.basaltmc.proxy.network.antibotIsVerified("NetBot") == 1) "net: verified yes" else "net: verified no")
    org.basaltmc.proxy.network.antibotUnverify("NetBot")
    org.basaltmc.proxy.log.logInfo(if (org.basaltmc.proxy.network.antibotIsVerified("NetBot") == 0) "net: unverified" else "net: still verified")
    if (org.basaltmc.proxy.network.antibotBan("203.0.113.9") == 0) org.basaltmc.proxy.log.logInfo("net: ban ok")
    if (org.basaltmc.proxy.network.antibotUnban("203.0.113.9") == 0) org.basaltmc.proxy.log.logInfo("net: unban ok")
    if (org.basaltmc.proxy.network.antibotUnban("203.0.113.9") < 0) org.basaltmc.proxy.log.logInfo("net: unban absent")
    if (org.basaltmc.proxy.network.antibotBan("not an address") < 0) org.basaltmc.proxy.log.logInfo("net: bad address refused")
    org.basaltmc.proxy.log.logInfo(if (org.basaltmc.proxy.network.antibotUnderAttack() == 0) "net: no attack" else "net: under attack")
}

fun init() {
    rewriteMarker = org.basaltmc.proxy.config.configStr("rewrite-marker") ?: ""
    cancelMarker = org.basaltmc.proxy.config.configStr("cancel-marker") ?: ""
    channel = org.basaltmc.proxy.config.configStr("channel") ?: ""
    org.basaltmc.proxy.events.on("plugin.enable", org.basaltmc.proxy.core.Priority.NORMAL, ::onEnable)
    org.basaltmc.proxy.events.on("player.join", org.basaltmc.proxy.core.Priority.NORMAL, ::onJoin)
}
