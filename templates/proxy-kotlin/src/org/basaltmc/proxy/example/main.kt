package org.basaltmc.proxy.example

import org.basaltmc.proxy.core.Args
import org.basaltmc.proxy.core.Ctx

fun onJoin(ctx: Ctx) {
    val player = org.basaltmc.proxy.events.eventPlayer(ctx)
    if (player == 0) return
    val visits = org.basaltmc.proxy.data.dataGetI64(player, "visits", 0) + 1
    org.basaltmc.proxy.data.dataSetI64(player, "visits", visits)
    org.basaltmc.proxy.players.send(player, "@welcome", Args().add("name", org.basaltmc.proxy.players.playerName(player) ?: "").add("visits", visits.toString()))
    org.basaltmc.proxy.players.playerTitle(player, "<gold>Welcome", "<gray>Have fun")
}

fun hello(ctx: Ctx) {
    val player = org.basaltmc.proxy.events.eventPlayer(ctx)
    val server = if (player == 0) "console" else org.basaltmc.proxy.players.playerServer(player) ?: ""
    org.basaltmc.proxy.commands.commandReply(ctx, "@hello", Args().add("name", org.basaltmc.proxy.commands.commandSourceName(ctx) ?: "").add("server", server))
}

fun init() {
    org.basaltmc.proxy.log.logInfo("starting up")
    org.basaltmc.proxy.events.on("player.join", org.basaltmc.proxy.core.Priority.NORMAL, ::onJoin)
    org.basaltmc.proxy.commands.command("@ID@ hello", "", ::hello)

    val period = org.basaltmc.proxy.config.configI64("announce-period", 1200).toInt()
    org.basaltmc.proxy.tasks.every(period, period) {
        for (player in org.basaltmc.proxy.players.playersOnline()) org.basaltmc.proxy.players.send(player, "@announce")
    }
}
