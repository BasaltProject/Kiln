const basalt = @import("basalt_proxy");

fn onJoin(ctx: basalt.core.Ctx) void {
    const player = basalt.events.playerOf(ctx);
    if (player == 0) return;
    const visits = basalt.data.getI64(player, "visits", 0) + 1;
    _ = basalt.data.setI64(player, "visits", visits);
    var name: [64]u8 = undefined;
    var count: [24]u8 = undefined;
    var args = basalt.core.Args.init();
    _ = args.add("name", basalt.players.name(player, &name) orelse "").add("visits", basalt.core.formatI64(visits, &count));
    _ = basalt.players.send(player, "@welcome", &args);
    _ = basalt.players.title(player, "<gold>Welcome", "<gray>Have fun", -1, -1, -1);
}

fn hello(ctx: basalt.core.Ctx) void {
    const player = basalt.events.playerOf(ctx);
    var name: [64]u8 = undefined;
    var server: [64]u8 = undefined;
    var args = basalt.core.Args.init();
    _ = args.add("name", basalt.commands.sourceName(ctx, &name) orelse "");
    _ = args.add("server", if (player == 0) "console" else basalt.players.server(player, &server) orelse "");
    _ = basalt.commands.reply(ctx, "@hello", &args);
}

fn announce(_: basalt.core.Ctx) void {
    var online: [64]basalt.core.Player = undefined;
    const count = basalt.players.online(&online);
    for (online[0..count]) |player| _ = basalt.players.send(player, "@announce", null);
}

pub fn init() void {
    basalt.log.info("starting up");
    _ = basalt.events.on("player.join", basalt.core.Priority.normal, onJoin);
    _ = basalt.commands.register("@ID@ hello", "", hello);
    const period: i32 = @intCast(basalt.config.getI64("announce-period", 1200));
    _ = basalt.tasks.every(period, period, announce);
}
