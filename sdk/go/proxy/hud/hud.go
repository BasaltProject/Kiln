package hud

import "basaltmc.org/sdk/proxy/internal/runtime"

func HudClearTitle(player runtime.Player, reset bool) int32 {
	return runtime.ProxyHudClearTitle(player, reset)
}

func HudTabList(player runtime.Player, header, footer *string) int32 {
	return runtime.ProxyHudTabList(player, header, footer)
}

func HudBossbarShow(player runtime.Player, id, text string, progress float64, color, overlay string) int32 {
	return runtime.ProxyHudBossbarShow(player, id, text, progress, color, overlay)
}

func HudBossbarHide(player runtime.Player, id string) int32 {
	return runtime.ProxyHudBossbarHide(player, id)
}

func HudBossbarProgress(player runtime.Player, id string, progress float64) int32 {
	return runtime.ProxyHudBossbarProgress(player, id, progress)
}

func HudBossbarTitle(player runtime.Player, id, text string) int32 {
	return runtime.ProxyHudBossbarTitle(player, id, text)
}

func HudBossbarStyle(player runtime.Player, id, color, overlay string) int32 {
	return runtime.ProxyHudBossbarStyle(player, id, color, overlay)
}

func HudSidebar(player runtime.Player, title string, lines []string) int32 {
	return runtime.ProxyHudSidebar(player, title, lines)
}

func HudSidebarClear(player runtime.Player) int32 { return runtime.ProxyHudSidebarClear(player) }

func HudTabEntry(player runtime.Player, id, name, text string, latency int64) int32 {
	return runtime.ProxyHudTabEntry(player, id, name, text, latency)
}

func HudTabEntryRemove(player runtime.Player, id string) int32 {
	return runtime.ProxyHudTabEntryRemove(player, id)
}

func HudServerLinks(player runtime.Player, pairs []string) int32 {
	return runtime.ProxyHudServerLinks(player, pairs)
}

func HudChatCompletions(player runtime.Player, action string, words []string) int32 {
	return runtime.ProxyHudChatCompletions(player, action, words)
}
