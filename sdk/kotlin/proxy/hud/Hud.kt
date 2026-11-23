@file:OptIn(kotlin.wasm.unsafe.UnsafeWasmMemoryApi::class)

package org.basaltmc.proxy.hud

import kotlin.wasm.unsafe.withScopedMemoryAllocator
import org.basaltmc.proxy.core.*
import org.basaltmc.proxy.internal.*
import org.basaltmc.proxy.values.*

// Proxy hud area (abi/areas/proxy_hud.h): boss bars, sidebar, tab list, tab rows, server links and chat completions.
// A null optional text leaves that part unchanged; every call answers 0 or a negative code.



private const val INVALID = -3

private fun key(w: ValueWriter, name: String, text: String?) {
    if (text != null) w.key(name).string(text)
}

private inline fun send(w: ValueWriter, crossinline call: (Int, Int) -> Int): Int {
    val doc = w.finish() ?: return INVALID
    return withScopedMemoryAllocator { a ->
        val d = a.bytes(doc)
        call(d.ptr, d.len)
    }
}

fun clearTitle(player: Player, reset: Boolean): Int = rawProxyHudClearTitle(player, if (reset) 1 else 0)

fun tabList(player: Player, header: String?, footer: String?): Int {
    val w = ValueWriter().beginMap()
    key(w, "header", header)
    key(w, "footer", footer)
    w.end()
    return send(w) { d, n -> rawProxyHudTabList(player, d, n) }
}

fun bossbarShow(player: Player, id: String, text: String, progress: Double, color: String, overlay: String): Int {
    val w = ValueWriter().beginMap()
    key(w, "text", text)
    w.key("progress").float(progress)
    key(w, "color", color)
    key(w, "overlay", overlay)
    w.end()
    val doc = w.finish() ?: return INVALID
    return withScopedMemoryAllocator { a ->
        val i = a.str(id)
        val d = a.bytes(doc)
        rawProxyHudBossbarShow(player, i.ptr, i.len, d.ptr, d.len)
    }
}

fun bossbarHide(player: Player, id: String): Int = withScopedMemoryAllocator { a ->
    val i = a.str(id)
    rawProxyHudBossbarHide(player, i.ptr, i.len)
}

fun bossbarProgress(player: Player, id: String, progress: Double): Int = withScopedMemoryAllocator { a ->
    val i = a.str(id)
    rawProxyHudBossbarProgress(player, i.ptr, i.len, progress)
}

fun bossbarTitle(player: Player, id: String, text: String): Int = withScopedMemoryAllocator { a ->
    val i = a.str(id)
    val t = a.str(text)
    rawProxyHudBossbarTitle(player, i.ptr, i.len, t.ptr, t.len, 0, 0)
}

fun bossbarStyle(player: Player, id: String, color: String, overlay: String): Int = withScopedMemoryAllocator { a ->
    val i = a.str(id)
    val c = a.str(color)
    val o = a.str(overlay)
    rawProxyHudBossbarStyle(player, i.ptr, i.len, c.ptr, c.len, o.ptr, o.len)
}

fun sidebar(player: Player, title: String, lines: List<String>): Int {
    val w = ValueWriter().beginMap()
    key(w, "title", title)
    w.key("lines").beginList()
    for (line in lines) w.string(line)
    w.end()
    w.end()
    return send(w) { d, n -> rawProxyHudSidebar(player, d, n) }
}

fun sidebarClear(player: Player): Int = rawProxyHudSidebarClear(player)

fun tabEntry(player: Player, id: String, name: String, text: String, latency: Long): Int {
    val w = ValueWriter().beginMap()
    key(w, "name", name)
    key(w, "text", text)
    w.key("latency").int(latency)
    w.end()
    val doc = w.finish() ?: return INVALID
    return withScopedMemoryAllocator { a ->
        val i = a.str(id)
        val d = a.bytes(doc)
        rawProxyHudTabEntry(player, i.ptr, i.len, d.ptr, d.len)
    }
}

fun tabEntryRemove(player: Player, id: String): Int = withScopedMemoryAllocator { a ->
    val i = a.str(id)
    rawProxyHudTabEntryRemove(player, i.ptr, i.len)
}

/** Label then url for each link. */
fun serverLinks(player: Player, pairs: List<String>): Int {
    val w = ValueWriter().beginList()
    var at = 0
    while (at + 1 < pairs.size) {
        w.beginList().string(pairs[at]).string(pairs[at + 1]).end()
        at += 2
    }
    w.end()
    return send(w) { d, n -> rawProxyHudServerLinks(player, d, n) }
}

fun chatCompletions(player: Player, action: String, words: List<String>): Int {
    val w = ValueWriter().beginList()
    for (word in words) w.string(word)
    w.end()
    val doc = w.finish() ?: return INVALID
    return withScopedMemoryAllocator { a ->
        val s = a.str(action)
        val d = a.bytes(doc)
        rawProxyHudChatCompletions(player, s.ptr, s.len, d.ptr, d.len)
    }
}
