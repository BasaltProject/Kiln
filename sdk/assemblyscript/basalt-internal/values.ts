// Value builder and reader over the byte format of basalt_values.h. The writer grows a heap buffer (an optional limit
// acts as a fixed capacity); the reader works in place on a validated document and never aborts on malformed input.

export const VALUE_MAX_BYTES: i32 = 1 << 20;
export const VALUE_MAX_DEPTH: i32 = 16;
export const VALUE_MAX_NODES: i32 = 65536;
export const VALUE_MAX_KEY_BYTES: i32 = 256;

export namespace ValueKind {
  export const Absent: i32 = -1;
  export const Null: i32 = 0;
  export const Bool: i32 = 1;
  export const Int: i32 = 2;
  export const Float: i32 = 3;
  export const String: i32 = 4;
  export const Bytes: i32 = 5;
  export const List: i32 = 6;
  export const Map: i32 = 7;
}

export namespace ValueError {
  export const None: i32 = 0;
  export const Invalid: i32 = -3;
  export const Overflow: i32 = -5;
}

const TAG_NULL: u8 = 0;
const TAG_FALSE: u8 = 1;
const TAG_TRUE: u8 = 2;
const TAG_INT: u8 = 3;
const TAG_FLOAT: u8 = 4;
const TAG_STRING: u8 = 5;
const TAG_BYTES: u8 = 6;
const TAG_LIST: u8 = 7;
const TAG_MAP: u8 = 8;

class ValueFrame {
  start: i32 = 0;
  count: i32 = 0;
  isMap: bool = false;
  pendingKey: bool = false;
}

export class ValueWriter {
  private buf: Uint8Array;
  private len: i32 = 0;
  private limit: i32;
  private nodes: i32 = 0;
  private frames: Array<ValueFrame> = new Array<ValueFrame>();
  private failure: i32 = ValueError.None;

  /** A writer that fails with ValueError.Overflow once the document would pass `limit` bytes. */
  constructor(limit: i32 = VALUE_MAX_BYTES) {
    this.limit = limit < VALUE_MAX_BYTES ? limit : VALUE_MAX_BYTES;
    this.buf = new Uint8Array(64);
  }

  /** 0 while the writer is healthy, else a ValueError. */
  error(): i32 { return this.failure; }

  ok(): bool { return this.failure == ValueError.None; }

  private fail(reason: i32): void {
    if (this.failure == ValueError.None) this.failure = reason;
  }

  private room(extra: i32): bool {
    if (this.failure != ValueError.None) return false;
    if (extra < 0 || extra > this.limit - this.len) {
      this.fail(ValueError.Overflow);
      return false;
    }
    const need = this.len + extra;
    if (need > this.buf.length) {
      let size = this.buf.length * 2;
      while (size < need) size *= 2;
      const grown = new Uint8Array(size);
      grown.set(this.buf.subarray(0, this.len));
      this.buf = grown;
    }
    return true;
  }

  private put(b: u8): void {
    if (this.room(1)) {
      this.buf[this.len] = b;
      this.len++;
    }
  }

  private leb(value: u64): void {
    while (value >= 0x80) {
      this.put(<u8>(value & 0x7F) | 0x80);
      value >>= 7;
    }
    this.put(<u8>value);
  }

  private raw(data: Uint8Array): void {
    if (this.room(data.length)) {
      this.buf.set(data, this.len);
      this.len += data.length;
    }
  }

  private begin(): bool {
    if (this.failure != ValueError.None) return false;
    const n = this.frames.length;
    if (n == 0) {
      if (this.nodes > 0) {
        this.fail(ValueError.Invalid);
        return false;
      }
    } else {
      const top = this.frames[n - 1];
      if (top.isMap) {
        if (!top.pendingKey) {
          this.fail(ValueError.Invalid);
          return false;
        }
        top.pendingKey = false;
      } else {
        top.count++;
      }
    }
    this.nodes++;
    if (n > VALUE_MAX_DEPTH || this.nodes > VALUE_MAX_NODES) {
      this.fail(ValueError.Invalid);
      return false;
    }
    return true;
  }

  nullValue(): ValueWriter {
    if (this.begin()) this.put(TAG_NULL);
    return this;
  }

  boolean(value: bool): ValueWriter {
    if (this.begin()) this.put(value ? TAG_TRUE : TAG_FALSE);
    return this;
  }

  int(value: i64): ValueWriter {
    if (this.begin()) {
      this.put(TAG_INT);
      this.leb((<u64>value << 1) ^ <u64>(value >> 63));
    }
    return this;
  }

  float(value: f64): ValueWriter {
    if (this.begin()) {
      this.put(TAG_FLOAT);
      const bits = reinterpret<u64>(value);
      for (let shift: u64 = 0; shift < 64; shift += 8) this.put(<u8>(bits >> shift));
    }
    return this;
  }

  private blob(tag: u8, data: Uint8Array): void {
    if (this.begin()) {
      this.put(tag);
      this.leb(<u64>data.length);
      this.raw(data);
    }
  }

  string(text: string): ValueWriter {
    this.blob(TAG_STRING, Uint8Array.wrap(String.UTF8.encode(text)));
    return this;
  }

  bytes(data: Uint8Array): ValueWriter {
    this.blob(TAG_BYTES, data);
    return this;
  }

  private open(isMap: bool): void {
    if (this.begin()) {
      this.put(isMap ? TAG_MAP : TAG_LIST);
      if (this.failure != ValueError.None) return;
      const frame = new ValueFrame();
      frame.start = this.len;
      frame.isMap = isMap;
      this.frames.push(frame);
    }
  }

  beginList(): ValueWriter {
    this.open(false);
    return this;
  }

  beginMap(): ValueWriter {
    this.open(true);
    return this;
  }

  /** Closes the innermost list or map, inserting its count in front of the items. */
  end(): ValueWriter {
    if (this.failure != ValueError.None) return this;
    const n = this.frames.length;
    if (n == 0 || (this.frames[n - 1].isMap && this.frames[n - 1].pendingKey)) {
      this.fail(ValueError.Invalid);
      return this;
    }
    const top = this.frames.pop();
    let size = 1;
    for (let v = <u64>top.count; v >= 0x80; v >>= 7) size++;
    if (!this.room(size)) return this;
    for (let i = this.len - 1; i >= top.start; i--) this.buf[i + size] = this.buf[i];
    let c = <u64>top.count;
    let at = top.start;
    while (c >= 0x80) {
      this.buf[at++] = <u8>(c & 0x7F) | 0x80;
      c >>= 7;
    }
    this.buf[at] = <u8>c;
    this.len += size;
    return this;
  }

  /** The next key of the open map; the value follows. */
  key(name: string): ValueWriter {
    if (this.failure != ValueError.None) return this;
    const n = this.frames.length;
    const encoded = Uint8Array.wrap(String.UTF8.encode(name));
    if (n == 0 || !this.frames[n - 1].isMap || this.frames[n - 1].pendingKey || encoded.length > VALUE_MAX_KEY_BYTES) {
      this.fail(ValueError.Invalid);
      return this;
    }
    this.leb(<u64>encoded.length);
    this.raw(encoded);
    this.frames[n - 1].count++;
    this.frames[n - 1].pendingKey = true;
    return this;
  }

  /** The finished document, or null when the writer failed or a container is still open (see error()). */
  finish(): Uint8Array | null {
    if (this.failure != ValueError.None) return null;
    if (this.frames.length != 0 || this.nodes == 0) {
      this.failure = ValueError.Invalid;
      return null;
    }
    return this.buf.slice(0, this.len);
  }
}

// Positions travel in a one-element array so helpers can advance them; a bad varint sets lebBad.
let lebBad: bool = false;

function lebValue(p: usize, end: i32, pos: Array<i32>): u64 {
  let result: u64 = 0;
  let at = pos[0];
  lebBad = false;
  for (let shift = 0; shift < 70; shift += 7) {
    if (at >= end) {
      lebBad = true;
      return 0;
    }
    const b = load<u8>(p + <usize>at);
    at++;
    if (shift == 63 && b > 1) {
      lebBad = true;
      return 0;
    }
    result |= (<u64>(b & 0x7F)) << <u64>shift;
    if ((b & 0x80) == 0) {
      pos[0] = at;
      return result;
    }
  }
  lebBad = true;
  return 0;
}

function skipValue(p: usize, end: i32, start: i32): i32 {
  if (start < 0 || start >= end) return -1;
  const tag = load<u8>(p + <usize>start);
  const pos = [start + 1];
  if (tag == TAG_INT) {
    lebValue(p, end, pos);
    return lebBad ? -1 : pos[0];
  }
  if (tag == TAG_FLOAT) return end - pos[0] >= 8 ? pos[0] + 8 : -1;
  if (tag == TAG_STRING || tag == TAG_BYTES) {
    const n = lebValue(p, end, pos);
    return !lebBad && n <= <u64>(end - pos[0]) ? pos[0] + <i32>n : -1;
  }
  if (tag == TAG_LIST || tag == TAG_MAP) {
    let n = lebValue(p, end, pos);
    if (lebBad) return -1;
    while (n > 0) {
      if (tag == TAG_MAP) {
        const k = lebValue(p, end, pos);
        if (lebBad || k > <u64>(end - pos[0])) return -1;
        pos[0] += <i32>k;
      }
      const next = skipValue(p, end, pos[0]);
      if (next < 0) return -1;
      pos[0] = next;
      n--;
    }
    return pos[0];
  }
  return pos[0];
}

function sameBytes(p: usize, a: i32, b: i32, n: i32): bool {
  for (let i = 0; i < n; i++) {
    if (load<u8>(p + <usize>(a + i)) != load<u8>(p + <usize>(b + i))) return false;
  }
  return true;
}

class WalkState {
  nodes: i32 = 0;
}

function walkValue(p: usize, end: i32, start: i32, depth: i32, state: WalkState): i32 {
  if (depth > VALUE_MAX_DEPTH) return -1;
  state.nodes++;
  if (state.nodes > VALUE_MAX_NODES || start >= end) return -1;
  const tag = load<u8>(p + <usize>start);
  const pos = [start + 1];
  if (tag == TAG_NULL || tag == TAG_FALSE || tag == TAG_TRUE) return pos[0];
  if (tag == TAG_INT) {
    lebValue(p, end, pos);
    return lebBad ? -1 : pos[0];
  }
  if (tag == TAG_FLOAT) return end - pos[0] >= 8 ? pos[0] + 8 : -1;
  if (tag == TAG_STRING || tag == TAG_BYTES) {
    const n = lebValue(p, end, pos);
    return !lebBad && n <= <u64>(end - pos[0]) ? pos[0] + <i32>n : -1;
  }
  if (tag == TAG_LIST) {
    const n = lebValue(p, end, pos);
    if (lebBad || n > <u64>(end - pos[0])) return -1;
    for (let i: u64 = 0; i < n; i++) {
      const next = walkValue(p, end, pos[0], depth + 1, state);
      if (next < 0) return -1;
      pos[0] = next;
    }
    return pos[0];
  }
  if (tag == TAG_MAP) {
    const n = lebValue(p, end, pos);
    if (lebBad || n > <u64>(end - pos[0])) return -1;
    const first = pos[0];
    for (let i: u64 = 0; i < n; i++) {
      const klen = lebValue(p, end, pos);
      if (lebBad || klen > <u64>VALUE_MAX_KEY_BYTES || klen > <u64>(end - pos[0])) return -1;
      const name = pos[0];
      const scan = [first];
      for (let j: u64 = 0; j < i; j++) {
        const olen = lebValue(p, end, scan);
        if (olen == klen && sameBytes(p, scan[0], name, <i32>klen)) return -1;
        const next = skipValue(p, end, scan[0] + <i32>olen);
        if (next < 0) return -1;
        scan[0] = next;
      }
      const next = walkValue(p, end, name + <i32>klen, depth + 1, state);
      if (next < 0) return -1;
      pos[0] = next;
    }
    return pos[0];
  }
  return -1;
}

/** Iterates the items of a list or the entries of a map: call next(), then read item (and key for a map). */
export class ValueIter {
  /** The current item after a true next(). */
  item: Value = new Value(new Uint8Array(0), -1);
  /** The key of the current entry (empty for a list). */
  key: string = "";
  /** The raw UTF-8 bytes of the key. */
  keyBytes: Uint8Array = new Uint8Array(0);

  constructor(
    private bytes: Uint8Array,
    private pos: i32,
    private left: i32,
    private isMap: bool,
  ) {}

  next(): bool {
    const p = changetype<usize>(this.bytes.dataStart);
    const end = this.bytes.length;
    if (this.left <= 0 || this.pos >= end) return false;
    const cursor = [this.pos];
    if (this.isMap) {
      const klen = lebValue(p, end, cursor);
      if (lebBad || klen > <u64>(end - cursor[0])) {
        this.left = 0;
        return false;
      }
      this.keyBytes = this.bytes.subarray(cursor[0], cursor[0] + <i32>klen);
      this.key = String.UTF8.decodeUnsafe(p + <usize>cursor[0], <usize>klen);
      cursor[0] += <i32>klen;
    }
    this.item = new Value(this.bytes, cursor[0]);
    const after = skipValue(p, end, cursor[0]);
    this.pos = after < 0 ? end : after;
    this.left--;
    return true;
  }
}

/** A position in a validated document. An absent value (kind() == ValueKind.Absent) answers every getter with its
 * fallback. Check kind() first when a fallback would be ambiguous. */
export class Value {
  constructor(
    readonly bytes: Uint8Array,
    readonly off: i32,
  ) {}

  /** Validates the whole document (tags, limits, duplicate keys, trailing bytes); null when malformed. */
  static open(document: Uint8Array): Value | null {
    if (document.length == 0 || document.length > VALUE_MAX_BYTES) return null;
    const state = new WalkState();
    const end = walkValue(changetype<usize>(document.dataStart), document.length, 0, 0, state);
    return end == document.length ? new Value(document, 0) : null;
  }

  private get p(): usize {
    return changetype<usize>(this.bytes.dataStart);
  }

  kind(): i32 {
    if (this.off < 0 || this.off >= this.bytes.length) return ValueKind.Absent;
    const tag = load<u8>(this.p + <usize>this.off);
    if (tag == TAG_FALSE || tag == TAG_TRUE) return ValueKind.Bool;
    if (tag == TAG_INT) return ValueKind.Int;
    if (tag == TAG_FLOAT) return ValueKind.Float;
    if (tag == TAG_STRING) return ValueKind.String;
    if (tag == TAG_BYTES) return ValueKind.Bytes;
    if (tag == TAG_LIST) return ValueKind.List;
    if (tag == TAG_MAP) return ValueKind.Map;
    return ValueKind.Null;
  }

  isNull(): bool { return this.kind() == ValueKind.Null; }

  present(): bool { return this.kind() != ValueKind.Absent; }

  toBool(fallback: bool = false): bool {
    return this.kind() == ValueKind.Bool ? load<u8>(this.p + <usize>this.off) == TAG_TRUE : fallback;
  }

  toInt(fallback: i64 = 0): i64 {
    if (this.kind() != ValueKind.Int) return fallback;
    const pos = [this.off + 1];
    const raw = lebValue(this.p, this.bytes.length, pos);
    if (lebBad) return fallback;
    return <i64>(raw >> 1) ^ -(<i64>(raw & 1));
  }

  toFloat(fallback: f64 = 0): f64 {
    if (this.kind() != ValueKind.Float || this.bytes.length - this.off < 9) return fallback;
    return load<f64>(this.p + <usize>this.off + 1);
  }

  private payload(kind: i32): Uint8Array | null {
    if (this.kind() != kind) return null;
    const pos = [this.off + 1];
    const n = lebValue(this.p, this.bytes.length, pos);
    if (lebBad || n > <u64>(this.bytes.length - pos[0])) return null;
    return this.bytes.subarray(pos[0], pos[0] + <i32>n);
  }

  /** The text of a string value, or null for another kind. */
  toStr(): string | null {
    const data = this.payload(ValueKind.String);
    return data == null ? null : String.UTF8.decodeUnsafe(changetype<usize>(data.dataStart), <usize>data.length);
  }

  /** The raw UTF-8 payload of a string value. */
  toStrBytes(): Uint8Array | null { return this.payload(ValueKind.String); }

  toBytes(): Uint8Array | null { return this.payload(ValueKind.Bytes); }

  /** Items of a list or entries of a map, -1 for any other value. */
  count(): i32 {
    const k = this.kind();
    if (k != ValueKind.List && k != ValueKind.Map) return -1;
    const pos = [this.off + 1];
    const n = lebValue(this.p, this.bytes.length, pos);
    return lebBad ? -1 : <i32>n;
  }

  iter(): ValueIter {
    const k = this.kind();
    const pos = [this.off + 1];
    let left: i32 = 0;
    if (k == ValueKind.List || k == ValueKind.Map) {
      const n = lebValue(this.p, this.bytes.length, pos);
      left = lebBad ? 0 : <i32>n;
    }
    return new ValueIter(this.bytes, pos[0], left, k == ValueKind.Map);
  }

  at(index: i32): Value | null {
    if (this.kind() != ValueKind.List || index < 0) return null;
    const it = this.iter();
    for (let i = 0; it.next(); i++) {
      if (i == index) return it.item;
    }
    return null;
  }

  find(name: string): Value | null {
    if (this.kind() != ValueKind.Map) return null;
    const wanted = Uint8Array.wrap(String.UTF8.encode(name));
    const it = this.iter();
    while (it.next()) {
      const k = it.keyBytes;
      if (k.length != wanted.length) continue;
      let same = true;
      for (let i = 0; i < k.length; i++) {
        if (k[i] != wanted[i]) {
          same = false;
          break;
        }
      }
      if (same) return it.item;
    }
    return null;
  }
}

/** An import that fills (out, cap) and answers with the length it needs; subclass it to wrap one. */
export abstract class OutCall {
  abstract call(out: usize, cap: i32): i32;
}

export const CALL_TOO_LARGE: i32 = -100;

/** The outcome of callOut: `error` is 0 or a negative import result (CALL_TOO_LARGE when nothing fit). */
export class OutResult {
  error: i32 = 0;
  bytes: Uint8Array = new Uint8Array(0);

  /** The answer validated as a document, or null. */
  value(): Value | null { return this.error == 0 ? Value.open(this.bytes) : null; }
}

/** Calls `c` with `initial` bytes, then once more with exactly the needed size when the answer did not fit. */
export function callOut(c: OutCall, initial: i32 = 256): OutResult {
  const result = new OutResult();
  let buf = new Uint8Array(initial < 1 ? 1 : initial);
  let n = c.call(changetype<usize>(buf.dataStart), buf.length);
  if (n < 0) {
    result.error = n;
    return result;
  }
  if (n > buf.length) {
    if (n > VALUE_MAX_BYTES) {
      result.error = CALL_TOO_LARGE;
      return result;
    }
    buf = new Uint8Array(n);
    n = c.call(changetype<usize>(buf.dataStart), buf.length);
    if (n < 0) {
      result.error = n;
      return result;
    }
    if (n > buf.length) {
      result.error = CALL_TOO_LARGE;
      return result;
    }
  }
  result.bytes = buf.slice(0, n);
  return result;
}
