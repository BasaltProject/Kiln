// Proxy hud area (abi/areas/proxy_hud.h): boss bars, sidebar, tab list, tab rows, server links and chat completions.
// A null optional text leaves that part unchanged; every call answers 0 or a negative code.
import { Player } from "../basalt-internal/index";
import { raw_proxy_hud as raw } from "./raw/hud";
import { ValueWriter } from "./values";

const HUD_INVALID: i32 = -3;

function hudText(text: string): ArrayBuffer {
  return String.UTF8.encode(text);
}

function hudPtr(buffer: ArrayBuffer): usize {
  return changetype<usize>(buffer);
}

function hudKey(w: ValueWriter, name: string, text: string | null): void {
  if (text !== null) w.key(name).string(text);
}

export function proxyHudClearTitle(player: Player, reset: bool): i32 {
  return raw.hud_clear_title(player, reset ? 1 : 0);
}

export function proxyHudTabList(player: Player, header: string | null, footer: string | null): i32 {
  const w = new ValueWriter().beginMap();
  hudKey(w, "header", header);
  hudKey(w, "footer", footer);
  const doc = w.end().finish();
  if (doc === null) return HUD_INVALID;
  return raw.hud_tab_list(player, changetype<usize>(doc.dataStart), doc.length);
}

export function proxyHudBossbarShow(player: Player, id: string, text: string, progress: f64, color: string, overlay: string): i32 {
  const w = new ValueWriter().beginMap();
  hudKey(w, "text", text);
  w.key("progress").float(progress);
  hudKey(w, "color", color);
  hudKey(w, "overlay", overlay);
  const doc = w.end().finish();
  if (doc === null) return HUD_INVALID;
  const i = hudText(id);
  return raw.hud_bossbar_show(player, hudPtr(i), i.byteLength, changetype<usize>(doc.dataStart), doc.length);
}

export function proxyHudBossbarHide(player: Player, id: string): i32 {
  const i = hudText(id);
  return raw.hud_bossbar_hide(player, hudPtr(i), i.byteLength);
}

export function proxyHudBossbarProgress(player: Player, id: string, progress: f64): i32 {
  const i = hudText(id);
  return raw.hud_bossbar_progress(player, hudPtr(i), i.byteLength, progress);
}

export function proxyHudBossbarTitle(player: Player, id: string, text: string): i32 {
  const i = hudText(id);
  const t = hudText(text);
  return raw.hud_bossbar_title(player, hudPtr(i), i.byteLength, hudPtr(t), t.byteLength, 0, 0);
}

export function proxyHudBossbarStyle(player: Player, id: string, color: string, overlay: string): i32 {
  const i = hudText(id);
  const c = hudText(color);
  const o = hudText(overlay);
  return raw.hud_bossbar_style(player, hudPtr(i), i.byteLength, hudPtr(c), c.byteLength, hudPtr(o), o.byteLength);
}

export function proxyHudSidebar(player: Player, title: string, lines: Array<string>): i32 {
  const w = new ValueWriter().beginMap();
  hudKey(w, "title", title);
  w.key("lines").beginList();
  for (let i = 0; i < lines.length; i++) w.string(lines[i]);
  const doc = w.end().end().finish();
  if (doc === null) return HUD_INVALID;
  return raw.hud_sidebar(player, changetype<usize>(doc.dataStart), doc.length);
}

export function proxyHudSidebarClear(player: Player): i32 {
  return raw.hud_sidebar_clear(player);
}

export function proxyHudTabEntry(player: Player, id: string, name: string, text: string, latency: i64): i32 {
  const w = new ValueWriter().beginMap();
  hudKey(w, "name", name);
  hudKey(w, "text", text);
  w.key("latency").int(latency);
  const doc = w.end().finish();
  if (doc === null) return HUD_INVALID;
  const i = hudText(id);
  return raw.hud_tab_entry(player, hudPtr(i), i.byteLength, changetype<usize>(doc.dataStart), doc.length);
}

export function proxyHudTabEntryRemove(player: Player, id: string): i32 {
  const i = hudText(id);
  return raw.hud_tab_entry_remove(player, hudPtr(i), i.byteLength);
}

/** Label then url for each link. */
export function proxyHudServerLinks(player: Player, pairs: Array<string>): i32 {
  const w = new ValueWriter().beginList();
  for (let at = 0; at + 1 < pairs.length; at += 2) w.beginList().string(pairs[at]).string(pairs[at + 1]).end();
  const doc = w.end().finish();
  if (doc === null) return HUD_INVALID;
  return raw.hud_server_links(player, changetype<usize>(doc.dataStart), doc.length);
}

export function proxyHudChatCompletions(player: Player, action: string, words: Array<string>): i32 {
  const w = new ValueWriter().beginList();
  for (let i = 0; i < words.length; i++) w.string(words[i]);
  const doc = w.end().finish();
  if (doc === null) return HUD_INVALID;
  const a = hudText(action);
  return raw.hud_chat_completions(player, hudPtr(a), a.byteLength, changetype<usize>(doc.dataStart), doc.length);
}
