package interaction

import "basaltmc.org/sdk/proxy/internal/runtime"

type DialogButton = runtime.DialogButton

func WatchResourcePacks(handler runtime.Handler) int32 {
	return runtime.ProxyWatchResourcePacks(handler)
}

func WatchSettings(handler runtime.Handler) int32 { return runtime.ProxyWatchSettings(handler) }

func ValueResult(ctx runtime.Ctx) (runtime.Value, error) { return runtime.ProxyValueResult(ctx) }

func DialogShow(player runtime.Player, kind, title, body string, buttons []runtime.DialogButton, closable bool) int32 {
	return runtime.ProxyDialogShow(player, kind, title, body, buttons, closable)
}

func DialogClear(player runtime.Player) int32 { return runtime.ProxyDialogClear(player) }

func ResourcePackPush(player runtime.Player, url, hash string, required bool, prompt, id string) (string, int32) {
	return runtime.ProxyResourcePackPush(player, url, hash, required, prompt, id)
}

func ResourcePackRemove(player runtime.Player, id string) int32 {
	return runtime.ProxyResourcePackRemove(player, id)
}

func ResourcePackClear(player runtime.Player) int32 { return runtime.ProxyResourcePackClear(player) }

func CookieSet(player runtime.Player, key string, data []byte) int32 {
	return runtime.ProxyCookieSet(player, key, data)
}

func CookieRequest(player runtime.Player, key string, handler runtime.Handler) int32 {
	return runtime.ProxyCookieRequest(player, key, handler)
}

func CookieResult(ctx runtime.Ctx) ([]byte, bool) { return runtime.ProxyCookieResult(ctx) }

func RequestCancel(request int32) int32 { return runtime.ProxyRequestCancel(request) }

func PlayerTransfer(player runtime.Player, host string, port int32) int32 {
	return runtime.ProxyPlayerTransfer(player, host, port)
}

func PlayersByUUID(uuid string) runtime.Player { return runtime.ProxyPlayersByUUID(uuid) }

func PlayersCount(server string) int32 { return runtime.ProxyPlayersCount(server) }

func PlayersBroadcast(text string, args *runtime.Args) int32 {
	return runtime.ProxyPlayersBroadcast(text, args)
}

func PlayerAddress(player runtime.Player) (string, bool) { return runtime.ProxyPlayerAddress(player) }

func PlayerPort(player runtime.Player) int32 { return runtime.ProxyPlayerPort(player) }

func PlayerSettings(player runtime.Player) (runtime.Value, error) {
	return runtime.ProxyPlayerSettings(player)
}

func PlayerDataGetF64(player runtime.Player, key string, fallback float64) float64 {
	return runtime.ProxyPlayerDataGetF64(player, key, fallback)
}

func PlayerDataSetF64(player runtime.Player, key string, value float64) int32 {
	return runtime.ProxyPlayerDataSetF64(player, key, value)
}
