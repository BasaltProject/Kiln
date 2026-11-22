package runtime

// Proxy plugins only (plugin.toml `target = "proxy"`): the calls of basalt_proxy_abi.h.

//go:wasmimport basalt_proxy event_set_str
func rawProxyEventSetStr(ctx int32, field *byte, flen int32, value *byte, vlen int32) int32

//go:wasmimport basalt_proxy event_set_i64
func rawProxyEventSetI64(ctx int32, field *byte, flen int32, value int64) int32

//go:wasmimport basalt_proxy player_uuid
func rawProxyPlayerUUID(player int32, out *byte, cap int32) int32

//go:wasmimport basalt_proxy player_server
func rawProxyPlayerServer(player int32, out *byte, cap int32) int32

//go:wasmimport basalt_proxy player_ping
func rawProxyPlayerPing(player int32) int32

//go:wasmimport basalt_proxy player_setting_str
func rawProxyPlayerSettingStr(player int32, field *byte, flen int32, out *byte, cap int32) int32

//go:wasmimport basalt_proxy player_setting_i64
func rawProxyPlayerSettingI64(player int32, field *byte, flen int32, out *int64) int32

//go:wasmimport basalt_proxy player_has_permission
func rawProxyPlayerHasPermission(player int32, node *byte, nlen int32) int32

//go:wasmimport basalt_proxy player_title
func rawProxyPlayerTitle(player int32, title *byte, tlen int32, subtitle *byte, slen int32, fadeIn, stay, fadeOut int32) int32

//go:wasmimport basalt_proxy player_connect
func rawProxyPlayerConnect(player int32, server *byte, slen int32) int32

//go:wasmimport basalt_proxy player_kick
func rawProxyPlayerKick(player int32, reason *byte, rlen int32, args *byte, alen int32) int32

//go:wasmimport basalt_proxy servers_list
func rawProxyServersList(out *byte, cap int32) int32

//go:wasmimport basalt_proxy server_address
func rawProxyServerAddress(name *byte, nlen int32, out *byte, cap int32) int32

//go:wasmimport basalt_proxy players_on
func rawProxyPlayersOn(server *byte, slen int32, out *int32, cap int32) int32

func ProxyEventSetStr(ctx Ctx, field, value string) int32 {
	f, fl := ptr(field)
	v, vl := ptr(value)
	return rawProxyEventSetStr(ctx, f, fl, v, vl)
}
func ProxyEventSetI64(ctx Ctx, field string, value int64) int32 {
	f, fl := ptr(field)
	return rawProxyEventSetI64(ctx, f, fl, value)
}

func ProxyPlayerUUID(player Player) (string, bool) {
	return fetch(func(out *byte, cap int32) int32 { return rawProxyPlayerUUID(player, out, cap) })
}
func ProxyPlayerServer(player Player) (string, bool) {
	return fetch(func(out *byte, cap int32) int32 { return rawProxyPlayerServer(player, out, cap) })
}

// ProxyPlayerPing is in milliseconds, a negative code when unknown.
func ProxyPlayerPing(player Player) int32 { return rawProxyPlayerPing(player) }
func ProxyPlayerSettingStr(player Player, field string) (string, bool) {
	f, fl := ptr(field)
	return fetch(func(out *byte, cap int32) int32 { return rawProxyPlayerSettingStr(player, f, fl, out, cap) })
}
func ProxyPlayerSettingI64(player Player, field string) (int64, bool) {
	f, fl := ptr(field)
	var value int64
	ok := rawProxyPlayerSettingI64(player, f, fl, &value) >= 0
	return value, ok
}
func ProxyPlayerHasPermission(player Player, node string) bool {
	n, nl := ptr(node)
	return rawProxyPlayerHasPermission(player, n, nl) == 1
}

// ProxyPlayerTitle takes times in ticks; a negative one means the client default.
func ProxyPlayerTitle(player Player, title, subtitle string, fadeIn, stay, fadeOut int32) int32 {
	t, tl := ptr(title)
	s, sl := ptr(subtitle)
	return rawProxyPlayerTitle(player, t, tl, s, sl, fadeIn, stay, fadeOut)
}
func ProxyPlayerConnect(player Player, server string) int32 {
	s, sl := ptr(server)
	return rawProxyPlayerConnect(player, s, sl)
}
func ProxyPlayerKick(player Player, reason string, args *Args) int32 {
	r, rl := ptr(reason)
	a, al := args.packed()
	return rawProxyPlayerKick(player, r, rl, a, al)
}

func ProxyServersList() []string {
	text, ok := fetchWith(1024, func(out *byte, cap int32) int32 { return rawProxyServersList(out, cap) })
	if !ok {
		return nil
	}
	var names []string
	start := 0
	for i := 0; i <= len(text); i++ {
		if i == len(text) || text[i] == '\n' {
			if i > start {
				names = append(names, text[start:i])
			}
			start = i + 1
		}
	}
	return names
}
func ProxyServerAddress(name string) (string, bool) {
	n, nl := ptr(name)
	return fetch(func(out *byte, cap int32) int32 { return rawProxyServerAddress(n, nl, out, cap) })
}
func ProxyPlayersOn(server string) []Player {
	s, sl := ptr(server)
	out := make([]int32, 64)
	for {
		count := rawProxyPlayersOn(s, sl, &out[0], int32(len(out)))
		if count < 0 {
			return nil
		}
		if int(count) <= len(out) {
			return out[:count]
		}
		out = make([]int32, count)
	}
}
