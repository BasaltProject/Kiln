package runtime

// Proxy interaction area (abi/areas/proxy_interaction.h): dialogs, resource packs, cookies, transfer and extra player
// operations. Observed events and completions are read with ProxyValueResult and the Value reader.

const piInvalid int32 = -3

// DialogButton is a label and the command the client runs as the player when it is clicked.
type DialogButton struct {
	Label   string
	Command string
}

// ProxyWatchResourcePacks observes player.resource_pack: the value holds player, player_name, id and status.
func ProxyWatchResourcePacks(handler Handler) int32 {
	n, nl := ptr("player.resource_pack")
	return rawProxyInteractionWatch(n, nl, tokenFor(handler))
}

// ProxyWatchSettings observes player.settings: the value holds player, player_name and settings.
func ProxyWatchSettings(handler Handler) int32 {
	n, nl := ptr("player.settings")
	return rawProxyInteractionWatch(n, nl, tokenFor(handler))
}

// ProxyValueResult is the value of an observed event or a completion.
func ProxyValueResult(ctx Ctx) (Value, error) {
	return CallValue(256, func(out *byte, cap int32) int32 { return rawValueResult(ctx, out, cap) })
}

// ProxyDialogShow opens a dialog; kind is "notice", "confirmation" or "multi_action" and body may be empty.
func ProxyDialogShow(player Player, kind, title, body string, buttons []DialogButton, closable bool) int32 {
	w := NewValueWriter()
	w.BeginMap().Key("kind").String(kind).Key("title").String(title)
	if body != "" {
		w.Key("body").String(body)
	}
	w.Key("buttons").BeginList()
	for _, button := range buttons {
		w.BeginMap().Key("label").String(button.Label).Key("command").String(button.Command).End()
	}
	w.End().Key("can_close_with_escape").Bool(closable).End()
	doc, err := w.Finish()
	if err != nil {
		return piInvalid
	}
	return rawProxyDialogShow(player, &doc[0], int32(len(doc)))
}

func ProxyDialogClear(player Player) int32 { return rawProxyDialogClear(player) }

// ProxyResourcePackPush offers a pack and returns its id text; hash, prompt and id may be empty. The int is the ABI
// code when the call failed.
func ProxyResourcePackPush(player Player, url, hash string, required bool, prompt, id string) (string, int32) {
	w := NewValueWriter()
	w.BeginMap().Key("url").String(url)
	if hash != "" {
		w.Key("hash").String(hash)
	}
	w.Key("required").Bool(required)
	if prompt != "" {
		w.Key("prompt").String(prompt)
	}
	if id != "" {
		w.Key("id").String(id)
	}
	w.End()
	doc, err := w.Finish()
	if err != nil {
		return "", piInvalid
	}
	failure := int32(0)
	pack, ok := fetch(func(out *byte, cap int32) int32 {
		n := rawProxyResourcePackPush(player, &doc[0], int32(len(doc)), out, cap)
		if n < 0 {
			failure = n
		}
		return n
	})
	if !ok {
		return "", failure
	}
	return pack, 0
}

func ProxyResourcePackRemove(player Player, id string) int32 {
	i, il := ptr(id)
	return rawProxyResourcePackRemove(player, i, il)
}

func ProxyResourcePackClear(player Player) int32 { return rawProxyResourcePackClear(player) }

func ProxyCookieSet(player Player, key string, data []byte) int32 {
	k, kl := ptr(key)
	var d *byte
	if len(data) > 0 {
		d = &data[0]
	}
	return rawProxyCookieSet(player, k, kl, d, int32(len(data)))
}

// ProxyCookieRequest returns the request handle; the handler reads the cookie with ProxyCookieResult.
func ProxyCookieRequest(player Player, key string, handler Handler) int32 {
	k, kl := ptr(key)
	return rawProxyCookieRequest(player, k, kl, tokenFor(handler))
}

// ProxyCookieResult is the cookie of a completion; false when the client had none.
func ProxyCookieResult(ctx Ctx) ([]byte, bool) {
	value, err := ProxyValueResult(ctx)
	if err != nil {
		return nil, false
	}
	return value.AsBytes()
}

func ProxyRequestCancel(request int32) int32 { return rawRequestCancel(request) }

func ProxyPlayerTransfer(player Player, host string, port int32) int32 {
	h, hl := ptr(host)
	return rawProxyPlayerTransfer(player, h, hl, port)
}

func ProxyPlayersByUUID(uuid string) Player {
	u, ul := ptr(uuid)
	return rawProxyPlayersByUuid(u, ul)
}

// ProxyPlayersCount counts the players on the proxy, or on server when it is not empty.
func ProxyPlayersCount(server string) int32 {
	s, sl := ptr(server)
	return rawProxyPlayersCount(s, sl)
}

func ProxyPlayersBroadcast(text string, args *Args) int32 {
	t, tl := ptr(text)
	a, al := args.packed()
	return rawProxyPlayersBroadcast(t, tl, a, al)
}

func ProxyPlayerAddress(player Player) (string, bool) {
	return fetch(func(out *byte, cap int32) int32 { return rawProxyPlayerAddress(player, out, cap) })
}

// ProxyPlayerPort is the client port, a negative code when unknown.
func ProxyPlayerPort(player Player) int32 { return rawProxyPlayerPort(player) }

// ProxyPlayerSettings is the client settings map; an error until the client sent it.
func ProxyPlayerSettings(player Player) (Value, error) {
	return CallValue(256, func(out *byte, cap int32) int32 { return rawProxyPlayerSettings(player, out, cap) })
}

func ProxyPlayerDataGetF64(player Player, key string, fallback float64) float64 {
	k, kl := ptr(key)
	return rawProxyPlayerDataGetF64(player, k, kl, fallback)
}

func ProxyPlayerDataSetF64(player Player, key string, value float64) int32 {
	k, kl := ptr(key)
	return rawProxyPlayerDataSetF64(player, k, kl, value)
}
