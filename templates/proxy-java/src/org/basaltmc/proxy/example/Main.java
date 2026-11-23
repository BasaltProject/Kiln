package org.basaltmc.proxy.example;

import org.basaltmc.proxy.commands.Registry;
import org.basaltmc.proxy.core.Args;
import org.basaltmc.proxy.players.Directory;

import static org.basaltmc.proxy.commands.Sender.name;
import static org.basaltmc.proxy.commands.Sender.reply;
import static org.basaltmc.proxy.config.Settings.configI64;
import static org.basaltmc.proxy.core.Priority.NORMAL;
import static org.basaltmc.proxy.data.PlayerStore.dataGetI64;
import static org.basaltmc.proxy.data.PlayerStore.dataSetI64;
import static org.basaltmc.proxy.events.EventBus.eventPlayer;
import static org.basaltmc.proxy.events.EventBus.on;
import static org.basaltmc.proxy.log.Logger.logInfo;
import static org.basaltmc.proxy.players.Directory.playerName;
import static org.basaltmc.proxy.players.Directory.playersOnline;
import static org.basaltmc.proxy.players.Directory.send;
import static org.basaltmc.proxy.tasks.Scheduler.every;

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
        Directory.playerTitle(player, "<gold>Welcome", "<gray>Have fun");
    }

    private static void hello(int ctx) {
        int player = eventPlayer(ctx);
        String server = player == 0 ? "console" : orEmpty(Directory.playerServer(player));
        reply(ctx, "@hello", new Args().add("name", orEmpty(name(ctx))).add("server", server));
    }

    public static void init() {
        logInfo("starting up");
        on("player.join", NORMAL, Main::onJoin);
        Registry.register("@ID@ hello", "", Main::hello);

        int period = (int) configI64("announce-period", 1200);
        every(period, period, ctx -> {
            for (int player : playersOnline()) {
                send(player, "@announce");
            }
        });
    }
}
