package main

import (
	bsAdmin "basaltmc.org/sdk/proxy/admin"
	bsCommands "basaltmc.org/sdk/proxy/commands"
	bsCore "basaltmc.org/sdk/proxy/core"
	bsLog "basaltmc.org/sdk/proxy/log"
	bsPlayers "basaltmc.org/sdk/proxy/players"
)

import (
	"fmt"
)

func init() { bsCore.Init(setup) }

func main() {}

func sayInt(label string, number int64) { bsLog.LogInfo(fmt.Sprintf("%s%d", label, number)) }

func sayText(label, value string) { bsLog.LogInfo(label + value) }

func setup() {
	bsAdmin.PermissionProvide(onCheck, onMeta, 0)
	bsAdmin.PermissionChanged()
	status, _ := bsAdmin.ServersRegister("extra", "127.0.0.1", 25599, "lobby")
	sayInt("register extra: ", int64(status))
	status, _ = bsAdmin.ServersRegister("extra", "127.0.0.1", 25599, "lobby")
	sayInt("register again: ", int64(status))
	status, _ = bsAdmin.ServersRegister("bad", "127.0.0.1", 0, "")
	sayInt("register bad port: ", int64(status))
	sayInt("unregister extra: ", int64(bsAdmin.ServersUnregister("extra")))
	sayInt("unregister lobby: ", int64(bsAdmin.ServersUnregister("lobby")))
	sayInt("unregister none: ", int64(bsAdmin.ServersUnregister("never")))
	bsAdmin.DataSetInt("", "visits", 5)
	bsAdmin.DataSetStr("", "label", "hello")
	bsAdmin.DataSetInt("OfflineHolder", "coins", 42)
	bsAdmin.DataSetFloat("OfflineHolder", "ratio", 2.5)
	if value, err := bsAdmin.DataValue("OfflineHolder", "coins"); err == nil {
		coins, _ := value.AsInt()
		sayInt("offline coins: ", coins)
	}
	if value, err := bsAdmin.DataValue("OfflineHolder", "ratio"); err == nil {
		ratio, _ := value.AsFloat()
		sayInt("offline ratio x10: ", int64(ratio*10))
	}
	bsAdmin.DataRemove("OfflineHolder", "ratio")
	removed := int64(0)
	if _, err := bsAdmin.DataValue("OfflineHolder", "ratio"); err != nil {
		removed = -1
	}
	sayInt("removed ratio: ", removed)
	bsCommands.Command("pcon", "", onConsole)
	sayInt("console run: ", int64(bsAdmin.ConsoleRun("pcon")))
	bsAdmin.ServersPing("lobby", onPing)
	bsAdmin.Wait(2, onWait)
	bsCommands.Command("padmin", "proxyadmin.granted", onCommand)
	bsCommands.Command("pkick", "proxyadmin.granted", onKickCommand)
	bsCommands.Command("pdenied", "proxyadmin.denied", onCommand)
}

func onCheck(ctx bsCore.Ctx) {
	request, err := bsAdmin.PermissionRequest(ctx)
	if err != nil {
		return
	}
	text, _ := request.Find("text")
	node, _ := text.AsString()
	if node == "proxyadmin.granted" {
		bsAdmin.PermissionAnswer(ctx, 1)
	} else if node == "proxyadmin.denied" {
		bsAdmin.PermissionAnswer(ctx, 0)
	}
}

func onMeta(ctx bsCore.Ctx) { bsAdmin.PermissionAnswerText(ctx, "[Admin] ") }

func onPing(ctx bsCore.Ctx) {
	root, err := bsAdmin.CompletionValue(ctx)
	if err != nil {
		bsLog.LogInfo("ping failed")
		return
	}
	if root.IsNull() {
		bsLog.LogInfo("ping offline")
		return
	}
	online := int64(-1)
	if field, ok := root.Find("online"); ok {
		online, _ = field.AsInt()
	}
	bsLog.LogInfo("ping answered")
	sayInt("ping online: ", online)
}

func onWait(bsCore.Ctx) {
	bsLog.LogInfo("waited")
	if value, err := bsAdmin.DataValue("", "visits"); err == nil {
		visits, _ := value.AsInt()
		sayInt("store visits: ", visits)
	}
	if top, err := bsAdmin.StoreTop("visits", 3); err == nil {
		sayInt("store top entries: ", int64(top.Count()))
	}
}

func onCommand(ctx bsCore.Ctx) {
	name, ok := bsCommands.CommandSourceName(ctx)
	if !ok {
		return
	}
	player := bsPlayers.PlayerByName(name)
	sayText("command by: ", name)
	bsAdmin.GroupSet(name, "vip")
	if group, ok := bsAdmin.GroupGet(name); ok {
		sayText("group is: ", group)
	}
	bsAdmin.PermissionOverride(name, "proxyadmin.extra", 1)
	sayInt("override check: ", int64(bsAdmin.PermissionCheck(name, "proxyadmin.extra", "")))
	sayInt("granted check: ", int64(bsAdmin.PermissionCheck(name, "proxyadmin.granted", "")))
	sayInt("denied check: ", int64(bsAdmin.PermissionCheck(name, "proxyadmin.denied", "")))
	bsAdmin.PermissionOverride(name, "proxyadmin.extra", -1)
	sayInt("override removed: ", int64(bsAdmin.PermissionCheck(name, "proxyadmin.extra", "")))
	bsAdmin.PlayerChat(player, "plain chat from the plugin")
	bsAdmin.PlayerSendJson(player, `{"text":"json message","color":"green"}`)
	bsAdmin.DataSetInt(name, "coins", 7)
	bsLog.LogInfo("command done")
}

func onConsole(bsCore.Ctx) { bsLog.LogInfo("console command ran") }

func onKickCommand(ctx bsCore.Ctx) {
	if name, ok := bsCommands.CommandSourceName(ctx); ok {
		bsAdmin.PlayerKickJson(bsPlayers.PlayerByName(name), `{"text":"kicked by json","color":"red"}`)
	}
}
