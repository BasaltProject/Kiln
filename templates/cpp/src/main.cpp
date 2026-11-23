#include <basalt/server.hpp>

static int g_shop = 0;

static void buy_sparkles(basalt::server::core::Ctx ctx) {
    const basalt::server::core::Player player = basalt::server::menus::menu_click_player(ctx);
    const long long price = basalt::server::config::config_i64("shop.sparkles-price", 100);
    const long long balance = basalt::server::data::data_get_i64(player, "coins", 0);
    basalt::server::core::Args args;
    if (balance < price) {
        char amount[24];
        char cost[24];
        basalt::server::core::format_i64(balance, amount);
        basalt::server::core::format_i64(price, cost);
        args.add("price", cost).add("amount", amount);
        basalt::server::players::send(player, "@shop-cant-afford", &args);
        return;
    }
    basalt::server::data::data_set_i64(player, "coins", balance - price);
    basalt::server::players::particle(player, "minecraft:happy_villager", 20, 0.5);
    char cost[24];
    basalt::server::core::format_i64(price, cost);
    args.add("item", "Sparkles").add("price", cost);
    basalt::server::players::send(player, "@shop-bought", &args);
}

static void on_join(basalt::server::core::Ctx ctx) {
    const basalt::server::core::Player player = basalt::server::events::event_player(ctx);
    if (player == 0) {
        return;
    }
    const long long bonus = basalt::server::config::config_i64("welcome-bonus", 50);
    const long long balance = basalt::server::data::data_get_i64(player, "coins", 0) + bonus;
    basalt::server::data::data_set_i64(player, "coins", balance);
    char amount[24];
    basalt::server::core::format_i64(balance, amount);
    basalt::server::core::Args args;
    args.add("name", basalt::server::players::player_name(player).c_str()).add("amount", amount);
    basalt::server::players::send(player, "@welcome", &args);
}

static void hello(basalt::server::core::Ctx ctx) {
    long long times = 1;
    basalt::server::commands::command_arg_i64(ctx, "times", &times);
    basalt::server::core::Args args;
    args.add("name", basalt::server::commands::command_source_name(ctx).c_str());
    for (long long i = 0; i < times; ++i) {
        basalt::server::commands::command_reply(ctx, "@hello", &args);
    }
}

static void shop(basalt::server::core::Ctx ctx) {
    if (basalt::server::commands::command_is_console(ctx)) {
        basalt::server::commands::command_reply(ctx, "<red>The shop needs a player.");
        return;
    }
    basalt::server::menus::menu_open(g_shop, basalt::server::players::player_by_name(basalt::server::commands::command_source_name(ctx).c_str()));
}

static void announce(basalt::server::core::Ctx) {
    static basalt::server::core::Player online[1024];
    const int count = basalt::server::players::players_online(online, 1024);
    if (count < 0 || count > 1024) {
        return;
    }
    for (int i = 0; i < count; ++i) {
        basalt::server::players::send(online[i], "@announce");
    }
}

void plugin_init() {
    basalt::server::log::log_info("starting up");
    basalt::server::events::on("player.join", basalt::server::core::kNormal, on_join);
    basalt::server::commands::command("@ID@ hello [times: int 1..5]", "", hello);
    basalt::server::commands::command("@ID@ shop", "", shop);

    const auto title = basalt::server::text::translate("shop-title");
    g_shop = basalt::server::menus::menu_create(title.c_str(), 3);
    const auto name = basalt::server::text::translate("shop-item-sparkles");
    basalt::server::core::Args price;
    price.add("price", "100");
    const auto lore = basalt::server::text::translate("shop-item-sparkles-lore", &price);
    basalt::server::menus::menu_set(g_shop, 13, "minecraft:glowstone_dust", name.c_str(), lore.c_str(), buy_sparkles);

    const int period = static_cast<int>(basalt::server::config::config_i64("announce-period", 1200));
    basalt::server::tasks::every(period, period, announce);
}
