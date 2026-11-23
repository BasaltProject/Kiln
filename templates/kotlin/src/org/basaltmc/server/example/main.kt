package org.basaltmc.server.example

import org.basaltmc.server.commands.command
import org.basaltmc.server.commands.commandArgI64
import org.basaltmc.server.commands.commandIsConsole
import org.basaltmc.server.commands.commandReply
import org.basaltmc.server.commands.commandSourceName
import org.basaltmc.server.config.configI64
import org.basaltmc.server.core.Args
import org.basaltmc.server.core.Ctx
import org.basaltmc.server.core.Priority
import org.basaltmc.server.data.dataGetI64
import org.basaltmc.server.data.dataSetI64
import org.basaltmc.server.events.eventPlayer
import org.basaltmc.server.events.on
import org.basaltmc.server.log.logInfo
import org.basaltmc.server.menus.menuClickPlayer
import org.basaltmc.server.menus.menuCreate
import org.basaltmc.server.menus.menuOpen
import org.basaltmc.server.menus.menuSet
import org.basaltmc.server.players.particle
import org.basaltmc.server.players.playerByName
import org.basaltmc.server.players.playerName
import org.basaltmc.server.players.playersOnline
import org.basaltmc.server.players.send
import org.basaltmc.server.tasks.every
import org.basaltmc.server.text.translate

var shop = 0

fun buySparkles(ctx: Ctx) {
    val player = menuClickPlayer(ctx)
    val price = configI64("shop.sparkles-price", 100)
    val balance = dataGetI64(player, "coins", 0)
    if (balance < price) {
        send(player, "@shop-cant-afford", Args().add("price", price.toString()).add("amount", balance.toString()))
        return
    }
    dataSetI64(player, "coins", balance - price)
    particle(player, "minecraft:happy_villager", 20, 0.5)
    send(player, "@shop-bought", Args().add("item", "Sparkles").add("price", price.toString()))
}

fun onJoin(ctx: Ctx) {
    val player = eventPlayer(ctx)
    if (player == 0) return
    val balance = dataGetI64(player, "coins", 0) + configI64("welcome-bonus", 50)
    dataSetI64(player, "coins", balance)
    send(player, "@welcome", Args().add("name", playerName(player) ?: "").add("amount", balance.toString()))
}

fun hello(ctx: Ctx) {
    val times = commandArgI64(ctx, "times") ?: 1L
    val args = Args().add("name", commandSourceName(ctx) ?: "")
    for (i in 0 until times) commandReply(ctx, "@hello", args)
}

fun openShop(ctx: Ctx) {
    if (commandIsConsole(ctx)) {
        commandReply(ctx, "<red>The shop needs a player.")
        return
    }
    menuOpen(shop, playerByName(commandSourceName(ctx) ?: ""))
}

fun init() {
    logInfo("starting up")
    on("player.join", Priority.NORMAL, ::onJoin)
    command("@ID@ hello [times: int 1..5]", "", ::hello)
    command("@ID@ shop", "", ::openShop)

    shop = menuCreate(translate("shop-title") ?: "", 3)
    val lore = translate("shop-item-sparkles-lore", Args().add("price", "100")) ?: ""
    menuSet(shop, 13, "minecraft:glowstone_dust", translate("shop-item-sparkles") ?: "", lore, ::buySparkles)

    val period = configI64("announce-period", 1200).toInt()
    every(period, period) {
        for (player in playersOnline()) send(player, "@announce")
    }
}
