package players

import "basaltmc.org/sdk/proxy/internal/runtime"

func PlayerByName(name string) runtime.Player { return runtime.PlayerByName(name) }

func PlayersOnline() []runtime.Player { return runtime.PlayersOnline() }

func PlayerName(player runtime.Player) (string, bool) { return runtime.PlayerName(player) }

func Send(player runtime.Player, text string, args *runtime.Args) int32 {
	return runtime.Send(player, text, args)
}

func ActionBar(player runtime.Player, text string, args *runtime.Args) int32 {
	return runtime.ActionBar(player, text, args)
}

func PlayerUUID(player runtime.Player) (string, bool) { return runtime.ProxyPlayerUUID(player) }

func PlayerServer(player runtime.Player) (string, bool) { return runtime.ProxyPlayerServer(player) }

func PlayerPing(player runtime.Player) int32 { return runtime.ProxyPlayerPing(player) }

func PlayerSettingStr(player runtime.Player, field string) (string, bool) {
	return runtime.ProxyPlayerSettingStr(player, field)
}

func PlayerSettingI64(player runtime.Player, field string) (int64, bool) {
	return runtime.ProxyPlayerSettingI64(player, field)
}

func PlayerHasPermission(player runtime.Player, node string) bool {
	return runtime.ProxyPlayerHasPermission(player, node)
}

func PlayerTitle(player runtime.Player, title, subtitle string, fadeIn, stay, fadeOut int32) int32 {
	return runtime.ProxyPlayerTitle(player, title, subtitle, fadeIn, stay, fadeOut)
}

func PlayerConnect(player runtime.Player, server string) int32 {
	return runtime.ProxyPlayerConnect(player, server)
}

func PlayerKick(player runtime.Player, reason string, args *runtime.Args) int32 {
	return runtime.ProxyPlayerKick(player, reason, args)
}
