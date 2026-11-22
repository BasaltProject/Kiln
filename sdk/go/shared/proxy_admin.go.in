package runtime

// Proxy admin area (abi/areas/proxy_admin.h): backends, console and chat, permission providers, groups, component
// JSON, the plugin store, data by holder and waiting. Calls that give a value answer a Value; the others the ABI result.

// ProxyServersRegister adds a backend until the proxy stops; a negative status comes with the reason text.
func ProxyServersRegister(name, host string, port int32, like string) (int32, string) {
	n, nl := ptr(name)
	h, hl := ptr(host)
	l, ll := ptr(like)
	reason := make([]byte, 256)
	status := rawProxyServersRegister(n, nl, h, hl, port, l, ll, &reason[0], int32(len(reason)))
	end := 0
	for end < len(reason) && reason[end] != 0 {
		end++
	}
	if status >= 0 {
		return status, ""
	}
	return status, string(reason[:end])
}

func ProxyServersUnregister(name string) int32 {
	n, nl := ptr(name)
	return rawProxyServersUnregister(n, nl)
}

// ProxyServersPing pings a backend; the handler reads the answer with ProxyCompletionValue. Returns the request.
func ProxyServersPing(name string, handler Handler) int32 {
	n, nl := ptr(name)
	return rawProxyServersPing(n, nl, tokenFor(handler))
}

// ProxyWait runs the handler after ticks ticks of 50 ms. Returns the request.
func ProxyWait(ticks int32, handler Handler) int32 { return rawProxyWait(ticks, tokenFor(handler)) }

func ProxyCompletionValue(ctx Ctx) (Value, error) {
	return CallValue(256, func(out *byte, cap int32) int32 { return rawValueResult(ctx, out, cap) })
}

// ProxyConsoleRun answers 1 when the line named a command, 0 when not, -4 without the capability and the grant.
func ProxyConsoleRun(line string) int32 {
	l, ll := ptr(line)
	return rawProxyConsoleRun(l, ll)
}

func ProxyPlayerChat(player Player, text string) int32 {
	t, tl := ptr(text)
	return rawProxyPlayerChat(player, t, tl)
}

func ProxyPlayerSendJson(player Player, json string) int32 {
	j, jl := ptr(json)
	return rawProxyPlayerSendJson(player, j, jl)
}

func ProxyPlayerKickJson(player Player, json string) int32 {
	j, jl := ptr(json)
	return rawProxyPlayerKickJson(player, j, jl)
}

// ProxyPermissionCheck answers 1 granted, 0 denied, -1 undecided. who "" is the console; server may be empty.
func ProxyPermissionCheck(who, node, server string) int32 {
	w, wl := ptr(who)
	n, nl := ptr(node)
	var context []byte
	if server != "" {
		doc, err := NewValueWriter().BeginMap().Key("server").String(server).End().Finish()
		if err != nil {
			return -3
		}
		context = doc
	}
	if len(context) == 0 {
		return rawProxyPermissionCheck(w, wl, n, nl, w, 0)
	}
	return rawProxyPermissionCheck(w, wl, n, nl, &context[0], int32(len(context)))
}

func ProxyPermissionMeta(who, key string) (string, bool) {
	w, wl := ptr(who)
	k, kl := ptr(key)
	return fetch(func(out *byte, cap int32) int32 { return rawProxyPermissionMeta(w, wl, k, kl, k, 0, out, cap) })
}

// ProxyPermissionProvide registers provider callbacks (nil for none); returns the provider handle.
func ProxyPermissionProvide(check, meta Handler, priority int32) int32 {
	var checkToken, metaToken int32
	if check != nil {
		checkToken = tokenFor(check)
	}
	if meta != nil {
		metaToken = tokenFor(meta)
	}
	return rawProxyPermissionProvide(checkToken, metaToken, priority)
}

func ProxyPermissionUnprovide(provider int32) int32 { return rawProxyPermissionUnprovide(provider) }

// ProxyPermissionRequest is the question of a provider callback: a map with kind, name, uuid, console, text, context.
func ProxyPermissionRequest(ctx Ctx) (Value, error) {
	return CallValue(256, func(out *byte, cap int32) int32 { return rawProxyPermissionRequest(ctx, out, cap) })
}

// ProxyPermissionAnswer answers 1 grants, 0 denies, negative leaves it to the next provider.
func ProxyPermissionAnswer(ctx Ctx, answer int32) int32 { return rawProxyPermissionAnswer(ctx, answer) }

func ProxyPermissionAnswerText(ctx Ctx, text string) int32 {
	t, tl := ptr(text)
	return rawProxyPermissionAnswerText(ctx, t, tl)
}

func ProxyPermissionChanged() int32 { return rawProxyPermissionChanged() }

// ProxyPermissionOverride grants (1), denies (0) or removes (-1) a node for a player name.
func ProxyPermissionOverride(name, node string, value int32) int32 {
	n, nl := ptr(name)
	o, ol := ptr(node)
	return rawProxyPermissionOverride(n, nl, o, ol, value)
}

func ProxyPermissionList(name string) (Value, error) {
	n, nl := ptr(name)
	return CallValue(256, func(out *byte, cap int32) int32 { return rawProxyPermissionList(n, nl, out, cap) })
}

func ProxyGroupGet(name string) (string, bool) {
	n, nl := ptr(name)
	return fetch(func(out *byte, cap int32) int32 { return rawProxyGroupGet(n, nl, out, cap) })
}

func ProxyGroupSet(name, group string) int32 {
	n, nl := ptr(name)
	g, gl := ptr(group)
	return rawProxyGroupSet(n, nl, g, gl)
}

// ProxyDataValue reads a store key (empty holder) or a player data key of holder; an error when it is unset.
func ProxyDataValue(holder, key string) (Value, error) {
	k, kl := ptr(key)
	h, hl := ptr(holder)
	return CallValue(256, func(out *byte, cap int32) int32 {
		if holder == "" {
			return rawProxyStoreGet(k, kl, out, cap)
		}
		return rawProxyPlayerDataGet(h, hl, k, kl, out, cap)
	})
}

func ProxyDataSetValue(holder, key string, doc []byte) int32 {
	k, kl := ptr(key)
	h, hl := ptr(holder)
	if len(doc) == 0 {
		return -3
	}
	if holder == "" {
		return rawProxyStoreSet(k, kl, &doc[0], int32(len(doc)))
	}
	return rawProxyPlayerDataSet(h, hl, k, kl, &doc[0], int32(len(doc)))
}

func proxyDataSetWith(holder, key string, fill func(w *ValueWriter)) int32 {
	w := NewValueWriter()
	fill(w)
	doc, err := w.Finish()
	if err != nil {
		return -3
	}
	return ProxyDataSetValue(holder, key, doc)
}

func ProxyDataSetInt(holder, key string, number int64) int32 {
	return proxyDataSetWith(holder, key, func(w *ValueWriter) { w.Int(number) })
}

func ProxyDataSetFloat(holder, key string, number float64) int32 {
	return proxyDataSetWith(holder, key, func(w *ValueWriter) { w.Float(number) })
}

func ProxyDataSetBool(holder, key string, flag bool) int32 {
	return proxyDataSetWith(holder, key, func(w *ValueWriter) { w.Bool(flag) })
}

func ProxyDataSetStr(holder, key, text string) int32 {
	return proxyDataSetWith(holder, key, func(w *ValueWriter) { w.String(text) })
}

func ProxyDataRemove(holder, key string) int32 {
	return proxyDataSetWith(holder, key, func(w *ValueWriter) { w.Null() })
}

// ProxyStoreTop is the best limit numeric values of a store key: a list of maps with uuid, name and value.
func ProxyStoreTop(key string, limit int32) (Value, error) {
	k, kl := ptr(key)
	return CallValue(256, func(out *byte, cap int32) int32 { return rawProxyStoreTop(k, kl, limit, out, cap) })
}
