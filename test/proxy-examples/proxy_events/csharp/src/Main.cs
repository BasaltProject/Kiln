using Basalt.Proxy.Core;
using Basalt.Proxy.Values;
using SdkCommands = global::Basalt.Proxy.Commands.Commands;
using SdkConfig = global::Basalt.Proxy.Config.Config;
using SdkEvents = global::Basalt.Proxy.Events.Events;
using SdkLogger = global::Basalt.Proxy.Log.Logger;
using SdkProxyEvents = global::Basalt.Proxy.Events.ProxyEvents;
public static class Plugin
{
    static string blocked = "";

    static void OnEnable(int ctx)
    {
        string id = SdkProxyEvents.PluginInfo("id");
        string version = SdkProxyEvents.PluginInfo("version");
        if (id != null)
        {
            SdkLogger.LogInfo("plugin id:");
            SdkLogger.LogInfo(id);
        }
        if (version != null)
        {
            SdkLogger.LogInfo("plugin version:");
            SdkLogger.LogInfo(version);
        }
        if (SdkProxyEvents.PluginManifest() != null)
        {
            SdkLogger.LogInfo("manifest ok");
        }
        if (SdkProxyEvents.ConfigValue("banned-words") != null)
        {
            SdkLogger.LogInfo("config array ok");
        }
        SdkLogger.LogInfo(blocked);
    }

    static void OnChat(int ctx)
    {
        string message = SdkProxyEvents.EventGetStr(ctx, "message");
        if (message == null)
        {
            return;
        }
        if (SdkProxyEvents.EventFields(ctx) == null)
        {
            SdkLogger.LogError("event fields failed");
        }
        if (message == "ping")
        {
            SdkProxyEvents.EventSetText(ctx, "message", "pong");
            SdkLogger.LogInfo("chat rewritten");
        }
        else if (message == blocked)
        {
            SdkProxyEvents.EventDeny(ctx, "<red>Watch your language");
            SdkLogger.LogInfo("chat denied");
        }
    }

    static void OnChatSeen(int ctx)
    {
        string message = SdkProxyEvents.EventGetStr(ctx, "message");
        if (message != null)
        {
            SdkLogger.LogInfo("chat seen:");
            SdkLogger.LogInfo(message);
        }
    }

    static void OnCommand(int ctx)
    {
        if (SdkProxyEvents.EventGetStr(ctx, "command") == "secret")
        {
            SdkProxyEvents.EventDeny(ctx, "<red>No secrets here");
            SdkLogger.LogInfo("command denied");
        }
    }

    static void OnKick(int ctx)
    {
        SdkProxyEvents.EventSetText(ctx, "reason", "<yellow>Kicked by the backend, come back soon");
        if (SdkProxyEvents.EventSetText(ctx, "server", "nowhere") < 0)
        {
            SdkLogger.LogInfo("redirect refused");
        }
        if (SdkProxyEvents.EventSetFlag(ctx, "cancelled", true) < 0)
        {
            SdkLogger.LogInfo("kick cannot be cancelled");
        }
    }

    static void OnKickSeen(int ctx)
    {
        string reason = SdkProxyEvents.EventGetStr(ctx, "reason");
        if (reason != null)
        {
            SdkLogger.LogInfo("kick seen:");
            SdkLogger.LogInfo(reason);
        }
    }

    static void OnCommandRun(int ctx)
    {
        double? factor = SdkProxyEvents.CommandArgF64(ctx, "factor");
        bool? flag = SdkProxyEvents.CommandArgBool(ctx, "flag");
        if (factor == null || flag == null)
        {
            SdkCommands.CommandReply(ctx, "<red>bad arguments");
            return;
        }
        SdkCommands.CommandReply(ctx, "<green>factor is {size}, flag is {flag}",
            new Args().Add("size", factor.Value > 1.5 ? "big" : "small").Add("flag", flag.Value ? "yes" : "no"));
    }

    public static void Init()
    {
        blocked = SdkConfig.ConfigStr("blocked-word") ?? "";
        SdkEvents.On("plugin.enable", Priority.Normal, OnEnable);
        SdkEvents.On("player.chat", Priority.Normal, OnChat);
        SdkProxyEvents.Watch("player.chat", OnChatSeen);
        SdkEvents.On("player.command", Priority.Normal, OnCommand);
        SdkEvents.On("player.kick", Priority.Normal, OnKick);
        SdkProxyEvents.Watch("player.kick", OnKickSeen);
        SdkCommands.Command("proxyevents <factor: float> <flag: bool>", "", OnCommandRun);
    }
}
