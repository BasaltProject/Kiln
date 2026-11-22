// UTF-8 views and answer buffers the wrappers pass to the raw imports.
import { Args } from "./core";

export class Utf8 {
  constructor(public buffer: ArrayBuffer) {}
  get ptr(): usize { return changetype<usize>(this.buffer); }
  get len(): i32 { return this.buffer.byteLength; }
}

export function u(text: string): Utf8 {
  return new Utf8(String.UTF8.encode(text));
}

export function finish(buffer: ArrayBuffer, needed: i32): string | null {
  return needed < 0 ? null : String.UTF8.decodeUnsafe(changetype<usize>(buffer), needed);
}

export function ap(args: Args | null): ArrayBuffer | null {
  return args === null ? null : args.packed();
}
export function ptrOf(buffer: ArrayBuffer | null): usize {
  return buffer === null ? 0 : changetype<usize>(buffer);
}
export function lenOf(buffer: ArrayBuffer | null): i32 {
  return buffer === null ? 0 : buffer.byteLength;
}
