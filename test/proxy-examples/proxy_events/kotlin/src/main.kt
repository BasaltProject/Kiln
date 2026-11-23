import org.basaltmc.proxy.commands.command
import org.basaltmc.proxy.commands.commandReply
import org.basaltmc.proxy.config.configStr
import org.basaltmc.proxy.core.Args
import org.basaltmc.proxy.core.Ctx
import org.basaltmc.proxy.core.Priority
import org.basaltmc.proxy.events.commandArgBool
import org.basaltmc.proxy.events.commandArgF64
import org.basaltmc.proxy.events.configValue
import org.basaltmc.proxy.events.eventDeny
import org.basaltmc.proxy.events.eventFields
import org.basaltmc.proxy.events.eventGetStr
import org.basaltmc.proxy.events.eventSetFlag
import org.basaltmc.proxy.events.eventSetText
import org.basaltmc.proxy.events.on
import org.basaltmc.proxy.events.pluginInfo
import org.basaltmc.proxy.events.pluginManifest
import org.basaltmc.proxy.events.watch
import org.basaltmc.proxy.log.logError
import org.basaltmc.proxy.log.logInfo

var blocked = ""

fun onEnable(ctx: Ctx) {
    pluginInfo("id")?.let {
        logInfo("plugin id:")
        logInfo(it)
    }
    pluginInfo("version")?.let {
        logInfo("plugin version:")
        logInfo(it)
    }
    if (pluginManifest() != null) logInfo("manifest ok")
    if (configValue("banned-words") != null) logInfo("config array ok")
    logInfo(blocked)
}

fun onChat(ctx: Ctx) {
    val message = eventGetStr(ctx, "message") ?: return
    if (eventFields(ctx) == null) logError("event fields failed")
    if (message == "ping") {
        eventSetText(ctx, "message", "pong")
        logInfo("chat rewritten")
    } else if (message == blocked) {
        eventDeny(ctx, "<red>Watch your language")
        logInfo("chat denied")
    }
}

fun onChatSeen(ctx: Ctx) {
    eventGetStr(ctx, "message")?.let {
        logInfo("chat seen:")
        logInfo(it)
    }
}

fun onCommand(ctx: Ctx) {
    if (eventGetStr(ctx, "command") == "secret") {
        eventDeny(ctx, "<red>No secrets here")
        logInfo("command denied")
    }
}

fun onKick(ctx: Ctx) {
    eventSetText(ctx, "reason", "<yellow>Kicked by the backend, come back soon")
    if (eventSetText(ctx, "server", "nowhere") < 0) logInfo("redirect refused")
    if (eventSetFlag(ctx, "cancelled", true) < 0) logInfo("kick cannot be cancelled")
}

fun onKickSeen(ctx: Ctx) {
    eventGetStr(ctx, "reason")?.let {
        logInfo("kick seen:")
        logInfo(it)
    }
}

fun onCommandRun(ctx: Ctx) {
    val factor = commandArgF64(ctx, "factor")
    val flag = commandArgBool(ctx, "flag")
    if (factor == null || flag == null) {
        commandReply(ctx, "<red>bad arguments")
        return
    }
    commandReply(
        ctx, "<green>factor is {size}, flag is {flag}",
        Args().add("size", if (factor > 1.5) "big" else "small").add("flag", if (flag) "yes" else "no")
    )
}

fun init() {
    blocked = configStr("blocked-word") ?: ""
    on("plugin.enable", Priority.NORMAL, ::onEnable)
    on("player.chat", Priority.NORMAL, ::onChat)
    watch("player.chat", ::onChatSeen)
    on("player.command", Priority.NORMAL, ::onCommand)
    on("player.kick", Priority.NORMAL, ::onKick)
    watch("player.kick", ::onKickSeen)
    command("proxyevents <factor: float> <flag: bool>", "", ::onCommandRun)
}
