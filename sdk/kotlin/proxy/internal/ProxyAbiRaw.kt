@file:OptIn(kotlin.wasm.ExperimentalWasmInterop::class)

package org.basaltmc.proxy.internal

import kotlin.wasm.WasmImport

// Raw imports of basalt_proxy_abi.h.


@WasmImport("basalt_proxy", "event_set_str")
internal external fun rawProxyEventSetStr(ctx: Int, field: Int, flen: Int, value: Int, vlen: Int): Int

@WasmImport("basalt_proxy", "event_set_i64")
internal external fun rawProxyEventSetI64(ctx: Int, field: Int, flen: Int, value: Long): Int

@WasmImport("basalt_proxy", "player_uuid")
internal external fun rawProxyPlayerUuid(player: Int, out: Int, cap: Int): Int

@WasmImport("basalt_proxy", "player_server")
internal external fun rawProxyPlayerServer(player: Int, out: Int, cap: Int): Int

@WasmImport("basalt_proxy", "player_ping")
internal external fun rawProxyPlayerPing(player: Int): Int

@WasmImport("basalt_proxy", "player_setting_str")
internal external fun rawProxyPlayerSettingStr(player: Int, field: Int, flen: Int, out: Int, cap: Int): Int

@WasmImport("basalt_proxy", "player_setting_i64")
internal external fun rawProxyPlayerSettingI64(player: Int, field: Int, flen: Int, out: Int): Int

@WasmImport("basalt_proxy", "player_has_permission")
internal external fun rawProxyPlayerHasPermission(player: Int, node: Int, nlen: Int): Int

@WasmImport("basalt_proxy", "player_title")
internal external fun rawProxyPlayerTitle(player: Int, title: Int, tlen: Int, subtitle: Int, slen: Int, fadeIn: Int, stay: Int, fadeOut: Int): Int

@WasmImport("basalt_proxy", "player_connect")
internal external fun rawProxyPlayerConnect(player: Int, server: Int, slen: Int): Int

@WasmImport("basalt_proxy", "player_kick")
internal external fun rawProxyPlayerKick(player: Int, reason: Int, rlen: Int, args: Int, alen: Int): Int

@WasmImport("basalt_proxy", "servers_list")
internal external fun rawProxyServersList(out: Int, cap: Int): Int

@WasmImport("basalt_proxy", "server_address")
internal external fun rawProxyServerAddress(name: Int, nlen: Int, out: Int, cap: Int): Int

@WasmImport("basalt_proxy", "players_on")
internal external fun rawProxyPlayersOn(server: Int, slen: Int, out: Int, cap: Int): Int
