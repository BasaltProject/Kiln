import org.basaltmc.proxy.commands.Registry;
import org.basaltmc.proxy.core.Args;
import org.basaltmc.proxy.events.EventBus;

import static org.basaltmc.proxy.commands.Sender.reply;
import static org.basaltmc.proxy.config.Settings.configStr;
import static org.basaltmc.proxy.core.Priority.NORMAL;
import static org.basaltmc.proxy.events.EventBus.on;
import static org.basaltmc.proxy.log.Logger.logError;
import static org.basaltmc.proxy.log.Logger.logInfo;

public final class Main {
    private static String blocked = "";

    private static void onEnable(int ctx) {
        String id = EventBus.pluginInfo("id");
        String version = EventBus.pluginInfo("version");
        if (id != null) {
            logInfo("plugin id:");
            logInfo(id);
        }
        if (version != null) {
            logInfo("plugin version:");
            logInfo(version);
        }
        if (EventBus.pluginManifest() != null) {
            logInfo("manifest ok");
        }
        if (EventBus.configValue("banned-words") != null) {
            logInfo("config array ok");
        }
        logInfo(blocked);
    }

    private static void onChat(int ctx) {
        String message = EventBus.eventGetStr(ctx, "message");
        if (message == null) {
            return;
        }
        if (EventBus.eventFields(ctx) == null) {
            logError("event fields failed");
        }
        if (message.equals("ping")) {
            EventBus.eventSetText(ctx, "message", "pong");
            logInfo("chat rewritten");
        } else if (message.equals(blocked)) {
            EventBus.eventDeny(ctx, "<red>Watch your language");
            logInfo("chat denied");
        }
    }

    private static void onChatSeen(int ctx) {
        String message = EventBus.eventGetStr(ctx, "message");
        if (message != null) {
            logInfo("chat seen:");
            logInfo(message);
        }
    }

    private static void onCommand(int ctx) {
        if ("secret".equals(EventBus.eventGetStr(ctx, "command"))) {
            EventBus.eventDeny(ctx, "<red>No secrets here");
            logInfo("command denied");
        }
    }

    private static void onKick(int ctx) {
        EventBus.eventSetText(ctx, "reason", "<yellow>Kicked by the backend, come back soon");
        if (EventBus.eventSetText(ctx, "server", "nowhere") < 0) {
            logInfo("redirect refused");
        }
        if (EventBus.eventSetFlag(ctx, "cancelled", true) < 0) {
            logInfo("kick cannot be cancelled");
        }
    }

    private static void onKickSeen(int ctx) {
        String reason = EventBus.eventGetStr(ctx, "reason");
        if (reason != null) {
            logInfo("kick seen:");
            logInfo(reason);
        }
    }

    private static void onCommandRun(int ctx) {
        Double factor = EventBus.commandArgF64(ctx, "factor");
        Boolean flag = EventBus.commandArgBool(ctx, "flag");
        if (factor == null || flag == null) {
            reply(ctx, "<red>bad arguments");
            return;
        }
        reply(ctx, "<green>factor is {size}, flag is {flag}",
                new Args().add("size", factor > 1.5 ? "big" : "small").add("flag", flag ? "yes" : "no"));
    }

    public static void init() {
        String word = configStr("blocked-word");
        blocked = word == null ? "" : word;
        on("plugin.enable", NORMAL, Main::onEnable);
        on("player.chat", NORMAL, Main::onChat);
        EventBus.watch("player.chat", Main::onChatSeen);
        on("player.command", NORMAL, Main::onCommand);
        on("player.kick", NORMAL, Main::onKick);
        EventBus.watch("player.kick", Main::onKickSeen);
        Registry.register("proxyevents <factor: float> <flag: bool>", "", Main::onCommandRun);
    }
}
