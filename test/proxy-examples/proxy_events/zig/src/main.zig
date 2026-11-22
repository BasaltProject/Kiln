const std = @import("std");
const basalt = @import("basalt_proxy");
const pe = basalt.events;

var blocked_buffer: [64]u8 = undefined;
var blocked: []const u8 = "";

fn onEnable(_: basalt.core.Ctx) void {
    var buffer: [512]u8 = undefined;
    var text: [128]u8 = undefined;
    if (pe.pluginInfo("id", &text)) |id| {
        basalt.log.info("plugin id:");
        basalt.log.info(id);
    }
    if (pe.pluginInfo("version", &text)) |version| {
        basalt.log.info("plugin version:");
        basalt.log.info(version);
    }
    if (pe.pluginManifest(&buffer) > 0) basalt.log.info("manifest ok");
    if (pe.configValue("banned-words", &buffer) > 0) basalt.log.info("config array ok");
    basalt.log.info(blocked);
}

fn onChat(ctx: basalt.core.Ctx) void {
    var message: [256]u8 = undefined;
    var fields: [2048]u8 = undefined;
    const text = pe.eventGetStr(ctx, "message", &message) orelse return;
    if (pe.eventFields(ctx, &fields) < 0) basalt.log.err("event fields failed");
    if (std.mem.eql(u8, text, "ping")) {
        _ = pe.eventSetText(ctx, "message", "pong");
        basalt.log.info("chat rewritten");
    } else if (std.mem.eql(u8, text, blocked)) {
        _ = pe.eventDeny(ctx, "<red>Watch your language");
        basalt.log.info("chat denied");
    }
}

fn onChatSeen(ctx: basalt.core.Ctx) void {
    var message: [256]u8 = undefined;
    if (pe.eventGetStr(ctx, "message", &message)) |text| {
        basalt.log.info("chat seen:");
        basalt.log.info(text);
    }
}

fn onCommand(ctx: basalt.core.Ctx) void {
    var line: [128]u8 = undefined;
    const text = pe.eventGetStr(ctx, "command", &line) orelse return;
    if (std.mem.eql(u8, text, "secret")) {
        _ = pe.eventDeny(ctx, "<red>No secrets here");
        basalt.log.info("command denied");
    }
}

fn onKick(ctx: basalt.core.Ctx) void {
    _ = pe.eventSetText(ctx, "reason", "<yellow>Kicked by the backend, come back soon");
    if (pe.eventSetText(ctx, "server", "nowhere") < 0) basalt.log.info("redirect refused");
    if (pe.eventSetFlag(ctx, "cancelled", true) < 0) basalt.log.info("kick cannot be cancelled");
}

fn onKickSeen(ctx: basalt.core.Ctx) void {
    var reason: [256]u8 = undefined;
    if (pe.eventGetStr(ctx, "reason", &reason)) |text| {
        basalt.log.info("kick seen:");
        basalt.log.info(text);
    }
}

fn onCommandRun(ctx: basalt.core.Ctx) void {
    const factor = pe.commandArgF64(ctx, "factor");
    const flag = pe.commandArgBool(ctx, "flag");
    if (factor == null or flag == null) {
        _ = basalt.commands.reply(ctx, "<red>bad arguments", null);
        return;
    }
    var args = basalt.core.Args.init();
    _ = args.add("size", if (factor.? > 1.5) "big" else "small").add("flag", if (flag.?) "yes" else "no");
    _ = basalt.commands.reply(ctx, "<green>factor is {size}, flag is {flag}", &args);
}

pub fn init() void {
    blocked = basalt.config.getStr("blocked-word", &blocked_buffer) orelse "";
    _ = basalt.events.on("plugin.enable", basalt.core.Priority.normal, onEnable);
    _ = basalt.events.on("player.chat", basalt.core.Priority.normal, onChat);
    _ = pe.watch("player.chat", onChatSeen);
    _ = basalt.events.on("player.command", basalt.core.Priority.normal, onCommand);
    _ = basalt.events.on("player.kick", basalt.core.Priority.normal, onKick);
    _ = pe.watch("player.kick", onKickSeen);
    _ = basalt.commands.register("proxyevents <factor: float> <flag: bool>", "", onCommandRun);
}
