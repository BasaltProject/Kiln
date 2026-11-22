package runtime

// Proxy network area (abi/areas/proxy_network.h): messaging channels, raw packets and antibot control. The calls that
// answer a value return its raw encoded bytes for the values reader and the ABI status (negative on failure).

func netFetch(call func(out *byte, cap int32) int32) ([]byte, int32) {
	buffer := make([]byte, 256)
	for {
		needed := call(&buffer[0], int32(len(buffer)))
		if needed < 0 {
			return nil, needed
		}
		if int(needed) <= len(buffer) {
			return buffer[:needed], 0
		}
		buffer = make([]byte, needed)
	}
}

func netBytes(b []byte) (*byte, int32) {
	if len(b) == 0 {
		return nil, 0
	}
	return &b[0], int32(len(b))
}

func NetChannelRegister(name string) int32 {
	n, nl := ptr(name)
	return rawProxyNetworkChannelRegister(n, nl)
}

func NetChannelUnregister(name string) int32 {
	n, nl := ptr(name)
	return rawProxyNetworkChannelUnregister(n, nl)
}

// NetMessageOn observes the messages of the plugin's channels; an empty filter takes all of them.
func NetMessageOn(filter string, handler Handler) int32 {
	f, fl := ptr(filter)
	return rawProxyNetworkMessageOn(f, fl, tokenFor(handler))
}

// NetMessageGet returns the encoded map of the message (player_name, player_uuid, channel, data, source, server).
func NetMessageGet(ctx Ctx) ([]byte, int32) {
	return netFetch(func(out *byte, cap int32) int32 { return rawProxyNetworkMessageGet(ctx, out, cap) })
}

// NetMessageSend sends `data` on a channel; towardClient false goes to the player's backend, true to the client.
func NetMessageSend(player Player, channel string, data []byte, towardClient bool) int32 {
	c, cl := ptr(channel)
	d, dl := netBytes(data)
	var toward int32
	if towardClient {
		toward = 1
	}
	return rawProxyNetworkMessageSend(player, c, cl, d, dl, toward)
}

func NetPacketResolve(name string) ([]byte, int32) {
	n, nl := ptr(name)
	return netFetch(func(out *byte, cap int32) int32 { return rawProxyPacketResolve(n, nl, out, cap) })
}

func NetPacketNames() ([]byte, int32) {
	return netFetch(func(out *byte, cap int32) int32 { return rawProxyPacketNames(out, cap) })
}

// NetPacketOn registers a decider listener; opts is an encoded value map (priority, fail) or empty. Returns the listener.
func NetPacketOn(name string, opts []byte, handler Handler) int32 {
	n, nl := ptr(name)
	o, ol := netBytes(opts)
	return rawProxyPacketOn(n, nl, o, ol, tokenFor(handler))
}

// NetPacketWatch registers an observer listener: it sees the final packet and cannot cancel or change it.
func NetPacketWatch(name string, opts []byte, handler Handler) int32 {
	n, nl := ptr(name)
	o, ol := netBytes(opts)
	return rawProxyPacketWatch(n, nl, o, ol, tokenFor(handler))
}

func NetPacketRemove(listener int32) int32 { return rawProxyPacketRemove(listener) }

func NetPacketGet(ctx Ctx) ([]byte, int32) {
	return netFetch(func(out *byte, cap int32) int32 { return rawProxyPacketGet(ctx, out, cap) })
}

func NetPacketBody(ctx Ctx) ([]byte, int32) {
	return netFetch(func(out *byte, cap int32) int32 { return rawProxyPacketBody(ctx, out, cap) })
}

func NetPacketSetBody(ctx Ctx, body []byte) int32 {
	b, bl := netBytes(body)
	return rawProxyPacketSetBody(ctx, b, bl)
}

func NetPacketCancel(ctx Ctx) int32 { return rawProxyPacketCancel(ctx) }

func NetPacketInject(ctx Ctx, name string, body []byte, intercept bool) int32 {
	n, nl := ptr(name)
	b, bl := netBytes(body)
	var flag int32
	if intercept {
		flag = 1
	}
	return rawProxyPacketInject(ctx, n, nl, b, bl, flag)
}

func NetPacketSend(player Player, name string, body []byte) int32 {
	n, nl := ptr(name)
	b, bl := netBytes(body)
	return rawProxyPacketSend(player, n, nl, b, bl)
}

func NetPacketReceive(player Player, name string, body []byte) int32 {
	n, nl := ptr(name)
	b, bl := netBytes(body)
	return rawProxyPacketReceive(player, n, nl, b, bl)
}

func NetAntibotVerify(name string) int32 {
	n, nl := ptr(name)
	return rawProxyAntibotVerify(n, nl)
}

func NetAntibotUnverify(name string) int32 {
	n, nl := ptr(name)
	return rawProxyAntibotUnverify(n, nl)
}

func NetAntibotIsVerified(name string) int32 {
	n, nl := ptr(name)
	return rawProxyAntibotIsVerified(n, nl)
}

func NetAntibotBan(address string) int32 {
	a, al := ptr(address)
	return rawProxyAntibotBan(a, al)
}

func NetAntibotUnban(address string) int32 {
	a, al := ptr(address)
	return rawProxyAntibotUnban(a, al)
}

func NetAntibotUnderAttack() int32 { return rawProxyAntibotUnderAttack() }

func NetAntibotOn(handler Handler) int32 { return rawProxyAntibotOn(tokenFor(handler)) }

func NetAntibotEventGet(ctx Ctx) ([]byte, int32) {
	return netFetch(func(out *byte, cap int32) int32 { return rawProxyAntibotEventGet(ctx, out, cap) })
}
