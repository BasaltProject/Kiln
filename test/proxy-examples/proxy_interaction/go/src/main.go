package main

import (
	bsCommands "basaltmc.org/sdk/proxy/commands"
	bsCore "basaltmc.org/sdk/proxy/core"
	bsEvents "basaltmc.org/sdk/proxy/events"
	bsInteraction "basaltmc.org/sdk/proxy/interaction"
	bsLog "basaltmc.org/sdk/proxy/log"
	bsPlayers "basaltmc.org/sdk/proxy/players"
	bsTasks "basaltmc.org/sdk/proxy/tasks"
	bsValues "basaltmc.org/sdk/proxy/values"
)

var seen bsCore.Player

func init() { bsCore.Init(setup) }

func main() {}

func say(text string) { bsLog.LogInfo(text) }

func ok(result int32) bool { return result >= 0 }

func choose(condition bool, yes, no string) string {
	if condition {
		return yes
	}
	return no
}

func textOf(root bsValues.Value, key string) string {
	if child, found := root.Find(key); found {
		if text, isText := child.AsString(); isText {
			return text
		}
	}
	return ""
}

func setup() {
	bsEvents.On("player.join", bsCore.Normal, onJoin)
	bsEvents.On("player.quit", bsCore.Normal, onQuit)
	bsInteraction.WatchResourcePacks(onPackStatus)
	bsInteraction.WatchSettings(onSettings)
	bsCommands.Command("pidialog", "", onDialogClick)
	bsCommands.Command("pitransfer", "", onTransfer)
	bsCommands.Command("picleanup", "", onCleanup)
	say("interaction ready")
}

func onPackStatus(ctx bsCore.Ctx) {
	event, err := bsInteraction.ValueResult(ctx)
	if err != nil {
		say("pack event unreadable")
		return
	}
	say("pack status:")
	say(textOf(event, "status"))
}

func onSettings(ctx bsCore.Ctx) {
	event, err := bsInteraction.ValueResult(ctx)
	if err != nil {
		say("settings event unreadable")
		return
	}
	settings, _ := event.Find("settings")
	say("settings locale:")
	say(textOf(settings, "locale"))
}

func onCookie(ctx bsCore.Ctx) {
	cookie, found := bsInteraction.CookieResult(ctx)
	if !found {
		say("cookie absent")
		return
	}
	say("cookie received:")
	say(string(cookie))
}

func onDialogClick(ctx bsCore.Ctx) {
	say("dialog clicked")
	bsCommands.CommandReply(ctx, "<green>dialog clicked", nil)
}

func onTransfer(ctx bsCore.Ctx) {
	player := bsEvents.EventPlayer(ctx)
	say(choose(ok(bsInteraction.PlayerTransfer(player, "127.0.0.1", 25970)), "transfer sent", "transfer failed"))
}

func onCleanup(ctx bsCore.Ctx) {
	player := bsEvents.EventPlayer(ctx)
	removed := ok(bsInteraction.ResourcePackRemove(player, "demo"))
	cleared := ok(bsInteraction.ResourcePackClear(player))
	closed := ok(bsInteraction.DialogClear(player))
	say(choose(removed && cleared && closed, "cleanup ok", "cleanup failed"))
}

func onStale(bsCore.Ctx) {
	say(choose(bsInteraction.DialogClear(seen) == -2, "stale after quit", "stale not detected"))
}

func onQuit(bsCore.Ctx) { bsTasks.After(20, onStale) }

func onJoin(ctx bsCore.Ctx) {
	player := bsEvents.EventPlayer(ctx)
	seen = player
	say(choose(bsInteraction.PlayersCount("") >= 1, "count ok", "count failed"))
	say(choose(bsInteraction.PlayersCount("lobby") >= 1, "count lobby ok", "count lobby failed"))
	say(choose(bsInteraction.PlayersCount("nowhere") == 0, "count nowhere ok", "count nowhere failed"))
	address, _ := bsInteraction.PlayerAddress(player)
	say(choose(address != "" && bsInteraction.PlayerPort(player) > 0, "address ok", "address failed"))
	uuid, _ := bsPlayers.PlayerUUID(player)
	say(choose(bsInteraction.PlayersByUUID(uuid) == player, "by uuid ok", "by uuid failed"))
	say(choose(bsInteraction.PlayersByUUID("00000000-0000-0000-0000-000000000000") == 0, "by uuid none ok", "by uuid none failed"))

	buttons := []bsInteraction.DialogButton{{Label: "Accept", Command: "pidialog"}, {Label: "Decline", Command: "pidialog"}}
	shown := bsInteraction.DialogShow(player, "multi_action", "<gold>Interaction", "<gray>Pick one", buttons, true)
	say(choose(ok(shown), "dialog ok", "dialog failed"))
	say(choose(bsInteraction.DialogShow(player, "bogus", "x", "", nil, true) == -3, "bad kind invalid", "bad kind accepted"))

	id, code := bsInteraction.ResourcePackPush(player, "https://example.invalid/pack.zip", "", false, "<gold>Test pack", "demo")
	if code == 0 {
		say("pack pushed:")
		say(id)
	} else {
		say("pack failed")
	}
	_, bad := bsInteraction.ResourcePackPush(player, "https://example.invalid/pack.zip", "nothex", false, "", "")
	say(choose(bad == -3, "bad hash invalid", "bad hash accepted"))

	say(choose(ok(bsInteraction.CookieSet(player, "basalt:demo", []byte("abcd"))), "cookie stored", "cookie store failed"))
	say(choose(bsInteraction.CookieRequest(player, "basalt:demo", onCookie) > 0, "cookie requested", "cookie request failed"))

	say(choose(ok(bsInteraction.PlayerDataSetF64(player, "ratio", 2.5)), "float stored", "float store failed"))
	say(choose(bsInteraction.PlayerDataGetF64(player, "ratio", 0.0) == 2.5, "float read ok", "float read failed"))
	say(choose(bsInteraction.PlayerDataGetF64(player, "missing", 7.5) == 7.5, "float default ok", "float default failed"))

	args := bsCore.NewArgs().Add("who", "everyone")
	say(choose(ok(bsInteraction.PlayersBroadcast("<green>Interaction hello {who}", args)), "broadcast ok", "broadcast failed"))
	say(choose(bsInteraction.PlayerTransfer(player, "", 25565) == -3, "bad host invalid", "bad host accepted"))
}
