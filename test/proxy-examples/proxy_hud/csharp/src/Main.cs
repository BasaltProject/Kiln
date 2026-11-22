using Basalt.Proxy.Core;
using Basalt.Proxy.Values;
using SdkEvents = global::Basalt.Proxy.Events.Events;
using SdkLogger = global::Basalt.Proxy.Log.Logger;
using SdkProxyHud = global::Basalt.Proxy.Hud.ProxyHud;
using SdkTasks = global::Basalt.Proxy.Tasks.Tasks;
public static class Plugin
{
    static int target;

    static void Step(string name, int code) => SdkLogger.LogInfo("hud " + name + (code == 0 ? " ok" : " failed"));

    static void OnLater(int ctx)
    {
        Step("bossbar_progress", SdkProxyHud.BossbarProgress(target, "bar", 0.25));
        Step("bossbar_title", SdkProxyHud.BossbarTitle(target, "bar", "<yellow>Updated"));
        Step("bossbar_style", SdkProxyHud.BossbarStyle(target, "bar", "green", "notched_6"));
        Step("bossbar_hide", SdkProxyHud.BossbarHide(target, "bar"));
        Step("sidebar_clear", SdkProxyHud.SidebarClear(target));
        Step("tab_entry_remove", SdkProxyHud.TabEntryRemove(target, "row"));
        Step("tab_list_keep", SdkProxyHud.TabList(target, null, "<gray>Footer two"));
        Step("chat_completions_remove", SdkProxyHud.ChatCompletions(target, "remove", new[] { "alpha" }));
        Step("clear_title", SdkProxyHud.ClearTitle(target, true));
    }

    static void OnJoin(int ctx)
    {
        target = SdkEvents.EventPlayer(ctx);
        if (target == 0) return;
        Step("clear_title", SdkProxyHud.ClearTitle(target, false));
        Step("tab_list", SdkProxyHud.TabList(target, "<gold>Header", "<gray>Footer"));
        Step("bossbar_show", SdkProxyHud.BossbarShow(target, "bar", "<red>Boss", 0.75, "red", "notched_10"));
        Step("sidebar", SdkProxyHud.Sidebar(target, "<gold>Title", new[] { "<white>first line", "<white>second line" }));
        Step("tab_entry", SdkProxyHud.TabEntry(target, "row", "Fake", "<green>Fake row", 25));
        Step("server_links", SdkProxyHud.ServerLinks(target, new[] { "website", "https://basaltmc.org", "<aqua>Custom", "https://example.org" }));
        Step("chat_completions_set", SdkProxyHud.ChatCompletions(target, "set", new[] { "alpha", "beta" }));
        Step("chat_completions_add", SdkProxyHud.ChatCompletions(target, "add", new[] { "gamma" }));
        Step("invalid_id", SdkProxyHud.BossbarHide(target, "") == -3 ? 0 : 1);
        Step("stale_player", SdkProxyHud.BossbarHide(0x7fff0000, "bar") == -2 ? 0 : 1);
        SdkTasks.After(100, OnLater);
    }

    public static void Init()
    {
        SdkEvents.On("player.join", Priority.Normal, OnJoin);
    }
}
