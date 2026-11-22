import * as bsCommands from "basalt-proxy/commands";
import * as bsConfig from "basalt-proxy/config";
import * as bsCore from "basalt-proxy/core";
import * as bsData from "basalt-proxy/data";
import * as bsEvents from "basalt-proxy/events";
import * as bsLog from "basalt-proxy/log";
import * as bsPlayers from "basalt-proxy/players";
import * as bsTasks from "basalt-proxy/tasks";

function onJoin(ctx: bsCore.Ctx): void {
  const player = bsEvents.eventPlayer(ctx);
  if (player == 0) return;
  const visits = bsData.dataGetI64(player, "visits", 0) + 1;
  bsData.dataSetI64(player, "visits", visits);
  const name = bsPlayers.playerName(player);
  bsPlayers.send(player, "@welcome", new bsCore.Args().add("name", name ? name : "").add("visits", visits.toString()));
  bsPlayers.playerTitle(player, "<gold>Welcome", "<gray>Have fun");
}

function hello(ctx: bsCore.Ctx): void {
  const player = bsEvents.eventPlayer(ctx);
  const source = bsCommands.commandSourceName(ctx);
  const server = player == 0 ? "console" : bsPlayers.playerServer(player);
  const args = new bsCore.Args().add("name", source ? source : "").add("server", server ? server : "");
  bsCommands.commandReply(ctx, "@hello", args);
}

function announce(ctx: bsCore.Ctx): void {
  const online = bsPlayers.playersOnline();
  for (let i = 0; i < online.length; i++) bsPlayers.send(online[i], "@announce");
}

export function init(): void {
  bsLog.logInfo("starting up");
  bsEvents.on("player.join", bsCore.NORMAL, onJoin);
  bsCommands.command("@ID@ hello", "", hello);
  const period = <i32>bsConfig.configI64("announce-period", 1200);
  bsTasks.every(period, period, announce);
}
