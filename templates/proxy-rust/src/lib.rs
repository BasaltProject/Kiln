use basalt_proxy::core::{Args, Ctx};

pub fn init() {
    basalt_proxy::log::info("starting up");
    basalt_proxy::events::on("player.join", basalt_proxy::core::NORMAL, on_join);
    basalt_proxy::commands::register("@ID@ hello", "", hello);

    let period = basalt_proxy::config::i64("announce-period", 1200) as i32;
    basalt_proxy::tasks::every(period, period, |_| {
        for player in basalt_proxy::players::online() {
            basalt_proxy::players::send(player, "@announce", &Args::new());
        }
    });
}

fn on_join(ctx: Ctx) {
    let player = basalt_proxy::events::player(ctx);
    if player == 0 {
        return;
    }
    let visits = basalt_proxy::data::get_i64(player, "visits", 0) + 1;
    basalt_proxy::data::set_i64(player, "visits", visits);
    let name = basalt_proxy::players::name(player).unwrap_or_default();
    basalt_proxy::players::send(player, "@welcome", &Args::new().add("name", &name).add("visits", &visits.to_string()));
    basalt_proxy::players::player_title(player, "<gold>Welcome", "<gray>Have fun", -1, -1, -1);
}

fn hello(ctx: Ctx) {
    let player = basalt_proxy::events::player(ctx);
    let server = if player == 0 { None } else { basalt_proxy::players::player_server(player) };
    let args = Args::new()
        .add("name", &basalt_proxy::commands::source_name(ctx).unwrap_or_default())
        .add("server", &server.unwrap_or_else(|| "console".to_string()));
    basalt_proxy::commands::reply(ctx, "@hello", &args);
}
