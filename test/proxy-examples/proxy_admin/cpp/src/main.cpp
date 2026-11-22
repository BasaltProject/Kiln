#include <basalt/proxy.hpp>

static bool same(const char* a, const char* b) {
    while (*a != 0 && *a == *b) {
        ++a;
        ++b;
    }
    return *a == *b;
}

static void say_int(const char* label, long long number) {
    char text[64];
    int n = 0;
    while (label[n] != 0 && n < 40) {
        text[n] = label[n];
        ++n;
    }
    char digits[24];
    int d = 0;
    unsigned long long magnitude = number < 0 ? 0ULL - static_cast<unsigned long long>(number)
                                              : static_cast<unsigned long long>(number);
    do {
        digits[d++] = static_cast<char>('0' + magnitude % 10);
        magnitude /= 10;
    } while (magnitude != 0);
    if (number < 0) {
        text[n++] = '-';
    }
    while (d > 0) {
        text[n++] = digits[--d];
    }
    text[n] = 0;
    basalt::proxy::log::log_info(text);
}

static void say_text(const char* label, const char* a) {
    char text[200];
    int n = 0;
    while (label[n] != 0 && n < 60) {
        text[n] = label[n];
        ++n;
    }
    for (int i = 0; a[i] != 0 && n < 198; ++i) {
        text[n++] = a[i];
    }
    text[n] = 0;
    basalt::proxy::log::log_info(text);
}

static void on_check(basalt::proxy::core::Ctx ctx) {
    char buffer[512];
    basalt::proxy::values::Value root;
    if (basalt::proxy::admin::permission_request(ctx, buffer, sizeof buffer, root) != 0) {
        return;
    }
    const char* node = nullptr;
    int length = 0;
    if (!root.find("text").as_str(node, length)) {
        return;
    }
    char text[96];
    const int n = length < 95 ? length : 95;
    for (int i = 0; i < n; ++i) {
        text[i] = node[i];
    }
    text[n] = 0;
    if (same(text, "proxyadmin.granted")) {
        basalt::proxy::admin::permission_answer(ctx, 1);
    } else if (same(text, "proxyadmin.denied")) {
        basalt::proxy::admin::permission_answer(ctx, 0);
    }
}

static void on_meta(basalt::proxy::core::Ctx ctx) { basalt::proxy::admin::permission_answer_text(ctx, "[Admin] "); }

static void on_ping(basalt::proxy::core::Ctx ctx) {
    char buffer[512];
    basalt::proxy::values::Value root;
    long long online = -1;
    if (basalt::proxy::admin::completion_value(ctx, buffer, sizeof buffer, root) != 0) {
        basalt::proxy::log::log_info("ping failed");
        return;
    }
    if (root.is_null()) {
        basalt::proxy::log::log_info("ping offline");
        return;
    }
    root.find("online").as_i64(online);
    basalt::proxy::log::log_info("ping answered");
    say_int("ping online: ", online);
}

static void on_wait(basalt::proxy::core::Ctx) {
    char buffer[256];
    basalt::proxy::values::Value root;
    basalt::proxy::log::log_info("waited");
    if (basalt::proxy::admin::data_value(nullptr, "visits", buffer, sizeof buffer, root) == 0) {
        long long visits = 0;
        root.as_i64(visits);
        say_int("store visits: ", visits);
    }
    if (basalt::proxy::admin::store_top("visits", 3, buffer, sizeof buffer, root) == 0) {
        say_int("store top entries: ", root.count());
    }
}

static void on_command(basalt::proxy::core::Ctx ctx) {
    const auto source = basalt::proxy::commands::command_source_name(ctx);
    if (!source.ok()) {
        return;
    }
    const char* name = source.c_str();
    const basalt::proxy::core::Player player = basalt::proxy::players::player_by_name(name);
    say_text("command by: ", name);
    basalt::proxy::admin::group_set(name, "vip");
    const auto group = basalt::proxy::admin::group_get(name);
    if (group.ok()) {
        say_text("group is: ", group.c_str());
    }
    basalt::proxy::admin::permission_override(name, "proxyadmin.extra", 1);
    say_int("override check: ", basalt::proxy::admin::permission_check(name, "proxyadmin.extra"));
    say_int("granted check: ", basalt::proxy::admin::permission_check(name, "proxyadmin.granted"));
    say_int("denied check: ", basalt::proxy::admin::permission_check(name, "proxyadmin.denied"));
    basalt::proxy::admin::permission_override(name, "proxyadmin.extra", -1);
    say_int("override removed: ", basalt::proxy::admin::permission_check(name, "proxyadmin.extra"));
    basalt::proxy::admin::player_chat(player, "plain chat from the plugin");
    basalt::proxy::admin::player_send_json(player, "{\"text\":\"json message\",\"color\":\"green\"}");
    basalt::proxy::admin::data_set_i64(name, "coins", 7);
    basalt::proxy::log::log_info("command done");
}

static void on_console(basalt::proxy::core::Ctx) { basalt::proxy::log::log_info("console command ran"); }

static void on_kick_command(basalt::proxy::core::Ctx ctx) {
    const auto source = basalt::proxy::commands::command_source_name(ctx);
    if (source.ok()) {
        basalt::proxy::admin::player_kick_json(basalt::proxy::players::player_by_name(source.c_str()),
                                        "{\"text\":\"kicked by json\",\"color\":\"red\"}");
    }
}

void plugin_init() {
    char reason[128];
    char buffer[256];
    basalt::proxy::values::Value root;
    basalt::proxy::admin::permission_provide(on_check, on_meta);
    basalt::proxy::admin::permission_changed();
    say_int("register extra: ", basalt::proxy::admin::servers_register("extra", "127.0.0.1", 25599, "lobby", reason, sizeof reason - 1));
    say_int("register again: ", basalt::proxy::admin::servers_register("extra", "127.0.0.1", 25599, "lobby", reason, sizeof reason - 1));
    say_int("register bad port: ", basalt::proxy::admin::servers_register("bad", "127.0.0.1", 0, "", reason, sizeof reason - 1));
    say_int("unregister extra: ", basalt::proxy::admin::servers_unregister("extra"));
    say_int("unregister lobby: ", basalt::proxy::admin::servers_unregister("lobby"));
    say_int("unregister none: ", basalt::proxy::admin::servers_unregister("never"));
    basalt::proxy::admin::data_set_i64(nullptr, "visits", 5);
    basalt::proxy::admin::data_set_str(nullptr, "label", "hello");
    basalt::proxy::admin::data_set_i64("OfflineHolder", "coins", 42);
    basalt::proxy::admin::data_set_f64("OfflineHolder", "ratio", 2.5);
    if (basalt::proxy::admin::data_value("OfflineHolder", "coins", buffer, sizeof buffer, root) == 0) {
        long long coins = 0;
        root.as_i64(coins);
        say_int("offline coins: ", coins);
    }
    if (basalt::proxy::admin::data_value("OfflineHolder", "ratio", buffer, sizeof buffer, root) == 0) {
        double ratio = 0;
        root.as_f64(ratio);
        say_int("offline ratio x10: ", static_cast<long long>(ratio * 10));
    }
    basalt::proxy::admin::data_remove("OfflineHolder", "ratio");
    say_int("removed ratio: ", basalt::proxy::admin::data_value("OfflineHolder", "ratio", buffer, sizeof buffer, root));
    basalt::proxy::commands::command("pcon", "", on_console);
    say_int("console run: ", basalt::proxy::admin::console_run("pcon"));
    basalt::proxy::admin::servers_ping("lobby", on_ping);
    basalt::proxy::admin::wait(2, on_wait);
    basalt::proxy::commands::command("padmin", "proxyadmin.granted", on_command);
    basalt::proxy::commands::command("pkick", "proxyadmin.granted", on_kick_command);
    basalt::proxy::commands::command("pdenied", "proxyadmin.denied", on_command);
}
