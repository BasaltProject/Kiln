#include <basalt/proxy.h>

static int same(const char *a, const char *b) {
    while (*a && *a == *b) {
        ++a;
        ++b;
    }
    return *a == *b;
}

static void say_int(const char *label, long long number) {
    char text[64];
    int n = 0;
    while (label[n] && n < 40) {
        text[n] = label[n];
        ++n;
    }
    char digits[24];
    int d = 0;
    unsigned long long magnitude = number < 0 ? 0ULL - (unsigned long long)number : (unsigned long long)number;
    do {
        digits[d++] = (char)('0' + magnitude % 10);
        magnitude /= 10;
    } while (magnitude != 0);
    if (number < 0) {
        text[n++] = '-';
    }
    while (d > 0) {
        text[n++] = digits[--d];
    }
    text[n] = 0;
    bs_info(text);
}

static void say_text(const char *label, const char *a) {
    char text[200];
    int n = 0;
    while (label[n] && n < 60) {
        text[n] = label[n];
        ++n;
    }
    for (int i = 0; a[i] && n < 198; ++i) {
        text[n++] = a[i];
    }
    text[n] = 0;
    bs_info(text);
}

static void on_check(bs_ctx ctx) {
    char buffer[512];
    bs_val root;
    if (bs_proxy_permission_request(ctx, buffer, sizeof buffer, &root) != 0) {
        return;
    }
    const char *node = 0;
    int32_t length = 0;
    if (!bs_value_as_str(bs_value_find(root, "text"), &node, &length)) {
        return;
    }
    char text[96];
    int32_t n = length < 95 ? length : 95;
    for (int32_t i = 0; i < n; ++i) {
        text[i] = node[i];
    }
    text[n] = 0;
    if (same(text, "proxyadmin.granted")) {
        bs_proxy_permission_answer(ctx, 1);
    } else if (same(text, "proxyadmin.denied")) {
        bs_proxy_permission_answer(ctx, 0);
    }
}

static void on_meta(bs_ctx ctx) {
    bs_proxy_permission_answer_text(ctx, "[Admin] ");
}

static void on_ping(bs_ctx ctx) {
    char buffer[512];
    bs_val root;
    int64_t online = -1;
    if (bs_proxy_completion_value(ctx, buffer, sizeof buffer, &root) != 0) {
        bs_info("ping failed");
        return;
    }
    if (bs_value_is_null(root)) {
        bs_info("ping offline");
        return;
    }
    bs_value_as_i64(bs_value_find(root, "online"), &online);
    bs_info("ping answered");
    say_int("ping online: ", online);
}

static void on_wait(bs_ctx ctx) {
    char buffer[256];
    bs_val root;
    (void)ctx;
    bs_info("waited");
    if (bs_proxy_data_value(0, "visits", buffer, sizeof buffer, &root) == 0) {
        int64_t visits = 0;
        bs_value_as_i64(root, &visits);
        say_int("store visits: ", visits);
    }
    if (bs_proxy_store_top("visits", 3, buffer, sizeof buffer, &root) == 0) {
        say_int("store top entries: ", bs_value_count(root));
    }
}

static void on_command(bs_ctx ctx) {
    char name[64];
    char text[128];
    const int n = bs_command_source_name(ctx, name, sizeof name - 1);
    if (n < 0) {
        return;
    }
    name[n] = 0;
    bs_player player = bs_player_by_name(name);
    say_text("command by: ", name);
    bs_proxy_group_set(name, "vip");
    const int g = bs_proxy_group_get(name, text, sizeof text - 1);
    if (g >= 0) {
        text[g] = 0;
        say_text("group is: ", text);
    }
    bs_proxy_permission_override(name, "proxyadmin.extra", 1);
    say_int("override check: ", bs_proxy_permission_check(name, "proxyadmin.extra", 0));
    say_int("granted check: ", bs_proxy_permission_check(name, "proxyadmin.granted", 0));
    say_int("denied check: ", bs_proxy_permission_check(name, "proxyadmin.denied", 0));
    bs_proxy_permission_override(name, "proxyadmin.extra", -1);
    say_int("override removed: ", bs_proxy_permission_check(name, "proxyadmin.extra", 0));
    bs_proxy_player_chat(player, "plain chat from the plugin");
    bs_proxy_player_send_json(player, "{\"text\":\"json message\",\"color\":\"green\"}");
    bs_proxy_data_set_i64(name, "coins", 7);
    bs_info("command done");
}

static void on_console(bs_ctx ctx) {
    (void)ctx;
    bs_info("console command ran");
}

static void on_kick_command(bs_ctx ctx) {
    char name[64];
    const int n = bs_command_source_name(ctx, name, sizeof name - 1);
    if (n >= 0) {
        name[n] = 0;
        bs_proxy_player_kick_json(bs_player_by_name(name), "{\"text\":\"kicked by json\",\"color\":\"red\"}");
    }
}

void plugin_init(void) {
    char reason[128];
    char buffer[256];
    bs_val root;
    bs_proxy_permission_provide(on_check, on_meta, 0);
    bs_proxy_permission_changed();
    say_int("register extra: ", bs_proxy_servers_register("extra", "127.0.0.1", 25599, "lobby", reason, sizeof reason - 1));
    say_int("register again: ", bs_proxy_servers_register("extra", "127.0.0.1", 25599, "lobby", reason, sizeof reason - 1));
    say_int("register bad port: ", bs_proxy_servers_register("bad", "127.0.0.1", 0, "", reason, sizeof reason - 1));
    say_int("unregister extra: ", bs_proxy_servers_unregister("extra"));
    say_int("unregister lobby: ", bs_proxy_servers_unregister("lobby"));
    say_int("unregister none: ", bs_proxy_servers_unregister("never"));
    bs_proxy_data_set_i64(0, "visits", 5);
    bs_proxy_data_set_str(0, "label", "hello");
    bs_proxy_data_set_i64("OfflineHolder", "coins", 42);
    bs_proxy_data_set_f64("OfflineHolder", "ratio", 2.5);
    if (bs_proxy_data_value("OfflineHolder", "coins", buffer, sizeof buffer, &root) == 0) {
        int64_t coins = 0;
        bs_value_as_i64(root, &coins);
        say_int("offline coins: ", coins);
    }
    if (bs_proxy_data_value("OfflineHolder", "ratio", buffer, sizeof buffer, &root) == 0) {
        double ratio = 0;
        bs_value_as_f64(root, &ratio);
        say_int("offline ratio x10: ", (long long)(ratio * 10));
    }
    bs_proxy_data_remove("OfflineHolder", "ratio");
    say_int("removed ratio: ", bs_proxy_data_value("OfflineHolder", "ratio", buffer, sizeof buffer, &root));
    bs_command("pcon", "", on_console);
    say_int("console run: ", bs_proxy_console_run("pcon"));
    bs_proxy_servers_ping("lobby", on_ping);
    bs_proxy_wait(2, on_wait);
    bs_command("padmin", "proxyadmin.granted", on_command);
    bs_command("pkick", "proxyadmin.granted", on_kick_command);
    bs_command("pdenied", "proxyadmin.denied", on_command);
}
