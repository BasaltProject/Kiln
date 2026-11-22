use basalt_server::core::{Args, Ctx};

pub fn init() {
    basalt_server::log::info("starting up");
    basalt_server::events::on("player.join", basalt_server::core::NORMAL, on_join);
    basalt_server::commands::register("@ID@ hello [times: int 1..5]", "", hello);
    basalt_server::commands::register("@ID@ shop", "", open_shop);

    let shop = basalt_server::menus::menu_create(&basalt_server::text::translate("shop-title", &Args::new()).unwrap_or_default(), 3);
    let price = Args::new().add("price", "100");
    basalt_server::menus::menu_set(
        shop,
        13,
        "minecraft:glowstone_dust",
        &basalt_server::text::translate("shop-item-sparkles", &Args::new()).unwrap_or_default(),
        &basalt_server::text::translate("shop-item-sparkles-lore", &price).unwrap_or_default(),
        buy_sparkles,
    );
    SHOP.with(|cell| cell.set(shop));

    let period = basalt_server::config::i64("announce-period", 1200) as i32;
    basalt_server::tasks::every(period, period, |_| {
        for player in basalt_server::players::online() {
            basalt_server::players::send(player, "@announce", &Args::new());
        }
    });
}

thread_local! {
    static SHOP: std::cell::Cell<i32> = const { std::cell::Cell::new(0) };
}

fn buy_sparkles(ctx: Ctx) {
    let player = basalt_server::menus::menu_click_player(ctx);
    let price = basalt_server::config::i64("shop.sparkles-price", 100);
    let balance = basalt_server::data::get_i64(player, "coins", 0);
    if balance < price {
        let args = Args::new().add("price", &price.to_string()).add("amount", &balance.to_string());
        basalt_server::players::send(player, "@shop-cant-afford", &args);
        return;
    }
    basalt_server::data::set_i64(player, "coins", balance - price);
    basalt_server::players::particle(player, "minecraft:happy_villager", 20, 0.5);
    let args = Args::new().add("item", "Sparkles").add("price", &price.to_string());
    basalt_server::players::send(player, "@shop-bought", &args);
}

fn on_join(ctx: Ctx) {
    let player = basalt_server::events::player(ctx);
    if player == 0 {
        return;
    }
    let balance = basalt_server::data::get_i64(player, "coins", 0) + basalt_server::config::i64("welcome-bonus", 50);
    basalt_server::data::set_i64(player, "coins", balance);
    let name = basalt_server::players::name(player).unwrap_or_default();
    basalt_server::players::send(player, "@welcome", &Args::new().add("name", &name).add("amount", &balance.to_string()));
}

fn hello(ctx: Ctx) {
    let times = basalt_server::commands::arg_i64(ctx, "times").unwrap_or(1);
    let args = Args::new().add("name", &basalt_server::commands::source_name(ctx).unwrap_or_default());
    for _ in 0..times {
        basalt_server::commands::reply(ctx, "@hello", &args);
    }
}

fn open_shop(ctx: Ctx) {
    if basalt_server::commands::is_console(ctx) {
        basalt_server::commands::reply(ctx, "<red>The shop needs a player.", &Args::new());
        return;
    }
    let player = basalt_server::players::by_name(&basalt_server::commands::source_name(ctx).unwrap_or_default());
    basalt_server::menus::menu_open(SHOP.with(|cell| cell.get()), player);
}
