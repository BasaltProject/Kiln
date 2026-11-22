use basalt_proxy::network as net;
use basalt_proxy::values::{Value, Writer};
use basalt_proxy::core::Ctx;

thread_local! {
    static CONF: std::cell::RefCell<(String, String, String)> = std::cell::RefCell::new(Default::default());
    static REFUSED: std::cell::Cell<bool> = std::cell::Cell::new(false);
}

pub fn init() {
    CONF.with(|c| {
        *c.borrow_mut() = (
            basalt_proxy::config::str("rewrite-marker").unwrap_or_default(),
            basalt_proxy::config::str("cancel-marker").unwrap_or_default(),
            basalt_proxy::config::str("channel").unwrap_or_default(),
        )
    });
    basalt_proxy::events::on("plugin.enable", basalt_proxy::core::NORMAL, on_enable);
    basalt_proxy::events::on("player.join", basalt_proxy::core::NORMAL, on_join);
}

fn find(hay: &[u8], needle: &str) -> Option<usize> {
    let n = needle.as_bytes();
    if n.is_empty() || hay.len() < n.len() {
        return None;
    }
    (0..=hay.len() - n.len()).find(|&i| &hay[i..i + n.len()] == n)
}

fn text(map: &Value, key: &str) -> String {
    map.find(key).and_then(|v| v.as_str().map(String::from)).unwrap_or_default()
}

fn chat_body(text: &str) -> Vec<u8> {
    let mut out = vec![0x08, (text.len() >> 8) as u8, text.len() as u8];
    out.extend_from_slice(text.as_bytes());
    out.push(0);
    out
}

fn string_prefixed(first: &str, rest: &str) -> Vec<u8> {
    let mut out = vec![first.len() as u8];
    out.extend_from_slice(first.as_bytes());
    out.extend_from_slice(rest.as_bytes());
    out
}

fn on_message(ctx: Ctx) {
    let Ok(doc) = net::message_get(ctx) else {
        basalt_proxy::log::error("message read failed");
        return;
    };
    let Some(map) = Value::open(&doc) else { return };
    let data = map.find("data").and_then(|v| v.as_bytes()).unwrap_or(&[]);
    basalt_proxy::log::info(&format!(
        "net: message {} {} {}",
        text(&map, "source"),
        text(&map, "channel"),
        String::from_utf8_lossy(data)
    ));
    let player = basalt_proxy::events::player(ctx);
    if player > 0 && net::message_send(player, "basalt:demo", b"pong", true) == 0 {
        basalt_proxy::log::info("net: reply sent");
    }
}

fn on_decider(ctx: Ctx) {
    let (Ok(info), Ok(body)) = (net::packet_get(ctx), net::packet_body(ctx)) else {
        basalt_proxy::log::error("packet read failed");
        return;
    };
    let Some(map) = Value::open(&info) else { return };
    let (rewrite, cancel, _) = CONF.with(|c| c.borrow().clone());
    if find(&body, &cancel).is_some() {
        if net::packet_cancel(ctx) == 0 {
            basalt_proxy::log::info(&format!("net: cancel for {}", text(&map, "player_name")));
        }
    } else if let Some(at) = find(&body, &rewrite) {
        let mut edited = body.clone();
        edited[at..at + rewrite.len()].make_ascii_uppercase();
        if net::packet_set_body(ctx, &edited) == 0 {
            basalt_proxy::log::info(&format!("net: rewrite for {}", text(&map, "player_name")));
        }
    }
}

fn on_watch(ctx: Ctx) {
    let Ok(info) = net::packet_get(ctx) else { return };
    if !REFUSED.with(|r| r.get()) && net::packet_cancel(ctx) < 0 {
        REFUSED.with(|r| r.set(true));
        basalt_proxy::log::info("net: observer cancel refused");
    }
    if Value::open(&info).and_then(|m| m.find("cancelled")).and_then(|v| v.as_bool()) == Some(true) {
        basalt_proxy::log::info("net: watch saw a cancelled packet");
    }
}

fn on_antibot(ctx: Ctx) {
    let Ok(doc) = net::antibot_event_get(ctx) else { return };
    if let Some(map) = Value::open(&doc) {
        basalt_proxy::log::info(&format!("net: antibot {} {}", text(&map, "kind"), text(&map, "action")));
    }
}

fn on_join(ctx: Ctx) {
    let player = basalt_proxy::events::player(ctx);
    if player <= 0 {
        return;
    }
    let channel = CONF.with(|c| c.borrow().2.clone());
    let body = chat_body("net welcome");
    if net::packet_send(player, "clientbound.system_chat", &body) == 0 {
        basalt_proxy::log::info("net: sent welcome");
    }
    if net::packet_send(player, "serverbound.play.chat_command", &body) < 0 {
        basalt_proxy::log::info("net: wrong direction refused");
    }
    if net::message_send(player, &channel, b"hello", true) == 0 {
        basalt_proxy::log::info("net: sent hello message");
    }
    if net::packet_receive(player, "serverbound.play.custom_payload", &string_prefixed(&channel, "ping")) == 0 {
        basalt_proxy::log::info("net: injected custom payload");
    }
    if net::packet_receive(player, "serverbound.play.chat_command", &string_prefixed("list", "")) == 0 {
        basalt_proxy::log::info("net: injected command");
    }
}

fn on_enable(_: Ctx) {
    let channel = CONF.with(|c| c.borrow().2.clone());
    if let Ok(doc) = net::packet_names() {
        if Value::open(&doc).and_then(|v| v.count()).unwrap_or(0) > 100 {
            basalt_proxy::log::info("net: packets known");
        }
    }
    if let Ok(doc) = net::packet_resolve("clientbound.system_chat") {
        if Value::open(&doc).and_then(|m| m.find("serverbound")).and_then(|v| v.as_bool()) == Some(false) {
            basalt_proxy::log::info("net: resolved a clientbound play packet");
        }
    }
    if net::packet_resolve("nope.nothing").is_err() {
        basalt_proxy::log::info("net: unknown packet refused");
    }
    if net::channel_register(&channel) == 0 {
        basalt_proxy::log::info("net: channel registered");
    }
    if net::channel_register("BungeeCord") < 0 {
        basalt_proxy::log::info("net: bungee channel refused");
    }
    if net::channel_register("basalt:temp") == 0
        && net::channel_unregister("basalt:temp") == 0
        && net::channel_unregister("basalt:temp") < 0
    {
        basalt_proxy::log::info("net: temp channel unregistered");
    }
    net::message_on("basalt:demo", on_message);
    let mut opts = Writer::new();
    opts.begin_map().key("priority").string("late").key("fail").string("closed").end();
    let opts = opts.finish().unwrap_or(&[]).to_vec();
    if net::packet_on("clientbound.system_chat", &opts, on_decider) > 0 {
        basalt_proxy::log::info("net: decider listening");
    }
    net::packet_watch("clientbound.system_chat", &[], on_watch);
    let listener = net::packet_on("serverbound.play.*", &[], |_| {});
    if listener > 0 && net::packet_remove(listener) == 0 && net::packet_remove(listener) < 0 {
        basalt_proxy::log::info("net: listener removed");
    }
    if net::packet_on("nope.nothing", &[], |_| {}) < 0 {
        basalt_proxy::log::info("net: bad listener name refused");
    }
    if net::packet_get(9999).is_err() {
        basalt_proxy::log::info("net: bad context refused");
    }
    net::antibot_on(on_antibot);
    net::antibot_verify("NetBot");
    basalt_proxy::log::info(if net::antibot_is_verified("NetBot") == 1 { "net: verified yes" } else { "net: verified no" });
    net::antibot_unverify("NetBot");
    basalt_proxy::log::info(if net::antibot_is_verified("NetBot") == 0 { "net: unverified" } else { "net: still verified" });
    if net::antibot_ban("203.0.113.9") == 0 {
        basalt_proxy::log::info("net: ban ok");
    }
    if net::antibot_unban("203.0.113.9") == 0 {
        basalt_proxy::log::info("net: unban ok");
    }
    if net::antibot_unban("203.0.113.9") < 0 {
        basalt_proxy::log::info("net: unban absent");
    }
    if net::antibot_ban("not an address") < 0 {
        basalt_proxy::log::info("net: bad address refused");
    }
    basalt_proxy::log::info(if net::antibot_under_attack() == 0 { "net: no attack" } else { "net: under attack" });
}
