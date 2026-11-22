//! Online players: lookup by name, names and MiniMessage text sent to them.
use crate::core::{fetch, p, with_args, Args, Player};
use crate::raw;

pub fn by_name(name: &str) -> Player {
    let (n, nl) = p(name);
    unsafe { raw::player_by_name(n, nl) }
}

pub fn online() -> Vec<Player> {
    let mut out = vec![0i32; 64];
    loop {
        let count = unsafe { raw::players_online(out.as_mut_ptr(), out.len() as i32) };
        if count < 0 {
            return Vec::new();
        }
        if count as usize <= out.len() {
            out.truncate(count as usize);
            return out;
        }
        out.resize(count as usize, 0);
    }
}

pub fn name(player: Player) -> Option<String> {
    fetch(|out, cap| unsafe { raw::player_name(player, out, cap) })
}

pub fn send(player: Player, text: &str, args: &Args) -> i32 {
    let (t, tl) = p(text);
    with_args(args, |a, al| unsafe { raw::player_send(player, t, tl, a, al) })
}

pub fn action_bar(player: Player, text: &str, args: &Args) -> i32 {
    let (t, tl) = p(text);
    with_args(args, |a, al| unsafe { raw::player_action_bar(player, t, tl, a, al) })
}
