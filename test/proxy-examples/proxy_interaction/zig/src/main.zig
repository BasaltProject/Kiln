const basalt = @import("basalt_proxy");
const pi = basalt.interaction;
const values = basalt.values;

var seen: basalt.core.Player = 0;

fn say(text: []const u8) void {
    basalt.log.info(text);
}

fn ok(result: i32) bool {
    return result >= 0;
}

fn copyText(root: ?values.Value, key: []const u8, out: []u8) []const u8 {
    const text = (root orelse return "").find(key) orelse return "";
    const data = text.asString() orelse return "";
    if (data.len > out.len) return "";
    @memcpy(out[0..data.len], data);
    return out[0..data.len];
}

fn onPackStatus(ctx: basalt.core.Ctx) void {
    var buffer: [512]u8 = undefined;
    var status: [64]u8 = undefined;
    const n = pi.valueResult(ctx, &buffer);
    const root = if (n < 0 or n > buffer.len) null else values.Value.open(buffer[0..@intCast(n)]);
    if (root == null) return say("pack event unreadable");
    say("pack status:");
    say(copyText(root, "status", &status));
}

fn onSettings(ctx: basalt.core.Ctx) void {
    var buffer: [1024]u8 = undefined;
    var locale: [64]u8 = undefined;
    const n = pi.valueResult(ctx, &buffer);
    const root = if (n < 0 or n > buffer.len) null else values.Value.open(buffer[0..@intCast(n)]);
    if (root == null) return say("settings event unreadable");
    say("settings locale:");
    say(copyText(root.?.find("settings"), "locale", &locale));
}

fn onCookie(ctx: basalt.core.Ctx) void {
    var cookie: [64]u8 = undefined;
    const n = pi.cookieResult(ctx, &cookie);
    if (n < 0) return say("cookie absent");
    say("cookie received:");
    say(cookie[0..@intCast(n)]);
}

fn onDialogClick(ctx: basalt.core.Ctx) void {
    say("dialog clicked");
    _ = basalt.commands.reply(ctx, "<green>dialog clicked", null);
}

fn onTransfer(ctx: basalt.core.Ctx) void {
    const player = basalt.events.playerOf(ctx);
    say(if (ok(pi.playerTransfer(player, "127.0.0.1", 25970))) "transfer sent" else "transfer failed");
}

fn onCleanup(ctx: basalt.core.Ctx) void {
    const player = basalt.events.playerOf(ctx);
    const removed = ok(pi.resourcePackRemove(player, "demo"));
    const cleared = ok(pi.resourcePackClear(player));
    const closed = ok(pi.dialogClear(player));
    say(if (removed and cleared and closed) "cleanup ok" else "cleanup failed");
}

fn onStale(_: basalt.core.Ctx) void {
    say(if (pi.dialogClear(seen) == -2) "stale after quit" else "stale not detected");
}

fn onQuit(_: basalt.core.Ctx) void {
    _ = basalt.tasks.after(20, onStale);
}

fn onJoin(ctx: basalt.core.Ctx) void {
    const player = basalt.events.playerOf(ctx);
    var uuid: [64]u8 = undefined;
    var address: [64]u8 = undefined;
    var id: [64]u8 = undefined;
    seen = player;
    say(if (pi.playersCount("") >= 1) "count ok" else "count failed");
    say(if (pi.playersCount("lobby") >= 1) "count lobby ok" else "count lobby failed");
    say(if (pi.playersCount("nowhere") == 0) "count nowhere ok" else "count nowhere failed");
    say(if (pi.playerAddress(player, &address) > 0 and pi.playerPort(player) > 0) "address ok" else "address failed");
    const text = basalt.players.uuid(player, &uuid) orelse "";
    say(if (pi.playersByUuid(text) == player) "by uuid ok" else "by uuid failed");
    say(if (pi.playersByUuid("00000000-0000-0000-0000-000000000000") == 0) "by uuid none ok" else "by uuid none failed");

    const buttons = [_]pi.Button{ .{ .label = "Accept", .command = "pidialog" }, .{ .label = "Decline", .command = "pidialog" } };
    say(if (ok(pi.dialogShow(player, "multi_action", "<gold>Interaction", "<gray>Pick one", &buttons, true))) "dialog ok" else "dialog failed");
    say(if (pi.dialogShow(player, "bogus", "x", "", &.{}, true) == -3) "bad kind invalid" else "bad kind accepted");

    const length = pi.resourcePackPush(player, "https://example.invalid/pack.zip", "", false, "<gold>Test pack", "demo", &id);
    if (length > 0) {
        say("pack pushed:");
        say(id[0..@intCast(length)]);
    } else {
        say("pack failed");
    }
    say(if (pi.resourcePackPush(player, "https://example.invalid/pack.zip", "nothex", false, "", "", &id) == -3) "bad hash invalid" else "bad hash accepted");

    say(if (ok(pi.cookieSet(player, "basalt:demo", "abcd"))) "cookie stored" else "cookie store failed");
    say(if (pi.cookieRequest(player, "basalt:demo", onCookie) > 0) "cookie requested" else "cookie request failed");

    say(if (ok(pi.playerDataSetF64(player, "ratio", 2.5))) "float stored" else "float store failed");
    say(if (pi.playerDataGetF64(player, "ratio", 0.0) == 2.5) "float read ok" else "float read failed");
    say(if (pi.playerDataGetF64(player, "missing", 7.5) == 7.5) "float default ok" else "float default failed");

    var args = basalt.core.Args.init();
    _ = args.add("who", "everyone");
    say(if (ok(pi.playersBroadcast("<green>Interaction hello {who}", &args))) "broadcast ok" else "broadcast failed");
    say(if (pi.playerTransfer(player, "", 25565) == -3) "bad host invalid" else "bad host accepted");
}

pub fn init() void {
    _ = basalt.events.on("player.join", basalt.core.Priority.normal, onJoin);
    _ = basalt.events.on("player.quit", basalt.core.Priority.normal, onQuit);
    _ = pi.watchResourcePacks(onPackStatus);
    _ = pi.watchSettings(onSettings);
    _ = basalt.commands.register("pidialog", "", onDialogClick);
    _ = basalt.commands.register("pitransfer", "", onTransfer);
    _ = basalt.commands.register("picleanup", "", onCleanup);
    say("interaction ready");
}
