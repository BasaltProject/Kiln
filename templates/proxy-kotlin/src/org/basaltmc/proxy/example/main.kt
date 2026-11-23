package org.basaltmc.proxy.example

import org.basaltmc.proxy.commands.command
import org.basaltmc.proxy.commands.commandReply
import org.basaltmc.proxy.commands.commandSourceName
import org.basaltmc.proxy.config.configI64
import org.basaltmc.proxy.core.Args
import org.basaltmc.proxy.core.Ctx
import org.basaltmc.proxy.core.Priority
import org.basaltmc.proxy.data.dataGetI64
import org.basaltmc.proxy.data.dataSetI64
import org.basaltmc.proxy.events.eventPlayer
import org.basaltmc.proxy.events.on
import org.basaltmc.proxy.log.logInfo
import org.basaltmc.proxy.players.playerName
import org.basaltmc.proxy.players.playerServer
import org.basaltmc.proxy.players.playerTitle
import org.basaltmc.proxy.players.playersOnline
import org.basaltmc.proxy.players.send
import org.basaltmc.proxy.tasks.every

fun onJoin(ctx: Ctx) {
    val player = eventPlayer(ctx)
    if (player == 0) return
    val visits = dataGetI64(player, "visits", 0) + 1
    dataSetI64(player, "visits", visits)
    send(player, "@welcome", Args().add("name", playerName(player) ?: "").add("visits", visits.toString()))
    playerTitle(player, "<gold>Welcome", "<gray>Have fun")
}

fun hello(ctx: Ctx) {
    val player = eventPlayer(ctx)
    val server = if (player == 0) "console" else playerServer(player) ?: ""
    commandReply(ctx, "@hello", Args().add("name", commandSourceName(ctx) ?: "").add("server", server))
}

fun init() {
    logInfo("starting up")
    on("player.join", Priority.NORMAL, ::onJoin)
    command("@ID@ hello", "", ::hello)

    val period = configI64("announce-period", 1200).toInt()
    every(period, period) {
        for (player in playersOnline()) send(player, "@announce")
    }
}
