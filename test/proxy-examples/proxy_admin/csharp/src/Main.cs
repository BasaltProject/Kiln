using Basalt.Proxy.Core;
using Basalt.Proxy.Values;
using SdkCommands = global::Basalt.Proxy.Commands.Commands;
using SdkLogger = global::Basalt.Proxy.Log.Logger;
using SdkPlayers = global::Basalt.Proxy.Players.Players;
using SdkProxyAdmin = global::Basalt.Proxy.Admin.ProxyAdmin;
public static class Plugin
{
    static void SayInt(string label, long number) => SdkLogger.LogInfo(label + number);

    static void OnCheck(int ctx)
    {
        if (!SdkProxyAdmin.PermissionRequest(ctx, out Value request))
        {
            return;
        }
        if (!request.TryFind("text", out Value text) || !text.TryString(out string node))
        {
            return;
        }
        if (node == "proxyadmin.granted")
        {
            SdkProxyAdmin.PermissionAnswer(ctx, 1);
        }
        else if (node == "proxyadmin.denied")
        {
            SdkProxyAdmin.PermissionAnswer(ctx, 0);
        }
    }

    static void OnMeta(int ctx) => SdkProxyAdmin.PermissionAnswerText(ctx, "[Admin] ");

    static void OnPing(int ctx)
    {
        if (!SdkProxyAdmin.CompletionValue(ctx, out Value root))
        {
            SdkLogger.LogInfo("ping failed");
            return;
        }
        if (root.IsNull)
        {
            SdkLogger.LogInfo("ping offline");
            return;
        }
        long online = -1;
        if (root.TryFind("online", out Value field))
        {
            field.TryInt(out online);
        }
        SdkLogger.LogInfo("ping answered");
        SayInt("ping online: ", online);
    }

    static void OnWait(int ctx)
    {
        SdkLogger.LogInfo("waited");
        if (SdkProxyAdmin.DataValue(null, "visits", out Value visits))
        {
            visits.TryInt(out long count);
            SayInt("store visits: ", count);
        }
        if (SdkProxyAdmin.StoreTop("visits", 3, out Value top))
        {
            SayInt("store top entries: ", top.Count);
        }
    }

    static void OnCommand(int ctx)
    {
        string name = SdkCommands.CommandSourceName(ctx);
        if (name == null)
        {
            return;
        }
        int player = SdkPlayers.PlayerByName(name);
        SdkLogger.LogInfo("command by: " + name);
        SdkProxyAdmin.GroupSet(name, "vip");
        string group = SdkProxyAdmin.GroupGet(name);
        if (group != null)
        {
            SdkLogger.LogInfo("group is: " + group);
        }
        SdkProxyAdmin.PermissionOverride(name, "proxyadmin.extra", 1);
        SayInt("override check: ", SdkProxyAdmin.PermissionCheck(name, "proxyadmin.extra"));
        SayInt("granted check: ", SdkProxyAdmin.PermissionCheck(name, "proxyadmin.granted"));
        SayInt("denied check: ", SdkProxyAdmin.PermissionCheck(name, "proxyadmin.denied"));
        SdkProxyAdmin.PermissionOverride(name, "proxyadmin.extra", -1);
        SayInt("override removed: ", SdkProxyAdmin.PermissionCheck(name, "proxyadmin.extra"));
        SdkProxyAdmin.PlayerChat(player, "plain chat from the plugin");
        SdkProxyAdmin.PlayerSendJson(player, "{\"text\":\"json message\",\"color\":\"green\"}");
        SdkProxyAdmin.DataSetInt(name, "coins", 7);
        SdkLogger.LogInfo("command done");
    }

    static void OnConsole(int ctx) => SdkLogger.LogInfo("console command ran");

    static void OnKickCommand(int ctx)
    {
        string name = SdkCommands.CommandSourceName(ctx);
        if (name != null)
        {
            SdkProxyAdmin.PlayerKickJson(SdkPlayers.PlayerByName(name), "{\"text\":\"kicked by json\",\"color\":\"red\"}");
        }
    }

    public static void Init()
    {
        SdkProxyAdmin.PermissionProvide(OnCheck, OnMeta);
        SdkProxyAdmin.PermissionChanged();
        SayInt("register extra: ", SdkProxyAdmin.ServersRegister("extra", "127.0.0.1", 25599, "lobby", out _));
        SayInt("register again: ", SdkProxyAdmin.ServersRegister("extra", "127.0.0.1", 25599, "lobby", out _));
        SayInt("register bad port: ", SdkProxyAdmin.ServersRegister("bad", "127.0.0.1", 0, "", out _));
        SayInt("unregister extra: ", SdkProxyAdmin.ServersUnregister("extra"));
        SayInt("unregister lobby: ", SdkProxyAdmin.ServersUnregister("lobby"));
        SayInt("unregister none: ", SdkProxyAdmin.ServersUnregister("never"));
        SdkProxyAdmin.DataSetInt(null, "visits", 5);
        SdkProxyAdmin.DataSetStr(null, "label", "hello");
        SdkProxyAdmin.DataSetInt("OfflineHolder", "coins", 42);
        SdkProxyAdmin.DataSetFloat("OfflineHolder", "ratio", 2.5);
        if (SdkProxyAdmin.DataValue("OfflineHolder", "coins", out Value coins))
        {
            coins.TryInt(out long count);
            SayInt("offline coins: ", count);
        }
        if (SdkProxyAdmin.DataValue("OfflineHolder", "ratio", out Value ratio))
        {
            ratio.TryFloat(out double number);
            SayInt("offline ratio x10: ", (long)(number * 10));
        }
        SdkProxyAdmin.DataRemove("OfflineHolder", "ratio");
        SayInt("removed ratio: ", SdkProxyAdmin.DataValue("OfflineHolder", "ratio", out _) ? 0 : -1);
        SdkCommands.Command("pcon", "", OnConsole);
        SayInt("console run: ", SdkProxyAdmin.ConsoleRun("pcon"));
        SdkProxyAdmin.ServersPing("lobby", OnPing);
        SdkProxyAdmin.Wait(2, OnWait);
        SdkCommands.Command("padmin", "proxyadmin.granted", OnCommand);
        SdkCommands.Command("pkick", "proxyadmin.granted", OnKickCommand);
        SdkCommands.Command("pdenied", "proxyadmin.denied", OnCommand);
    }
}
