//! Value builder and reader over the byte format of basalt_values.h. The writer fills a growable buffer (with an
//! optional size limit that acts as a fixed capacity); the reader works in place on a validated document and never
//! panics on malformed input.
#![allow(dead_code)]

pub const MAX_BYTES: usize = 1 << 20;
pub const MAX_DEPTH: usize = 16;
pub const MAX_NODES: usize = 65536;
pub const MAX_KEY_BYTES: usize = 256;

const NULL: u8 = 0;
const FALSE: u8 = 1;
const TRUE: u8 = 2;
const INT: u8 = 3;
const FLOAT: u8 = 4;
const STRING: u8 = 5;
const BYTES: u8 = 6;
const LIST: u8 = 7;
const MAP: u8 = 8;

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Kind {
    Null,
    Bool,
    Int,
    Float,
    String,
    Bytes,
    List,
    Map,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum WriteError {
    Invalid,
    Overflow,
}

struct Frame {
    start: usize,
    count: usize,
    is_map: bool,
    pending_key: bool,
}

pub struct Writer {
    buf: Vec<u8>,
    limit: usize,
    nodes: usize,
    frames: Vec<Frame>,
    failure: Option<WriteError>,
}

impl Default for Writer {
    fn default() -> Self {
        Self::new()
    }
}

impl Writer {
    pub fn new() -> Writer {
        Writer::with_limit(MAX_BYTES)
    }

    /// A writer that fails with `Overflow` once the document would pass `limit` bytes.
    pub fn with_limit(limit: usize) -> Writer {
        Writer { buf: Vec::new(), limit: limit.min(MAX_BYTES), nodes: 0, frames: Vec::new(), failure: None }
    }

    pub fn ok(&self) -> bool {
        self.failure.is_none()
    }

    fn fail(&mut self, reason: WriteError) {
        if self.failure.is_none() {
            self.failure = Some(reason);
        }
    }

    fn room(&mut self, extra: usize) -> bool {
        if self.failure.is_some() {
            return false;
        }
        if extra > self.limit || self.buf.len() > self.limit - extra {
            self.fail(WriteError::Overflow);
            return false;
        }
        true
    }

    fn byte(&mut self, b: u8) {
        if self.room(1) {
            self.buf.push(b);
        }
    }

    fn leb(&mut self, mut value: u64) {
        while value >= 0x80 {
            self.byte((value & 0x7F) as u8 | 0x80);
            value >>= 7;
        }
        self.byte(value as u8);
    }

    fn raw(&mut self, data: &[u8]) {
        if self.room(data.len()) {
            self.buf.extend_from_slice(data);
        }
    }

    fn begin(&mut self) -> bool {
        if self.failure.is_some() {
            return false;
        }
        match self.frames.last_mut() {
            None => {
                if self.nodes > 0 {
                    self.fail(WriteError::Invalid);
                    return false;
                }
            }
            Some(top) if top.is_map => {
                if !top.pending_key {
                    self.fail(WriteError::Invalid);
                    return false;
                }
                top.pending_key = false;
            }
            Some(top) => top.count += 1,
        }
        self.nodes += 1;
        if self.frames.len() > MAX_DEPTH || self.nodes > MAX_NODES {
            self.fail(WriteError::Invalid);
            return false;
        }
        true
    }

    pub fn null(&mut self) -> &mut Self {
        if self.begin() {
            self.byte(NULL);
        }
        self
    }

    pub fn boolean(&mut self, value: bool) -> &mut Self {
        if self.begin() {
            self.byte(if value { TRUE } else { FALSE });
        }
        self
    }

    pub fn int(&mut self, value: i64) -> &mut Self {
        if self.begin() {
            self.byte(INT);
            self.leb(((value as u64) << 1) ^ ((value >> 63) as u64));
        }
        self
    }

    pub fn float(&mut self, value: f64) -> &mut Self {
        if self.begin() {
            self.byte(FLOAT);
            self.raw(&value.to_bits().to_le_bytes());
        }
        self
    }

    fn blob(&mut self, tag: u8, data: &[u8]) {
        if self.begin() {
            self.byte(tag);
            self.leb(data.len() as u64);
            self.raw(data);
        }
    }

    pub fn string(&mut self, text: &str) -> &mut Self {
        self.blob(STRING, text.as_bytes());
        self
    }

    pub fn bytes(&mut self, data: &[u8]) -> &mut Self {
        self.blob(BYTES, data);
        self
    }

    fn open(&mut self, is_map: bool) {
        if self.begin() {
            self.byte(if is_map { MAP } else { LIST });
            let start = self.buf.len();
            self.frames.push(Frame { start, count: 0, is_map, pending_key: false });
        }
    }

    pub fn begin_list(&mut self) -> &mut Self {
        self.open(false);
        self
    }

    pub fn begin_map(&mut self) -> &mut Self {
        self.open(true);
        self
    }

    /// Closes the innermost list or map, inserting its count in front of the items.
    pub fn end(&mut self) -> &mut Self {
        if self.failure.is_some() {
            return self;
        }
        match self.frames.last() {
            Some(top) if !(top.is_map && top.pending_key) => {}
            _ => {
                self.fail(WriteError::Invalid);
                return self;
            }
        }
        let top = self.frames.pop().unwrap();
        let mut count = Vec::new();
        let mut value = top.count as u64;
        while value >= 0x80 {
            count.push((value & 0x7F) as u8 | 0x80);
            value >>= 7;
        }
        count.push(value as u8);
        if self.room(count.len()) {
            self.buf.splice(top.start..top.start, count);
        }
        self
    }

    /// The next key of the open map; the value follows.
    pub fn key(&mut self, name: &str) -> &mut Self {
        if self.failure.is_some() {
            return self;
        }
        match self.frames.last_mut() {
            Some(top) if top.is_map && !top.pending_key && name.len() <= MAX_KEY_BYTES => {
                top.count += 1;
                top.pending_key = true;
            }
            _ => {
                self.fail(WriteError::Invalid);
                return self;
            }
        }
        self.leb(name.len() as u64);
        self.raw(name.as_bytes());
        self
    }

    /// The finished document.
    pub fn finish(&self) -> Result<&[u8], WriteError> {
        if let Some(reason) = self.failure {
            return Err(reason);
        }
        if !self.frames.is_empty() || self.nodes == 0 {
            return Err(WriteError::Invalid);
        }
        Ok(&self.buf)
    }

    pub fn into_vec(self) -> Result<Vec<u8>, WriteError> {
        self.finish()?;
        Ok(self.buf)
    }
}

fn read_leb(b: &[u8], pos: &mut usize) -> Option<u64> {
    let mut result = 0u64;
    let mut shift = 0u32;
    while shift < 70 {
        let byte = *b.get(*pos)?;
        *pos += 1;
        if shift == 63 && byte > 1 {
            return None;
        }
        result |= u64::from(byte & 0x7F) << shift;
        if byte & 0x80 == 0 {
            return Some(result);
        }
        shift += 7;
    }
    None
}

fn skip(b: &[u8], start: usize) -> Option<usize> {
    let tag = *b.get(start)?;
    let mut pos = start + 1;
    match tag {
        INT => {
            read_leb(b, &mut pos)?;
            Some(pos)
        }
        FLOAT => (b.len() - pos >= 8).then_some(pos + 8),
        STRING | BYTES => {
            let n = read_leb(b, &mut pos)?;
            (n <= (b.len() - pos) as u64).then(|| pos + n as usize)
        }
        LIST | MAP => {
            let mut n = read_leb(b, &mut pos)?;
            while n > 0 {
                if tag == MAP {
                    let k = read_leb(b, &mut pos)?;
                    if k > (b.len() - pos) as u64 {
                        return None;
                    }
                    pos += k as usize;
                }
                pos = skip(b, pos)?;
                n -= 1;
            }
            Some(pos)
        }
        _ => Some(pos),
    }
}

fn walk(b: &[u8], start: usize, depth: usize, nodes: &mut usize) -> Option<usize> {
    if depth > MAX_DEPTH {
        return None;
    }
    *nodes += 1;
    if *nodes > MAX_NODES {
        return None;
    }
    let tag = *b.get(start)?;
    let mut pos = start + 1;
    match tag {
        NULL | FALSE | TRUE => Some(pos),
        INT => {
            read_leb(b, &mut pos)?;
            Some(pos)
        }
        FLOAT => (b.len() - pos >= 8).then_some(pos + 8),
        STRING | BYTES => {
            let n = read_leb(b, &mut pos)?;
            (n <= (b.len() - pos) as u64).then(|| pos + n as usize)
        }
        LIST => {
            let n = read_leb(b, &mut pos)?;
            if n > (b.len() - pos) as u64 {
                return None;
            }
            for _ in 0..n {
                pos = walk(b, pos, depth + 1, nodes)?;
            }
            Some(pos)
        }
        MAP => {
            let n = read_leb(b, &mut pos)?;
            if n > (b.len() - pos) as u64 {
                return None;
            }
            let first = pos;
            for i in 0..n {
                let klen = read_leb(b, &mut pos)?;
                if klen > MAX_KEY_BYTES as u64 || klen > (b.len() - pos) as u64 {
                    return None;
                }
                let name = &b[pos..pos + klen as usize];
                let mut scan = first;
                for _ in 0..i {
                    let olen = read_leb(b, &mut scan)? as usize;
                    if &b[scan..scan + olen] == name {
                        return None;
                    }
                    scan = skip(b, scan + olen)?;
                }
                pos = walk(b, pos + name.len(), depth + 1, nodes)?;
            }
            Some(pos)
        }
        _ => None,
    }
}

/// A position in a validated document.
#[derive(Clone, Copy)]
pub struct Value<'a> {
    bytes: &'a [u8],
    off: usize,
}

impl<'a> Value<'a> {
    /// Validates the whole document (tags, limits, duplicate keys, trailing bytes); None when malformed.
    pub fn open(document: &'a [u8]) -> Option<Value<'a>> {
        if document.is_empty() || document.len() > MAX_BYTES {
            return None;
        }
        let mut nodes = 0;
        (walk(document, 0, 0, &mut nodes)? == document.len()).then_some(Value { bytes: document, off: 0 })
    }

    pub fn kind(&self) -> Kind {
        match self.bytes[self.off] {
            FALSE | TRUE => Kind::Bool,
            INT => Kind::Int,
            FLOAT => Kind::Float,
            STRING => Kind::String,
            BYTES => Kind::Bytes,
            LIST => Kind::List,
            MAP => Kind::Map,
            _ => Kind::Null,
        }
    }

    pub fn is_null(&self) -> bool {
        self.kind() == Kind::Null
    }

    pub fn as_bool(&self) -> Option<bool> {
        (self.kind() == Kind::Bool).then(|| self.bytes[self.off] == TRUE)
    }

    pub fn as_i64(&self) -> Option<i64> {
        if self.kind() != Kind::Int {
            return None;
        }
        let mut pos = self.off + 1;
        let raw = read_leb(self.bytes, &mut pos)?;
        Some(((raw >> 1) ^ (raw & 1).wrapping_neg()) as i64)
    }

    pub fn as_f64(&self) -> Option<f64> {
        if self.kind() != Kind::Float {
            return None;
        }
        let raw = self.bytes.get(self.off + 1..self.off + 9)?;
        Some(f64::from_le_bytes(raw.try_into().ok()?))
    }

    fn payload(&self, kind: Kind) -> Option<&'a [u8]> {
        if self.kind() != kind {
            return None;
        }
        let mut pos = self.off + 1;
        let n = read_leb(self.bytes, &mut pos)?;
        (n <= (self.bytes.len() - pos) as u64).then(|| &self.bytes[pos..pos + n as usize])
    }

    /// The text of a string; None for another kind or when the bytes are not valid UTF-8.
    pub fn as_str(&self) -> Option<&'a str> {
        std::str::from_utf8(self.payload(Kind::String)?).ok()
    }

    /// The raw payload of a string, without the UTF-8 check.
    pub fn as_str_bytes(&self) -> Option<&'a [u8]> {
        self.payload(Kind::String)
    }

    pub fn as_bytes(&self) -> Option<&'a [u8]> {
        self.payload(Kind::Bytes)
    }

    /// Items of a list or entries of a map; None for any other value.
    pub fn count(&self) -> Option<usize> {
        if !matches!(self.kind(), Kind::List | Kind::Map) {
            return None;
        }
        let mut pos = self.off + 1;
        read_leb(self.bytes, &mut pos).map(|n| n as usize)
    }

    /// Iterates the items of a list, or the values of a map; empty for any other value.
    pub fn iter(&self) -> Items<'a> {
        let mut pos = self.off + 1;
        let kind = self.kind();
        let left = if matches!(kind, Kind::List | Kind::Map) { read_leb(self.bytes, &mut pos).unwrap_or(0) } else { 0 };
        Items { bytes: self.bytes, pos, left, is_map: kind == Kind::Map }
    }

    /// Iterates `(key, value)` pairs of a map; the key is "" when it is not valid UTF-8. Empty for any other value.
    pub fn entries(&self) -> Entries<'a> {
        Entries(self.iter())
    }

    pub fn at(&self, index: usize) -> Option<Value<'a>> {
        if self.kind() != Kind::List {
            return None;
        }
        self.iter().nth(index)
    }

    pub fn find(&self, key: &str) -> Option<Value<'a>> {
        if self.kind() != Kind::Map {
            return None;
        }
        let mut it = self.iter();
        while let Some((name, value)) = it.next_entry() {
            if name == key.as_bytes() {
                return Some(value);
            }
        }
        None
    }
}

pub struct Items<'a> {
    bytes: &'a [u8],
    pos: usize,
    left: u64,
    is_map: bool,
}

impl<'a> Items<'a> {
    fn next_entry(&mut self) -> Option<(&'a [u8], Value<'a>)> {
        if self.left == 0 || self.pos >= self.bytes.len() {
            return None;
        }
        let mut pos = self.pos;
        let mut name: &[u8] = &[];
        if self.is_map {
            let klen = read_leb(self.bytes, &mut pos);
            match klen {
                Some(n) if n <= (self.bytes.len() - pos) as u64 => {
                    name = &self.bytes[pos..pos + n as usize];
                    pos += n as usize;
                }
                _ => {
                    self.left = 0;
                    return None;
                }
            }
        }
        let item = Value { bytes: self.bytes, off: pos };
        self.pos = skip(self.bytes, pos).unwrap_or(self.bytes.len());
        self.left -= 1;
        Some((name, item))
    }
}

impl<'a> Iterator for Items<'a> {
    type Item = Value<'a>;

    fn next(&mut self) -> Option<Value<'a>> {
        self.next_entry().map(|(_, value)| value)
    }
}

pub struct Entries<'a>(Items<'a>);

impl<'a> Iterator for Entries<'a> {
    type Item = (&'a str, Value<'a>);

    fn next(&mut self) -> Option<(&'a str, Value<'a>)> {
        self.0.next_entry().map(|(key, value)| (std::str::from_utf8(key).unwrap_or(""), value))
    }
}

/// A document that owns its bytes, as returned by `call_value`.
pub struct OwnedValue {
    bytes: Vec<u8>,
}

impl OwnedValue {
    pub fn open(bytes: Vec<u8>) -> Option<OwnedValue> {
        Value::open(&bytes)?;
        Some(OwnedValue { bytes })
    }

    pub fn root(&self) -> Value<'_> {
        Value { bytes: &self.bytes, off: 0 }
    }
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum CallError {
    Absent,
    Stale,
    Invalid,
    Denied,
    Failed(i32),
    TooLarge,
}

fn failure(code: i32) -> CallError {
    match code {
        -1 => CallError::Absent,
        -2 => CallError::Stale,
        -3 => CallError::Invalid,
        -4 => CallError::Denied,
        other => CallError::Failed(other),
    }
}

/// Calls `call(out, cap)` (an import that answers with the length it needs) with `initial` bytes, and once more with
/// exactly the needed size when the answer did not fit.
pub fn call_out(initial: usize, mut call: impl FnMut(*mut u8, i32) -> i32) -> Result<Vec<u8>, CallError> {
    let mut buf = vec![0u8; initial];
    let mut n = call(buf.as_mut_ptr(), buf.len() as i32);
    if n < 0 {
        return Err(failure(n));
    }
    if n as usize > buf.len() {
        if n as usize > MAX_BYTES {
            return Err(CallError::TooLarge);
        }
        buf = vec![0u8; n as usize];
        n = call(buf.as_mut_ptr(), buf.len() as i32);
        if n < 0 {
            return Err(failure(n));
        }
        if n as usize > buf.len() {
            return Err(CallError::TooLarge);
        }
    }
    buf.truncate(n as usize);
    Ok(buf)
}

/// `call_out` followed by validating the answer as a document.
pub fn call_value(initial: usize, call: impl FnMut(*mut u8, i32) -> i32) -> Result<OwnedValue, CallError> {
    OwnedValue::open(call_out(initial, call)?).ok_or(CallError::Invalid)
}
