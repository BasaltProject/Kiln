//! Handles, text arguments, log levels, event priorities, status codes and the callback table behind every handler.
use std::cell::RefCell;

pub const ABI_VERSION: i32 = 1;

pub type Player = i32;
pub type Ctx = i32;
pub type Handler = Box<dyn FnMut(Ctx)>;

pub const OK: i32 = 0;
pub const ERR_ABSENT: i32 = -1;
pub const ERR_STALE: i32 = -2;
pub const ERR_INVALID: i32 = -3;
pub const ERR_DENIED: i32 = -4;

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
#[repr(i32)]
pub enum Level {
    Debug = 0,
    Info = 1,
    Warn = 2,
    Error = 3,
}

/// Handlers run from `Lowest` to `Highest`.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
#[repr(i32)]
pub enum Priority {
    Lowest = 0,
    Low = 1,
    Normal = 2,
    High = 3,
    Highest = 4,
}

thread_local! {
    static HANDLERS: RefCell<Vec<Handler>> = RefCell::new(Vec::new());
}

pub(crate) fn token_for(handler: Handler) -> i32 {
    HANDLERS.with(|handlers| {
        let mut handlers = handlers.borrow_mut();
        handlers.push(handler);
        handlers.len() as i32
    })
}

#[doc(hidden)]
pub fn dispatch(token: i32, ctx: Ctx) {
    // A handler may register more handlers, so it runs outside the borrow.
    let taken = HANDLERS.with(|handlers| {
        let mut handlers = handlers.borrow_mut();
        let index = (token - 1) as usize;
        if token > 0 && index < handlers.len() {
            Some(std::mem::replace(&mut handlers[index], Box::new(|_| {})))
        } else {
            None
        }
    });
    if let Some(mut handler) = taken {
        handler(ctx);
        HANDLERS.with(|handlers| handlers.borrow_mut()[(token - 1) as usize] = handler);
    }
}

pub(crate) fn p(text: &str) -> (*const u8, i32) {
    (text.as_ptr(), text.len() as i32)
}

pub(crate) fn fetch(call: impl Fn(*mut u8, i32) -> i32) -> Option<String> {
    String::from_utf8(fetch_bytes(call).ok()?).ok()
}

/// The answer of a buffer-returning call, grown until it fits; a negative code is the Err.
pub(crate) fn fetch_bytes(call: impl Fn(*mut u8, i32) -> i32) -> Result<Vec<u8>, i32> {
    let mut buffer = vec![0u8; 256];
    loop {
        let needed = call(buffer.as_mut_ptr(), buffer.len() as i32);
        if needed < 0 {
            return Err(needed);
        }
        if needed as usize <= buffer.len() {
            buffer.truncate(needed as usize);
            return Ok(buffer);
        }
        buffer.resize(needed as usize, 0);
    }
}

/// Named arguments for text: add pairs, then pass them to `players::send`, `text::translate` or `commands::reply`.
#[derive(Default, Clone)]
pub struct Args {
    bytes: Vec<u8>,
    count: u32,
}

impl Args {
    pub fn new() -> Args {
        Args::default()
    }

    pub fn add(mut self, key: &str, value: &str) -> Args {
        self.count += 1;
        for part in [key, value] {
            self.bytes.extend_from_slice(&(part.len() as u32).to_le_bytes());
            self.bytes.extend_from_slice(part.as_bytes());
        }
        self
    }

    fn packed(&self) -> Vec<u8> {
        if self.count == 0 {
            return Vec::new();
        }
        let mut out = self.count.to_le_bytes().to_vec();
        out.extend_from_slice(&self.bytes);
        out
    }
}

pub(crate) fn with_args<R>(args: &Args, call: impl FnOnce(*const u8, i32) -> R) -> R {
    let packed = args.packed();
    call(if packed.is_empty() { std::ptr::null() } else { packed.as_ptr() }, packed.len() as i32)
}

pub const DEBUG: i32 = 0;
pub const INFO: i32 = 1;
pub const WARN: i32 = 2;
pub const ERROR: i32 = 3;
pub const LOWEST: i32 = 0;
pub const LOW: i32 = 1;
pub const NORMAL: i32 = 2;
pub const HIGH: i32 = 3;
pub const HIGHEST: i32 = 4;
