#include <basalt/proxy.hpp>

static void on_join(basalt::proxy::core::Ctx ctx) {
    const basalt::proxy::core::Player player = basalt::proxy::events::event_player(ctx);
    if (player == 0) {
        return;
    }
    const long long visits = basalt::proxy::data::data_get_i64(player, "visits", 0) + 1;
    basalt::proxy::data::data_set_i64(player, "visits", visits);
    char count[24];
    basalt::proxy::core::format_i64(visits, count);
    basalt::proxy::core::Args args;
    args.add("name", basalt::proxy::players::player_name(player).c_str()).add("visits", count);
    basalt::proxy::players::send(player, "@welcome", &args);
    basalt::proxy::players::player_title(player, "<gold>Welcome", "<gray>Have fun");
}

static void hello(basalt::proxy::core::Ctx ctx) {
    const basalt::proxy::core::Player player = basalt::proxy::events::event_player(ctx);
    basalt::proxy::core::Args args;
    args.add("name", basalt::proxy::commands::command_source_name(ctx).c_str());
    args.add("server", player == 0 ? "console" : basalt::proxy::players::player_server(player).c_str());
    basalt::proxy::commands::command_reply(ctx, "@hello", &args);
}

static void announce(basalt::proxy::core::Ctx) {
    basalt::proxy::core::Player online[64];
    const int count = basalt::proxy::players::players_online(online, 64);
    for (int i = 0; i < count && i < 64; ++i) {
        basalt::proxy::players::send(online[i], "@announce");
    }
}

void plugin_init() {
    basalt::proxy::log::log_info("starting up");
    basalt::proxy::events::on("player.join", basalt::proxy::core::kNormal, on_join);
    basalt::proxy::commands::command("@ID@ hello", "", hello);
    const int period = static_cast<int>(basalt::proxy::config::config_i64("announce-period", 1200));
    basalt::proxy::tasks::every(period, period, announce);
}
