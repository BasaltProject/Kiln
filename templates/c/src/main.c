#include <basalt/server.h>

static int g_shop = 0;

static void format_i64(int64_t value, char *out) { bs_format_i64(value, out); }

static void buy_sparkles(bs_ctx ctx) {
    const bs_player player = bs_menu_click_player(ctx);
    const int64_t price = bs_config_i64("shop.sparkles-price", 100);
    const int64_t balance = bs_data_get_i64(player, "coins", 0);
    bs_args args;
    bs_args_init(&args);
    char cost[24];
    format_i64(price, cost);
    if (balance < price) {
        char amount[24];
        format_i64(balance, amount);
        bs_args_add(&args, "price", cost);
        bs_args_add(&args, "amount", amount);
        bs_send(player, "@shop-cant-afford", &args);
        return;
    }
    bs_data_set_i64(player, "coins", balance - price);
    bs_particle(player, "minecraft:happy_villager", 20, 0.5);
    bs_args_add(&args, "item", "Sparkles");
    bs_args_add(&args, "price", cost);
    bs_send(player, "@shop-bought", &args);
}

static void on_join(bs_ctx ctx) {
    const bs_player player = bs_event_player(ctx);
    if (player == 0) {
        return;
    }
    const int64_t balance = bs_data_get_i64(player, "coins", 0) + bs_config_i64("welcome-bonus", 50);
    bs_data_set_i64(player, "coins", balance);
    char amount[24];
    char name[64];
    format_i64(balance, amount);
    bs_player_name(player, name, sizeof name);
    bs_args args;
    bs_args_init(&args);
    bs_args_add(&args, "name", name);
    bs_args_add(&args, "amount", amount);
    bs_send(player, "@welcome", &args);
}

static void hello(bs_ctx ctx) {
    int64_t times = 1;
    bs_command_arg_i64(ctx, "times", &times);
    char name[64];
    bs_command_source_name(ctx, name, sizeof name);
    bs_args args;
    bs_args_init(&args);
    bs_args_add(&args, "name", name);
    for (int64_t i = 0; i < times; ++i) {
        bs_command_reply(ctx, "@hello", &args);
    }
}

static void shop(bs_ctx ctx) {
    if (bs_command_is_console(ctx)) {
        bs_command_reply(ctx, "<red>The shop needs a player.", 0);
        return;
    }
    char name[64];
    bs_command_source_name(ctx, name, sizeof name);
    bs_menu_open(g_shop, bs_player_by_name(name));
}

static void announce(bs_ctx ctx) {
    (void)ctx;
    bs_player online[64];
    const int count = bs_players_online(online, 64);
    for (int i = 0; i < count && i < 64; ++i) {
        bs_send(online[i], "@announce", 0);
    }
}

void plugin_init(void) {
    bs_info("starting up");
    bs_on("player.join", 2, on_join);
    bs_command("@ID@ hello [times: int 1..5]", "", hello);
    bs_command("@ID@ shop", "", shop);

    char title[128];
    char name[128];
    char lore[128];
    bs_translate("shop-title", 0, title, sizeof title);
    g_shop = bs_menu_create(title, 3);
    bs_translate("shop-item-sparkles", 0, name, sizeof name);
    bs_args price;
    bs_args_init(&price);
    bs_args_add(&price, "price", "100");
    bs_translate("shop-item-sparkles-lore", &price, lore, sizeof lore);
    bs_menu_set(g_shop, 13, "minecraft:glowstone_dust", name, lore, buy_sparkles);

    const int period = (int)bs_config_i64("announce-period", 1200);
    bs_every(period, period, announce);
}
