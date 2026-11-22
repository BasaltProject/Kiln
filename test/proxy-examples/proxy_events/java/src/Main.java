import static org.basaltmc.proxy.commands.Commands.command;
import static org.basaltmc.proxy.commands.Commands.commandReply;
import static org.basaltmc.proxy.config.Config.configStr;
import static org.basaltmc.proxy.core.Priority.NORMAL;
import static org.basaltmc.proxy.events.Events.on;
import static org.basaltmc.proxy.log.Log.logError;
import static org.basaltmc.proxy.log.Log.logInfo;
import org.basaltmc.proxy.core.Args;
import org.basaltmc.proxy.events.Events;

public final class Main {
    private static String blocked = "";

    private static void onEnable(int ctx) {
        String id = Events.pluginInfo("id");
        String version = Events.pluginInfo("version");
        if (id != null) {
            logInfo("plugin id:");
            logInfo(id);
        }
        if (version != null) {
            logInfo("plugin version:");
            logInfo(version);
        }
        if (Events.pluginManifest() != null) {
            logInfo("manifest ok");
        }
        if (Events.configValue("banned-words") != null) {
            logInfo("config array ok");
        }
        logInfo(blocked);
    }

    private static void onChat(int ctx) {
        String message = Events.eventGetStr(ctx, "message");
        if (message == null) {
            return;
        }
        if (Events.eventFields(ctx) == null) {
            logError("event fields failed");
        }
        if (message.equals("ping")) {
            Events.eventSetText(ctx, "message", "pong");
            logInfo("chat rewritten");
        } else if (message.equals(blocked)) {
            Events.eventDeny(ctx, "<red>Watch your language");
            logInfo("chat denied");
        }
    }

    private static void onChatSeen(int ctx) {
        String message = Events.eventGetStr(ctx, "message");
        if (message != null) {
            logInfo("chat seen:");
            logInfo(message);
        }
    }

    private static void onCommand(int ctx) {
        if ("secret".equals(Events.eventGetStr(ctx, "command"))) {
            Events.eventDeny(ctx, "<red>No secrets here");
            logInfo("command denied");
        }
    }

    private static void onKick(int ctx) {
        Events.eventSetText(ctx, "reason", "<yellow>Kicked by the backend, come back soon");
        if (Events.eventSetText(ctx, "server", "nowhere") < 0) {
            logInfo("redirect refused");
        }
        if (Events.eventSetFlag(ctx, "cancelled", true) < 0) {
            logInfo("kick cannot be cancelled");
        }
    }

    private static void onKickSeen(int ctx) {
        String reason = Events.eventGetStr(ctx, "reason");
        if (reason != null) {
            logInfo("kick seen:");
            logInfo(reason);
        }
    }

    private static void onCommandRun(int ctx) {
        Double factor = Events.commandArgF64(ctx, "factor");
        Boolean flag = Events.commandArgBool(ctx, "flag");
        if (factor == null || flag == null) {
            commandReply(ctx, "<red>bad arguments");
            return;
        }
        commandReply(ctx, "<green>factor is {size}, flag is {flag}",
                new Args().add("size", factor > 1.5 ? "big" : "small").add("flag", flag ? "yes" : "no"));
    }

    public static void init() {
        String word = configStr("blocked-word");
        blocked = word == null ? "" : word;
        on("plugin.enable", NORMAL, Main::onEnable);
        on("player.chat", NORMAL, Main::onChat);
        Events.watch("player.chat", Main::onChatSeen);
        on("player.command", NORMAL, Main::onCommand);
        on("player.kick", NORMAL, Main::onKick);
        Events.watch("player.kick", Main::onKickSeen);
        command("proxyevents <factor: float> <flag: bool>", "", Main::onCommandRun);
    }
}
