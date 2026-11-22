use crate::core::*;
use crate::raw;
#[path = "../common/data.rs"]
mod shared;
pub use shared::*;

pub fn store_top(key: &str, limit: i32) -> Vec<(String, i64)> {
    let (k, kl) = p(key);
    let mut buffer = vec![0u8; 1024];
    let needed = loop {
        let needed = unsafe { raw::store_top(k, kl, limit, buffer.as_mut_ptr(), buffer.len() as i32) };
        if needed < 0 {
            return Vec::new();
        }
        if needed as usize <= buffer.len() {
            break needed as usize;
        }
        buffer.resize(needed as usize, 0);
    };
    let mut rows = Vec::new();
    let mut at = 0;
    while at + 4 <= needed {
        let name_len = u32::from_le_bytes(buffer[at..at + 4].try_into().unwrap()) as usize;
        at += 4;
        if at + name_len + 8 > needed {
            break;
        }
        let name = String::from_utf8_lossy(&buffer[at..at + name_len]).into_owned();
        at += name_len;
        let value = i64::from_le_bytes(buffer[at..at + 8].try_into().unwrap());
        at += 8;
        rows.push((name, value));
    }
    rows
}
