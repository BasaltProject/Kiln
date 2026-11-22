package main

import (
	bsCommands "basaltmc.org/sdk/proxy/commands"
	bsConfig "basaltmc.org/sdk/proxy/config"
	bsCore "basaltmc.org/sdk/proxy/core"
	bsEvents "basaltmc.org/sdk/proxy/events"
	bsLog "basaltmc.org/sdk/proxy/log"
)

var blocked string

func init() { bsCore.Init(setup) }

func main() {}

func setup() {
	blocked, _ = bsConfig.ConfigStr("blocked-word")
	bsEvents.On("plugin.enable", bsCore.Normal, onEnable)
	bsEvents.On("player.chat", bsCore.Normal, onChat)
	bsEvents.Watch("player.chat", onChatSeen)
	bsEvents.On("player.command", bsCore.Normal, onCommand)
	bsEvents.On("player.kick", bsCore.Normal, onKick)
	bsEvents.Watch("player.kick", onKickSeen)
	bsCommands.Command("proxyevents <factor: float> <flag: bool>", "", onCommandRun)
}

func onEnable(bsCore.Ctx) {
	if id, ok := bsEvents.PluginInfo("id"); ok {
		bsLog.LogInfo("plugin id:")
		bsLog.LogInfo(id)
	}
	if version, ok := bsEvents.PluginInfo("version"); ok {
		bsLog.LogInfo("plugin version:")
		bsLog.LogInfo(version)
	}
	if _, status := bsEvents.PluginManifest(); status >= 0 {
		bsLog.LogInfo("manifest ok")
	}
	if _, status := bsEvents.ConfigValue("banned-words"); status >= 0 {
		bsLog.LogInfo("config array ok")
	}
	bsLog.LogInfo(blocked)
}

func onChat(ctx bsCore.Ctx) {
	message, ok := bsEvents.EventGetStr(ctx, "message")
	if !ok {
		return
	}
	if _, status := bsEvents.EventFields(ctx); status < 0 {
		bsLog.LogError("event fields failed")
	}
	if message == "ping" {
		bsEvents.EventSetText(ctx, "message", "pong")
		bsLog.LogInfo("chat rewritten")
	} else if message == blocked {
		bsEvents.EventDeny(ctx, "<red>Watch your language")
		bsLog.LogInfo("chat denied")
	}
}

func onChatSeen(ctx bsCore.Ctx) {
	if message, ok := bsEvents.EventGetStr(ctx, "message"); ok {
		bsLog.LogInfo("chat seen:")
		bsLog.LogInfo(message)
	}
}

func onCommand(ctx bsCore.Ctx) {
	if line, ok := bsEvents.EventGetStr(ctx, "command"); ok && line == "secret" {
		bsEvents.EventDeny(ctx, "<red>No secrets here")
		bsLog.LogInfo("command denied")
	}
}

func onKick(ctx bsCore.Ctx) {
	bsEvents.EventSetText(ctx, "reason", "<yellow>Kicked by the backend, come back soon")
	if bsEvents.EventSetText(ctx, "server", "nowhere") < 0 {
		bsLog.LogInfo("redirect refused")
	}
	if bsEvents.EventSetFlag(ctx, "cancelled", true) < 0 {
		bsLog.LogInfo("kick cannot be cancelled")
	}
}

func onKickSeen(ctx bsCore.Ctx) {
	if reason, ok := bsEvents.EventGetStr(ctx, "reason"); ok {
		bsLog.LogInfo("kick seen:")
		bsLog.LogInfo(reason)
	}
}

func onCommandRun(ctx bsCore.Ctx) {
	factor, okFactor := bsEvents.CommandArgF64(ctx, "factor")
	flag, okFlag := bsEvents.CommandArgBool(ctx, "flag")
	if !okFactor || !okFlag {
		bsCommands.CommandReply(ctx, "<red>bad arguments", nil)
		return
	}
	size, answer := "small", "no"
	if factor > 1.5 {
		size = "big"
	}
	if flag {
		answer = "yes"
	}
	bsCommands.CommandReply(ctx, "<green>factor is {size}, flag is {flag}", bsCore.NewArgs().Add("size", size).Add("flag", answer))
}
