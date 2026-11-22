//! Commands declared by a spec such as `coins top [limit: int 1..25]`, their arguments and replies.
use crate::core::{fetch, p, token_for, with_args, Args, Ctx};
use crate::raw;

pub fn register(spec: &str, permission: &str, handler: impl FnMut(Ctx) + 'static) -> i32 {
    let (s, sl) = p(spec);
    let (pm, pl) = p(permission);
    unsafe { raw::command_register(s, sl, pm, pl, token_for(Box::new(handler))) }
}

pub fn reply(ctx: Ctx, text: &str, args: &Args) -> i32 {
    let (t, tl) = p(text);
    with_args(args, |a, al| unsafe { raw::command_reply(ctx, t, tl, a, al) })
}

pub fn arg_str(ctx: Ctx, name: &str) -> Option<String> {
    let (n, nl) = p(name);
    fetch(|out, cap| unsafe { raw::command_arg_str(ctx, n, nl, out, cap) })
}

pub fn arg_i64(ctx: Ctx, name: &str) -> Option<i64> {
    let (n, nl) = p(name);
    let mut value = 0i64;
    if unsafe { raw::command_arg_i64(ctx, n, nl, &mut value) } == 0 {
        Some(value)
    } else {
        None
    }
}

pub fn source_name(ctx: Ctx) -> Option<String> {
    fetch(|out, cap| unsafe { raw::command_source_name(ctx, out, cap) })
}

pub fn is_console(ctx: Ctx) -> bool {
    unsafe { raw::command_is_console(ctx) != 0 }
}
