import * as bsAdmin from "basalt-proxy/admin";
import * as bsCommands from "basalt-proxy/commands";
import * as bsCore from "basalt-proxy/core";
import * as bsLog from "basalt-proxy/log";
import * as bsPlayers from "basalt-proxy/players";

function sayInt(label: string, number: i64): void {
  bsLog.logInfo(label + number.toString());
}

function onCheck(ctx: bsCore.Ctx): void {
  const request = bsAdmin.proxyPermissionRequest(ctx);
  if (request === null) return;
  const text = request.find("text");
  if (text === null) return;
  const node = text.toStr();
  if (node == "proxyadmin.granted") bsAdmin.proxyPermissionAnswer(ctx, 1);
  else if (node == "proxyadmin.denied") bsAdmin.proxyPermissionAnswer(ctx, 0);
}

function onMeta(ctx: bsCore.Ctx): void {
  bsAdmin.proxyPermissionAnswerText(ctx, "[Admin] ");
}

function onPing(ctx: bsCore.Ctx): void {
  const root = bsAdmin.proxyCompletionValue(ctx);
  if (root === null) {
    bsLog.logInfo("ping failed");
    return;
  }
  if (root.isNull()) {
    bsLog.logInfo("ping offline");
    return;
  }
  const online = root.find("online");
  bsLog.logInfo("ping answered");
  sayInt("ping online: ", online === null ? -1 : online.toInt(-1));
}

function onWait(ctx: bsCore.Ctx): void {
  bsLog.logInfo("waited");
  const visits = bsAdmin.proxyDataValue(null, "visits");
  if (visits !== null) sayInt("store visits: ", visits.toInt(0));
  const top = bsAdmin.proxyStoreTop("visits", 3);
  if (top !== null) sayInt("store top entries: ", top.count());
}

function onCommand(ctx: bsCore.Ctx): void {
  const name = bsCommands.commandSourceName(ctx);
  if (name === null) return;
  const player = bsPlayers.playerByName(name);
  bsLog.logInfo("command by: " + name);
  bsAdmin.proxyGroupSet(name, "vip");
  const group = bsAdmin.proxyGroupGet(name);
  if (group !== null) bsLog.logInfo("group is: " + group);
  bsAdmin.proxyPermissionOverride(name, "proxyadmin.extra", 1);
  sayInt("override check: ", bsAdmin.proxyPermissionCheck(name, "proxyadmin.extra"));
  sayInt("granted check: ", bsAdmin.proxyPermissionCheck(name, "proxyadmin.granted"));
  sayInt("denied check: ", bsAdmin.proxyPermissionCheck(name, "proxyadmin.denied"));
  bsAdmin.proxyPermissionOverride(name, "proxyadmin.extra", -1);
  sayInt("override removed: ", bsAdmin.proxyPermissionCheck(name, "proxyadmin.extra"));
  bsAdmin.proxyPlayerChat(player, "plain chat from the plugin");
  bsAdmin.proxyPlayerSendJson(player, '{"text":"json message","color":"green"}');
  bsAdmin.proxyDataSetInt(name, "coins", 7);
  bsLog.logInfo("command done");
}

function onConsole(ctx: bsCore.Ctx): void {
  bsLog.logInfo("console command ran");
}

function onKickCommand(ctx: bsCore.Ctx): void {
  const name = bsCommands.commandSourceName(ctx);
  if (name !== null) bsAdmin.proxyPlayerKickJson(bsPlayers.playerByName(name), '{"text":"kicked by json","color":"red"}');
}

export function init(): void {
  bsAdmin.proxyPermissionProvide(onCheck, onMeta, 0);
  bsAdmin.proxyPermissionChanged();
  sayInt("register extra: ", bsAdmin.proxyServersRegister("extra", "127.0.0.1", 25599, "lobby").status);
  sayInt("register again: ", bsAdmin.proxyServersRegister("extra", "127.0.0.1", 25599, "lobby").status);
  sayInt("register bad port: ", bsAdmin.proxyServersRegister("bad", "127.0.0.1", 0, "").status);
  sayInt("unregister extra: ", bsAdmin.proxyServersUnregister("extra"));
  sayInt("unregister lobby: ", bsAdmin.proxyServersUnregister("lobby"));
  sayInt("unregister none: ", bsAdmin.proxyServersUnregister("never"));
  bsAdmin.proxyDataSetInt(null, "visits", 5);
  bsAdmin.proxyDataSetStr(null, "label", "hello");
  bsAdmin.proxyDataSetInt("OfflineHolder", "coins", 42);
  bsAdmin.proxyDataSetFloat("OfflineHolder", "ratio", 2.5);
  const coins = bsAdmin.proxyDataValue("OfflineHolder", "coins");
  if (coins !== null) sayInt("offline coins: ", coins.toInt(0));
  const ratio = bsAdmin.proxyDataValue("OfflineHolder", "ratio");
  if (ratio !== null) sayInt("offline ratio x10: ", <i64>(ratio.toFloat(0) * 10));
  bsAdmin.proxyDataRemove("OfflineHolder", "ratio");
  sayInt("removed ratio: ", bsAdmin.proxyDataValue("OfflineHolder", "ratio") === null ? -1 : 0);
  bsCommands.command("pcon", "", onConsole);
  sayInt("console run: ", bsAdmin.proxyConsoleRun("pcon"));
  bsAdmin.proxyServersPing("lobby", onPing);
  bsAdmin.proxyWait(2, onWait);
  bsCommands.command("padmin", "proxyadmin.granted", onCommand);
  bsCommands.command("pkick", "proxyadmin.granted", onKickCommand);
  bsCommands.command("pdenied", "proxyadmin.denied", onCommand);
}
