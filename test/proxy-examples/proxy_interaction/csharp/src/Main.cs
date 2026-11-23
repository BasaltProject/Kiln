using Basalt.Proxy.Commands;
using Basalt.Proxy.Core;
using Basalt.Proxy.Events;
using Basalt.Proxy.Interaction;
using Basalt.Proxy.Log;
using Basalt.Proxy.Players;
using Basalt.Proxy.Tasks;
using Basalt.Proxy.Values;
using System.Text;

public static class Plugin
{
    static int seen;

    static void Say(string text) => Logger.LogInfo(text);

    static bool Ok(int result) => result >= 0;

    static string TextOf(Value root, string key)
    {
        return root.TryFind(key, out Value child) && child.TryString(out string text) ? text : "";
    }

    static void OnPackStatus(int ctx)
    {
        if (!ClientActions.TryValueResult(ctx, out Value root))
        {
            Say("pack event unreadable");
            return;
        }
        Say("pack status:");
        Say(TextOf(root, "status"));
    }

    static void OnSettings(int ctx)
    {
        if (!ClientActions.TryValueResult(ctx, out Value root))
        {
            Say("settings event unreadable");
            return;
        }
        Say("settings locale:");
        Say(root.TryFind("settings", out Value settings) ? TextOf(settings, "locale") : "");
    }

    static void OnCookie(int ctx)
    {
        byte[] cookie = ClientActions.CookieResult(ctx);
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
        Sender.Reply(ctx, "<green>dialog clicked");
    }

    static void OnTransfer(int ctx)
    {
        int player = EventBus.EventPlayer(ctx);
        Say(Ok(ClientActions.PlayerTransfer(player, "127.0.0.1", 25970)) ? "transfer sent" : "transfer failed");
    }

    static void OnCleanup(int ctx)
    {
        int player = EventBus.EventPlayer(ctx);
        bool removed = Ok(ClientActions.ResourcePackRemove(player, "demo"));
        bool cleared = Ok(ClientActions.ResourcePackClear(player));
        bool closed = Ok(ClientActions.DialogClear(player));
        Say(removed && cleared && closed ? "cleanup ok" : "cleanup failed");
    }

    static void OnStale(int ctx)
    {
        Say(ClientActions.DialogClear(seen) == -2 ? "stale after quit" : "stale not detected");
    }

    static void OnQuit(int ctx)
    {
        Scheduler.After(20, OnStale);
    }

    static void OnJoin(int ctx)
    {
        int player = EventBus.EventPlayer(ctx);
        seen = player;
        Say(ClientActions.PlayersCount() >= 1 ? "count ok" : "count failed");
        Say(ClientActions.PlayersCount("lobby") >= 1 ? "count lobby ok" : "count lobby failed");
        Say(ClientActions.PlayersCount("nowhere") == 0 ? "count nowhere ok" : "count nowhere failed");
        string address = ClientActions.PlayerAddress(player);
        Say(!string.IsNullOrEmpty(address) && ClientActions.PlayerPort(player) > 0 ? "address ok" : "address failed");
        string uuid = Directory.PlayerUuid(player) ?? "";
        Say(ClientActions.PlayersByUuid(uuid) == player ? "by uuid ok" : "by uuid failed");
        Say(ClientActions.PlayersByUuid("00000000-0000-0000-0000-000000000000") == 0 ? "by uuid none ok" : "by uuid none failed");

        DialogButton[] buttons = { new DialogButton("Accept", "pidialog"), new DialogButton("Decline", "pidialog") };
        Say(Ok(ClientActions.DialogShow(player, "multi_action", "<gold>Interaction", "<gray>Pick one", buttons)) ? "dialog ok" : "dialog failed");
        Say(ClientActions.DialogShow(player, "bogus", "x", null, new DialogButton[0]) == -3 ? "bad kind invalid" : "bad kind accepted");

        string id = ClientActions.ResourcePackPush(player, "https://example.invalid/pack.zip", null, false, "<gold>Test pack", "demo");
        if (id != null)
        {
            Say("pack pushed:");
            Say(id);
        }
        else
        {
            Say("pack failed");
        }
        string bad = ClientActions.ResourcePackPush(player, "https://example.invalid/pack.zip", "nothex", false, null, null);
        Say(bad == null ? "bad hash invalid" : "bad hash accepted");

        Say(Ok(ClientActions.CookieSet(player, "basalt:demo", Encoding.UTF8.GetBytes("abcd"))) ? "cookie stored" : "cookie store failed");
        Say(ClientActions.CookieRequest(player, "basalt:demo", OnCookie) > 0 ? "cookie requested" : "cookie request failed");

        Say(Ok(ClientActions.PlayerDataSetF64(player, "ratio", 2.5)) ? "float stored" : "float store failed");
        Say(ClientActions.PlayerDataGetF64(player, "ratio", 0.0) == 2.5 ? "float read ok" : "float read failed");
        Say(ClientActions.PlayerDataGetF64(player, "missing", 7.5) == 7.5 ? "float default ok" : "float default failed");

        Say(Ok(ClientActions.PlayersBroadcast("<green>Interaction hello {who}", new Args().Add("who", "everyone"))) ? "broadcast ok" : "broadcast failed");
        Say(ClientActions.PlayerTransfer(player, "", 25565) == -3 ? "bad host invalid" : "bad host accepted");
    }

    public static void Init()
    {
        EventBus.On("player.join", Priority.Normal, OnJoin);
        EventBus.On("player.quit", Priority.Normal, OnQuit);
        ClientActions.WatchResourcePacks(OnPackStatus);
        ClientActions.WatchSettings(OnSettings);
        Registry.Register("pidialog", "", OnDialogClick);
        Registry.Register("pitransfer", "", OnTransfer);
        Registry.Register("picleanup", "", OnCleanup);
        Say("interaction ready");
    }
}
