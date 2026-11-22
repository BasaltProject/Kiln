package main

import (
	bsCore "basaltmc.org/sdk/proxy/core"
	bsEvents "basaltmc.org/sdk/proxy/events"
	bsHud "basaltmc.org/sdk/proxy/hud"
	bsLog "basaltmc.org/sdk/proxy/log"
	bsTasks "basaltmc.org/sdk/proxy/tasks"
)

var target bsCore.Player

func init() { bsCore.Init(setup) }

func main() {}

func step(name string, code int32) {
	state := " ok"
	if code != 0 {
		state = " failed"
	}
	bsLog.LogInfo("hud " + name + state)
}

func onLater(ctx bsCore.Ctx) {
	footer := "<gray>Footer two"
	step("bossbar_progress", bsHud.HudBossbarProgress(target, "bar", 0.25))
	step("bossbar_title", bsHud.HudBossbarTitle(target, "bar", "<yellow>Updated"))
	step("bossbar_style", bsHud.HudBossbarStyle(target, "bar", "green", "notched_6"))
	step("bossbar_hide", bsHud.HudBossbarHide(target, "bar"))
	step("sidebar_clear", bsHud.HudSidebarClear(target))
	step("tab_entry_remove", bsHud.HudTabEntryRemove(target, "row"))
	step("tab_list_keep", bsHud.HudTabList(target, nil, &footer))
	step("chat_completions_remove", bsHud.HudChatCompletions(target, "remove", []string{"alpha"}))
	step("clear_title", bsHud.HudClearTitle(target, true))
}

func onJoin(ctx bsCore.Ctx) {
	header, footer := "<gold>Header", "<gray>Footer"
	target = bsEvents.EventPlayer(ctx)
	if target == 0 {
		return
	}
	step("clear_title", bsHud.HudClearTitle(target, false))
	step("tab_list", bsHud.HudTabList(target, &header, &footer))
	step("bossbar_show", bsHud.HudBossbarShow(target, "bar", "<red>Boss", 0.75, "red", "notched_10"))
	step("sidebar", bsHud.HudSidebar(target, "<gold>Title", []string{"<white>first line", "<white>second line"}))
	step("tab_entry", bsHud.HudTabEntry(target, "row", "Fake", "<green>Fake row", 25))
	step("server_links", bsHud.HudServerLinks(target, []string{"website", "https://basaltmc.org", "<aqua>Custom", "https://example.org"}))
	step("chat_completions_set", bsHud.HudChatCompletions(target, "set", []string{"alpha", "beta"}))
	step("chat_completions_add", bsHud.HudChatCompletions(target, "add", []string{"gamma"}))
	step("invalid_id", boolCode(bsHud.HudBossbarHide(target, "") == -3))
	step("stale_player", boolCode(bsHud.HudBossbarHide(0x7fff0000, "bar") == -2))
	bsTasks.After(100, onLater)
}

func boolCode(ok bool) int32 {
	if ok {
		return 0
	}
	return 1
}

func setup() {
	bsEvents.On("player.join", bsCore.Normal, onJoin)
}
