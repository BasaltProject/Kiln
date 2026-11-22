#include <basalt/proxy.h>

static int same(const char *a, const char *b) {
    while (*a && *a == *b) {
        ++a;
        ++b;
    }
    return *a == *b;
}

static char blocked[64];

static void log_field(const char *prefix, const char *field) {
    char text[128];
    const int n = bs_proxy_plugin_info(field, text, sizeof text - 1);
    if (n >= 0) {
        text[n] = 0;
        bs_info(prefix);
        bs_info(text);
    }
}

static void on_enable(bs_ctx ctx) {
    (void)ctx;
    char buffer[512];
    log_field("plugin id:", "id");
    log_field("plugin version:", "version");
    if (bs_proxy_plugin_manifest(buffer, sizeof buffer) > 0) {
        bs_info("manifest ok");
    }
    if (bs_proxy_config_value("banned-words", buffer, sizeof buffer) > 0) {
        bs_info("config array ok");
    }
    bs_info(blocked);
}

static void on_chat(bs_ctx ctx) {
    char message[256];
    char fields[2048];
    const int n = bs_proxy_event_get_str(ctx, "message", message, sizeof message - 1);
    if (n < 0) {
        return;
    }
    message[n] = 0;
    if (bs_proxy_event_fields(ctx, fields, sizeof fields) < 0) {
        bs_error("event fields failed");
    }
    if (same(message, "ping")) {
        bs_proxy_event_set_text(ctx, "message", "pong");
        bs_info("chat rewritten");
    } else if (same(message, blocked)) {
        bs_proxy_event_deny(ctx, "<red>Watch your language");
        bs_info("chat denied");
    }
}

static void on_chat_seen(bs_ctx ctx) {
    char message[256];
    const int n = bs_proxy_event_get_str(ctx, "message", message, sizeof message - 1);
    if (n >= 0) {
        message[n] = 0;
        bs_info("chat seen:");
        bs_info(message);
    }
}

static void on_command(bs_ctx ctx) {
    char line[128];
    const int n = bs_proxy_event_get_str(ctx, "command", line, sizeof line - 1);
    if (n >= 0) {
        line[n] = 0;
        if (same(line, "secret")) {
            bs_proxy_event_deny(ctx, "<red>No secrets here");
            bs_info("command denied");
        }
    }
}

static void on_kick(bs_ctx ctx) {
    bs_proxy_event_set_text(ctx, "reason", "<yellow>Kicked by the backend, come back soon");
    if (bs_proxy_event_set_text(ctx, "server", "nowhere") < 0) {
        bs_info("redirect refused");
    }
    if (bs_proxy_event_set_flag(ctx, "cancelled", 1) < 0) {
        bs_info("kick cannot be cancelled");
    }
}

static void on_kick_seen(bs_ctx ctx) {
    char reason[256];
    const int n = bs_proxy_event_get_str(ctx, "reason", reason, sizeof reason - 1);
    if (n >= 0) {
        reason[n] = 0;
        bs_info("kick seen:");
        bs_info(reason);
    }
}

static void on_command_run(bs_ctx ctx) {
    double factor = 0;
    int flag = 0;
    bs_args args;
    bs_args_init(&args);
    if (bs_proxy_command_arg_f64(ctx, "factor", &factor) < 0 || bs_proxy_command_arg_bool(ctx, "flag", &flag) < 0) {
        bs_command_reply(ctx, "<red>bad arguments", 0);
        return;
    }
    bs_args_add(&args, "size", factor > 1.5 ? "big" : "small");
    bs_args_add(&args, "flag", flag ? "yes" : "no");
    bs_command_reply(ctx, "<green>factor is {size}, flag is {flag}", &args);
}

void plugin_init(void) {
    const int n = bs_config_str("blocked-word", blocked, sizeof blocked - 1);
    blocked[n > 0 ? n : 0] = 0;
    bs_on("plugin.enable", 2, on_enable);
    bs_on("player.chat", 2, on_chat);
    bs_proxy_event_watch("player.chat", on_chat_seen);
    bs_on("player.command", 2, on_command);
    bs_on("player.kick", 2, on_kick);
    bs_proxy_event_watch("player.kick", on_kick_seen);
    bs_command("proxyevents <factor: float> <flag: bool>", "", on_command_run);
}
