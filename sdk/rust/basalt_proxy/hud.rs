
use crate::raw::hud as raw;
use crate::values::Writer;
use crate::core::{p, Player};

const INVALID: i32 = -3;

fn doc(build: impl FnOnce(&mut Writer)) -> Result<Vec<u8>, i32> {
    let mut writer = Writer::new();
    build(&mut writer);
    writer.into_vec().map_err(|_| INVALID)
}

fn str_key(w: &mut Writer, name: &str, text: Option<&str>) {
    if let Some(text) = text {
        w.key(name).string(text);
    }
}

fn send(spec: Result<Vec<u8>, i32>, call: impl FnOnce(*const u8, i32) -> i32) -> i32 {
    match spec {
        Ok(bytes) => call(bytes.as_ptr(), bytes.len() as i32),
        Err(code) => code,
    }
}

pub fn clear_title(player: Player, reset: bool) -> i32 {
    unsafe { raw::hud_clear_title(player, reset as i32) }
}

pub fn tab_list(player: Player, header: Option<&str>, footer: Option<&str>) -> i32 {
    let spec = doc(|w| {
        w.begin_map();
        str_key(w, "header", header);
        str_key(w, "footer", footer);
        w.end();
    });
    send(spec, |d, n| unsafe { raw::hud_tab_list(player, d, n) })
}

pub fn bossbar_show(player: Player, id: &str, text: &str, progress: f64, color: &str, overlay: &str) -> i32 {
    let (i, il) = p(id);
    let spec = doc(|w| {
        w.begin_map();
        str_key(w, "text", Some(text));
        w.key("progress").float(progress);
        str_key(w, "color", Some(color));
        str_key(w, "overlay", Some(overlay));
        w.end();
    });
    send(spec, |d, n| unsafe { raw::hud_bossbar_show(player, i, il, d, n) })
}

pub fn bossbar_hide(player: Player, id: &str) -> i32 {
    let (i, il) = p(id);
    unsafe { raw::hud_bossbar_hide(player, i, il) }
}

pub fn bossbar_progress(player: Player, id: &str, progress: f64) -> i32 {
    let (i, il) = p(id);
    unsafe { raw::hud_bossbar_progress(player, i, il, progress) }
}

pub fn bossbar_title(player: Player, id: &str, text: &str) -> i32 {
    let (i, il) = p(id);
    let (t, tl) = p(text);
    unsafe { raw::hud_bossbar_title(player, i, il, t, tl, t, 0) }
}

pub fn bossbar_style(player: Player, id: &str, color: &str, overlay: &str) -> i32 {
    let (i, il) = p(id);
    let (c, cl) = p(color);
    let (o, ol) = p(overlay);
    unsafe { raw::hud_bossbar_style(player, i, il, c, cl, o, ol) }
}

pub fn sidebar(player: Player, title: &str, lines: &[&str]) -> i32 {
    let spec = doc(|w| {
        w.begin_map();
        str_key(w, "title", Some(title));
        w.key("lines").begin_list();
        for line in lines {
            w.string(line);
        }
        w.end();
        w.end();
    });
    send(spec, |d, n| unsafe { raw::hud_sidebar(player, d, n) })
}

pub fn sidebar_clear(player: Player) -> i32 {
    unsafe { raw::hud_sidebar_clear(player) }
}

pub fn tab_entry(player: Player, id: &str, name: &str, text: &str, latency: i64) -> i32 {
    let (i, il) = p(id);
    let spec = doc(|w| {
        w.begin_map();
        str_key(w, "name", Some(name));
        str_key(w, "text", Some(text));
        w.key("latency").int(latency);
        w.end();
    });
    send(spec, |d, n| unsafe { raw::hud_tab_entry(player, i, il, d, n) })
}

pub fn tab_entry_remove(player: Player, id: &str) -> i32 {
    let (i, il) = p(id);
    unsafe { raw::hud_tab_entry_remove(player, i, il) }
}

/// Each link is a (label, url) pair.
pub fn server_links(player: Player, links: &[(&str, &str)]) -> i32 {
    let spec = doc(|w| {
        w.begin_list();
        for (label, url) in links {
            w.begin_list().string(label).string(url).end();
        }
        w.end();
    });
    send(spec, |d, n| unsafe { raw::hud_server_links(player, d, n) })
}

pub fn chat_completions(player: Player, action: &str, words: &[&str]) -> i32 {
    let (a, al) = p(action);
    let spec = doc(|w| {
        w.begin_list();
        for word in words {
            w.string(word);
        }
        w.end();
    });
    send(spec, |d, n| unsafe { raw::hud_chat_completions(player, a, al, d, n) })
}
