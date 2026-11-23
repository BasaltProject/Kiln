#include <basalt/proxy.h>

static void terminate(char *buffer, int cap, int length) { buffer[length > 0 && length < cap ? length : 0] = '\0'; }

static void on_join(bs_ctx ctx) {
    const bs_player player = bs_event_player(ctx);
    if (player == 0) {
        return;
    }
    const int64_t visits = bs_data_get_i64(player, "visits", 0) + 1;
    bs_data_set_i64(player, "visits", visits);
    char count[24];
    char name[64];
    bs_format_i64(visits, count);
    terminate(name, sizeof name, bs_player_name(player, name, sizeof name));
    bs_args args;
    bs_args_init(&args);
    bs_args_add(&args, "name", name);
    bs_args_add(&args, "visits", count);
    bs_send(player, "@welcome", &args);
    bs_proxy_player_title(player, "<gold>Welcome", "<gray>Have fun", -1, -1, -1);
}

static void hello(bs_ctx ctx) {
    const bs_player player = bs_event_player(ctx);
    char name[64];
    char server[64] = "console";
    terminate(name, sizeof name, bs_command_source_name(ctx, name, sizeof name));
    if (player != 0) {
        terminate(server, sizeof server, bs_proxy_player_server(player, server, sizeof server));
    }
    bs_args args;
    bs_args_init(&args);
    bs_args_add(&args, "name", name);
    bs_args_add(&args, "server", server);
    bs_command_reply(ctx, "@hello", &args);
}

static void announce(bs_ctx ctx) {
    (void)ctx;
    static bs_player online[1024];
    const int count = bs_players_online(online, 1024);
    if (count < 0 || count > 1024) {
        return;
    }
    for (int i = 0; i < count; ++i) {
        bs_send(online[i], "@announce", 0);
    }
}

void plugin_init(void) {
    bs_info("starting up");
    bs_on("player.join", 2, on_join);
    bs_command("@ID@ hello", "", hello);
    const int period = (int)bs_config_i64("announce-period", 1200);
    bs_every(period, period, announce);
}
