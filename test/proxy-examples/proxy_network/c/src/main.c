#include <basalt/proxy.h>

static int find(const uint8_t *data, int len, const char *needle) {
    int n = 0;
    while (needle[n]) {
        ++n;
    }
    for (int at = 0; at + n <= len; ++at) {
        int i = 0;
        while (i < n && data[at + i] == (uint8_t)needle[i]) {
            ++i;
        }
        if (i == n) {
            return at;
        }
    }
    return -1;
}

static char rewrite_marker[64];
static char cancel_marker[64];
static char channel[64];

static void info2(const char *prefix, const char *text, int len) {
    char line[256];
    int n = 0;
    while (prefix[n] && n < 120) {
        line[n] = prefix[n];
        ++n;
    }
    for (int i = 0; i < len && n < 250; ++i) {
        line[n++] = text[i];
    }
    line[n] = 0;
    bs_info(line);
}

static int open_value(int n, char *buf, bs_val *root) { return n >= 0 && bs_value_open(root, buf, n) == 0; }

static void field_text(bs_val map, const char *key, const char **text, int *len) {
    *text = "";
    *len = 0;
    bs_value_as_str(bs_value_find(map, key), text, (int32_t *)len);
}

static int chat_body(uint8_t *out, const char *text) {
    int n = 0;
    while (text[n]) {
        ++n;
    }
    out[0] = 0x08;
    out[1] = (uint8_t)(n >> 8);
    out[2] = (uint8_t)n;
    for (int i = 0; i < n; ++i) {
        out[3 + i] = (uint8_t)text[i];
    }
    out[3 + n] = 0;
    return n + 4;
}

static int string_prefixed(uint8_t *out, const char *first, const char *rest) {
    int a = 0;
    int b = 0;
    while (first[a]) {
        ++a;
    }
    while (rest[b]) {
        ++b;
    }
    out[0] = (uint8_t)a;
    for (int i = 0; i < a; ++i) {
        out[1 + i] = (uint8_t)first[i];
    }
    for (int i = 0; i < b; ++i) {
        out[1 + a + i] = (uint8_t)rest[i];
    }
    return 1 + a + b;
}

static void on_message(bs_ctx ctx) {
    char buf[1024];
    bs_val map;
    const char *source;
    const char *chan;
    const uint8_t *data = 0;
    int source_len;
    int chan_len;
    int data_len = 0;
    char line[160];
    int n = 0;
    if (!open_value(bs_net_message_get(ctx, buf, sizeof buf), buf, &map)) {
        bs_error("message read failed");
        return;
    }
    field_text(map, "source", &source, &source_len);
    field_text(map, "channel", &chan, &chan_len);
    bs_value_as_bytes(bs_value_find(map, "data"), &data, (int32_t *)&data_len);
    for (int i = 0; i < source_len && n < 40; ++i) {
        line[n++] = source[i];
    }
    line[n++] = ' ';
    for (int i = 0; i < chan_len && n < 80; ++i) {
        line[n++] = chan[i];
    }
    line[n++] = ' ';
    for (int i = 0; i < data_len && n < 150; ++i) {
        line[n++] = (char)data[i];
    }
    line[n] = 0;
    info2("net: message ", line, n);
    {
        const bs_player player = bs_event_player(ctx);
        if (player > 0 && bs_net_message_send(player, "basalt:demo", "pong", 4, 1) == 0) {
            bs_info("net: reply sent");
        }
    }
}

static void on_decider(bs_ctx ctx) {
    char info[512];
    char body[2048];
    bs_val map;
    const char *name;
    int name_len;
    int n;
    int at;
    if (!open_value(bs_net_packet_get(ctx, info, sizeof info), info, &map)) {
        bs_error("packet read failed");
        return;
    }
    field_text(map, "player_name", &name, &name_len);
    n = bs_net_packet_body(ctx, body, sizeof body);
    if (n < 0 || n > (int)sizeof body) {
        return;
    }
    if (find((const uint8_t *)body, n, cancel_marker) >= 0) {
        if (bs_net_packet_cancel(ctx) == 0) {
            info2("net: cancel for ", name, name_len);
        }
    } else if ((at = find((const uint8_t *)body, n, rewrite_marker)) >= 0) {
        for (int k = 0; rewrite_marker[k]; ++k) {
            if (body[at + k] >= 'a' && body[at + k] <= 'z') {
                body[at + k] = (char)(body[at + k] - 32);
            }
        }
        if (bs_net_packet_set_body(ctx, body, n) == 0) {
            info2("net: rewrite for ", name, name_len);
        }
    }
}

static int refused_once;

static void on_watch(bs_ctx ctx) {
    char info[512];
    bs_val map;
    int cancelled = 0;
    if (!open_value(bs_net_packet_get(ctx, info, sizeof info), info, &map)) {
        return;
    }
    bs_value_as_bool(bs_value_find(map, "cancelled"), &cancelled);
    if (!refused_once && bs_net_packet_cancel(ctx) < 0) {
        refused_once = 1;
        bs_info("net: observer cancel refused");
    }
    if (cancelled) {
        bs_info("net: watch saw a cancelled packet");
    }
}

static void on_antibot(bs_ctx ctx) {
    char buf[512];
    bs_val map;
    const char *kind;
    const char *action;
    int kind_len;
    int action_len;
    char line[160];
    int n = 0;
    if (!open_value(bs_net_antibot_event_get(ctx, buf, sizeof buf), buf, &map)) {
        return;
    }
    field_text(map, "kind", &kind, &kind_len);
    field_text(map, "action", &action, &action_len);
    for (int i = 0; i < kind_len && n < 70; ++i) {
        line[n++] = kind[i];
    }
    line[n++] = ' ';
    for (int i = 0; i < action_len && n < 150; ++i) {
        line[n++] = action[i];
    }
    line[n] = 0;
    info2("net: antibot ", line, n);
}

static void spare(bs_ctx ctx) { (void)ctx; }

static void on_join(bs_ctx ctx) {
    const bs_player player = bs_event_player(ctx);
    uint8_t body[256];
    int n;
    if (player <= 0) {
        return;
    }
    n = chat_body(body, "net welcome");
    if (bs_net_packet_send(player, "clientbound.system_chat", (const char *)body, n) == 0) {
        bs_info("net: sent welcome");
    }
    if (bs_net_packet_send(player, "serverbound.play.chat_command", (const char *)body, n) < 0) {
        bs_info("net: wrong direction refused");
    }
    if (bs_net_message_send(player, channel, "hello", 5, 1) == 0) {
        bs_info("net: sent hello message");
    }
    n = string_prefixed(body, channel, "ping");
    if (bs_net_packet_receive(player, "serverbound.play.custom_payload", (const char *)body, n) == 0) {
        bs_info("net: injected custom payload");
    }
    n = string_prefixed(body, "list", "");
    if (bs_net_packet_receive(player, "serverbound.play.chat_command", (const char *)body, n) == 0) {
        bs_info("net: injected command");
    }
}

static void on_enable(bs_ctx ctx) {
    char buf[8192];
    uint8_t opts[96];
    bs_vw writer;
    bs_val map;
    const char *text;
    int len;
    int listener;
    (void)ctx;
    if (open_value(bs_net_packet_names(buf, sizeof buf), buf, &map) && bs_value_count(map) > 100) {
        bs_info("net: packets known");
    }
    if (open_value(bs_net_packet_resolve("clientbound.system_chat", buf, sizeof buf), buf, &map)) {
        const char *phase;
        int phase_len;
        field_text(map, "name", &text, &len);
        field_text(map, "phase", &phase, &phase_len);
        bs_info("net: resolved a clientbound play packet");
        (void)phase;
    }
    if (bs_net_packet_resolve("nope.nothing", buf, sizeof buf) < 0) {
        bs_info("net: unknown packet refused");
    }
    if (bs_net_channel_register(channel) == 0) {
        bs_info("net: channel registered");
    }
    if (bs_net_channel_register("BungeeCord") < 0) {
        bs_info("net: bungee channel refused");
    }
    if (bs_net_channel_register("basalt:temp") == 0 && bs_net_channel_unregister("basalt:temp") == 0 &&
        bs_net_channel_unregister("basalt:temp") < 0) {
        bs_info("net: temp channel unregistered");
    }
    bs_net_message_on("basalt:demo", on_message);
    bs_vw_init(&writer, opts, sizeof opts);
    bs_vw_map(&writer);
    bs_vw_key(&writer, "priority");
    bs_vw_str(&writer, "late");
    bs_vw_key(&writer, "fail");
    bs_vw_str(&writer, "closed");
    bs_vw_end(&writer);
    len = bs_vw_finish(&writer);
    if (bs_net_packet_on("clientbound.system_chat", (const char *)opts, len, on_decider) > 0) {
        bs_info("net: decider listening");
    }
    bs_net_packet_watch("clientbound.system_chat", 0, 0, on_watch);
    listener = bs_net_packet_on("serverbound.play.*", 0, 0, spare);
    if (listener > 0 && bs_net_packet_remove(listener) == 0 && bs_net_packet_remove(listener) < 0) {
        bs_info("net: listener removed");
    }
    if (bs_net_packet_on("nope.nothing", 0, 0, spare) < 0) {
        bs_info("net: bad listener name refused");
    }
    if (bs_net_packet_get(9999, buf, sizeof buf) < 0) {
        bs_info("net: bad context refused");
    }
    bs_net_antibot_on(on_antibot);
    bs_net_antibot_verify("NetBot");
    bs_info(bs_net_antibot_is_verified("NetBot") == 1 ? "net: verified yes" : "net: verified no");
    bs_net_antibot_unverify("NetBot");
    bs_info(bs_net_antibot_is_verified("NetBot") == 0 ? "net: unverified" : "net: still verified");
    if (bs_net_antibot_ban("203.0.113.9") == 0) {
        bs_info("net: ban ok");
    }
    if (bs_net_antibot_unban("203.0.113.9") == 0) {
        bs_info("net: unban ok");
    }
    if (bs_net_antibot_unban("203.0.113.9") < 0) {
        bs_info("net: unban absent");
    }
    if (bs_net_antibot_ban("not an address") < 0) {
        bs_info("net: bad address refused");
    }
    bs_info(bs_net_antibot_under_attack() == 0 ? "net: no attack" : "net: under attack");
}

void plugin_init(void) {
    int n = bs_config_str("rewrite-marker", rewrite_marker, sizeof rewrite_marker - 1);
    rewrite_marker[n > 0 ? n : 0] = 0;
    n = bs_config_str("cancel-marker", cancel_marker, sizeof cancel_marker - 1);
    cancel_marker[n > 0 ? n : 0] = 0;
    n = bs_config_str("channel", channel, sizeof channel - 1);
    channel[n > 0 ? n : 0] = 0;
    bs_on("plugin.enable", 2, on_enable);
    bs_on("player.join", 2, on_join);
}
