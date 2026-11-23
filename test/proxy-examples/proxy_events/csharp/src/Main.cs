using Basalt.Proxy.Commands;
using Basalt.Proxy.Config;
using Basalt.Proxy.Core;
using Basalt.Proxy.Events;
using Basalt.Proxy.Log;
using Basalt.Proxy.Values;

public static class Plugin
{
    static string blocked = "";

    static void OnEnable(int ctx)
    {
        string id = EventBus.PluginInfo("id");
        string version = EventBus.PluginInfo("version");
        if (id != null)
        {
            Logger.LogInfo("plugin id:");
            Logger.LogInfo(id);
        }
        if (version != null)
        {
            Logger.LogInfo("plugin version:");
            Logger.LogInfo(version);
        }
        if (EventBus.PluginManifest() != null)
        {
            Logger.LogInfo("manifest ok");
        }
        if (EventBus.ConfigValue("banned-words") != null)
        {
            Logger.LogInfo("config array ok");
        }
        Logger.LogInfo(blocked);
    }

    static void OnChat(int ctx)
    {
        string message = EventBus.EventGetStr(ctx, "message");
        if (message == null)
        {
            return;
        }
        if (EventBus.EventFields(ctx) == null)
        {
            Logger.LogError("event fields failed");
        }
        if (message == "ping")
        {
            EventBus.EventSetText(ctx, "message", "pong");
            Logger.LogInfo("chat rewritten");
        }
        else if (message == blocked)
        {
            EventBus.EventDeny(ctx, "<red>Watch your language");
            Logger.LogInfo("chat denied");
        }
    }

    static void OnChatSeen(int ctx)
    {
        string message = EventBus.EventGetStr(ctx, "message");
        if (message != null)
        {
            Logger.LogInfo("chat seen:");
            Logger.LogInfo(message);
        }
    }

    static void OnCommand(int ctx)
    {
        if (EventBus.EventGetStr(ctx, "command") == "secret")
        {
            EventBus.EventDeny(ctx, "<red>No secrets here");
            Logger.LogInfo("command denied");
        }
    }

    static void OnKick(int ctx)
    {
        EventBus.EventSetText(ctx, "reason", "<yellow>Kicked by the backend, come back soon");
        if (EventBus.EventSetText(ctx, "server", "nowhere") < 0)
        {
            Logger.LogInfo("redirect refused");
        }
        if (EventBus.EventSetFlag(ctx, "cancelled", true) < 0)
        {
            Logger.LogInfo("kick cannot be cancelled");
        }
    }

    static void OnKickSeen(int ctx)
    {
        string reason = EventBus.EventGetStr(ctx, "reason");
        if (reason != null)
        {
            Logger.LogInfo("kick seen:");
            Logger.LogInfo(reason);
        }
    }

    static void OnCommandRun(int ctx)
    {
        double? factor = EventBus.CommandArgF64(ctx, "factor");
        bool? flag = EventBus.CommandArgBool(ctx, "flag");
        if (factor == null || flag == null)
        {
            Sender.Reply(ctx, "<red>bad arguments");
            return;
        }
        Sender.Reply(ctx, "<green>factor is {size}, flag is {flag}",
            new Args().Add("size", factor.Value > 1.5 ? "big" : "small").Add("flag", flag.Value ? "yes" : "no"));
    }

    public static void Init()
    {
        blocked = Settings.ConfigStr("blocked-word") ?? "";
        EventBus.On("plugin.enable", Priority.Normal, OnEnable);
        EventBus.On("player.chat", Priority.Normal, OnChat);
        EventBus.Watch("player.chat", OnChatSeen);
        EventBus.On("player.command", Priority.Normal, OnCommand);
        EventBus.On("player.kick", Priority.Normal, OnKick);
        EventBus.Watch("player.kick", OnKickSeen);
        Registry.Register("proxyevents <factor: float> <flag: bool>", "", OnCommandRun);
    }
}
