//! Lines in the console log, tagged with the plugin id.
use crate::core::{p, Level};
use crate::raw;

pub fn log(level: i32, message: &str) {
    let (m, ml) = p(message);
    unsafe { raw::log(level as i32, m, ml) }
}

pub fn debug(message: &str) {
    log(Level::Debug as i32, message)
}

pub fn info(message: &str) {
    log(Level::Info as i32, message)
}

pub fn warn(message: &str) {
    log(Level::Warn as i32, message)
}

pub fn error(message: &str) {
    log(Level::Error as i32, message)
}
