const basalt = @import("basalt_server");

var shop: i32 = 0;

fn buySparkles(ctx: basalt.core.Ctx) void {
    const player = basalt.menus.clickPlayer(ctx);
    const price = basalt.config.getI64("shop.sparkles-price", 100);
    const balance = basalt.data.getI64(player, "coins", 0);
    var number: [24]u8 = undefined;
    var owned: [24]u8 = undefined;
    var args = basalt.core.Args.init();
    if (balance < price) {
        _ = args.add("price", basalt.core.formatI64(price, &number)).add("amount", basalt.core.formatI64(balance, &owned));
        _ = basalt.players.send(player, "@shop-cant-afford", &args);
        return;
    }
    _ = basalt.data.setI64(player, "coins", balance - price);
    _ = basalt.players.particle(player, "minecraft:happy_villager", 20, 0.5);
    _ = args.add("item", "Sparkles").add("price", basalt.core.formatI64(price, &number));
    _ = basalt.players.send(player, "@shop-bought", &args);
}

fn onJoin(ctx: basalt.core.Ctx) void {
    const player = basalt.events.playerOf(ctx);
    if (player == 0) return;
    const balance = basalt.data.getI64(player, "coins", 0) + basalt.config.getI64("welcome-bonus", 50);
    _ = basalt.data.setI64(player, "coins", balance);
    var name: [64]u8 = undefined;
    var amount: [24]u8 = undefined;
    var args = basalt.core.Args.init();
    _ = args.add("name", basalt.players.name(player, &name) orelse "").add("amount", basalt.core.formatI64(balance, &amount));
    _ = basalt.players.send(player, "@welcome", &args);
}

fn hello(ctx: basalt.core.Ctx) void {
    const times = basalt.commands.argI64(ctx, "times") orelse 1;
    var name: [64]u8 = undefined;
    var args = basalt.core.Args.init();
    _ = args.add("name", basalt.commands.sourceName(ctx, &name) orelse "");
    var i: i64 = 0;
    while (i < times) : (i += 1) _ = basalt.commands.reply(ctx, "@hello", &args);
}

fn openShop(ctx: basalt.core.Ctx) void {
    if (basalt.commands.isConsole(ctx)) {
        _ = basalt.commands.reply(ctx, "<red>The shop needs a player.", null);
        return;
    }
    var name: [64]u8 = undefined;
    _ = basalt.menus.open(shop, basalt.players.byName(basalt.commands.sourceName(ctx, &name) orelse ""));
}

fn announce(_: basalt.core.Ctx) void {
    var online: [64]basalt.core.Player = undefined;
    const count = basalt.players.online(&online);
    for (online[0..count]) |player| _ = basalt.players.send(player, "@announce", null);
}

pub fn init() void {
    basalt.log.info("starting up");
    _ = basalt.events.on("player.join", basalt.core.Priority.normal, onJoin);
    _ = basalt.commands.register("@ID@ hello [times: int 1..5]", "", hello);
    _ = basalt.commands.register("@ID@ shop", "", openShop);

    var title: [128]u8 = undefined;
    var name: [128]u8 = undefined;
    var lore: [128]u8 = undefined;
    var price = basalt.core.Args.init();
    _ = price.add("price", "100");
    shop = basalt.menus.create(basalt.text.translate("shop-title", null, &title) orelse "", 3);
    _ = basalt.menus.set(shop, 13, "minecraft:glowstone_dust", basalt.text.translate("shop-item-sparkles", null, &name) orelse "", basalt.text.translate("shop-item-sparkles-lore", &price, &lore) orelse "", buySparkles);

    const period: i32 = @intCast(basalt.config.getI64("announce-period", 1200));
    _ = basalt.tasks.every(period, period, announce);
}
