package main

import (
	bsCommands "basaltmc.org/sdk/server/commands"
	bsConfig "basaltmc.org/sdk/server/config"
	bsCore "basaltmc.org/sdk/server/core"
	bsData "basaltmc.org/sdk/server/data"
	bsEvents "basaltmc.org/sdk/server/events"
	bsLog "basaltmc.org/sdk/server/log"
	bsMenus "basaltmc.org/sdk/server/menus"
	bsPlayers "basaltmc.org/sdk/server/players"
	bsTasks "basaltmc.org/sdk/server/tasks"
	bsText "basaltmc.org/sdk/server/text"
)

import (
	"strconv"
)

var shop int32

func init() { bsCore.Init(setup) }

func main() {}

func setup() {
	bsLog.LogInfo("starting up")
	bsEvents.On("player.join", bsCore.Normal, onJoin)
	bsCommands.Command("@ID@ hello [times: int 1..5]", "", hello)
	bsCommands.Command("@ID@ shop", "", openShop)

	title, _ := bsText.Translate("shop-title", nil)
	shop = bsMenus.MenuCreate(title, 3)
	name, _ := bsText.Translate("shop-item-sparkles", nil)
	lore, _ := bsText.Translate("shop-item-sparkles-lore", bsCore.NewArgs().Add("price", "100"))
	bsMenus.MenuSet(shop, 13, "minecraft:glowstone_dust", name, lore, buySparkles)

	period := int32(bsConfig.ConfigI64("announce-period", 1200))
	bsTasks.Every(period, period, func(bsCore.Ctx) {
		for _, player := range bsPlayers.PlayersOnline() {
			bsPlayers.Send(player, "@announce", nil)
		}
	})
}

func buySparkles(ctx bsCore.Ctx) {
	player := bsMenus.MenuClickPlayer(ctx)
	price := bsConfig.ConfigI64("shop.sparkles-price", 100)
	balance := bsData.DataGetI64(player, "coins", 0)
	if balance < price {
		args := bsCore.NewArgs().Add("price", strconv.FormatInt(price, 10)).Add("amount", strconv.FormatInt(balance, 10))
		bsPlayers.Send(player, "@shop-cant-afford", args)
		return
	}
	bsData.DataSetI64(player, "coins", balance-price)
	bsPlayers.Particle(player, "minecraft:happy_villager", 20, 0.5)
	bsPlayers.Send(player, "@shop-bought", bsCore.NewArgs().Add("item", "Sparkles").Add("price", strconv.FormatInt(price, 10)))
}

func onJoin(ctx bsCore.Ctx) {
	player := bsEvents.EventPlayer(ctx)
	if player == 0 {
		return
	}
	balance := bsData.DataGetI64(player, "coins", 0) + bsConfig.ConfigI64("welcome-bonus", 50)
	bsData.DataSetI64(player, "coins", balance)
	name, _ := bsPlayers.PlayerName(player)
	bsPlayers.Send(player, "@welcome", bsCore.NewArgs().Add("name", name).Add("amount", strconv.FormatInt(balance, 10)))
}

func hello(ctx bsCore.Ctx) {
	times, ok := bsCommands.CommandArgI64(ctx, "times")
	if !ok {
		times = 1
	}
	source, _ := bsCommands.CommandSourceName(ctx)
	args := bsCore.NewArgs().Add("name", source)
	for i := int64(0); i < times; i++ {
		bsCommands.CommandReply(ctx, "@hello", args)
	}
}

func openShop(ctx bsCore.Ctx) {
	if bsCommands.CommandIsConsole(ctx) {
		bsCommands.CommandReply(ctx, "<red>The shop needs a player.", nil)
		return
	}
	source, _ := bsCommands.CommandSourceName(ctx)
	bsMenus.MenuOpen(shop, bsPlayers.PlayerByName(source))
}
