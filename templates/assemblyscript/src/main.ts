import * as bsCommands from "basalt-server/commands";
import * as bsConfig from "basalt-server/config";
import * as bsCore from "basalt-server/core";
import * as bsData from "basalt-server/data";
import * as bsEvents from "basalt-server/events";
import * as bsLog from "basalt-server/log";
import * as bsMenus from "basalt-server/menus";
import * as bsPlayers from "basalt-server/players";
import * as bsTasks from "basalt-server/tasks";
import * as bsText from "basalt-server/text";

let shop: i32 = 0;

function buySparkles(ctx: bsCore.Ctx): void {
  const player = bsMenus.menuClickPlayer(ctx);
  const price = bsConfig.configI64("shop.sparkles-price", 100);
  const balance = bsData.dataGetI64(player, "coins", 0);
  if (balance < price) {
    bsPlayers.send(player, "@shop-cant-afford", new bsCore.Args().add("price", price.toString()).add("amount", balance.toString()));
    return;
  }
  bsData.dataSetI64(player, "coins", balance - price);
  bsPlayers.particle(player, "minecraft:happy_villager", 20, 0.5);
  bsPlayers.send(player, "@shop-bought", new bsCore.Args().add("item", "Sparkles").add("price", price.toString()));
}

function onJoin(ctx: bsCore.Ctx): void {
  const player = bsEvents.eventPlayer(ctx);
  if (player == 0) return;
  const balance = bsData.dataGetI64(player, "coins", 0) + bsConfig.configI64("welcome-bonus", 50);
  bsData.dataSetI64(player, "coins", balance);
  const name = bsPlayers.playerName(player);
  bsPlayers.send(player, "@welcome", new bsCore.Args().add("name", name ? name : "").add("amount", balance.toString()));
}

function hello(ctx: bsCore.Ctx): void {
  const times = bsCommands.commandArgI64(ctx, "times", 1);
  const source = bsCommands.commandSourceName(ctx);
  const args = new bsCore.Args().add("name", source ? source : "");
  for (let i: i64 = 0; i < times; i++) bsCommands.commandReply(ctx, "@hello", args);
}

function openShop(ctx: bsCore.Ctx): void {
  if (bsCommands.commandIsConsole(ctx)) {
    bsCommands.commandReply(ctx, "<red>The shop needs a player.");
    return;
  }
  const source = bsCommands.commandSourceName(ctx);
  bsMenus.menuOpen(shop, bsPlayers.playerByName(source ? source : ""));
}

function announce(ctx: bsCore.Ctx): void {
  const online = bsPlayers.playersOnline();
  for (let i = 0; i < online.length; i++) bsPlayers.send(online[i], "@announce");
}

export function init(): void {
  bsLog.logInfo("starting up");
  bsEvents.on("player.join", bsCore.NORMAL, onJoin);
  bsCommands.command("@ID@ hello [times: int 1..5]", "", hello);
  bsCommands.command("@ID@ shop", "", openShop);

  const title = bsText.translate("shop-title");
  shop = bsMenus.menuCreate(title ? title : "", 3);
  const name = bsText.translate("shop-item-sparkles");
  const lore = bsText.translate("shop-item-sparkles-lore", new bsCore.Args().add("price", "100"));
  bsMenus.menuSet(shop, 13, "minecraft:glowstone_dust", name ? name : "", lore ? lore : "", buySparkles);

  const period = <i32>bsConfig.configI64("announce-period", 1200);
  bsTasks.every(period, period, announce);
}
