use basalt_proxy::interaction as pi;
use basalt_proxy::values::Value;
use basalt_proxy::core::{Args, Ctx};

static SEEN: std::sync::atomic::AtomicI32 = std::sync::atomic::AtomicI32::new(0);

fn say(text: &str) {
    basalt_proxy::log::info(text);
}

fn ok(result: i32) -> bool {
    result >= 0
}

fn text_of(root: Option<Value>, key: &str) -> String {
    root.and_then(|value| value.find(key)).and_then(|value| value.as_str().map(str::to_string)).unwrap_or_default()
}

fn on_pack_status(ctx: Ctx) {
    match pi::value_result(ctx) {
        Ok(event) => {
            say("pack status:");
            say(&text_of(Some(event.root()), "status"));
        }
        Err(_) => say("pack event unreadable"),
    }
}

fn on_settings(ctx: Ctx) {
    match pi::value_result(ctx) {
        Ok(event) => {
            say("settings locale:");
            say(&text_of(event.root().find("settings"), "locale"));
        }
        Err(_) => say("settings event unreadable"),
    }
}

fn on_cookie(ctx: Ctx) {
    match pi::cookie_result(ctx) {
        Some(cookie) => {
            say("cookie received:");
            say(&String::from_utf8_lossy(&cookie));
        }
        None => say("cookie absent"),
    }
}

fn on_dialog_click(ctx: Ctx) {
    say("dialog clicked");
    basalt_proxy::commands::reply(ctx, "<green>dialog clicked", &Args::new());
}

fn on_transfer(ctx: Ctx) {
    let player = basalt_proxy::events::player(ctx);
    say(if ok(pi::player_transfer(player, "127.0.0.1", 25970)) { "transfer sent" } else { "transfer failed" });
}

fn on_cleanup(ctx: Ctx) {
    let player = basalt_proxy::events::player(ctx);
    let removed = ok(pi::resource_pack_remove(player, "demo"));
    let cleared = ok(pi::resource_pack_clear(player));
    let closed = ok(pi::dialog_clear(player));
    say(if removed && cleared && closed { "cleanup ok" } else { "cleanup failed" });
}

fn on_stale(_: Ctx) {
    let stale = pi::dialog_clear(SEEN.load(std::sync::atomic::Ordering::SeqCst)) == -2;
    say(if stale { "stale after quit" } else { "stale not detected" });
}

fn on_quit(_: Ctx) {
    basalt_proxy::tasks::after(20, on_stale);
}

fn on_join(ctx: Ctx) {
    let player = basalt_proxy::events::player(ctx);
    SEEN.store(player, std::sync::atomic::Ordering::SeqCst);
    say(if pi::players_count(None) >= 1 { "count ok" } else { "count failed" });
    say(if pi::players_count(Some("lobby")) >= 1 { "count lobby ok" } else { "count lobby failed" });
    say(if pi::players_count(Some("nowhere")) == 0 { "count nowhere ok" } else { "count nowhere failed" });
    let address = pi::player_address(player).unwrap_or_default();
    say(if !address.is_empty() && pi::player_port(player) > 0 { "address ok" } else { "address failed" });
    let uuid = basalt_proxy::players::player_uuid(player).unwrap_or_default();
    say(if pi::players_by_uuid(&uuid) == player { "by uuid ok" } else { "by uuid failed" });
    say(if pi::players_by_uuid("00000000-0000-0000-0000-000000000000") == 0 { "by uuid none ok" } else { "by uuid none failed" });

    let buttons = [("Accept", "pidialog"), ("Decline", "pidialog")];
    let shown = pi::dialog_show(player, "multi_action", "<gold>Interaction", Some("<gray>Pick one"), &buttons, true);
    say(if ok(shown) { "dialog ok" } else { "dialog failed" });
    say(if pi::dialog_show(player, "bogus", "x", None, &[], true) == -3 { "bad kind invalid" } else { "bad kind accepted" });

    match pi::resource_pack_push(player, "https://example.invalid/pack.zip", None, false, Some("<gold>Test pack"), Some("demo")) {
        Ok(id) => {
            say("pack pushed:");
            say(&id);
        }
        Err(_) => say("pack failed"),
    }
    let bad = pi::resource_pack_push(player, "https://example.invalid/pack.zip", Some("nothex"), false, None, None);
    say(if bad == Err(-3) { "bad hash invalid" } else { "bad hash accepted" });

    say(if ok(pi::cookie_set(player, "basalt:demo", b"abcd")) { "cookie stored" } else { "cookie store failed" });
    say(if pi::cookie_request(player, "basalt:demo", on_cookie) > 0 { "cookie requested" } else { "cookie request failed" });

    say(if ok(pi::player_data_set_f64(player, "ratio", 2.5)) { "float stored" } else { "float store failed" });
    say(if pi::player_data_get_f64(player, "ratio", 0.0) == 2.5 { "float read ok" } else { "float read failed" });
    say(if pi::player_data_get_f64(player, "missing", 7.5) == 7.5 { "float default ok" } else { "float default failed" });

    let args = Args::new().add("who", "everyone");
    say(if ok(pi::players_broadcast("<green>Interaction hello {who}", &args)) { "broadcast ok" } else { "broadcast failed" });
    say(if pi::player_transfer(player, "", 25565) == -3 { "bad host invalid" } else { "bad host accepted" });
}

pub fn init() {
    basalt_proxy::events::on("player.join", basalt_proxy::core::NORMAL, on_join);
    basalt_proxy::events::on("player.quit", basalt_proxy::core::NORMAL, on_quit);
    pi::watch_resource_packs(on_pack_status);
    pi::watch_settings(on_settings);
    basalt_proxy::commands::register("pidialog", "", on_dialog_click);
    basalt_proxy::commands::register("pitransfer", "", on_transfer);
    basalt_proxy::commands::register("picleanup", "", on_cleanup);
    say("interaction ready");
}
