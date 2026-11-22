import org.basaltmc.proxy.core.Ctx

fun sayInt(label: String, number: Long) = org.basaltmc.proxy.log.logInfo("$label$number")

fun onCheck(ctx: Ctx) {
    val request = org.basaltmc.proxy.admin.permissionRequest(ctx) ?: return
    val node = request.find("text")?.asString() ?: return
    if (node == "proxyadmin.granted") org.basaltmc.proxy.admin.permissionAnswer(ctx, 1)
    else if (node == "proxyadmin.denied") org.basaltmc.proxy.admin.permissionAnswer(ctx, 0)
}

fun onMeta(ctx: Ctx) {
    org.basaltmc.proxy.admin.permissionAnswerText(ctx, "[Admin] ")
}

fun onPing(ctx: Ctx) {
    val root = org.basaltmc.proxy.admin.completionValue(ctx)
    if (root == null) {
        org.basaltmc.proxy.log.logInfo("ping failed")
        return
    }
    if (root.kind() == org.basaltmc.proxy.values.ValueKind.Null) {
        org.basaltmc.proxy.log.logInfo("ping offline")
        return
    }
    org.basaltmc.proxy.log.logInfo("ping answered")
    sayInt("ping online: ", root.find("online")?.asLong() ?: -1L)
}

fun onWait(ctx: Ctx) {
    org.basaltmc.proxy.log.logInfo("waited")
    org.basaltmc.proxy.admin.dataValue(null, "visits")?.let { sayInt("store visits: ", it.asLong() ?: 0L) }
    org.basaltmc.proxy.admin.storeTop("visits", 3)?.let { sayInt("store top entries: ", it.count().toLong()) }
}

fun onCommand(ctx: Ctx) {
    val name = org.basaltmc.proxy.commands.commandSourceName(ctx) ?: return
    val player = org.basaltmc.proxy.players.playerByName(name)
    org.basaltmc.proxy.log.logInfo("command by: $name")
    org.basaltmc.proxy.admin.groupSet(name, "vip")
    org.basaltmc.proxy.admin.groupGet(name)?.let { org.basaltmc.proxy.log.logInfo("group is: $it") }
    org.basaltmc.proxy.admin.permissionOverride(name, "proxyadmin.extra", 1)
    sayInt("override check: ", org.basaltmc.proxy.admin.permissionCheck(name, "proxyadmin.extra").toLong())
    sayInt("granted check: ", org.basaltmc.proxy.admin.permissionCheck(name, "proxyadmin.granted").toLong())
    sayInt("denied check: ", org.basaltmc.proxy.admin.permissionCheck(name, "proxyadmin.denied").toLong())
    org.basaltmc.proxy.admin.permissionOverride(name, "proxyadmin.extra", -1)
    sayInt("override removed: ", org.basaltmc.proxy.admin.permissionCheck(name, "proxyadmin.extra").toLong())
    org.basaltmc.proxy.admin.playerChat(player, "plain chat from the plugin")
    org.basaltmc.proxy.admin.playerSendJson(player, "{\"text\":\"json message\",\"color\":\"green\"}")
    org.basaltmc.proxy.admin.dataSetLong(name, "coins", 7L)
    org.basaltmc.proxy.log.logInfo("command done")
}

fun onConsole(ctx: Ctx) {
    org.basaltmc.proxy.log.logInfo("console command ran")
}

fun onKickCommand(ctx: Ctx) {
    val name = org.basaltmc.proxy.commands.commandSourceName(ctx) ?: return
    org.basaltmc.proxy.admin.playerKickJson(org.basaltmc.proxy.players.playerByName(name), "{\"text\":\"kicked by json\",\"color\":\"red\"}")
}

fun init() {
    org.basaltmc.proxy.admin.permissionProvide(::onCheck, ::onMeta, 0)
    org.basaltmc.proxy.admin.permissionChanged()
    sayInt("register extra: ", org.basaltmc.proxy.admin.serversRegister("extra", "127.0.0.1", 25599, "lobby").status.toLong())
    sayInt("register again: ", org.basaltmc.proxy.admin.serversRegister("extra", "127.0.0.1", 25599, "lobby").status.toLong())
    sayInt("register bad port: ", org.basaltmc.proxy.admin.serversRegister("bad", "127.0.0.1", 0, "").status.toLong())
    sayInt("unregister extra: ", org.basaltmc.proxy.admin.serversUnregister("extra").toLong())
    sayInt("unregister lobby: ", org.basaltmc.proxy.admin.serversUnregister("lobby").toLong())
    sayInt("unregister none: ", org.basaltmc.proxy.admin.serversUnregister("never").toLong())
    org.basaltmc.proxy.admin.dataSetLong(null, "visits", 5L)
    org.basaltmc.proxy.admin.dataSetStr(null, "label", "hello")
    org.basaltmc.proxy.admin.dataSetLong("OfflineHolder", "coins", 42L)
    org.basaltmc.proxy.admin.dataSetDouble("OfflineHolder", "ratio", 2.5)
    org.basaltmc.proxy.admin.dataValue("OfflineHolder", "coins")?.let { sayInt("offline coins: ", it.asLong() ?: 0L) }
    org.basaltmc.proxy.admin.dataValue("OfflineHolder", "ratio")?.let { sayInt("offline ratio x10: ", ((it.asDouble() ?: 0.0) * 10).toLong()) }
    org.basaltmc.proxy.admin.dataRemove("OfflineHolder", "ratio")
    sayInt("removed ratio: ", if (org.basaltmc.proxy.admin.dataValue("OfflineHolder", "ratio") == null) -1L else 0L)
    org.basaltmc.proxy.commands.command("pcon", "", ::onConsole)
    sayInt("console run: ", org.basaltmc.proxy.admin.consoleRun("pcon").toLong())
    org.basaltmc.proxy.admin.serversPing("lobby", ::onPing)
    org.basaltmc.proxy.admin.wait(2, ::onWait)
    org.basaltmc.proxy.commands.command("padmin", "proxyadmin.granted", ::onCommand)
    org.basaltmc.proxy.commands.command("pkick", "proxyadmin.granted", ::onKickCommand)
    org.basaltmc.proxy.commands.command("pdenied", "proxyadmin.denied", ::onCommand)
}
