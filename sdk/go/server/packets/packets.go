package packets

import "basaltmc.org/sdk/server/internal/runtime"

func ChannelListen(name string, handler runtime.Handler) int32 {
	return runtime.ChannelListen(name, handler)
}

func ChannelSend(player runtime.Player, name string, body []byte) int32 {
	return runtime.ChannelSend(player, name, body)
}

func PacketListen(pattern, priority string, observe bool, handler runtime.Handler) int32 {
	return runtime.PacketListen(pattern, priority, observe, handler)
}

func PacketUnlisten(listener int32) int32 { return runtime.PacketUnlisten(listener) }

func PacketSend(player runtime.Player, name string, body []byte, intercept bool) int32 {
	return runtime.PacketSend(player, name, body, intercept)
}

func PacketReceive(player runtime.Player, name string, body []byte, intercept bool) int32 {
	return runtime.PacketReceive(player, name, body, intercept)
}

func PacketLookup(name string, out []byte) int32 { return runtime.PacketLookup(name, out) }

func PacketNames(prefix string, out []byte) int32 { return runtime.PacketNames(prefix, out) }

func PacketPlayerActions(out []byte) int32 { return runtime.PacketPlayerActions(out) }

func PacketPlayerReceive(player runtime.Player, action string, args []byte) int32 {
	return runtime.PacketPlayerReceive(player, action, args)
}

func PacketCtxInfo(ctx runtime.Ctx, out []byte) int32 { return runtime.PacketCtxInfo(ctx, out) }

func PacketCtxBody(ctx runtime.Ctx, out []byte) int32 { return runtime.PacketCtxBody(ctx, out) }

func PacketCtxPlayer(ctx runtime.Ctx) runtime.Player { return runtime.PacketCtxPlayer(ctx) }

func PacketCtxCancel(ctx runtime.Ctx) int32 { return runtime.PacketCtxCancel(ctx) }

func PacketCtxSetBody(ctx runtime.Ctx, body []byte) int32 { return runtime.PacketCtxSetBody(ctx, body) }

func PacketCtxSend(ctx runtime.Ctx, name string, body []byte, intercept bool) int32 {
	return runtime.PacketCtxSend(ctx, name, body, intercept)
}

func PacketCtxReceive(ctx runtime.Ctx, name string, body []byte, intercept bool) int32 {
	return runtime.PacketCtxReceive(ctx, name, body, intercept)
}
