#include <basalt/proxy.hpp>

static basalt::proxy::core::Player seen = 0;

static void say(const char* text) { basalt::proxy::log::log_info(text); }

static bool ok(int result) { return result >= 0; }

static void copy_text(basalt::proxy::values::Value root, const char* key, char* out, int cap) {
    const char* data = nullptr;
    int length = 0;
    out[0] = 0;
    if (root.find(key).as_str(data, length) && length < cap) {
        for (int i = 0; i < length; ++i) {
            out[i] = data[i];
        }
        out[length] = 0;
    }
}

static void on_pack_status(basalt::proxy::core::Ctx ctx) {
    char buffer[512];
    char status[64];
    basalt::proxy::values::Value root;
    const int n = basalt::proxy::interaction::value_result(ctx, buffer, sizeof buffer);
    if (n < 0 || !root.open(buffer, n)) {
        say("pack event unreadable");
        return;
    }
    copy_text(root, "status", status, sizeof status);
    say("pack status:");
    say(status);
}

static void on_settings(basalt::proxy::core::Ctx ctx) {
    char buffer[1024];
    char locale[64];
    basalt::proxy::values::Value root;
    const int n = basalt::proxy::interaction::value_result(ctx, buffer, sizeof buffer);
    if (n < 0 || !root.open(buffer, n)) {
        say("settings event unreadable");
        return;
    }
    copy_text(root.find("settings"), "locale", locale, sizeof locale);
    say("settings locale:");
    say(locale);
}

static void on_cookie(basalt::proxy::core::Ctx ctx) {
    char cookie[64] = {};
    const int n = basalt::proxy::interaction::cookie_result(ctx, cookie, sizeof cookie - 1);
    if (n < 0) {
        say("cookie absent");
        return;
    }
    cookie[n] = 0;
    say("cookie received:");
    say(cookie);
}

static void on_dialog_click(basalt::proxy::core::Ctx ctx) {
    say("dialog clicked");
    basalt::proxy::commands::command_reply(ctx, "<green>dialog clicked");
}

static void on_transfer(basalt::proxy::core::Ctx ctx) {
    const basalt::proxy::core::Player player = basalt::proxy::events::event_player(ctx);
    say(ok(basalt::proxy::interaction::player_transfer(player, "127.0.0.1", 25970)) ? "transfer sent" : "transfer failed");
}

static void on_cleanup(basalt::proxy::core::Ctx ctx) {
    const basalt::proxy::core::Player player = basalt::proxy::events::event_player(ctx);
    const bool removed = ok(basalt::proxy::interaction::resource_pack_remove(player, "demo"));
    const bool cleared = ok(basalt::proxy::interaction::resource_pack_clear(player));
    const bool closed = ok(basalt::proxy::interaction::dialog_clear(player));
    say(removed && cleared && closed ? "cleanup ok" : "cleanup failed");
}

static void on_stale(basalt::proxy::core::Ctx) {
    say(basalt::proxy::interaction::dialog_clear(seen) == BASALT_ERR_STALE ? "stale after quit" : "stale not detected");
}

static void on_quit(basalt::proxy::core::Ctx) { basalt::proxy::tasks::after(20, on_stale); }

static void on_join(basalt::proxy::core::Ctx ctx) {
    const basalt::proxy::core::Player player = basalt::proxy::events::event_player(ctx);
    seen = player;
    say(basalt::proxy::interaction::players_count() >= 1 ? "count ok" : "count failed");
    say(basalt::proxy::interaction::players_count("lobby") >= 1 ? "count lobby ok" : "count lobby failed");
    say(basalt::proxy::interaction::players_count("nowhere") == 0 ? "count nowhere ok" : "count nowhere failed");
    say(basalt::proxy::interaction::player_address(player).length > 0 && basalt::proxy::interaction::player_port(player) > 0 ? "address ok"
                                                                                                    : "address failed");
    const auto uuid = basalt::proxy::players::player_uuid(player);
    say(basalt::proxy::interaction::players_by_uuid(uuid.c_str()) == player ? "by uuid ok" : "by uuid failed");
    say(basalt::proxy::interaction::players_by_uuid("00000000-0000-0000-0000-000000000000") == 0 ? "by uuid none ok"
                                                                                     : "by uuid none failed");

    const char* labels[] = {"Accept", "Decline"};
    const char* commands[] = {"pidialog", "pidialog"};
    say(ok(basalt::proxy::interaction::dialog_show(player, "multi_action", "<gold>Interaction", "<gray>Pick one", labels, commands,
                                      2))
            ? "dialog ok"
            : "dialog failed");
    say(basalt::proxy::interaction::dialog_show(player, "bogus", "x", nullptr, labels, commands, 0) == BASALT_ERR_INVALID
            ? "bad kind invalid"
            : "bad kind accepted");

    char id[64] = {};
    const int length = basalt::proxy::interaction::resource_pack_push(player, "https://example.invalid/pack.zip", nullptr, false,
                                                         "<gold>Test pack", "demo", id, sizeof id - 1);
    if (length > 0) {
        id[length] = 0;
        say("pack pushed:");
        say(id);
    } else {
        say("pack failed");
    }
    say(basalt::proxy::interaction::resource_pack_push(player, "https://example.invalid/pack.zip", "nothex", false, nullptr, nullptr,
                                          id, sizeof id - 1) == BASALT_ERR_INVALID
            ? "bad hash invalid"
            : "bad hash accepted");

    say(ok(basalt::proxy::interaction::cookie_set(player, "basalt:demo", "abcd", 4)) ? "cookie stored" : "cookie store failed");
    say(basalt::proxy::interaction::cookie_request(player, "basalt:demo", on_cookie) > 0 ? "cookie requested"
                                                                              : "cookie request failed");

    say(ok(basalt::proxy::interaction::player_data_set_f64(player, "ratio", 2.5)) ? "float stored" : "float store failed");
    say(basalt::proxy::interaction::player_data_get_f64(player, "ratio", 0.0) == 2.5 ? "float read ok" : "float read failed");
    say(basalt::proxy::interaction::player_data_get_f64(player, "missing", 7.5) == 7.5 ? "float default ok" : "float default failed");

    basalt::proxy::core::Args args;
    args.add("who", "everyone");
    say(ok(basalt::proxy::interaction::players_broadcast("<green>Interaction hello {who}", &args)) ? "broadcast ok"
                                                                                         : "broadcast failed");
    say(basalt::proxy::interaction::player_transfer(player, "", 25565) == BASALT_ERR_INVALID ? "bad host invalid"
                                                                                  : "bad host accepted");
}

void plugin_init() {
    basalt::proxy::events::on("player.join", basalt::proxy::core::kNormal, on_join);
    basalt::proxy::events::on("player.quit", basalt::proxy::core::kNormal, on_quit);
    basalt::proxy::interaction::watch_resource_packs(on_pack_status);
    basalt::proxy::interaction::watch_settings(on_settings);
    basalt::proxy::commands::command("pidialog", "", on_dialog_click);
    basalt::proxy::commands::command("pitransfer", "", on_transfer);
    basalt::proxy::commands::command("picleanup", "", on_cleanup);
    say("interaction ready");
}
