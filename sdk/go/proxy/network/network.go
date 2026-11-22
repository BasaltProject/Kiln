package network

import "basaltmc.org/sdk/proxy/internal/runtime"

func ServersList() []string { return runtime.ProxyServersList() }

func ServerAddress(name string) (string, bool) { return runtime.ProxyServerAddress(name) }

func PlayersOn(server string) []runtime.Player { return runtime.ProxyPlayersOn(server) }

func NetChannelRegister(name string) int32 { return runtime.NetChannelRegister(name) }

func NetChannelUnregister(name string) int32 { return runtime.NetChannelUnregister(name) }

func NetMessageOn(filter string, handler runtime.Handler) int32 {
	return runtime.NetMessageOn(filter, handler)
}

func NetMessageGet(ctx runtime.Ctx) ([]byte, int32) { return runtime.NetMessageGet(ctx) }

func NetMessageSend(player runtime.Player, channel string, data []byte, towardClient bool) int32 {
	return runtime.NetMessageSend(player, channel, data, towardClient)
}

func NetPacketResolve(name string) ([]byte, int32) { return runtime.NetPacketResolve(name) }

func NetPacketNames() ([]byte, int32) { return runtime.NetPacketNames() }

func NetPacketOn(name string, opts []byte, handler runtime.Handler) int32 {
	return runtime.NetPacketOn(name, opts, handler)
}

func NetPacketWatch(name string, opts []byte, handler runtime.Handler) int32 {
	return runtime.NetPacketWatch(name, opts, handler)
}

func NetPacketRemove(listener int32) int32 { return runtime.NetPacketRemove(listener) }

func NetPacketGet(ctx runtime.Ctx) ([]byte, int32) { return runtime.NetPacketGet(ctx) }

func NetPacketBody(ctx runtime.Ctx) ([]byte, int32) { return runtime.NetPacketBody(ctx) }

func NetPacketSetBody(ctx runtime.Ctx, body []byte) int32 { return runtime.NetPacketSetBody(ctx, body) }

func NetPacketCancel(ctx runtime.Ctx) int32 { return runtime.NetPacketCancel(ctx) }

func NetPacketInject(ctx runtime.Ctx, name string, body []byte, intercept bool) int32 {
	return runtime.NetPacketInject(ctx, name, body, intercept)
}

func NetPacketSend(player runtime.Player, name string, body []byte) int32 {
	return runtime.NetPacketSend(player, name, body)
}

func NetPacketReceive(player runtime.Player, name string, body []byte) int32 {
	return runtime.NetPacketReceive(player, name, body)
}

func NetAntibotVerify(name string) int32 { return runtime.NetAntibotVerify(name) }

func NetAntibotUnverify(name string) int32 { return runtime.NetAntibotUnverify(name) }

func NetAntibotIsVerified(name string) int32 { return runtime.NetAntibotIsVerified(name) }

func NetAntibotBan(address string) int32 { return runtime.NetAntibotBan(address) }

func NetAntibotUnban(address string) int32 { return runtime.NetAntibotUnban(address) }

func NetAntibotUnderAttack() int32 { return runtime.NetAntibotUnderAttack() }

func NetAntibotOn(handler runtime.Handler) int32 { return runtime.NetAntibotOn(handler) }

func NetAntibotEventGet(ctx runtime.Ctx) ([]byte, int32) { return runtime.NetAntibotEventGet(ctx) }
