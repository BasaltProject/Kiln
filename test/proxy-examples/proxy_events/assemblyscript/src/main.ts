import * as bsCommands from "basalt-proxy/commands";
import * as bsConfig from "basalt-proxy/config";
import * as bsCore from "basalt-proxy/core";
import * as bsEvents from "basalt-proxy/events";
import * as bsLog from "basalt-proxy/log";

let blocked = "";

function onEnable(ctx: bsCore.Ctx): void {
  const id = bsEvents.proxyPluginInfo("id");
  const version = bsEvents.proxyPluginInfo("version");
  if (id !== null) {
    bsLog.logInfo("plugin id:");
    bsLog.logInfo(id);
  }
  if (version !== null) {
    bsLog.logInfo("plugin version:");
    bsLog.logInfo(version);
  }
  if (bsEvents.proxyPluginManifest() !== null) bsLog.logInfo("manifest ok");
  if (bsEvents.proxyConfigValue("banned-words") !== null) bsLog.logInfo("config array ok");
  bsLog.logInfo(blocked);
}

function onChat(ctx: bsCore.Ctx): void {
  const message = bsEvents.proxyEventGetStr(ctx, "message");
  if (message === null) return;
  if (bsEvents.proxyEventFields(ctx) === null) bsLog.logError("event fields failed");
  if (message == "ping") {
    bsEvents.proxyEventSetText(ctx, "message", "pong");
    bsLog.logInfo("chat rewritten");
  } else if (message == blocked) {
    bsEvents.proxyEventDeny(ctx, "<red>Watch your language");
    bsLog.logInfo("chat denied");
  }
}

function onChatSeen(ctx: bsCore.Ctx): void {
  const message = bsEvents.proxyEventGetStr(ctx, "message");
  if (message !== null) {
    bsLog.logInfo("chat seen:");
    bsLog.logInfo(message);
  }
}

function onCommand(ctx: bsCore.Ctx): void {
  const line = bsEvents.proxyEventGetStr(ctx, "command");
  if (line !== null && line == "secret") {
    bsEvents.proxyEventDeny(ctx, "<red>No secrets here");
    bsLog.logInfo("command denied");
  }
}

function onKick(ctx: bsCore.Ctx): void {
  bsEvents.proxyEventSetText(ctx, "reason", "<yellow>Kicked by the backend, come back soon");
  if (bsEvents.proxyEventSetText(ctx, "server", "nowhere") < 0) bsLog.logInfo("redirect refused");
  if (bsEvents.proxyEventSetFlag(ctx, "cancelled", true) < 0) bsLog.logInfo("kick cannot be cancelled");
}

function onKickSeen(ctx: bsCore.Ctx): void {
  const reason = bsEvents.proxyEventGetStr(ctx, "reason");
  if (reason !== null) {
    bsLog.logInfo("kick seen:");
    bsLog.logInfo(reason);
  }
}

function onCommandRun(ctx: bsCore.Ctx): void {
  const factor = bsEvents.proxyCommandArgF64(ctx, "factor", -1);
  const flag = bsEvents.proxyCommandArgBool(ctx, "flag", false);
  const args = new bsCore.Args().add("size", factor > 1.5 ? "big" : "small").add("flag", flag ? "yes" : "no");
  bsCommands.commandReply(ctx, "<green>factor is {size}, flag is {flag}", args);
}

export function init(): void {
  const word = bsConfig.configStr("blocked-word");
  blocked = word !== null ? word : "";
  bsEvents.on("plugin.enable", bsCore.NORMAL, onEnable);
  bsEvents.on("player.chat", bsCore.NORMAL, onChat);
  bsEvents.proxyWatch("player.chat", onChatSeen);
  bsEvents.on("player.command", bsCore.NORMAL, onCommand);
  bsEvents.on("player.kick", bsCore.NORMAL, onKick);
  bsEvents.proxyWatch("player.kick", onKickSeen);
  bsCommands.command("proxyevents <factor: float> <flag: bool>", "", onCommandRun);
}
