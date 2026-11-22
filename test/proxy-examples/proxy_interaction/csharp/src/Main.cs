using Basalt.Proxy.Interaction;
using Basalt.Proxy.Core;
using Basalt.Proxy.Values;
using LegacyPlayers = global::Basalt.Proxy.Players.Players;
using SdkCommands = global::Basalt.Proxy.Commands.Commands;
using SdkEvents = global::Basalt.Proxy.Events.Events;
using SdkLogger = global::Basalt.Proxy.Log.Logger;
using SdkProxyInteraction = global::Basalt.Proxy.Interaction.ProxyInteraction;
using SdkTasks = global::Basalt.Proxy.Tasks.Tasks;
using System.Text;

public static class Plugin
{
    static int seen;

    static void Say(string text) => SdkLogger.LogInfo(text);

    static bool Ok(int result) => result >= 0;

    static string TextOf(Value root, string key)
    {
        return root.TryFind(key, out Value child) && child.TryString(out string text) ? text : "";
    }

    static void OnPackStatus(int ctx)
    {
        if (!SdkProxyInteraction.TryValueResult(ctx, out Value root))
        {
            Say("pack event unreadable");
            return;
        }
        Say("pack status:");
        Say(TextOf(root, "status"));
    }

    static void OnSettings(int ctx)
    {
        if (!SdkProxyInteraction.TryValueResult(ctx, out Value root))
        {
            Say("settings event unreadable");
            return;
        }
        Say("settings locale:");
        Say(root.TryFind("settings", out Value settings) ? TextOf(settings, "locale") : "");
    }

    static void OnCookie(int ctx)
    {
        byte[] cookie = SdkProxyInteraction.CookieResult(ctx);
        if (cookie == null)
        {
            Say("cookie absent");
            return;
        }
        Say("cookie received:");
        Say(Encoding.UTF8.GetString(cookie));
    }

    static void OnDialogClick(int ctx)
    {
        Say("dialog clicked");
        SdkCommands.CommandReply(ctx, "<green>dialog clicked");
    }

    static void OnTransfer(int ctx)
    {
        int player = SdkEvents.EventPlayer(ctx);
        Say(Ok(SdkProxyInteraction.PlayerTransfer(player, "127.0.0.1", 25970)) ? "transfer sent" : "transfer failed");
    }

    static void OnCleanup(int ctx)
    {
        int player = SdkEvents.EventPlayer(ctx);
        bool removed = Ok(SdkProxyInteraction.ResourcePackRemove(player, "demo"));
        bool cleared = Ok(SdkProxyInteraction.ResourcePackClear(player));
        bool closed = Ok(SdkProxyInteraction.DialogClear(player));
        Say(removed && cleared && closed ? "cleanup ok" : "cleanup failed");
    }

    static void OnStale(int ctx)
    {
        Say(SdkProxyInteraction.DialogClear(seen) == -2 ? "stale after quit" : "stale not detected");
    }

    static void OnQuit(int ctx)
    {
        SdkTasks.After(20, OnStale);
    }

    static void OnJoin(int ctx)
    {
        int player = SdkEvents.EventPlayer(ctx);
        seen = player;
        Say(SdkProxyInteraction.PlayersCount() >= 1 ? "count ok" : "count failed");
        Say(SdkProxyInteraction.PlayersCount("lobby") >= 1 ? "count lobby ok" : "count lobby failed");
        Say(SdkProxyInteraction.PlayersCount("nowhere") == 0 ? "count nowhere ok" : "count nowhere failed");
        string address = SdkProxyInteraction.PlayerAddress(player);
        Say(!string.IsNullOrEmpty(address) && SdkProxyInteraction.PlayerPort(player) > 0 ? "address ok" : "address failed");
        string uuid = LegacyPlayers.PlayerUuid(player) ?? "";
        Say(SdkProxyInteraction.PlayersByUuid(uuid) == player ? "by uuid ok" : "by uuid failed");
        Say(SdkProxyInteraction.PlayersByUuid("00000000-0000-0000-0000-000000000000") == 0 ? "by uuid none ok" : "by uuid none failed");

        DialogButton[] buttons = { new DialogButton("Accept", "pidialog"), new DialogButton("Decline", "pidialog") };
        Say(Ok(SdkProxyInteraction.DialogShow(player, "multi_action", "<gold>Interaction", "<gray>Pick one", buttons)) ? "dialog ok" : "dialog failed");
        Say(SdkProxyInteraction.DialogShow(player, "bogus", "x", null, new DialogButton[0]) == -3 ? "bad kind invalid" : "bad kind accepted");

        string id = SdkProxyInteraction.ResourcePackPush(player, "https://example.invalid/pack.zip", null, false, "<gold>Test pack", "demo");
        if (id != null)
        {
            Say("pack pushed:");
            Say(id);
        }
        else
        {
            Say("pack failed");
        }
        string bad = SdkProxyInteraction.ResourcePackPush(player, "https://example.invalid/pack.zip", "nothex", false, null, null);
        Say(bad == null ? "bad hash invalid" : "bad hash accepted");

        Say(Ok(SdkProxyInteraction.CookieSet(player, "basalt:demo", Encoding.UTF8.GetBytes("abcd"))) ? "cookie stored" : "cookie store failed");
        Say(SdkProxyInteraction.CookieRequest(player, "basalt:demo", OnCookie) > 0 ? "cookie requested" : "cookie request failed");

        Say(Ok(SdkProxyInteraction.PlayerDataSetF64(player, "ratio", 2.5)) ? "float stored" : "float store failed");
        Say(SdkProxyInteraction.PlayerDataGetF64(player, "ratio", 0.0) == 2.5 ? "float read ok" : "float read failed");
        Say(SdkProxyInteraction.PlayerDataGetF64(player, "missing", 7.5) == 7.5 ? "float default ok" : "float default failed");

        Say(Ok(SdkProxyInteraction.PlayersBroadcast("<green>Interaction hello {who}", new Args().Add("who", "everyone"))) ? "broadcast ok" : "broadcast failed");
        Say(SdkProxyInteraction.PlayerTransfer(player, "", 25565) == -3 ? "bad host invalid" : "bad host accepted");
    }

    public static void Init()
    {
        SdkEvents.On("player.join", Priority.Normal, OnJoin);
        SdkEvents.On("player.quit", Priority.Normal, OnQuit);
        SdkProxyInteraction.WatchResourcePacks(OnPackStatus);
        SdkProxyInteraction.WatchSettings(OnSettings);
        SdkCommands.Command("pidialog", "", OnDialogClick);
        SdkCommands.Command("pitransfer", "", OnTransfer);
        SdkCommands.Command("picleanup", "", OnCleanup);
        Say("interaction ready");
    }
}
