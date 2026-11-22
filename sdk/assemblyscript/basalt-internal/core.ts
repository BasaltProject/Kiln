// Handles, levels, priorities and named text arguments shared by server and proxy plugins.
export type Player = i32;
export type Ctx = i32;
export type Handler = (ctx: Ctx) => void;

export const DEBUG: i32 = 0;
export const INFO: i32 = 1;
export const WARN: i32 = 2;
export const ERROR: i32 = 3;

export const LOWEST: i32 = 0;
export const LOW: i32 = 1;
export const NORMAL: i32 = 2;
export const HIGH: i32 = 3;
export const HIGHEST: i32 = 4;

/** Named arguments for text: add pairs, then pass them to send, translate or commandReply. */
export class Args {
  private bytes: Array<u8> = [];
  private count: u32 = 0;

  add(key: string, value: string): Args {
    this.count++;
    this.put(key);
    this.put(value);
    return this;
  }

  private put(part: string): void {
    const b = String.UTF8.encode(part);
    const n = b.byteLength;
    this.bytes.push(<u8>n);
    this.bytes.push(<u8>(n >> 8));
    this.bytes.push(<u8>(n >> 16));
    this.bytes.push(<u8>(n >> 24));
    const view = Uint8Array.wrap(b);
    for (let i = 0; i < n; i++) this.bytes.push(view[i]);
  }

  packed(): ArrayBuffer | null {
    if (this.count == 0) return null;
    const out = new Uint8Array(4 + this.bytes.length);
    out[0] = <u8>this.count;
    out[1] = <u8>(this.count >> 8);
    out[2] = <u8>(this.count >> 16);
    out[3] = <u8>(this.count >> 24);
    for (let i = 0; i < this.bytes.length; i++) out[4 + i] = this.bytes[i];
    return out.buffer;
  }
}
