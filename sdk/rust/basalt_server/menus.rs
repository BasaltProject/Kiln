use crate::core::*;
use crate::raw;

pub fn placeholder(name: &str, handler: impl FnMut(Ctx) + 'static) -> i32 {
    let (n, nl) = p(name);
    unsafe { raw::placeholder_register(n, nl, token_for(Box::new(handler))) }
}

pub fn placeholder_player(ctx: Ctx) -> Player {
    unsafe { raw::placeholder_player(ctx) }
}

pub fn placeholder_result(ctx: Ctx, text: &str) -> i32 {
    let (t, tl) = p(text);
    unsafe { raw::placeholder_result(ctx, t, tl) }
}

pub fn menu_create(title: &str, rows: i32) -> i32 {
    let (t, tl) = p(title);
    unsafe { raw::menu_create(t, tl, rows) }
}

pub fn menu_set(menu: i32, slot: i32, item: &str, name: &str, lore: &str, on_click: impl FnMut(Ctx) + 'static) -> i32 {
    let (i, il) = p(item);
    let (n, nl) = p(name);
    let (l, ll) = p(lore);
    unsafe { raw::menu_set(menu, slot, i, il, n, nl, l, ll, token_for(Box::new(on_click))) }
}

pub fn menu_open(menu: i32, player: Player) -> i32 {
    unsafe { raw::menu_open(menu, player) }
}

pub fn menu_click_player(ctx: Ctx) -> Player {
    unsafe { raw::menu_click_player(ctx) }
}
