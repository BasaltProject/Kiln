using Basalt.Proxy.Admin;
using Basalt.Proxy.Commands;
using Basalt.Proxy.Core;
using Basalt.Proxy.Log;
using Basalt.Proxy.Players;
using Basalt.Proxy.Values;

public static class Plugin
{
    static void SayInt(string label, long number) => Logger.LogInfo(label + number);

    static void OnCheck(int ctx)
    {
        if (!Management.PermissionRequest(ctx, out Value request))
        {
            return;
        }
        if (!request.TryFind("text", out Value text) || !text.TryString(out string node))
        {
            return;
        }
        if (node == "proxyadmin.granted")
        {
            Management.PermissionAnswer(ctx, 1);
        }
        else if (node == "proxyadmin.denied")
        {
            Management.PermissionAnswer(ctx, 0);
        }
    }

    static void OnMeta(int ctx) => Management.PermissionAnswerText(ctx, "[Admin] ");

    static void OnPing(int ctx)
    {
        if (!Management.CompletionValue(ctx, out Value root))
        {
            Logger.LogInfo("ping failed");
            return;
        }
        if (root.IsNull)
        {
            Logger.LogInfo("ping offline");
            return;
        }
        long online = -1;
        if (root.TryFind("online", out Value field))
        {
            field.TryInt(out online);
        }
        Logger.LogInfo("ping answered");
        SayInt("ping online: ", online);
    }

    static void OnWait(int ctx)
    {
        Logger.LogInfo("waited");
        if (Management.DataValue(null, "visits", out Value visits))
        {
            visits.TryInt(out long count);
            SayInt("store visits: ", count);
        }
        if (Management.StoreTop("visits", 3, out Value top))
        {
            SayInt("store top entries: ", top.Count);
        }
    }

    static void OnCommand(int ctx)
    {
        string name = Sender.Name(ctx);
        if (name == null)
        {
            return;
        }
        int player = Directory.PlayerByName(name);
        Logger.LogInfo("command by: " + name);
        Management.GroupSet(name, "vip");
        string group = Management.GroupGet(name);
        if (group != null)
        {
            Logger.LogInfo("group is: " + group);
        }
        Management.PermissionOverride(name, "proxyadmin.extra", 1);
        SayInt("override check: ", Management.PermissionCheck(name, "proxyadmin.extra"));
        SayInt("granted check: ", Management.PermissionCheck(name, "proxyadmin.granted"));
        SayInt("denied check: ", Management.PermissionCheck(name, "proxyadmin.denied"));
        Management.PermissionOverride(name, "proxyadmin.extra", -1);
        SayInt("override removed: ", Management.PermissionCheck(name, "proxyadmin.extra"));
        Management.PlayerChat(player, "plain chat from the plugin");
        Management.PlayerSendJson(player, "{\"text\":\"json message\",\"color\":\"green\"}");
        Management.DataSetInt(name, "coins", 7);
        Logger.LogInfo("command done");
    }

    static void OnConsole(int ctx) => Logger.LogInfo("console command ran");

    static void OnKickCommand(int ctx)
    {
        string name = Sender.Name(ctx);
        if (name != null)
        {
            Management.PlayerKickJson(Directory.PlayerByName(name), "{\"text\":\"kicked by json\",\"color\":\"red\"}");
        }
    }

    public static void Init()
    {
        Management.PermissionProvide(OnCheck, OnMeta);
        Management.PermissionChanged();
        SayInt("register extra: ", Management.ServersRegister("extra", "127.0.0.1", 25599, "lobby", out _));
        SayInt("register again: ", Management.ServersRegister("extra", "127.0.0.1", 25599, "lobby", out _));
        SayInt("register bad port: ", Management.ServersRegister("bad", "127.0.0.1", 0, "", out _));
        SayInt("unregister extra: ", Management.ServersUnregister("extra"));
        SayInt("unregister lobby: ", Management.ServersUnregister("lobby"));
        SayInt("unregister none: ", Management.ServersUnregister("never"));
        Management.DataSetInt(null, "visits", 5);
        Management.DataSetStr(null, "label", "hello");
        Management.DataSetInt("OfflineHolder", "coins", 42);
        Management.DataSetFloat("OfflineHolder", "ratio", 2.5);
        if (Management.DataValue("OfflineHolder", "coins", out Value coins))
        {
            coins.TryInt(out long count);
            SayInt("offline coins: ", count);
        }
        if (Management.DataValue("OfflineHolder", "ratio", out Value ratio))
        {
            ratio.TryFloat(out double number);
            SayInt("offline ratio x10: ", (long)(number * 10));
        }
        Management.DataRemove("OfflineHolder", "ratio");
        SayInt("removed ratio: ", Management.DataValue("OfflineHolder", "ratio", out _) ? 0 : -1);
        Registry.Register("pcon", "", OnConsole);
        SayInt("console run: ", Management.ConsoleRun("pcon"));
        Management.ServersPing("lobby", OnPing);
        Management.Wait(2, OnWait);
        Registry.Register("padmin", "proxyadmin.granted", OnCommand);
        Registry.Register("pkick", "proxyadmin.granted", OnKickCommand);
        Registry.Register("pdenied", "proxyadmin.denied", OnCommand);
    }
}
