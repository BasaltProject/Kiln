using Basalt.Proxy.Core;
using Basalt.Proxy.Events;
using Basalt.Proxy.Hud;
using Basalt.Proxy.Log;
using Basalt.Proxy.Tasks;
using Basalt.Proxy.Values;

public static class Plugin
{
    static int target;

    static void Step(string name, int code) => Logger.LogInfo("hud " + name + (code == 0 ? " ok" : " failed"));

    static void OnLater(int ctx)
    {
        Step("bossbar_progress", Displays.BossbarProgress(target, "bar", 0.25));
        Step("bossbar_title", Displays.BossbarTitle(target, "bar", "<yellow>Updated"));
        Step("bossbar_style", Displays.BossbarStyle(target, "bar", "green", "notched_6"));
        Step("bossbar_hide", Displays.BossbarHide(target, "bar"));
        Step("sidebar_clear", Displays.SidebarClear(target));
        Step("tab_entry_remove", Displays.TabEntryRemove(target, "row"));
        Step("tab_list_keep", Displays.TabList(target, null, "<gray>Footer two"));
        Step("chat_completions_remove", Displays.ChatCompletions(target, "remove", new[] { "alpha" }));
        Step("clear_title", Displays.ClearTitle(target, true));
    }

    static void OnJoin(int ctx)
    {
        target = EventBus.EventPlayer(ctx);
        if (target == 0) return;
        Step("clear_title", Displays.ClearTitle(target, false));
        Step("tab_list", Displays.TabList(target, "<gold>Header", "<gray>Footer"));
        Step("bossbar_show", Displays.BossbarShow(target, "bar", "<red>Boss", 0.75, "red", "notched_10"));
        Step("sidebar", Displays.Sidebar(target, "<gold>Title", new[] { "<white>first line", "<white>second line" }));
        Step("tab_entry", Displays.TabEntry(target, "row", "Fake", "<green>Fake row", 25));
        Step("server_links", Displays.ServerLinks(target, new[] { "website", "https://basaltmc.org", "<aqua>Custom", "https://example.org" }));
        Step("chat_completions_set", Displays.ChatCompletions(target, "set", new[] { "alpha", "beta" }));
        Step("chat_completions_add", Displays.ChatCompletions(target, "add", new[] { "gamma" }));
        Step("invalid_id", Displays.BossbarHide(target, "") == -3 ? 0 : 1);
        Step("stale_player", Displays.BossbarHide(0x7fff0000, "bar") == -2 ? 0 : 1);
        Scheduler.After(100, OnLater);
    }

    public static void Init()
    {
        EventBus.On("player.join", Priority.Normal, OnJoin);
    }
}
