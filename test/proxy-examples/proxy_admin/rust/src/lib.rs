use basalt_proxy::admin as pa;
use basalt_proxy::core::Ctx;

fn say_int(label: &str, number: i64) {
    basalt_proxy::log::info(&format!("{label}{number}"));
}

fn say_text(label: &str, value: &str) {
    basalt_proxy::log::info(&format!("{label}{value}"));
}

pub fn init() {
    pa::permission_provide(Some(Box::new(on_check)), Some(Box::new(on_meta)), 0);
    pa::permission_changed();
    let status = |result: Result<(), String>| if result.is_ok() { 0 } else { -3 };
    say_int("register extra: ", status(pa::servers_register("extra", "127.0.0.1", 25599, "lobby")) as i64);
    say_int("register again: ", status(pa::servers_register("extra", "127.0.0.1", 25599, "lobby")) as i64);
    say_int("register bad port: ", status(pa::servers_register("bad", "127.0.0.1", 0, "")) as i64);
    say_int("unregister extra: ", pa::servers_unregister("extra") as i64);
    say_int("unregister lobby: ", pa::servers_unregister("lobby") as i64);
    say_int("unregister none: ", pa::servers_unregister("never") as i64);
    pa::data_set_i64(None, "visits", 5);
    pa::data_set_str(None, "label", "hello");
    pa::data_set_i64(Some("OfflineHolder"), "coins", 42);
    pa::data_set_f64(Some("OfflineHolder"), "ratio", 2.5);
    if let Ok(value) = pa::data_value(Some("OfflineHolder"), "coins") {
        say_int("offline coins: ", value.root().as_i64().unwrap_or(0));
    }
    if let Ok(value) = pa::data_value(Some("OfflineHolder"), "ratio") {
        say_int("offline ratio x10: ", (value.root().as_f64().unwrap_or(0.0) * 10.0) as i64);
    }
    pa::data_remove(Some("OfflineHolder"), "ratio");
    say_int("removed ratio: ", if pa::data_value(Some("OfflineHolder"), "ratio").is_err() { -1 } else { 0 });
    basalt_proxy::commands::register("pcon", "", on_console);
    say_int("console run: ", pa::console_run("pcon") as i64);
    pa::servers_ping("lobby", on_ping);
    pa::wait(2, on_wait);
    basalt_proxy::commands::register("padmin", "proxyadmin.granted", on_command);
    basalt_proxy::commands::register("pkick", "proxyadmin.granted", on_kick_command);
    basalt_proxy::commands::register("pdenied", "proxyadmin.denied", on_command);
}

fn on_check(ctx: Ctx) {
    let Ok(request) = pa::permission_request(ctx) else { return };
    let Some(node) = request.root().find("text").and_then(|text| text.as_str()) else { return };
    if node == "proxyadmin.granted" {
        pa::permission_answer(ctx, 1);
    } else if node == "proxyadmin.denied" {
        pa::permission_answer(ctx, 0);
    }
}

fn on_meta(ctx: Ctx) {
    pa::permission_answer_text(ctx, "[Admin] ");
}

fn on_ping(ctx: Ctx) {
    let Ok(answer) = pa::completion_value(ctx) else {
        basalt_proxy::log::info("ping failed");
        return;
    };
    let root = answer.root();
    if root.is_null() {
        basalt_proxy::log::info("ping offline");
        return;
    }
    basalt_proxy::log::info("ping answered");
    say_int("ping online: ", root.find("online").and_then(|v| v.as_i64()).unwrap_or(-1));
}

fn on_wait(_: Ctx) {
    basalt_proxy::log::info("waited");
    if let Ok(value) = pa::data_value(None, "visits") {
        say_int("store visits: ", value.root().as_i64().unwrap_or(0));
    }
    if let Ok(top) = pa::store_top("visits", 3) {
        say_int("store top entries: ", top.root().count().unwrap_or(0) as i64);
    }
}

fn on_command(ctx: Ctx) {
    let Some(name) = basalt_proxy::commands::source_name(ctx) else { return };
    let name = name.as_str();
    let player = basalt_proxy::players::by_name(name);
    say_text("command by: ", name);
    pa::group_set(name, "vip");
    if let Some(group) = pa::group_get(name) {
        say_text("group is: ", &group);
    }
    pa::permission_override(name, "proxyadmin.extra", 1);
    say_int("override check: ", pa::permission_check(name, "proxyadmin.extra", "") as i64);
    say_int("granted check: ", pa::permission_check(name, "proxyadmin.granted", "") as i64);
    say_int("denied check: ", pa::permission_check(name, "proxyadmin.denied", "") as i64);
    pa::permission_override(name, "proxyadmin.extra", -1);
    say_int("override removed: ", pa::permission_check(name, "proxyadmin.extra", "") as i64);
    pa::player_chat(player, "plain chat from the plugin");
    pa::player_send_json(player, "{\"text\":\"json message\",\"color\":\"green\"}");
    pa::data_set_i64(Some(name), "coins", 7);
    basalt_proxy::log::info("command done");
}

fn on_console(_: Ctx) {
    basalt_proxy::log::info("console command ran");
}

fn on_kick_command(ctx: Ctx) {
    if let Some(name) = basalt_proxy::commands::source_name(ctx) {
        pa::player_kick_json(basalt_proxy::players::by_name(&name), "{\"text\":\"kicked by json\",\"color\":\"red\"}");
    }
}
