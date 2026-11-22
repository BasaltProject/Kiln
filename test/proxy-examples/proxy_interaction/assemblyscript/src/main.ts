import * as bsCommands from "basalt-proxy/commands";
import * as bsCore from "basalt-proxy/core";
import * as bsEvents from "basalt-proxy/events";
import * as bsInteraction from "basalt-proxy/interaction";
import * as bsLog from "basalt-proxy/log";
import * as bsPlayers from "basalt-proxy/players";
import * as bsTasks from "basalt-proxy/tasks";
import * as bsValues from "basalt-proxy/values";

let seen: i32 = 0;

function say(text: string): void {
  bsLog.logInfo(text);
}

function ok(result: i32): bool {
  return result >= 0;
}

function textOf(root: bsValues.Value | null, key: string): string {
  if (root === null) return "";
  const child = root.find(key);
  if (child === null) return "";
  const text = child.toStr();
  return text === null ? "" : text;
}

function onPackStatus(ctx: bsCore.Ctx): void {
  const event = bsInteraction.proxyValueResult(ctx);
  if (event === null) {
    say("pack event unreadable");
    return;
  }
  say("pack status:");
  say(textOf(event, "status"));
}

function onSettings(ctx: bsCore.Ctx): void {
  const event = bsInteraction.proxyValueResult(ctx);
  if (event === null) {
    say("settings event unreadable");
    return;
  }
  say("settings locale:");
  say(textOf(event.find("settings"), "locale"));
}

function onCookie(ctx: bsCore.Ctx): void {
  const cookie = bsInteraction.proxyCookieResult(ctx);
  if (cookie === null) {
    say("cookie absent");
    return;
  }
  say("cookie received:");
  say(String.UTF8.decodeUnsafe(changetype<usize>(cookie.dataStart), cookie.length));
}

function onDialogClick(ctx: bsCore.Ctx): void {
  say("dialog clicked");
  bsCommands.commandReply(ctx, "<green>dialog clicked");
}

function onTransfer(ctx: bsCore.Ctx): void {
  const player = bsEvents.eventPlayer(ctx);
  say(ok(bsInteraction.proxyPlayerTransfer(player, "127.0.0.1", 25970)) ? "transfer sent" : "transfer failed");
}

function onCleanup(ctx: bsCore.Ctx): void {
  const player = bsEvents.eventPlayer(ctx);
  const removed = ok(bsInteraction.proxyResourcePackRemove(player, "demo"));
  const cleared = ok(bsInteraction.proxyResourcePackClear(player));
  const closed = ok(bsInteraction.proxyDialogClear(player));
  say(removed && cleared && closed ? "cleanup ok" : "cleanup failed");
}

function onStale(ctx: bsCore.Ctx): void {
  say(bsInteraction.proxyDialogClear(seen) == -2 ? "stale after quit" : "stale not detected");
}

function onQuit(ctx: bsCore.Ctx): void {
  bsTasks.after(20, onStale);
}

function onJoin(ctx: bsCore.Ctx): void {
  const player = bsEvents.eventPlayer(ctx);
  seen = player;
  say(bsInteraction.proxyPlayersCount("") >= 1 ? "count ok" : "count failed");
  say(bsInteraction.proxyPlayersCount("lobby") >= 1 ? "count lobby ok" : "count lobby failed");
  say(bsInteraction.proxyPlayersCount("nowhere") == 0 ? "count nowhere ok" : "count nowhere failed");
  const address = bsInteraction.proxyPlayerAddress(player);
  say(address !== null && address.length > 0 && bsInteraction.proxyPlayerPort(player) > 0 ? "address ok" : "address failed");
  const uuid = bsPlayers.playerUuid(player);
  say(uuid !== null && bsInteraction.proxyPlayersByUuid(uuid) == player ? "by uuid ok" : "by uuid failed");
  say(bsInteraction.proxyPlayersByUuid("00000000-0000-0000-0000-000000000000") == 0 ? "by uuid none ok" : "by uuid none failed");

  const buttons = [new bsInteraction.DialogButton("Accept", "pidialog"), new bsInteraction.DialogButton("Decline", "pidialog")];
  say(ok(bsInteraction.proxyDialogShow(player, "multi_action", "<gold>Interaction", "<gray>Pick one", buttons, true)) ? "dialog ok" : "dialog failed");
  say(bsInteraction.proxyDialogShow(player, "bogus", "x", null, new Array<bsInteraction.DialogButton>(), true) == -3 ? "bad kind invalid" : "bad kind accepted");

  const id = bsInteraction.proxyResourcePackPush(player, "https://example.invalid/pack.zip", null, false, "<gold>Test pack", "demo");
  if (id !== null) {
    say("pack pushed:");
    say(id);
  } else {
    say("pack failed");
  }
  const bad = bsInteraction.proxyResourcePackPush(player, "https://example.invalid/pack.zip", "nothex", false, null, null);
  say(bad === null ? "bad hash invalid" : "bad hash accepted");

  const data = new Uint8Array(4);
  data[0] = 97;
  data[1] = 98;
  data[2] = 99;
  data[3] = 100;
  say(ok(bsInteraction.proxyCookieSet(player, "basalt:demo", data)) ? "cookie stored" : "cookie store failed");
  say(bsInteraction.proxyCookieRequest(player, "basalt:demo", onCookie) > 0 ? "cookie requested" : "cookie request failed");

  say(ok(bsInteraction.proxyPlayerDataSetF64(player, "ratio", 2.5)) ? "float stored" : "float store failed");
  say(bsInteraction.proxyPlayerDataGetF64(player, "ratio", 0.0) == 2.5 ? "float read ok" : "float read failed");
  say(bsInteraction.proxyPlayerDataGetF64(player, "missing", 7.5) == 7.5 ? "float default ok" : "float default failed");

  say(ok(bsInteraction.proxyPlayersBroadcast("<green>Interaction hello {who}", new bsCore.Args().add("who", "everyone"))) ? "broadcast ok" : "broadcast failed");
  say(bsInteraction.proxyPlayerTransfer(player, "", 25565) == -3 ? "bad host invalid" : "bad host accepted");
}

export function init(): void {
  bsEvents.on("player.join", bsCore.NORMAL, onJoin);
  bsEvents.on("player.quit", bsCore.NORMAL, onQuit);
  bsInteraction.proxyWatchResourcePacks(onPackStatus);
  bsInteraction.proxyWatchSettings(onSettings);
  bsCommands.command("pidialog", "", onDialogClick);
  bsCommands.command("pitransfer", "", onTransfer);
  bsCommands.command("picleanup", "", onCleanup);
  say("interaction ready");
}
