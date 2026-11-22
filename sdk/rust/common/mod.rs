//! The modules both profiles share. Each crate re-exports them, or extends them under the same name.
pub mod commands;
pub mod config;
pub mod core;
pub mod data;
pub mod events;
pub mod log;
pub mod players;
pub mod raw;
pub mod tasks;
pub mod text;
pub mod values;
