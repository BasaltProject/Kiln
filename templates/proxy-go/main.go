package main

import (
	bsCommands "basaltmc.org/sdk/proxy/commands"
	bsConfig "basaltmc.org/sdk/proxy/config"
	bsCore "basaltmc.org/sdk/proxy/core"
	bsData "basaltmc.org/sdk/proxy/data"
	bsEvents "basaltmc.org/sdk/proxy/events"
	bsLog "basaltmc.org/sdk/proxy/log"
	bsPlayers "basaltmc.org/sdk/proxy/players"
	bsTasks "basaltmc.org/sdk/proxy/tasks"
)

import (
	"strconv"
)

func init() { bsCore.Init(setup) }

func main() {}

func setup() {
	bsLog.LogInfo("starting up")
	bsEvents.On("player.join", bsCore.Normal, onJoin)
	bsCommands.Command("@ID@ hello", "", hello)

	period := int32(bsConfig.ConfigI64("announce-period", 1200))
	bsTasks.Every(period, period, func(bsCore.Ctx) {
		for _, player := range bsPlayers.PlayersOnline() {
			bsPlayers.Send(player, "@announce", nil)
		}
	})
}

func onJoin(ctx bsCore.Ctx) {
	player := bsEvents.EventPlayer(ctx)
	if player == 0 {
		return
	}
	visits := bsData.DataGetI64(player, "visits", 0) + 1
	bsData.DataSetI64(player, "visits", visits)
	name, _ := bsPlayers.PlayerName(player)
	bsPlayers.Send(player, "@welcome", bsCore.NewArgs().Add("name", name).Add("visits", strconv.FormatInt(visits, 10)))
	bsPlayers.PlayerTitle(player, "<gold>Welcome", "<gray>Have fun", -1, -1, -1)
}

func hello(ctx bsCore.Ctx) {
	source, _ := bsCommands.CommandSourceName(ctx)
	server := "console"
	if player := bsEvents.EventPlayer(ctx); player != 0 {
		server, _ = bsPlayers.PlayerServer(player)
	}
	bsCommands.CommandReply(ctx, "@hello", bsCore.NewArgs().Add("name", source).Add("server", server))
}
