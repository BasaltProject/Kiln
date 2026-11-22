import org.basaltmc.proxy.core.Args
import org.basaltmc.proxy.core.Ctx

var blocked = ""

fun onEnable(ctx: Ctx) {
    org.basaltmc.proxy.events.pluginInfo("id")?.let {
        org.basaltmc.proxy.log.logInfo("plugin id:")
        org.basaltmc.proxy.log.logInfo(it)
    }
    org.basaltmc.proxy.events.pluginInfo("version")?.let {
        org.basaltmc.proxy.log.logInfo("plugin version:")
        org.basaltmc.proxy.log.logInfo(it)
    }
    if (org.basaltmc.proxy.events.pluginManifest() != null) org.basaltmc.proxy.log.logInfo("manifest ok")
    if (org.basaltmc.proxy.events.configValue("banned-words") != null) org.basaltmc.proxy.log.logInfo("config array ok")
    org.basaltmc.proxy.log.logInfo(blocked)
}

fun onChat(ctx: Ctx) {
    val message = org.basaltmc.proxy.events.eventGetStr(ctx, "message") ?: return
    if (org.basaltmc.proxy.events.eventFields(ctx) == null) org.basaltmc.proxy.log.logError("event fields failed")
    if (message == "ping") {
        org.basaltmc.proxy.events.eventSetText(ctx, "message", "pong")
        org.basaltmc.proxy.log.logInfo("chat rewritten")
    } else if (message == blocked) {
        org.basaltmc.proxy.events.eventDeny(ctx, "<red>Watch your language")
        org.basaltmc.proxy.log.logInfo("chat denied")
    }
}

fun onChatSeen(ctx: Ctx) {
    org.basaltmc.proxy.events.eventGetStr(ctx, "message")?.let {
        org.basaltmc.proxy.log.logInfo("chat seen:")
        org.basaltmc.proxy.log.logInfo(it)
    }
}

fun onCommand(ctx: Ctx) {
    if (org.basaltmc.proxy.events.eventGetStr(ctx, "command") == "secret") {
        org.basaltmc.proxy.events.eventDeny(ctx, "<red>No secrets here")
        org.basaltmc.proxy.log.logInfo("command denied")
    }
}

fun onKick(ctx: Ctx) {
    org.basaltmc.proxy.events.eventSetText(ctx, "reason", "<yellow>Kicked by the backend, come back soon")
    if (org.basaltmc.proxy.events.eventSetText(ctx, "server", "nowhere") < 0) org.basaltmc.proxy.log.logInfo("redirect refused")
    if (org.basaltmc.proxy.events.eventSetFlag(ctx, "cancelled", true) < 0) org.basaltmc.proxy.log.logInfo("kick cannot be cancelled")
}

fun onKickSeen(ctx: Ctx) {
    org.basaltmc.proxy.events.eventGetStr(ctx, "reason")?.let {
        org.basaltmc.proxy.log.logInfo("kick seen:")
        org.basaltmc.proxy.log.logInfo(it)
    }
}

fun onCommandRun(ctx: Ctx) {
    val factor = org.basaltmc.proxy.events.commandArgF64(ctx, "factor")
    val flag = org.basaltmc.proxy.events.commandArgBool(ctx, "flag")
    if (factor == null || flag == null) {
        org.basaltmc.proxy.commands.commandReply(ctx, "<red>bad arguments")
        return
    }
    org.basaltmc.proxy.commands.commandReply(
        ctx, "<green>factor is {size}, flag is {flag}",
        Args().add("size", if (factor > 1.5) "big" else "small").add("flag", if (flag) "yes" else "no")
    )
}

fun init() {
    blocked = org.basaltmc.proxy.config.configStr("blocked-word") ?: ""
    org.basaltmc.proxy.events.on("plugin.enable", org.basaltmc.proxy.core.Priority.NORMAL, ::onEnable)
    org.basaltmc.proxy.events.on("player.chat", org.basaltmc.proxy.core.Priority.NORMAL, ::onChat)
    org.basaltmc.proxy.events.watch("player.chat", ::onChatSeen)
    org.basaltmc.proxy.events.on("player.command", org.basaltmc.proxy.core.Priority.NORMAL, ::onCommand)
    org.basaltmc.proxy.events.on("player.kick", org.basaltmc.proxy.core.Priority.NORMAL, ::onKick)
    org.basaltmc.proxy.events.watch("player.kick", ::onKickSeen)
    org.basaltmc.proxy.commands.command("proxyevents <factor: float> <flag: bool>", "", ::onCommandRun)
}
