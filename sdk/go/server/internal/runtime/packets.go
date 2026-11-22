package runtime

import "unsafe"

// Plugin channels and raw packets (abi/areas/packets.h). Calls return the import result: a count or handle, or a
// negative ABI code. The out-calls fill your buffer and return the length the full answer needs; open the slice with
// OpenValue for the value ones.

func pktBytes(body []byte) (*byte, int32) { return unsafe.SliceData(body), int32(len(body)) }

func pktFlag(value bool) int32 {
	if value {
		return 1
	}
	return 0
}

func ChannelListen(name string, handler Handler) int32 {
	n, nl := ptr(name)
	return rawChannelListen(n, nl, tokenFor(handler))
}

func ChannelSend(player Player, name string, body []byte) int32 {
	n, nl := ptr(name)
	b, bl := pktBytes(body)
	return rawChannelSend(player, n, nl, b, bl)
}

// PacketListen listens to the packets a name or wildcard covers. priority is "first", "early", "normal", "late",
// "last" or "" for the default.
func PacketListen(pattern, priority string, observe bool, handler Handler) int32 {
	var document []byte
	if priority != "" || observe {
		writer := NewValueWriter().BeginMap()
		if priority != "" {
			writer.Key("priority").String(priority)
		}
		if observe {
			writer.Key("observe").Bool(true)
		}
		built, err := writer.End().Finish()
		if err != nil {
			return -3
		}
		document = built
	}
	p, pl := ptr(pattern)
	d, dl := pktBytes(document)
	return rawPacketListen(p, pl, d, dl, tokenFor(handler))
}

func PacketUnlisten(listener int32) int32 { return rawPacketUnlisten(listener) }

func PacketSend(player Player, name string, body []byte, intercept bool) int32 {
	n, nl := ptr(name)
	b, bl := pktBytes(body)
	return rawPacketSend(player, n, nl, b, bl, pktFlag(intercept))
}

func PacketReceive(player Player, name string, body []byte, intercept bool) int32 {
	n, nl := ptr(name)
	b, bl := pktBytes(body)
	return rawPacketReceive(player, n, nl, b, bl, pktFlag(intercept))
}

func PacketLookup(name string, out []byte) int32 {
	n, nl := ptr(name)
	return rawPacketLookup(n, nl, unsafe.SliceData(out), int32(len(out)))
}

func PacketNames(prefix string, out []byte) int32 {
	p, pl := ptr(prefix)
	return rawPacketNames(p, pl, unsafe.SliceData(out), int32(len(out)))
}

func PacketPlayerActions(out []byte) int32 {
	return rawPacketPlayerActions(unsafe.SliceData(out), int32(len(out)))
}

// PacketPlayerReceive takes the arguments as an encoded list value.
func PacketPlayerReceive(player Player, action string, args []byte) int32 {
	a, al := ptr(action)
	g, gl := pktBytes(args)
	return rawPacketPlayerReceive(player, a, al, g, gl)
}

func PacketCtxInfo(ctx Ctx, out []byte) int32 {
	return rawPacketCtxInfo(ctx, unsafe.SliceData(out), int32(len(out)))
}

func PacketCtxBody(ctx Ctx, out []byte) int32 {
	return rawPacketCtxBody(ctx, unsafe.SliceData(out), int32(len(out)))
}

func PacketCtxPlayer(ctx Ctx) Player { return rawPacketCtxPlayer(ctx) }

func PacketCtxCancel(ctx Ctx) int32 { return rawPacketCtxCancel(ctx) }

func PacketCtxSetBody(ctx Ctx, body []byte) int32 {
	b, bl := pktBytes(body)
	return rawPacketCtxSetBody(ctx, b, bl)
}

func PacketCtxSend(ctx Ctx, name string, body []byte, intercept bool) int32 {
	n, nl := ptr(name)
	b, bl := pktBytes(body)
	return rawPacketCtxSend(ctx, n, nl, b, bl, pktFlag(intercept))
}

func PacketCtxReceive(ctx Ctx, name string, body []byte, intercept bool) int32 {
	n, nl := ptr(name)
	b, bl := pktBytes(body)
	return rawPacketCtxReceive(ctx, n, nl, b, bl, pktFlag(intercept))
}
