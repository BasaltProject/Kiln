package org.basaltmc.proxy.example;

import static org.basaltmc.proxy.commands.Commands.command;
import static org.basaltmc.proxy.commands.Commands.commandReply;
import static org.basaltmc.proxy.commands.Commands.commandSourceName;
import static org.basaltmc.proxy.config.Config.configI64;
import static org.basaltmc.proxy.core.Priority.NORMAL;
import static org.basaltmc.proxy.data.SharedData.dataGetI64;
import static org.basaltmc.proxy.data.SharedData.dataSetI64;
import static org.basaltmc.proxy.events.Events.eventPlayer;
import static org.basaltmc.proxy.events.Events.on;
import static org.basaltmc.proxy.log.Log.logInfo;
import static org.basaltmc.proxy.players.SharedPlayers.playerName;
import static org.basaltmc.proxy.players.SharedPlayers.playersOnline;
import static org.basaltmc.proxy.players.SharedPlayers.send;
import static org.basaltmc.proxy.tasks.Tasks.every;
import org.basaltmc.proxy.core.Args;

public final class Main {
    private static String orEmpty(String text) {
        return text == null ? "" : text;
    }

    private static void onJoin(int ctx) {
        int player = eventPlayer(ctx);
        if (player == 0) {
            return;
        }
        long visits = dataGetI64(player, "visits", 0) + 1;
        dataSetI64(player, "visits", visits);
        send(player, "@welcome",
                new Args().add("name", orEmpty(playerName(player))).add("visits", String.valueOf(visits)));
        org.basaltmc.proxy.players.Players.playerTitle(player, "<gold>Welcome", "<gray>Have fun");
    }

    private static void hello(int ctx) {
        int player = eventPlayer(ctx);
        String server = player == 0 ? "console" : orEmpty(org.basaltmc.proxy.players.Players.playerServer(player));
        commandReply(ctx, "@hello", new Args().add("name", orEmpty(commandSourceName(ctx))).add("server", server));
    }

    public static void init() {
        logInfo("starting up");
        on("player.join", NORMAL, Main::onJoin);
        command("@ID@ hello", "", Main::hello);

        int period = (int) configI64("announce-period", 1200);
        every(period, period, ctx -> {
            for (int player : playersOnline()) {
                send(player, "@announce");
            }
        });
    }
}
