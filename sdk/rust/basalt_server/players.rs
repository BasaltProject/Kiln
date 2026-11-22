use crate::core::*;
use crate::raw;
#[path = "../common/players.rs"]
mod shared;
pub use shared::*;

pub fn particle(player: Player, id: &str, count: i32, spread: f64) -> i32 {
    let (i, il) = p(id);
    unsafe { raw::player_particle(player, i, il, count, spread) }
}

pub fn sound(player: Player, id: &str, volume: f32, pitch: f32) -> i32 {
    let (i, il) = p(id);
    unsafe { raw::player_sound(player, i, il, volume, pitch) }
}
