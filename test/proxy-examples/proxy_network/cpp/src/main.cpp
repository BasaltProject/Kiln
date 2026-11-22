#include <basalt/proxy.hpp>

namespace net = basalt::proxy::network;

static basalt::proxy::core::Text<256> rewrite_marker;
static basalt::proxy::core::Text<256> cancel_marker;
static basalt::proxy::core::Text<256> channel;
static bool refused_once = false;

static int find(const unsigned char* data, int length, const char* needle) {
    int n = 0;
    while (needle[n] != 0) {
        ++n;
    }
    for (int at = 0; at + n <= length; ++at) {
        int i = 0;
        while (i < n && data[at + i] == static_cast<unsigned char>(needle[i])) {
            ++i;
        }
        if (i == n) {
            return at;
        }
    }
    return -1;
}

static void info_with(const char* prefix, const char* text, int length) {
    char line[256];
    int n = 0;
    while (prefix[n] != 0 && n < 120) {
        line[n] = prefix[n];
        ++n;
    }
    for (int i = 0; i < length && n < 250; ++i) {
        line[n++] = text[i];
    }
    line[n] = 0;
    basalt::proxy::log::log_info(line);
}

static bool open_map(int status, char* buffer, basalt::proxy::values::Value& root) { return status >= 0 && root.open(buffer, status); }

static void text_of(const basalt::proxy::values::Value& map, const char* key, const char*& text, int& length) {
    text = "";
    length = 0;
    map.find(key).as_str(text, length);
}

static int chat_body(unsigned char* out, const char* text) {
    int n = 0;
    while (text[n] != 0) {
        ++n;
    }
    out[0] = 0x08;
    out[1] = static_cast<unsigned char>(n >> 8);
    out[2] = static_cast<unsigned char>(n);
    for (int i = 0; i < n; ++i) {
        out[3 + i] = static_cast<unsigned char>(text[i]);
    }
    out[3 + n] = 0;
    return n + 4;
}

static int string_prefixed(unsigned char* out, const char* first, const char* rest) {
    int a = 0;
    int b = 0;
    while (first[a] != 0) {
        ++a;
    }
    while (rest[b] != 0) {
        ++b;
    }
    out[0] = static_cast<unsigned char>(a);
    for (int i = 0; i < a; ++i) {
        out[1 + i] = static_cast<unsigned char>(first[i]);
    }
    for (int i = 0; i < b; ++i) {
        out[1 + a + i] = static_cast<unsigned char>(rest[i]);
    }
    return 1 + a + b;
}

static void on_message(basalt::proxy::core::Ctx ctx) {
    char buffer[1024];
    basalt::proxy::values::Value map;
    if (!open_map(net::message_get(ctx, buffer, sizeof buffer), buffer, map)) {
        basalt::proxy::log::log_error("message read failed");
        return;
    }
    const char* source;
    const char* chan;
    int source_length;
    int chan_length;
    const unsigned char* data = nullptr;
    int data_length = 0;
    text_of(map, "source", source, source_length);
    text_of(map, "channel", chan, chan_length);
    map.find("data").as_bytes(data, data_length);
    char line[160];
    int n = 0;
    for (int i = 0; i < source_length && n < 40; ++i) {
        line[n++] = source[i];
    }
    line[n++] = ' ';
    for (int i = 0; i < chan_length && n < 80; ++i) {
        line[n++] = chan[i];
    }
    line[n++] = ' ';
    for (int i = 0; i < data_length && n < 150; ++i) {
        line[n++] = static_cast<char>(data[i]);
    }
    line[n] = 0;
    info_with("net: message ", line, n);
    const basalt::proxy::core::Player player = basalt::proxy::events::event_player(ctx);
    if (player > 0 && net::message_send(player, "basalt:demo", "pong", 4, true) == 0) {
        basalt::proxy::log::log_info("net: reply sent");
    }
}

static void on_decider(basalt::proxy::core::Ctx ctx) {
    char info[512];
    char body[2048];
    basalt::proxy::values::Value map;
    if (!open_map(net::packet_get(ctx, info, sizeof info), info, map)) {
        basalt::proxy::log::log_error("packet read failed");
        return;
    }
    const char* name;
    int name_length;
    text_of(map, "player_name", name, name_length);
    const int n = net::packet_body(ctx, body, sizeof body);
    if (n < 0 || n > static_cast<int>(sizeof body)) {
        return;
    }
    const auto* bytes = reinterpret_cast<const unsigned char*>(body);
    const int at = find(bytes, n, rewrite_marker.c_str());
    if (find(bytes, n, cancel_marker.c_str()) >= 0) {
        if (net::packet_cancel(ctx) == 0) {
            info_with("net: cancel for ", name, name_length);
        }
    } else if (at >= 0) {
        for (int k = 0; rewrite_marker.c_str()[k] != 0; ++k) {
            if (body[at + k] >= 'a' && body[at + k] <= 'z') {
                body[at + k] = static_cast<char>(body[at + k] - 32);
            }
        }
        if (net::packet_set_body(ctx, body, n) == 0) {
            info_with("net: rewrite for ", name, name_length);
        }
    }
}

static void on_watch(basalt::proxy::core::Ctx ctx) {
    char info[512];
    basalt::proxy::values::Value map;
    if (!open_map(net::packet_get(ctx, info, sizeof info), info, map)) {
        return;
    }
    if (!refused_once && net::packet_cancel(ctx) < 0) {
        refused_once = true;
        basalt::proxy::log::log_info("net: observer cancel refused");
    }
    if (map.find("cancelled").bool_or(false)) {
        basalt::proxy::log::log_info("net: watch saw a cancelled packet");
    }
}

static void on_antibot(basalt::proxy::core::Ctx ctx) {
    char buffer[512];
    basalt::proxy::values::Value map;
    if (!open_map(net::antibot_event_get(ctx, buffer, sizeof buffer), buffer, map)) {
        return;
    }
    const char* kind;
    const char* action;
    int kind_length;
    int action_length;
    text_of(map, "kind", kind, kind_length);
    text_of(map, "action", action, action_length);
    char line[160];
    int n = 0;
    for (int i = 0; i < kind_length && n < 70; ++i) {
        line[n++] = kind[i];
    }
    line[n++] = ' ';
    for (int i = 0; i < action_length && n < 150; ++i) {
        line[n++] = action[i];
    }
    line[n] = 0;
    info_with("net: antibot ", line, n);
}

static void idle(basalt::proxy::core::Ctx) {}

static void on_join(basalt::proxy::core::Ctx ctx) {
    const basalt::proxy::core::Player player = basalt::proxy::events::event_player(ctx);
    if (player <= 0) {
        return;
    }
    unsigned char body[256];
    int n = chat_body(body, "net welcome");
    const auto* raw = reinterpret_cast<const char*>(body);
    if (net::packet_send(player, "clientbound.system_chat", raw, n) == 0) {
        basalt::proxy::log::log_info("net: sent welcome");
    }
    if (net::packet_send(player, "serverbound.play.chat_command", raw, n) < 0) {
        basalt::proxy::log::log_info("net: wrong direction refused");
    }
    if (net::message_send(player, channel.c_str(), "hello", 5, true) == 0) {
        basalt::proxy::log::log_info("net: sent hello message");
    }
    n = string_prefixed(body, channel.c_str(), "ping");
    if (net::packet_receive(player, "serverbound.play.custom_payload", raw, n) == 0) {
        basalt::proxy::log::log_info("net: injected custom payload");
    }
    n = string_prefixed(body, "list", "");
    if (net::packet_receive(player, "serverbound.play.chat_command", raw, n) == 0) {
        basalt::proxy::log::log_info("net: injected command");
    }
}

static void on_enable(basalt::proxy::core::Ctx) {
    char buffer[8192];
    basalt::proxy::values::Value map;
    if (open_map(net::packet_names(buffer, sizeof buffer), buffer, map) && map.count() > 100) {
        basalt::proxy::log::log_info("net: packets known");
    }
    if (open_map(net::packet_resolve("clientbound.system_chat", buffer, sizeof buffer), buffer, map) &&
        !map.find("serverbound").bool_or(true)) {
        basalt::proxy::log::log_info("net: resolved a clientbound play packet");
    }
    if (net::packet_resolve("nope.nothing", buffer, sizeof buffer) < 0) {
        basalt::proxy::log::log_info("net: unknown packet refused");
    }
    if (net::channel_register(channel.c_str()) == 0) {
        basalt::proxy::log::log_info("net: channel registered");
    }
    if (net::channel_register("BungeeCord") < 0) {
        basalt::proxy::log::log_info("net: bungee channel refused");
    }
    if (net::channel_register("basalt:temp") == 0 && net::channel_unregister("basalt:temp") == 0 &&
        net::channel_unregister("basalt:temp") < 0) {
        basalt::proxy::log::log_info("net: temp channel unregistered");
    }
    net::message_on("basalt:demo", on_message);
    basalt::proxy::values::ValueBuffer<96> opts;
    opts.map().key("priority").string("late").key("fail").string("closed").end();
    const int length = opts.finish();
    if (net::packet_on("clientbound.system_chat", opts.data(), length, on_decider) > 0) {
        basalt::proxy::log::log_info("net: decider listening");
    }
    net::packet_watch("clientbound.system_chat", nullptr, 0, on_watch);
    const int listener = net::packet_on("serverbound.play.*", nullptr, 0, idle);
    if (listener > 0 && net::packet_remove(listener) == 0 && net::packet_remove(listener) < 0) {
        basalt::proxy::log::log_info("net: listener removed");
    }
    if (net::packet_on("nope.nothing", nullptr, 0, idle) < 0) {
        basalt::proxy::log::log_info("net: bad listener name refused");
    }
    if (net::packet_get(9999, buffer, sizeof buffer) < 0) {
        basalt::proxy::log::log_info("net: bad context refused");
    }
    net::antibot_on(on_antibot);
    net::antibot_verify("NetBot");
    basalt::proxy::log::log_info(net::antibot_is_verified("NetBot") == 1 ? "net: verified yes" : "net: verified no");
    net::antibot_unverify("NetBot");
    basalt::proxy::log::log_info(net::antibot_is_verified("NetBot") == 0 ? "net: unverified" : "net: still verified");
    if (net::antibot_ban("203.0.113.9") == 0) {
        basalt::proxy::log::log_info("net: ban ok");
    }
    if (net::antibot_unban("203.0.113.9") == 0) {
        basalt::proxy::log::log_info("net: unban ok");
    }
    if (net::antibot_unban("203.0.113.9") < 0) {
        basalt::proxy::log::log_info("net: unban absent");
    }
    if (net::antibot_ban("not an address") < 0) {
        basalt::proxy::log::log_info("net: bad address refused");
    }
    basalt::proxy::log::log_info(net::antibot_under_attack() == 0 ? "net: no attack" : "net: under attack");
}

void plugin_init() {
    rewrite_marker = basalt::proxy::config::config_str("rewrite-marker");
    cancel_marker = basalt::proxy::config::config_str("cancel-marker");
    channel = basalt::proxy::config::config_str("channel");
    basalt::proxy::events::on("plugin.enable", basalt::proxy::core::kNormal, on_enable);
    basalt::proxy::events::on("player.join", basalt::proxy::core::kNormal, on_join);
}
