//! Items area: an item stack builder and an Inventory over the inventory handles.
#![allow(dead_code)]

pub use crate::raw::items as raw;
use crate::values::{Value, Writer};
use crate::core::{token_for, Handler, Player};

const ERROR_INVALID: i32 = -3;
const ITEM_BUFFER: usize = 512;

fn p(text: &str) -> (*const u8, i32) {
    (text.as_ptr(), text.len() as i32)
}

fn out(buffer: &mut [u8]) -> (*mut u8, i32) {
    (buffer.as_mut_ptr(), buffer.len() as i32)
}

/// An item stack document, ready to pass to any import that takes a stack.
pub struct Item {
    document: Result<Vec<u8>, i32>,
}

impl Item {
    pub fn new(id: &str, count: i32, name: Option<&str>) -> Item {
        let mut writer = Writer::with_limit(ITEM_BUFFER);
        writer.begin_map().key("id").string(id).key("count").int(count as i64);
        if let Some(text) = name {
            writer.key("name").string(text);
        }
        writer.end();
        Item { document: writer.finish().map(|bytes| bytes.to_vec()).map_err(|_| ERROR_INVALID) }
    }

    pub fn ok(&self) -> bool {
        self.document.is_ok()
    }

    pub fn bytes(&self) -> &[u8] {
        self.document.as_deref().unwrap_or(&[])
    }
}

/// A handle on a player, block, ender chest, window or entity inventory. Released when dropped.
pub struct Inventory {
    handle: i32,
}

impl Inventory {
    pub fn of_player(player: Player) -> Inventory {
        Inventory { handle: unsafe { raw::inventory_of_player(player) } }
    }

    pub fn of_window(player: Player) -> Inventory {
        Inventory { handle: unsafe { raw::inventory_of_window(player) } }
    }

    pub fn of_ender(player: Player) -> Inventory {
        Inventory { handle: unsafe { raw::inventory_of_ender(player) } }
    }

    pub fn of_entity(entity: i32) -> Inventory {
        Inventory { handle: unsafe { raw::inventory_of_entity(entity) } }
    }

    pub fn of_block(world: &str, x: i32, y: i32, z: i32) -> Inventory {
        let (w, wl) = p(world);
        Inventory { handle: unsafe { raw::inventory_of_block(w, wl, x, y, z) } }
    }

    /// The handle, or the negative error that made it.
    pub fn handle(&self) -> i32 {
        self.handle
    }

    pub fn valid(&self) -> bool {
        self.handle > 0
    }

    pub fn size(&self) -> i32 {
        unsafe { raw::inventory_size(self.handle) }
    }

    pub fn held_slot(&self) -> i32 {
        unsafe { raw::inventory_held_slot(self.handle) }
    }

    pub fn set_held_slot(&self, slot: i32) -> i32 {
        unsafe { raw::inventory_set_held_slot(self.handle, slot) }
    }

    pub fn put_item(&self, slot: i32, item: &Item) -> i32 {
        match &item.document {
            Ok(bytes) => unsafe { raw::inventory_set(self.handle, slot, bytes.as_ptr(), bytes.len() as i32) },
            Err(code) => *code,
        }
    }

    pub fn put(&self, slot: i32, id: &str, count: i32) -> i32 {
        self.put_item(slot, &Item::new(id, count, None))
    }

    pub fn clear_slot(&self, slot: i32) -> i32 {
        let empty = [0u8];
        unsafe { raw::inventory_set(self.handle, slot, empty.as_ptr(), 1) }
    }

    /// Returns how many items did not fit.
    pub fn give_item(&self, item: &Item) -> i32 {
        match &item.document {
            Ok(bytes) => unsafe { raw::inventory_add(self.handle, bytes.as_ptr(), bytes.len() as i32) },
            Err(code) => *code,
        }
    }

    pub fn give(&self, id: &str, count: i32, name: Option<&str>) -> i32 {
        self.give_item(&Item::new(id, count, name))
    }

    pub fn take(&self, id: &str, count: i32) -> i32 {
        let (i, il) = p(id);
        unsafe { raw::inventory_remove(self.handle, i, il, count) }
    }

    pub fn count(&self, id: &str) -> i32 {
        let (i, il) = p(id);
        unsafe { raw::inventory_count(self.handle, i, il) }
    }

    /// The count of the stack in `slot`, 0 for an empty slot, negative for an error.
    pub fn slot_count(&self, slot: i32) -> i32 {
        let mut buffer = [0u8; ITEM_BUFFER];
        let n = self.get(slot, &mut buffer);
        if n < 0 {
            return n;
        }
        if n as usize > ITEM_BUFFER {
            return ERROR_INVALID;
        }
        match Value::open(&buffer[..n as usize]) {
            None => ERROR_INVALID,
            Some(root) if root.is_null() => 0,
            Some(root) => root.find("count").and_then(|count| count.as_i64()).unwrap_or(0) as i32,
        }
    }

    pub fn clear(&self) -> i32 {
        unsafe { raw::inventory_clear(self.handle) }
    }

    /// Writes the slot document to `buffer`; the result is the length it needs.
    pub fn get(&self, slot: i32, buffer: &mut [u8]) -> i32 {
        let (o, cap) = out(buffer);
        unsafe { raw::inventory_get(self.handle, slot, o, cap) }
    }

    pub fn contents(&self, buffer: &mut [u8]) -> i32 {
        let (o, cap) = out(buffer);
        unsafe { raw::inventory_contents(self.handle, o, cap) }
    }

    pub fn release(&mut self) {
        if self.handle > 0 {
            unsafe { raw::inventory_release(self.handle) };
        }
        self.handle = 0;
    }
}

impl Drop for Inventory {
    fn drop(&mut self) {
        self.release();
    }
}

/// Puts an item in a container menu slot; `on_click` runs when a player clicks it.
pub fn menu_slot(menu: i32, slot: i32, id: &str, count: i32, name: Option<&str>, on_click: Option<Handler>) -> i32 {
    let item = Item::new(id, count, name);
    let bytes = match &item.document {
        Ok(bytes) => bytes,
        Err(code) => return *code,
    };
    let token = match on_click {
        Some(handler) => token_for(handler),
        None => 0,
    };
    unsafe { raw::container_menu_set(menu, slot, bytes.as_ptr(), bytes.len() as i32, token) }
}

pub fn item_cooldown(player: Player, id: &str) -> i32 {
    let (i, il) = p(id);
    unsafe { raw::item_cooldown(player, i, il) }
}

pub fn item_set_cooldown(player: Player, id: &str, ticks: i32) -> i32 {
    let (i, il) = p(id);
    unsafe { raw::item_set_cooldown(player, i, il, ticks) }
}

pub fn recipe_known(id: &str) -> bool {
    let (i, il) = p(id);
    unsafe { raw::recipe_has(i, il) > 0 }
}

pub fn registry_get(kind: &str, key: &str, buffer: &mut [u8]) -> i32 {
    let (k, kl) = p(kind);
    let (n, nl) = p(key);
    let (o, cap) = out(buffer);
    unsafe { raw::registry_get(k, kl, n, nl, o, cap) }
}
