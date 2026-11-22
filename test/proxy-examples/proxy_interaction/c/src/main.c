#include <basalt/proxy.h>

static bs_player seen = 0;

static void say(const char *text) { bs_info(text); }

static int ok(int result) { return result >= 0; }

static void copy_text(bs_val root, const char *key, char *out, int cap) {
    const char *data = 0;
    int32_t length = 0;
    out[0] = 0;
    if (bs_value_as_str(bs_value_find(root, key), &data, &length) && length < cap) {
        for (int i = 0; i < length; ++i) {
            out[i] = data[i];
        }
        out[length] = 0;
    }
}

static void on_pack_status(bs_ctx ctx) {
    char buffer[512];
    char status[64];
    bs_val root;
    const int n = bs_proxy_value_result(ctx, buffer, sizeof buffer);
    if (n < 0 || bs_value_open(&root, buffer, n) != 0) {
        say("pack event unreadable");
        return;
    }
    copy_text(root, "status", status, sizeof status);
    say("pack status:");
    say(status);
}

static void on_settings(bs_ctx ctx) {
    char buffer[1024];
    char locale[64];
    bs_val root;
    const int n = bs_proxy_value_result(ctx, buffer, sizeof buffer);
    if (n < 0 || bs_value_open(&root, buffer, n) != 0) {
        say("settings event unreadable");
        return;
    }
    copy_text(bs_value_find(root, "settings"), "locale", locale, sizeof locale);
    say("settings locale:");
    say(locale);
}

static void on_cookie(bs_ctx ctx) {
    char cookie[64];
    const int n = bs_proxy_cookie_result(ctx, cookie, sizeof cookie - 1);
    if (n < 0) {
        say("cookie absent");
        return;
    }
    cookie[n] = 0;
    say("cookie received:");
    say(cookie);
}

static void on_dialog_click(bs_ctx ctx) {
    say("dialog clicked");
    bs_command_reply(ctx, "<green>dialog clicked", 0);
}

static void on_transfer(bs_ctx ctx) {
    const bs_player player = bs_event_player(ctx);
    say(ok(bs_proxy_player_transfer(player, "127.0.0.1", 25970)) ? "transfer sent" : "transfer failed");
}

static void on_cleanup(bs_ctx ctx) {
    const bs_player player = bs_event_player(ctx);
    const int removed = ok(bs_proxy_resource_pack_remove(player, "demo"));
    const int cleared = ok(bs_proxy_resource_pack_clear(player));
    const int closed = ok(bs_proxy_dialog_clear(player));
    say(removed && cleared && closed ? "cleanup ok" : "cleanup failed");
}

static void on_stale(bs_ctx ctx) {
    (void)ctx;
    say(bs_proxy_dialog_clear(seen) == BASALT_ERR_STALE ? "stale after quit" : "stale not detected");
}

static void on_quit(bs_ctx ctx) {
    (void)ctx;
    bs_after(20, on_stale);
}

static void on_join(bs_ctx ctx) {
    const bs_player player = bs_event_player(ctx);
    char uuid[64] = {0};
    char address[64] = {0};
    char id[64] = {0};
    const char *labels[] = {"Accept", "Decline"};
    const char *commands[] = {"pidialog", "pidialog"};
    bs_args args;
    seen = player;
    say(bs_proxy_players_count("") >= 1 ? "count ok" : "count failed");
    say(bs_proxy_players_count("lobby") >= 1 ? "count lobby ok" : "count lobby failed");
    say(bs_proxy_players_count("nowhere") == 0 ? "count nowhere ok" : "count nowhere failed");
    say(bs_proxy_player_address(player, address, sizeof address - 1) > 0 && bs_proxy_player_port(player) > 0
            ? "address ok"
            : "address failed");
    const int uuid_length = bs_proxy_player_uuid(player, uuid, sizeof uuid - 1);
    uuid[uuid_length > 0 ? uuid_length : 0] = 0;
    say(bs_proxy_players_by_uuid(uuid) == player ? "by uuid ok" : "by uuid failed");
    say(bs_proxy_players_by_uuid("00000000-0000-0000-0000-000000000000") == 0 ? "by uuid none ok"
                                                                                : "by uuid none failed");

    say(ok(bs_proxy_dialog_show(player, "multi_action", "<gold>Interaction", "<gray>Pick one", labels, commands, 2, 1))
            ? "dialog ok"
            : "dialog failed");
    say(bs_proxy_dialog_show(player, "bogus", "x", 0, labels, commands, 0, 1) == BASALT_ERR_INVALID
            ? "bad kind invalid"
            : "bad kind accepted");

    const int length = bs_proxy_resource_pack_push(player, "https://example.invalid/pack.zip", 0, 0, "<gold>Test pack",
                                                   "demo", id, sizeof id - 1);
    if (length > 0) {
        id[length] = 0;
        say("pack pushed:");
        say(id);
    } else {
        say("pack failed");
    }
    say(bs_proxy_resource_pack_push(player, "https://example.invalid/pack.zip", "nothex", 0, 0, 0, id,
                                    sizeof id - 1) == BASALT_ERR_INVALID
            ? "bad hash invalid"
            : "bad hash accepted");

    say(ok(bs_proxy_cookie_set(player, "basalt:demo", "abcd", 4)) ? "cookie stored" : "cookie store failed");
    say(bs_proxy_cookie_request(player, "basalt:demo", on_cookie) > 0 ? "cookie requested" : "cookie request failed");

    say(ok(bs_proxy_player_data_set_f64(player, "ratio", 2.5)) ? "float stored" : "float store failed");
    say(bs_proxy_player_data_get_f64(player, "ratio", 0.0) == 2.5 ? "float read ok" : "float read failed");
    say(bs_proxy_player_data_get_f64(player, "missing", 7.5) == 7.5 ? "float default ok" : "float default failed");

    bs_args_init(&args);
    bs_args_add(&args, "who", "everyone");
    say(ok(bs_proxy_players_broadcast("<green>Interaction hello {who}", &args)) ? "broadcast ok" : "broadcast failed");
    say(bs_proxy_player_transfer(player, "", 25565) == BASALT_ERR_INVALID ? "bad host invalid" : "bad host accepted");
}

void plugin_init(void) {
    bs_on("player.join", BASALT_PRIORITY_NORMAL, on_join);
    bs_on("player.quit", BASALT_PRIORITY_NORMAL, on_quit);
    bs_proxy_watch_resource_packs(on_pack_status);
    bs_proxy_watch_settings(on_settings);
    bs_command("pidialog", "", on_dialog_click);
    bs_command("pitransfer", "", on_transfer);
    bs_command("picleanup", "", on_cleanup);
    say("interaction ready");
}
