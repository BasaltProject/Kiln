package runtime

// Proxy hud area (abi/areas/proxy_hud.h): boss bars, sidebar, tab list, tab rows, server links and chat completions.
// An empty optional text is sent as present; pass nil pointers to leave a part of the tab list unchanged.

const hudInvalid int32 = -3

func hudSend(w *ValueWriter, call func(doc *byte, n int32) int32) int32 {
	doc, err := w.Finish()
	if err != nil || len(doc) == 0 {
		return hudInvalid
	}
	return call(&doc[0], int32(len(doc)))
}

func hudKey(w *ValueWriter, name string, text *string) {
	if text != nil {
		w.Key(name).String(*text)
	}
}

func ProxyHudClearTitle(player Player, reset bool) int32 {
	flag := int32(0)
	if reset {
		flag = 1
	}
	return rawProxyHudClearTitle(player, flag)
}

// ProxyHudTabList sets the header and footer; a nil part keeps what the player has.
func ProxyHudTabList(player Player, header, footer *string) int32 {
	w := NewValueWriter()
	w.BeginMap()
	hudKey(w, "header", header)
	hudKey(w, "footer", footer)
	w.End()
	return hudSend(w, func(doc *byte, n int32) int32 { return rawProxyHudTabList(player, doc, n) })
}

func ProxyHudBossbarShow(player Player, id, text string, progress float64, color, overlay string) int32 {
	w := NewValueWriter()
	w.BeginMap()
	hudKey(w, "text", &text)
	w.Key("progress").Float(progress)
	hudKey(w, "color", &color)
	hudKey(w, "overlay", &overlay)
	w.End()
	i, il := ptr(id)
	return hudSend(w, func(doc *byte, n int32) int32 { return rawProxyHudBossbarShow(player, i, il, doc, n) })
}

func ProxyHudBossbarHide(player Player, id string) int32 {
	i, il := ptr(id)
	return rawProxyHudBossbarHide(player, i, il)
}

func ProxyHudBossbarProgress(player Player, id string, progress float64) int32 {
	i, il := ptr(id)
	return rawProxyHudBossbarProgress(player, i, il, progress)
}

func ProxyHudBossbarTitle(player Player, id, text string) int32 {
	i, il := ptr(id)
	t, tl := ptr(text)
	return rawProxyHudBossbarTitle(player, i, il, t, tl, t, 0)
}

func ProxyHudBossbarStyle(player Player, id, color, overlay string) int32 {
	i, il := ptr(id)
	c, cl := ptr(color)
	o, ol := ptr(overlay)
	return rawProxyHudBossbarStyle(player, i, il, c, cl, o, ol)
}

func ProxyHudSidebar(player Player, title string, lines []string) int32 {
	w := NewValueWriter()
	w.BeginMap()
	hudKey(w, "title", &title)
	w.Key("lines").BeginList()
	for _, line := range lines {
		w.String(line)
	}
	w.End()
	w.End()
	return hudSend(w, func(doc *byte, n int32) int32 { return rawProxyHudSidebar(player, doc, n) })
}

func ProxyHudSidebarClear(player Player) int32 { return rawProxyHudSidebarClear(player) }

func ProxyHudTabEntry(player Player, id, name, text string, latency int64) int32 {
	w := NewValueWriter()
	w.BeginMap()
	hudKey(w, "name", &name)
	hudKey(w, "text", &text)
	w.Key("latency").Int(latency)
	w.End()
	i, il := ptr(id)
	return hudSend(w, func(doc *byte, n int32) int32 { return rawProxyHudTabEntry(player, i, il, doc, n) })
}

func ProxyHudTabEntryRemove(player Player, id string) int32 {
	i, il := ptr(id)
	return rawProxyHudTabEntryRemove(player, i, il)
}

// ProxyHudServerLinks takes label then url for each link.
func ProxyHudServerLinks(player Player, pairs []string) int32 {
	w := NewValueWriter()
	w.BeginList()
	for at := 0; at+1 < len(pairs); at += 2 {
		w.BeginList().String(pairs[at]).String(pairs[at+1]).End()
	}
	w.End()
	return hudSend(w, func(doc *byte, n int32) int32 { return rawProxyHudServerLinks(player, doc, n) })
}

func ProxyHudChatCompletions(player Player, action string, words []string) int32 {
	w := NewValueWriter()
	w.BeginList()
	for _, word := range words {
		w.String(word)
	}
	w.End()
	a, al := ptr(action)
	return hudSend(w, func(doc *byte, n int32) int32 { return rawProxyHudChatCompletions(player, a, al, doc, n) })
}
