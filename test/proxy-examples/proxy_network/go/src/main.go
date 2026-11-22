package main

import (
	bsConfig "basaltmc.org/sdk/proxy/config"
	bsCore "basaltmc.org/sdk/proxy/core"
	bsEvents "basaltmc.org/sdk/proxy/events"
	bsLog "basaltmc.org/sdk/proxy/log"
	bsNetwork "basaltmc.org/sdk/proxy/network"
	bsValues "basaltmc.org/sdk/proxy/values"
)

import (
	"bytes"
	"strings"
)

var rewriteMarker, cancelMarker, channel string
var refusedOnce bool

func init() { bsCore.Init(setup) }

func main() {}

func setup() {
	rewriteMarker, _ = bsConfig.ConfigStr("rewrite-marker")
	cancelMarker, _ = bsConfig.ConfigStr("cancel-marker")
	channel, _ = bsConfig.ConfigStr("channel")
	bsEvents.On("plugin.enable", bsCore.Normal, onEnable)
	bsEvents.On("player.join", bsCore.Normal, onJoin)
}

func open(doc []byte, status int32) (bsValues.Value, bool) {
	if status < 0 {
		return bsValues.Value{}, false
	}
	return bsValues.OpenValue(doc)
}

func text(m bsValues.Value, key string) string {
	v, _ := m.Find(key)
	s, _ := v.AsString()
	return s
}

func chatBody(s string) []byte {
	out := []byte{0x08, byte(len(s) >> 8), byte(len(s))}
	out = append(out, s...)
	return append(out, 0)
}

func stringPrefixed(first, rest string) []byte {
	return append(append([]byte{byte(len(first))}, first...), rest...)
}

func onMessage(ctx bsCore.Ctx) {
	doc, status := bsNetwork.NetMessageGet(ctx)
	m, ok := open(doc, status)
	if !ok {
		bsLog.LogError("message read failed")
		return
	}
	v, _ := m.Find("data")
	data, _ := v.AsBytes()
	bsLog.LogInfo("net: message " + text(m, "source") + " " + text(m, "channel") + " " + string(data))
	player := bsEvents.EventPlayer(ctx)
	if player > 0 && bsNetwork.NetMessageSend(player, "basalt:demo", []byte("pong"), true) == 0 {
		bsLog.LogInfo("net: reply sent")
	}
}

func onDecider(ctx bsCore.Ctx) {
	info, s1 := bsNetwork.NetPacketGet(ctx)
	body, s2 := bsNetwork.NetPacketBody(ctx)
	m, ok := open(info, s1)
	if !ok || s2 < 0 {
		bsLog.LogError("packet read failed")
		return
	}
	if bytes.Contains(body, []byte(cancelMarker)) {
		if bsNetwork.NetPacketCancel(ctx) == 0 {
			bsLog.LogInfo("net: cancel for " + text(m, "player_name"))
		}
	} else if at := bytes.Index(body, []byte(rewriteMarker)); at >= 0 {
		copy(body[at:], strings.ToUpper(rewriteMarker))
		if bsNetwork.NetPacketSetBody(ctx, body) == 0 {
			bsLog.LogInfo("net: rewrite for " + text(m, "player_name"))
		}
	}
}

func onWatch(ctx bsCore.Ctx) {
	info, status := bsNetwork.NetPacketGet(ctx)
	m, ok := open(info, status)
	if !ok {
		return
	}
	if !refusedOnce && bsNetwork.NetPacketCancel(ctx) < 0 {
		refusedOnce = true
		bsLog.LogInfo("net: observer cancel refused")
	}
	if v, _ := m.Find("cancelled"); true {
		if b, _ := v.AsBool(); b {
			bsLog.LogInfo("net: watch saw a cancelled packet")
		}
	}
}

func onAntibot(ctx bsCore.Ctx) {
	doc, status := bsNetwork.NetAntibotEventGet(ctx)
	if m, ok := open(doc, status); ok {
		bsLog.LogInfo("net: antibot " + text(m, "kind") + " " + text(m, "action"))
	}
}

func idle(bsCore.Ctx) {}

func onJoin(ctx bsCore.Ctx) {
	player := bsEvents.EventPlayer(ctx)
	if player <= 0 {
		return
	}
	body := chatBody("net welcome")
	if bsNetwork.NetPacketSend(player, "clientbound.system_chat", body) == 0 {
		bsLog.LogInfo("net: sent welcome")
	}
	if bsNetwork.NetPacketSend(player, "serverbound.play.chat_command", body) < 0 {
		bsLog.LogInfo("net: wrong direction refused")
	}
	if bsNetwork.NetMessageSend(player, channel, []byte("hello"), true) == 0 {
		bsLog.LogInfo("net: sent hello message")
	}
	if bsNetwork.NetPacketReceive(player, "serverbound.play.custom_payload", stringPrefixed(channel, "ping")) == 0 {
		bsLog.LogInfo("net: injected custom payload")
	}
	if bsNetwork.NetPacketReceive(player, "serverbound.play.chat_command", stringPrefixed("list", "")) == 0 {
		bsLog.LogInfo("net: injected command")
	}
}

func onEnable(bsCore.Ctx) {
	if doc, status := bsNetwork.NetPacketNames(); status >= 0 {
		if v, ok := bsValues.OpenValue(doc); ok && v.Count() > 100 {
			bsLog.LogInfo("net: packets known")
		}
	}
	if doc, status := bsNetwork.NetPacketResolve("clientbound.system_chat"); status >= 0 {
		if m, ok := bsValues.OpenValue(doc); ok {
			v, _ := m.Find("serverbound")
			if b, got := v.AsBool(); got && !b {
				bsLog.LogInfo("net: resolved a clientbound play packet")
			}
		}
	}
	if _, status := bsNetwork.NetPacketResolve("nope.nothing"); status < 0 {
		bsLog.LogInfo("net: unknown packet refused")
	}
	if bsNetwork.NetChannelRegister(channel) == 0 {
		bsLog.LogInfo("net: channel registered")
	}
	if bsNetwork.NetChannelRegister("BungeeCord") < 0 {
		bsLog.LogInfo("net: bungee channel refused")
	}
	if bsNetwork.NetChannelRegister("basalt:temp") == 0 && bsNetwork.NetChannelUnregister("basalt:temp") == 0 &&
		bsNetwork.NetChannelUnregister("basalt:temp") < 0 {
		bsLog.LogInfo("net: temp channel unregistered")
	}
	bsNetwork.NetMessageOn("basalt:demo", onMessage)
	opts, _ := bsValues.NewValueWriter().BeginMap().Key("priority").String("late").Key("fail").String("closed").End().Finish()
	if bsNetwork.NetPacketOn("clientbound.system_chat", opts, onDecider) > 0 {
		bsLog.LogInfo("net: decider listening")
	}
	bsNetwork.NetPacketWatch("clientbound.system_chat", nil, onWatch)
	listener := bsNetwork.NetPacketOn("serverbound.play.*", nil, idle)
	if listener > 0 && bsNetwork.NetPacketRemove(listener) == 0 && bsNetwork.NetPacketRemove(listener) < 0 {
		bsLog.LogInfo("net: listener removed")
	}
	if bsNetwork.NetPacketOn("nope.nothing", nil, idle) < 0 {
		bsLog.LogInfo("net: bad listener name refused")
	}
	if _, status := bsNetwork.NetPacketGet(9999); status < 0 {
		bsLog.LogInfo("net: bad context refused")
	}
	bsNetwork.NetAntibotOn(onAntibot)
	bsNetwork.NetAntibotVerify("NetBot")
	if bsNetwork.NetAntibotIsVerified("NetBot") == 1 {
		bsLog.LogInfo("net: verified yes")
	} else {
		bsLog.LogInfo("net: verified no")
	}
	bsNetwork.NetAntibotUnverify("NetBot")
	if bsNetwork.NetAntibotIsVerified("NetBot") == 0 {
		bsLog.LogInfo("net: unverified")
	} else {
		bsLog.LogInfo("net: still verified")
	}
	if bsNetwork.NetAntibotBan("203.0.113.9") == 0 {
		bsLog.LogInfo("net: ban ok")
	}
	if bsNetwork.NetAntibotUnban("203.0.113.9") == 0 {
		bsLog.LogInfo("net: unban ok")
	}
	if bsNetwork.NetAntibotUnban("203.0.113.9") < 0 {
		bsLog.LogInfo("net: unban absent")
	}
	if bsNetwork.NetAntibotBan("not an address") < 0 {
		bsLog.LogInfo("net: bad address refused")
	}
	if bsNetwork.NetAntibotUnderAttack() == 0 {
		bsLog.LogInfo("net: no attack")
	} else {
		bsLog.LogInfo("net: under attack")
	}
}
