#include <basalt/proxy.hpp>

static bool same(const char* a, const char* b) {
    while (*a != 0 && *a == *b) {
        ++a;
        ++b;
    }
    return *a == *b;
}

static basalt::proxy::core::Text<256> blocked;

static void on_enable(basalt::proxy::core::Ctx) {
    char buffer[512];
    const auto id = basalt::proxy::events::plugin_info("id");
    const auto version = basalt::proxy::events::plugin_info("version");
    if (id.ok()) {
        basalt::proxy::log::log_info("plugin id:");
        basalt::proxy::log::log_info(id.c_str());
    }
    if (version.ok()) {
        basalt::proxy::log::log_info("plugin version:");
        basalt::proxy::log::log_info(version.c_str());
    }
    if (basalt::proxy::events::plugin_manifest(buffer, sizeof buffer) > 0) {
        basalt::proxy::log::log_info("manifest ok");
    }
    if (basalt::proxy::events::config_value("banned-words", buffer, sizeof buffer) > 0) {
        basalt::proxy::log::log_info("config array ok");
    }
    basalt::proxy::log::log_info(blocked.c_str());
}

static void on_chat(basalt::proxy::core::Ctx ctx) {
    const auto message = basalt::proxy::events::event_get_str(ctx, "message");
    char fields[2048];
    if (!message.ok()) {
        return;
    }
    if (basalt::proxy::events::event_fields(ctx, fields, sizeof fields) < 0) {
        basalt::proxy::log::log_error("event fields failed");
    }
    if (same(message.c_str(), "ping")) {
        basalt::proxy::events::event_set_text(ctx, "message", "pong");
        basalt::proxy::log::log_info("chat rewritten");
    } else if (same(message.c_str(), blocked.c_str())) {
        basalt::proxy::events::event_deny(ctx, "<red>Watch your language");
        basalt::proxy::log::log_info("chat denied");
    }
}

static void on_chat_seen(basalt::proxy::core::Ctx ctx) {
    const auto message = basalt::proxy::events::event_get_str(ctx, "message");
    if (message.ok()) {
        basalt::proxy::log::log_info("chat seen:");
        basalt::proxy::log::log_info(message.c_str());
    }
}

static void on_command(basalt::proxy::core::Ctx ctx) {
    const auto line = basalt::proxy::events::event_get_str(ctx, "command");
    if (line.ok() && same(line.c_str(), "secret")) {
        basalt::proxy::events::event_deny(ctx, "<red>No secrets here");
        basalt::proxy::log::log_info("command denied");
    }
}

static void on_kick(basalt::proxy::core::Ctx ctx) {
    basalt::proxy::events::event_set_text(ctx, "reason", "<yellow>Kicked by the backend, come back soon");
    if (basalt::proxy::events::event_set_text(ctx, "server", "nowhere") < 0) {
        basalt::proxy::log::log_info("redirect refused");
    }
    if (basalt::proxy::events::event_set_flag(ctx, "cancelled", true) < 0) {
        basalt::proxy::log::log_info("kick cannot be cancelled");
    }
}

static void on_kick_seen(basalt::proxy::core::Ctx ctx) {
    const auto reason = basalt::proxy::events::event_get_str(ctx, "reason");
    if (reason.ok()) {
        basalt::proxy::log::log_info("kick seen:");
        basalt::proxy::log::log_info(reason.c_str());
    }
}

static void on_command_run(basalt::proxy::core::Ctx ctx) {
    double factor = 0;
    bool flag = false;
    if (basalt::proxy::events::command_arg_f64(ctx, "factor", &factor) < 0 ||
        basalt::proxy::events::command_arg_bool(ctx, "flag", &flag) < 0) {
        basalt::proxy::commands::command_reply(ctx, "<red>bad arguments");
        return;
    }
    basalt::proxy::core::Args args;
    args.add("size", factor > 1.5 ? "big" : "small").add("flag", flag ? "yes" : "no");
    basalt::proxy::commands::command_reply(ctx, "<green>factor is {size}, flag is {flag}", &args);
}

void plugin_init() {
    blocked = basalt::proxy::config::config_str("blocked-word");
    basalt::proxy::events::on("plugin.enable", basalt::proxy::core::kNormal, on_enable);
    basalt::proxy::events::on("player.chat", basalt::proxy::core::kNormal, on_chat);
    basalt::proxy::events::watch("player.chat", on_chat_seen);
    basalt::proxy::events::on("player.command", basalt::proxy::core::kNormal, on_command);
    basalt::proxy::events::on("player.kick", basalt::proxy::core::kNormal, on_kick);
    basalt::proxy::events::watch("player.kick", on_kick_seen);
    basalt::proxy::commands::command("proxyevents <factor: float> <flag: bool>", "", on_command_run);
}
