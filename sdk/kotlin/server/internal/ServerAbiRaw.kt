@file:OptIn(kotlin.wasm.ExperimentalWasmInterop::class)

// Raw imports of the server-only calls of basalt_abi.h.
package org.basaltmc.server.internal

import kotlin.wasm.WasmImport

@WasmImport("basalt", "player_particle")
internal external fun rawPlayerParticle(player: Int, id: Int, idlen: Int, count: Int, spread: Double): Int

@WasmImport("basalt", "player_sound")
internal external fun rawPlayerSound(player: Int, id: Int, idlen: Int, volume: Float, pitch: Float): Int

@WasmImport("basalt", "store_top")
internal external fun rawStoreTop(key: Int, klen: Int, limit: Int, out: Int, cap: Int): Int

@WasmImport("basalt", "placeholder_register")
internal external fun rawPlaceholderRegister(name: Int, nlen: Int, token: Int): Int

@WasmImport("basalt", "placeholder_player")
internal external fun rawPlaceholderPlayer(ctx: Int): Int

@WasmImport("basalt", "placeholder_result")
internal external fun rawPlaceholderResult(ctx: Int, text: Int, len: Int): Int

@WasmImport("basalt", "menu_create")
internal external fun rawMenuCreate(title: Int, tlen: Int, rows: Int): Int

@WasmImport("basalt", "menu_set")
internal external fun rawMenuSet(menu: Int, slot: Int, item: Int, ilen: Int, name: Int, nlen: Int, lore: Int, llen: Int, token: Int): Int

@WasmImport("basalt", "menu_open")
internal external fun rawMenuOpen(menu: Int, player: Int): Int

@WasmImport("basalt", "menu_click_player")
internal external fun rawMenuClickPlayer(ctx: Int): Int
