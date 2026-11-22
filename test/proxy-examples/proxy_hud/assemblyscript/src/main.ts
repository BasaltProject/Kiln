import * as bsCore from "basalt-proxy/core";
import * as bsEvents from "basalt-proxy/events";
import * as bsHud from "basalt-proxy/hud";
import * as bsLog from "basalt-proxy/log";
import * as bsTasks from "basalt-proxy/tasks";

let target: bsCore.Player = 0;

function step(name: string, code: i32): void {
  bsLog.logInfo("hud " + name + (code == 0 ? " ok" : " failed"));
}

function onLater(ctx: bsCore.Ctx): void {
  step("bossbar_progress", bsHud.proxyHudBossbarProgress(target, "bar", 0.25));
  step("bossbar_title", bsHud.proxyHudBossbarTitle(target, "bar", "<yellow>Updated"));
  step("bossbar_style", bsHud.proxyHudBossbarStyle(target, "bar", "green", "notched_6"));
  step("bossbar_hide", bsHud.proxyHudBossbarHide(target, "bar"));
  step("sidebar_clear", bsHud.proxyHudSidebarClear(target));
  step("tab_entry_remove", bsHud.proxyHudTabEntryRemove(target, "row"));
  step("tab_list_keep", bsHud.proxyHudTabList(target, null, "<gray>Footer two"));
  step("chat_completions_remove", bsHud.proxyHudChatCompletions(target, "remove", ["alpha"]));
  step("clear_title", bsHud.proxyHudClearTitle(target, true));
}

function onJoin(ctx: bsCore.Ctx): void {
  target = bsEvents.eventPlayer(ctx);
  if (target == 0) return;
  step("clear_title", bsHud.proxyHudClearTitle(target, false));
  step("tab_list", bsHud.proxyHudTabList(target, "<gold>Header", "<gray>Footer"));
  step("bossbar_show", bsHud.proxyHudBossbarShow(target, "bar", "<red>Boss", 0.75, "red", "notched_10"));
  step("sidebar", bsHud.proxyHudSidebar(target, "<gold>Title", ["<white>first line", "<white>second line"]));
  step("tab_entry", bsHud.proxyHudTabEntry(target, "row", "Fake", "<green>Fake row", 25));
  step("server_links", bsHud.proxyHudServerLinks(target, ["website", "https://basaltmc.org", "<aqua>Custom", "https://example.org"]));
  step("chat_completions_set", bsHud.proxyHudChatCompletions(target, "set", ["alpha", "beta"]));
  step("chat_completions_add", bsHud.proxyHudChatCompletions(target, "add", ["gamma"]));
  step("invalid_id", bsHud.proxyHudBossbarHide(target, "") == -3 ? 0 : 1);
  step("stale_player", bsHud.proxyHudBossbarHide(0x7fff0000, "bar") == -2 ? 0 : 1);
  bsTasks.after(100, onLater);
}

export function init(): void {
  bsEvents.on("player.join", bsCore.NORMAL, onJoin);
}
