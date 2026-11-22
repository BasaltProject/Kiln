use basalt_proxy::events as pe;
use basalt_proxy::core::{Args, Ctx};

thread_local! {
    static BLOCKED: std::cell::RefCell<String> = std::cell::RefCell::new(String::new());
}

pub fn init() {
    BLOCKED.with(|blocked| *blocked.borrow_mut() = basalt_proxy::config::str("blocked-word").unwrap_or_default());
    basalt_proxy::events::on("plugin.enable", basalt_proxy::core::NORMAL, on_enable);
    basalt_proxy::events::on("player.chat", basalt_proxy::core::NORMAL, on_chat);
    pe::watch("player.chat", on_chat_seen);
    basalt_proxy::events::on("player.command", basalt_proxy::core::NORMAL, on_command);
    basalt_proxy::events::on("player.kick", basalt_proxy::core::NORMAL, on_kick);
    pe::watch("player.kick", on_kick_seen);
    basalt_proxy::commands::register("proxyevents <factor: float> <flag: bool>", "", on_command_run);
}

fn on_enable(_: Ctx) {
    if let Some(id) = pe::plugin_info("id") {
        basalt_proxy::log::info("plugin id:");
        basalt_proxy::log::info(&id);
    }
    if let Some(version) = pe::plugin_info("version") {
        basalt_proxy::log::info("plugin version:");
        basalt_proxy::log::info(&version);
    }
    if pe::plugin_manifest().is_ok() {
        basalt_proxy::log::info("manifest ok");
    }
    if pe::config_value("banned-words").is_ok() {
        basalt_proxy::log::info("config array ok");
    }
    BLOCKED.with(|blocked| basalt_proxy::log::info(&blocked.borrow()));
}

fn on_chat(ctx: Ctx) {
    let Some(message) = pe::event_get_str(ctx, "message") else { return };
    if pe::event_fields(ctx).is_err() {
        basalt_proxy::log::error("event fields failed");
    }
    if message == "ping" {
        pe::event_set_text(ctx, "message", "pong");
        basalt_proxy::log::info("chat rewritten");
    } else if BLOCKED.with(|blocked| *blocked.borrow() == message) {
        pe::event_deny(ctx, "<red>Watch your language");
        basalt_proxy::log::info("chat denied");
    }
}

fn on_chat_seen(ctx: Ctx) {
    if let Some(message) = pe::event_get_str(ctx, "message") {
        basalt_proxy::log::info("chat seen:");
        basalt_proxy::log::info(&message);
    }
}

fn on_command(ctx: Ctx) {
    if pe::event_get_str(ctx, "command").as_deref() == Some("secret") {
        pe::event_deny(ctx, "<red>No secrets here");
        basalt_proxy::log::info("command denied");
    }
}

fn on_kick(ctx: Ctx) {
    pe::event_set_text(ctx, "reason", "<yellow>Kicked by the backend, come back soon");
    if pe::event_set_text(ctx, "server", "nowhere") < 0 {
        basalt_proxy::log::info("redirect refused");
    }
    if pe::event_set_flag(ctx, "cancelled", true) < 0 {
        basalt_proxy::log::info("kick cannot be cancelled");
    }
}

fn on_kick_seen(ctx: Ctx) {
    if let Some(reason) = pe::event_get_str(ctx, "reason") {
        basalt_proxy::log::info("kick seen:");
        basalt_proxy::log::info(&reason);
    }
}

fn on_command_run(ctx: Ctx) {
    let (Some(factor), Some(flag)) = (pe::command_arg_f64(ctx, "factor"), pe::command_arg_bool(ctx, "flag")) else {
        basalt_proxy::commands::reply(ctx, "<red>bad arguments", &Args::new());
        return;
    };
    let args = Args::new().add("size", if factor > 1.5 { "big" } else { "small" }).add("flag", if flag { "yes" } else { "no" });
    basalt_proxy::commands::reply(ctx, "<green>factor is {size}, flag is {flag}", &args);
}
