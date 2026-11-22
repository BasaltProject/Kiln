package org.basaltmc.server.example;

import static org.basaltmc.server.commands.Commands.command;
import static org.basaltmc.server.commands.Commands.commandArgI64;
import static org.basaltmc.server.commands.Commands.commandIsConsole;
import static org.basaltmc.server.commands.Commands.commandReply;
import static org.basaltmc.server.commands.Commands.commandSourceName;
import static org.basaltmc.server.config.Config.configI64;
import static org.basaltmc.server.core.Priority.NORMAL;
import static org.basaltmc.server.data.SharedData.dataGetI64;
import static org.basaltmc.server.data.SharedData.dataSetI64;
import static org.basaltmc.server.events.Events.eventPlayer;
import static org.basaltmc.server.events.Events.on;
import static org.basaltmc.server.log.Log.logInfo;
import static org.basaltmc.server.menus.Menus.menuClickPlayer;
import static org.basaltmc.server.menus.Menus.menuCreate;
import static org.basaltmc.server.menus.Menus.menuOpen;
import static org.basaltmc.server.menus.Menus.menuSet;
import static org.basaltmc.server.players.Players.particle;
import static org.basaltmc.server.players.SharedPlayers.playerByName;
import static org.basaltmc.server.players.SharedPlayers.playerName;
import static org.basaltmc.server.players.SharedPlayers.playersOnline;
import static org.basaltmc.server.players.SharedPlayers.send;
import static org.basaltmc.server.tasks.Tasks.every;
import static org.basaltmc.server.text.Text.translate;
import org.basaltmc.server.core.Args;

public final class Main {
    private static int shop = 0;

    private static String orEmpty(String text) {
        return text == null ? "" : text;
    }

    private static void buySparkles(int ctx) {
        int player = menuClickPlayer(ctx);
        long price = configI64("shop.sparkles-price", 100);
        long balance = dataGetI64(player, "coins", 0);
        if (balance < price) {
            send(player, "@shop-cant-afford",
                    new Args().add("price", String.valueOf(price)).add("amount", String.valueOf(balance)));
            return;
        }
        dataSetI64(player, "coins", balance - price);
        particle(player, "minecraft:happy_villager", 20, 0.5);
        send(player, "@shop-bought", new Args().add("item", "Sparkles").add("price", String.valueOf(price)));
    }

    private static void onJoin(int ctx) {
        int player = eventPlayer(ctx);
        if (player == 0) {
            return;
        }
        long balance = dataGetI64(player, "coins", 0) + configI64("welcome-bonus", 50);
        dataSetI64(player, "coins", balance);
        send(player, "@welcome",
                new Args().add("name", orEmpty(playerName(player))).add("amount", String.valueOf(balance)));
    }

    private static void hello(int ctx) {
        Long given = commandArgI64(ctx, "times");
        long times = given == null ? 1 : given;
        Args args = new Args().add("name", orEmpty(commandSourceName(ctx)));
        for (long i = 0; i < times; i++) {
            commandReply(ctx, "@hello", args);
        }
    }

    private static void openShop(int ctx) {
        if (commandIsConsole(ctx)) {
            commandReply(ctx, "<red>The shop needs a player.");
            return;
        }
        menuOpen(shop, playerByName(orEmpty(commandSourceName(ctx))));
    }

    public static void init() {
        logInfo("starting up");
        on("player.join", NORMAL, Main::onJoin);
        command("@ID@ hello [times: int 1..5]", "", Main::hello);
        command("@ID@ shop", "", Main::openShop);

        shop = menuCreate(orEmpty(translate("shop-title")), 3);
        String lore = orEmpty(translate("shop-item-sparkles-lore", new Args().add("price", "100")));
        menuSet(shop, 13, "minecraft:glowstone_dust", orEmpty(translate("shop-item-sparkles")), lore,
                Main::buySparkles);

        int period = (int) configI64("announce-period", 1200);
        every(period, period, ctx -> {
            for (int player : playersOnline()) {
                send(player, "@announce");
            }
        });
    }
}
