import org.basaltmc.proxy.admin.completionValue
import org.basaltmc.proxy.admin.consoleRun
import org.basaltmc.proxy.admin.dataRemove
import org.basaltmc.proxy.admin.dataSetDouble
import org.basaltmc.proxy.admin.dataSetLong
import org.basaltmc.proxy.admin.dataSetStr
import org.basaltmc.proxy.admin.dataValue
import org.basaltmc.proxy.admin.groupGet
import org.basaltmc.proxy.admin.groupSet
import org.basaltmc.proxy.admin.permissionAnswer
import org.basaltmc.proxy.admin.permissionAnswerText
import org.basaltmc.proxy.admin.permissionChanged
import org.basaltmc.proxy.admin.permissionCheck
import org.basaltmc.proxy.admin.permissionOverride
import org.basaltmc.proxy.admin.permissionProvide
import org.basaltmc.proxy.admin.permissionRequest
import org.basaltmc.proxy.admin.playerChat
import org.basaltmc.proxy.admin.playerKickJson
import org.basaltmc.proxy.admin.playerSendJson
import org.basaltmc.proxy.admin.serversPing
import org.basaltmc.proxy.admin.serversRegister
import org.basaltmc.proxy.admin.serversUnregister
import org.basaltmc.proxy.admin.storeTop
import org.basaltmc.proxy.admin.wait
import org.basaltmc.proxy.commands.command
import org.basaltmc.proxy.commands.commandSourceName
import org.basaltmc.proxy.core.Ctx
import org.basaltmc.proxy.log.logInfo
import org.basaltmc.proxy.players.playerByName
import org.basaltmc.proxy.values.ValueKind.Null

fun sayInt(label: String, number: Long) = logInfo("$label$number")

fun onCheck(ctx: Ctx) {
    val request = permissionRequest(ctx) ?: return
    val node = request.find("text")?.asString() ?: return
    if (node == "proxyadmin.granted") permissionAnswer(ctx, 1)
    else if (node == "proxyadmin.denied") permissionAnswer(ctx, 0)
}

fun onMeta(ctx: Ctx) {
    permissionAnswerText(ctx, "[Admin] ")
}

fun onPing(ctx: Ctx) {
    val root = completionValue(ctx)
    if (root == null) {
        logInfo("ping failed")
        return
    }
    if (root.kind() == Null) {
        logInfo("ping offline")
        return
    }
    logInfo("ping answered")
    sayInt("ping online: ", root.find("online")?.asLong() ?: -1L)
}

fun onWait(ctx: Ctx) {
    logInfo("waited")
    dataValue(null, "visits")?.let { sayInt("store visits: ", it.asLong() ?: 0L) }
    storeTop("visits", 3)?.let { sayInt("store top entries: ", it.count().toLong()) }
}

fun onCommand(ctx: Ctx) {
    val name = commandSourceName(ctx) ?: return
    val player = playerByName(name)
    logInfo("command by: $name")
    groupSet(name, "vip")
    groupGet(name)?.let { logInfo("group is: $it") }
    permissionOverride(name, "proxyadmin.extra", 1)
    sayInt("override check: ", permissionCheck(name, "proxyadmin.extra").toLong())
    sayInt("granted check: ", permissionCheck(name, "proxyadmin.granted").toLong())
    sayInt("denied check: ", permissionCheck(name, "proxyadmin.denied").toLong())
    permissionOverride(name, "proxyadmin.extra", -1)
    sayInt("override removed: ", permissionCheck(name, "proxyadmin.extra").toLong())
    playerChat(player, "plain chat from the plugin")
    playerSendJson(player, "{\"text\":\"json message\",\"color\":\"green\"}")
    dataSetLong(name, "coins", 7L)
    logInfo("command done")
}

fun onConsole(ctx: Ctx) {
    logInfo("console command ran")
}

fun onKickCommand(ctx: Ctx) {
    val name = commandSourceName(ctx) ?: return
    playerKickJson(playerByName(name), "{\"text\":\"kicked by json\",\"color\":\"red\"}")
}

fun init() {
    permissionProvide(::onCheck, ::onMeta, 0)
    permissionChanged()
    sayInt("register extra: ", serversRegister("extra", "127.0.0.1", 25599, "lobby").status.toLong())
    sayInt("register again: ", serversRegister("extra", "127.0.0.1", 25599, "lobby").status.toLong())
    sayInt("register bad port: ", serversRegister("bad", "127.0.0.1", 0, "").status.toLong())
    sayInt("unregister extra: ", serversUnregister("extra").toLong())
    sayInt("unregister lobby: ", serversUnregister("lobby").toLong())
    sayInt("unregister none: ", serversUnregister("never").toLong())
    dataSetLong(null, "visits", 5L)
    dataSetStr(null, "label", "hello")
    dataSetLong("OfflineHolder", "coins", 42L)
    dataSetDouble("OfflineHolder", "ratio", 2.5)
    dataValue("OfflineHolder", "coins")?.let { sayInt("offline coins: ", it.asLong() ?: 0L) }
    dataValue("OfflineHolder", "ratio")?.let { sayInt("offline ratio x10: ", ((it.asDouble() ?: 0.0) * 10).toLong()) }
    dataRemove("OfflineHolder", "ratio")
    sayInt("removed ratio: ", if (dataValue("OfflineHolder", "ratio") == null) -1L else 0L)
    command("pcon", "", ::onConsole)
    sayInt("console run: ", consoleRun("pcon").toLong())
    serversPing("lobby", ::onPing)
    wait(2, ::onWait)
    command("padmin", "proxyadmin.granted", ::onCommand)
    command("pkick", "proxyadmin.granted", ::onKickCommand)
    command("pdenied", "proxyadmin.denied", ::onCommand)
}
