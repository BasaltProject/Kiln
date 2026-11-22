//! Messages of the plugin's lang files.
use crate::core::{fetch, p, with_args, Args};
use crate::raw;

pub fn translate(key: &str, args: &Args) -> Option<String> {
    let (k, kl) = p(key);
    with_args(args, |a, al| fetch(|out, cap| unsafe { raw::translate(k, kl, a, al, out, cap) }))
}
