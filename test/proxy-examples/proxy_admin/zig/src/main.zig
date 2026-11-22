const std = @import("std");
const basalt = @import("basalt_proxy");
const pa = basalt.admin;

fn sayInt(label: []const u8, number: i64) void {
    var text: [96]u8 = undefined;
    basalt.log.info(std.fmt.bufPrint(&text, "{s}{d}", .{ label, number }) catch return);
}

fn sayText(label: []const u8, value: []const u8) void {
    var text: [256]u8 = undefined;
    basalt.log.info(std.fmt.bufPrint(&text, "{s}{s}", .{ label, value }) catch return);
}

fn onCheck(ctx: basalt.core.Ctx) void {
    var buffer: [512]u8 = undefined;
    const request = pa.permissionRequest(ctx, &buffer) orelse return;
    const node = (request.find("text") orelse return).asString() orelse return;
    if (std.mem.eql(u8, node, "proxyadmin.granted")) {
        _ = pa.permissionAnswer(ctx, 1);
    } else if (std.mem.eql(u8, node, "proxyadmin.denied")) {
        _ = pa.permissionAnswer(ctx, 0);
    }
}

fn onMeta(ctx: basalt.core.Ctx) void {
    _ = pa.permissionAnswerText(ctx, "[Admin] ");
}

fn onPing(ctx: basalt.core.Ctx) void {
    var buffer: [512]u8 = undefined;
    const root = pa.completionValue(ctx, &buffer) orelse {
        basalt.log.info("ping failed");
        return;
    };
    if (root.isNull()) {
        basalt.log.info("ping offline");
        return;
    }
    const online = if (root.find("online")) |v| v.asInt() orelse -1 else -1;
    basalt.log.info("ping answered");
    sayInt("ping online: ", online);
}

fn onWait(_: basalt.core.Ctx) void {
    var buffer: [256]u8 = undefined;
    basalt.log.info("waited");
    if (pa.dataValue(null, "visits", &buffer)) |v| sayInt("store visits: ", v.asInt() orelse 0);
    if (pa.storeTop("visits", 3, &buffer)) |top| sayInt("store top entries: ", @intCast(top.count() orelse 0));
}

fn onCommand(ctx: basalt.core.Ctx) void {
    var source: [64]u8 = undefined;
    var text: [128]u8 = undefined;
    const name = basalt.commands.sourceName(ctx, &source) orelse return;
    const player = basalt.players.byName(name);
    sayText("command by: ", name);
    _ = pa.groupSet(name, "vip");
    if (pa.groupGet(name, &text)) |group| sayText("group is: ", group);
    _ = pa.permissionOverride(name, "proxyadmin.extra", 1);
    sayInt("override check: ", pa.permissionCheck(name, "proxyadmin.extra", ""));
    sayInt("granted check: ", pa.permissionCheck(name, "proxyadmin.granted", ""));
    sayInt("denied check: ", pa.permissionCheck(name, "proxyadmin.denied", ""));
    _ = pa.permissionOverride(name, "proxyadmin.extra", -1);
    sayInt("override removed: ", pa.permissionCheck(name, "proxyadmin.extra", ""));
    _ = pa.playerChat(player, "plain chat from the plugin");
    _ = pa.playerSendJson(player, "{\"text\":\"json message\",\"color\":\"green\"}");
    _ = pa.dataSetInt(name, "coins", 7);
    basalt.log.info("command done");
}

fn onConsole(_: basalt.core.Ctx) void {
    basalt.log.info("console command ran");
}

fn onKickCommand(ctx: basalt.core.Ctx) void {
    var source: [64]u8 = undefined;
    const name = basalt.commands.sourceName(ctx, &source) orelse return;
    _ = pa.playerKickJson(basalt.players.byName(name), "{\"text\":\"kicked by json\",\"color\":\"red\"}");
}

pub fn init() void {
    var reason: [128]u8 = undefined;
    var buffer: [256]u8 = undefined;
    _ = pa.permissionProvide(onCheck, onMeta, 0);
    _ = pa.permissionChanged();
    sayInt("register extra: ", pa.serversRegister("extra", "127.0.0.1", 25599, "lobby", &reason));
    sayInt("register again: ", pa.serversRegister("extra", "127.0.0.1", 25599, "lobby", &reason));
    sayInt("register bad port: ", pa.serversRegister("bad", "127.0.0.1", 0, "", &reason));
    sayInt("unregister extra: ", pa.serversUnregister("extra"));
    sayInt("unregister lobby: ", pa.serversUnregister("lobby"));
    sayInt("unregister none: ", pa.serversUnregister("never"));
    _ = pa.dataSetInt(null, "visits", 5);
    _ = pa.dataSetStr(null, "label", "hello");
    _ = pa.dataSetInt("OfflineHolder", "coins", 42);
    _ = pa.dataSetFloat("OfflineHolder", "ratio", 2.5);
    if (pa.dataValue("OfflineHolder", "coins", &buffer)) |v| sayInt("offline coins: ", v.asInt() orelse 0);
    if (pa.dataValue("OfflineHolder", "ratio", &buffer)) |v| {
        sayInt("offline ratio x10: ", @intFromFloat((v.asFloat() orelse 0) * 10));
    }
    _ = pa.dataRemove("OfflineHolder", "ratio");
    sayInt("removed ratio: ", if (pa.dataValue("OfflineHolder", "ratio", &buffer) == null) -1 else 0);
    _ = basalt.commands.register("pcon", "", onConsole);
    sayInt("console run: ", pa.consoleRun("pcon"));
    _ = pa.serversPing("lobby", onPing);
    _ = pa.wait(2, onWait);
    _ = basalt.commands.register("padmin", "proxyadmin.granted", onCommand);
    _ = basalt.commands.register("pkick", "proxyadmin.granted", onKickCommand);
    _ = basalt.commands.register("pdenied", "proxyadmin.denied", onCommand);
}
