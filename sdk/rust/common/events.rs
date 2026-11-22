//! Event listeners and the fields of the event being handled.
use crate::core::{fetch, p, token_for, Ctx, Player, Priority};
use crate::raw;

pub fn on(event: &str, priority: i32, handler: impl FnMut(Ctx) + 'static) -> i32 {
    let (e, el) = p(event);
    unsafe { raw::event_on(e, el, priority as i32, token_for(Box::new(handler))) }
}

pub fn player(ctx: Ctx) -> Player {
    unsafe { raw::event_player(ctx) }
}

pub fn field_str(ctx: Ctx, field: &str) -> Option<String> {
    let (f, fl) = p(field);
    fetch(|out, cap| unsafe { raw::event_field_str(ctx, f, fl, out, cap) })
}

pub fn field_i64(ctx: Ctx, field: &str) -> Option<i64> {
    let (f, fl) = p(field);
    let mut value = 0i64;
    if unsafe { raw::event_field_i64(ctx, f, fl, &mut value) } == 0 {
        Some(value)
    } else {
        None
    }
}

pub fn cancel(ctx: Ctx, cancelled: bool) -> i32 {
    unsafe { raw::event_cancel(ctx, cancelled as i32) }
}
