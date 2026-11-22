package org.basaltmc.server.example

import org.basaltmc.server.core.Args
import org.basaltmc.server.core.Ctx

var shop = 0

fun buySparkles(ctx: Ctx) {
    val player = org.basaltmc.server.menus.menuClickPlayer(ctx)
    val price = org.basaltmc.server.config.configI64("shop.sparkles-price", 100)
    val balance = org.basaltmc.server.data.dataGetI64(player, "coins", 0)
    if (balance < price) {
        org.basaltmc.server.players.send(player, "@shop-cant-afford", Args().add("price", price.toString()).add("amount", balance.toString()))
        return
    }
    org.basaltmc.server.data.dataSetI64(player, "coins", balance - price)
    org.basaltmc.server.players.particle(player, "minecraft:happy_villager", 20, 0.5)
    org.basaltmc.server.players.send(player, "@shop-bought", Args().add("item", "Sparkles").add("price", price.toString()))
}

fun onJoin(ctx: Ctx) {
    val player = org.basaltmc.server.events.eventPlayer(ctx)
    if (player == 0) return
    val balance = org.basaltmc.server.data.dataGetI64(player, "coins", 0) + org.basaltmc.server.config.configI64("welcome-bonus", 50)
    org.basaltmc.server.data.dataSetI64(player, "coins", balance)
    org.basaltmc.server.players.send(player, "@welcome", Args().add("name", org.basaltmc.server.players.playerName(player) ?: "").add("amount", balance.toString()))
}

fun hello(ctx: Ctx) {
    val times = org.basaltmc.server.commands.commandArgI64(ctx, "times") ?: 1L
    val args = Args().add("name", org.basaltmc.server.commands.commandSourceName(ctx) ?: "")
    for (i in 0 until times) org.basaltmc.server.commands.commandReply(ctx, "@hello", args)
}

fun openShop(ctx: Ctx) {
    if (org.basaltmc.server.commands.commandIsConsole(ctx)) {
        org.basaltmc.server.commands.commandReply(ctx, "<red>The shop needs a player.")
        return
    }
    org.basaltmc.server.menus.menuOpen(shop, org.basaltmc.server.players.playerByName(org.basaltmc.server.commands.commandSourceName(ctx) ?: ""))
}

fun init() {
    org.basaltmc.server.log.logInfo("starting up")
    org.basaltmc.server.events.on("player.join", org.basaltmc.server.core.Priority.NORMAL, ::onJoin)
    org.basaltmc.server.commands.command("@ID@ hello [times: int 1..5]", "", ::hello)
    org.basaltmc.server.commands.command("@ID@ shop", "", ::openShop)

    shop = org.basaltmc.server.menus.menuCreate(org.basaltmc.server.text.translate("shop-title") ?: "", 3)
    val lore = org.basaltmc.server.text.translate("shop-item-sparkles-lore", Args().add("price", "100")) ?: ""
    org.basaltmc.server.menus.menuSet(shop, 13, "minecraft:glowstone_dust", org.basaltmc.server.text.translate("shop-item-sparkles") ?: "", lore, ::buySparkles)

    val period = org.basaltmc.server.config.configI64("announce-period", 1200).toInt()
    org.basaltmc.server.tasks.every(period, period) {
        for (player in org.basaltmc.server.players.playersOnline()) org.basaltmc.server.players.send(player, "@announce")
    }
}
