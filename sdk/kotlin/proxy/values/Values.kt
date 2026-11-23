@file:OptIn(kotlin.wasm.unsafe.UnsafeWasmMemoryApi::class)

package org.basaltmc.proxy.values

import kotlin.wasm.unsafe.MemoryAllocator
import kotlin.wasm.unsafe.Pointer
import kotlin.wasm.unsafe.withScopedMemoryAllocator
import org.basaltmc.proxy.core.*
import org.basaltmc.proxy.internal.*

// Value builder and reader over the byte format of basalt_values.h. The writer grows a heap buffer (an optional limit
// acts as a fixed capacity); the reader works in place on a validated document and never throws on malformed input.



const val VALUE_MAX_BYTES = 1 shl 20
const val VALUE_MAX_DEPTH = 16
const val VALUE_MAX_NODES = 65536
const val VALUE_MAX_KEY_BYTES = 256

enum class ValueKind { Null, Bool, Int, Float, String, Bytes, List, Map }

enum class ValueError { None, Invalid, Overflow }

private const val TAG_NULL = 0
private const val TAG_FALSE = 1
private const val TAG_TRUE = 2
private const val TAG_INT = 3
private const val TAG_FLOAT = 4
private const val TAG_STRING = 5
private const val TAG_BYTES = 6
private const val TAG_LIST = 7
private const val TAG_MAP = 8

private class ValueFrame(val start: Int, val isMap: Boolean) {
    var count = 0
    var pendingKey = false
}

/** Builds one document. A failure is sticky: later calls do nothing and finish() returns null. */
class ValueWriter(limit: Int = VALUE_MAX_BYTES) {
    private var buf = ByteArray(64)
    private var len = 0
    private val limit = if (limit < VALUE_MAX_BYTES) limit else VALUE_MAX_BYTES
    private var nodes = 0
    private val frames = ArrayList<ValueFrame>()

    /** None while healthy, else Invalid or Overflow. */
    var error = ValueError.None
        private set

    val ok: Boolean get() = error == ValueError.None

    private fun fail(reason: ValueError) {
        if (error == ValueError.None) error = reason
    }

    private fun room(extra: Int): Boolean {
        if (error != ValueError.None) return false
        if (extra < 0 || extra > limit - len) {
            fail(ValueError.Overflow)
            return false
        }
        val need = len + extra
        if (need > buf.size) {
            var size = buf.size * 2
            while (size < need) size *= 2
            buf = buf.copyOf(size)
        }
        return true
    }

    private fun put(b: Int) {
        if (room(1)) buf[len++] = b.toByte()
    }

    private fun leb(value: Long) {
        var v = value
        while ((v and 0x7FL.inv()) != 0L) {
            put((v and 0x7F).toInt() or 0x80)
            v = v ushr 7
        }
        put(v.toInt())
    }

    private fun raw(data: ByteArray) {
        if (room(data.size)) {
            data.copyInto(buf, len)
            len += data.size
        }
    }

    private fun begin(): Boolean {
        if (error != ValueError.None) return false
        val n = frames.size
        if (n == 0) {
            if (nodes > 0) {
                fail(ValueError.Invalid)
                return false
            }
        } else {
            val top = frames[n - 1]
            if (top.isMap) {
                if (!top.pendingKey) {
                    fail(ValueError.Invalid)
                    return false
                }
                top.pendingKey = false
            } else {
                top.count++
            }
        }
        nodes++
        if (n > VALUE_MAX_DEPTH || nodes > VALUE_MAX_NODES) {
            fail(ValueError.Invalid)
            return false
        }
        return true
    }

    fun nul(): ValueWriter {
        if (begin()) put(TAG_NULL)
        return this
    }

    fun bool(value: Boolean): ValueWriter {
        if (begin()) put(if (value) TAG_TRUE else TAG_FALSE)
        return this
    }

    fun int(value: Long): ValueWriter {
        if (begin()) {
            put(TAG_INT)
            leb((value shl 1) xor (value shr 63))
        }
        return this
    }

    fun float(value: Double): ValueWriter {
        if (begin()) {
            put(TAG_FLOAT)
            val bits = value.toRawBits()
            for (shift in 0 until 64 step 8) put(((bits ushr shift) and 0xFF).toInt())
        }
        return this
    }

    private fun blob(tag: Int, data: ByteArray) {
        if (begin()) {
            put(tag)
            leb(data.size.toLong())
            raw(data)
        }
    }

    fun string(text: String): ValueWriter {
        blob(TAG_STRING, text.encodeToByteArray())
        return this
    }

    fun bytes(data: ByteArray): ValueWriter {
        blob(TAG_BYTES, data)
        return this
    }

    private fun open(isMap: Boolean) {
        if (begin()) {
            put(if (isMap) TAG_MAP else TAG_LIST)
            if (error == ValueError.None) frames.add(ValueFrame(len, isMap))
        }
    }

    fun beginList(): ValueWriter {
        open(false)
        return this
    }

    fun beginMap(): ValueWriter {
        open(true)
        return this
    }

    /** Closes the innermost list or map, inserting its count in front of the items. */
    fun end(): ValueWriter {
        if (error != ValueError.None) return this
        val n = frames.size
        if (n == 0 || (frames[n - 1].isMap && frames[n - 1].pendingKey)) {
            fail(ValueError.Invalid)
            return this
        }
        val top = frames.removeAt(n - 1)
        var size = 1
        var v = top.count.toLong()
        while (v >= 0x80) {
            size++
            v = v ushr 7
        }
        if (!room(size)) return this
        buf.copyInto(buf, top.start + size, top.start, len)
        var c = top.count.toLong()
        var at = top.start
        while (c >= 0x80) {
            buf[at++] = ((c and 0x7F) or 0x80).toByte()
            c = c ushr 7
        }
        buf[at] = c.toByte()
        len += size
        return this
    }

    /** The next key of the open map; the value follows. */
    fun key(name: String): ValueWriter {
        if (error != ValueError.None) return this
        val n = frames.size
        val encoded = name.encodeToByteArray()
        if (n == 0 || !frames[n - 1].isMap || frames[n - 1].pendingKey || encoded.size > VALUE_MAX_KEY_BYTES) {
            fail(ValueError.Invalid)
            return this
        }
        leb(encoded.size.toLong())
        raw(encoded)
        frames[n - 1].count++
        frames[n - 1].pendingKey = true
        return this
    }

    /** A copy of the finished document, or null when the writer failed or a container is still open. */
    fun finish(): ByteArray? {
        if (error != ValueError.None) return null
        if (frames.isNotEmpty() || nodes == 0) {
            error = ValueError.Invalid
            return null
        }
        return buf.copyOf(len)
    }
}

private class ValueCursor(var at: Int)

private var valueLebBad = false

private fun valueLeb(b: ByteArray, end: Int, pos: ValueCursor): Long {
    var result = 0L
    var at = pos.at
    valueLebBad = false
    var shift = 0
    while (shift < 70) {
        if (at >= end) {
            valueLebBad = true
            return 0
        }
        val c = b[at++].toInt() and 0xFF
        if (shift == 63 && c > 1) {
            valueLebBad = true
            return 0
        }
        result = result or ((c and 0x7F).toLong() shl shift)
        if ((c and 0x80) == 0) {
            pos.at = at
            return result
        }
        shift += 7
    }
    valueLebBad = true
    return 0
}

private fun valueFits(n: Long, room: Int) = n >= 0 && n <= room

private fun valueSkip(b: ByteArray, end: Int, start: Int): Int {
    if (start < 0 || start >= end) return -1
    val tag = b[start].toInt()
    val pos = ValueCursor(start + 1)
    when (tag) {
        TAG_INT -> {
            valueLeb(b, end, pos)
            return if (valueLebBad) -1 else pos.at
        }
        TAG_FLOAT -> return if (end - pos.at >= 8) pos.at + 8 else -1
        TAG_STRING, TAG_BYTES -> {
            val n = valueLeb(b, end, pos)
            return if (!valueLebBad && valueFits(n, end - pos.at)) pos.at + n.toInt() else -1
        }
        TAG_LIST, TAG_MAP -> {
            var n = valueLeb(b, end, pos)
            if (valueLebBad) return -1
            while (n > 0) {
                if (tag == TAG_MAP) {
                    val k = valueLeb(b, end, pos)
                    if (valueLebBad || !valueFits(k, end - pos.at)) return -1
                    pos.at += k.toInt()
                }
                val next = valueSkip(b, end, pos.at)
                if (next < 0) return -1
                pos.at = next
                n--
            }
            return pos.at
        }
        else -> return pos.at
    }
}

private fun valueSame(b: ByteArray, x: Int, y: Int, n: Int): Boolean {
    for (i in 0 until n) if (b[x + i] != b[y + i]) return false
    return true
}

private fun valueWalk(b: ByteArray, end: Int, start: Int, depth: Int, nodes: IntArray): Int {
    if (depth > VALUE_MAX_DEPTH) return -1
    nodes[0]++
    if (nodes[0] > VALUE_MAX_NODES || start >= end) return -1
    val tag = b[start].toInt()
    val pos = ValueCursor(start + 1)
    when (tag) {
        TAG_NULL, TAG_FALSE, TAG_TRUE -> return pos.at
        TAG_INT -> {
            valueLeb(b, end, pos)
            return if (valueLebBad) -1 else pos.at
        }
        TAG_FLOAT -> return if (end - pos.at >= 8) pos.at + 8 else -1
        TAG_STRING, TAG_BYTES -> {
            val n = valueLeb(b, end, pos)
            return if (!valueLebBad && valueFits(n, end - pos.at)) pos.at + n.toInt() else -1
        }
        TAG_LIST -> {
            val n = valueLeb(b, end, pos)
            if (valueLebBad || !valueFits(n, end - pos.at)) return -1
            var i = 0L
            while (i < n) {
                val next = valueWalk(b, end, pos.at, depth + 1, nodes)
                if (next < 0) return -1
                pos.at = next
                i++
            }
            return pos.at
        }
        TAG_MAP -> {
            val n = valueLeb(b, end, pos)
            if (valueLebBad || !valueFits(n, end - pos.at)) return -1
            val first = pos.at
            var i = 0L
            while (i < n) {
                val klen = valueLeb(b, end, pos)
                if (valueLebBad || klen > VALUE_MAX_KEY_BYTES || !valueFits(klen, end - pos.at)) return -1
                val name = pos.at
                val scan = ValueCursor(first)
                var j = 0L
                while (j < i) {
                    val olen = valueLeb(b, end, scan)
                    if (olen == klen && valueSame(b, scan.at, name, klen.toInt())) return -1
                    val after = valueSkip(b, end, scan.at + olen.toInt())
                    if (after < 0) return -1
                    scan.at = after
                    j++
                }
                val next = valueWalk(b, end, name + klen.toInt(), depth + 1, nodes)
                if (next < 0) return -1
                pos.at = next
                i++
            }
            return pos.at
        }
        else -> return -1
    }
}

/** Iterates the items of a list or the entries of a map: call next(), then read item (and key for a map). */
class ValueIter internal constructor(
    private val bytes: ByteArray,
    private var pos: Int,
    private var left: Long,
    private val isMap: Boolean,
) {
    /** The current item after a true next(). */
    var item: Value = Value(bytes, -1)
        private set

    /** The key of the current entry, "" for a list. */
    var key: String = ""
        private set

    /** The raw UTF-8 bytes of the key. */
    var keyBytes: ByteArray = ByteArray(0)
        private set

    fun next(): Boolean {
        val end = bytes.size
        if (left <= 0 || pos >= end) return false
        val cursor = ValueCursor(pos)
        if (isMap) {
            val klen = valueLeb(bytes, end, cursor)
            if (valueLebBad || !valueFits(klen, end - cursor.at)) {
                left = 0
                return false
            }
            keyBytes = bytes.copyOfRange(cursor.at, cursor.at + klen.toInt())
            key = keyBytes.decodeToString()
            cursor.at += klen.toInt()
        }
        item = Value(bytes, cursor.at)
        val after = valueSkip(bytes, end, cursor.at)
        pos = if (after < 0) end else after
        left--
        return true
    }
}

/** A position in a validated document. An absent value has kind() == null and answers every getter with null. */
class Value internal constructor(private val bytes: ByteArray, private val off: Int) {
    companion object {
        /** Validates the whole document (tags, limits, duplicate keys, trailing bytes); null when malformed. */
        fun open(document: ByteArray): Value? {
            if (document.isEmpty() || document.size > VALUE_MAX_BYTES) return null
            return if (valueWalk(document, document.size, 0, 0, IntArray(1)) == document.size) Value(document, 0) else null
        }
    }

    fun kind(): ValueKind? {
        if (off < 0 || off >= bytes.size) return null
        return when (bytes[off].toInt()) {
            TAG_FALSE, TAG_TRUE -> ValueKind.Bool
            TAG_INT -> ValueKind.Int
            TAG_FLOAT -> ValueKind.Float
            TAG_STRING -> ValueKind.String
            TAG_BYTES -> ValueKind.Bytes
            TAG_LIST -> ValueKind.List
            TAG_MAP -> ValueKind.Map
            else -> ValueKind.Null
        }
    }

    val isNull: Boolean get() = kind() == ValueKind.Null

    fun asBool(): Boolean? = if (kind() == ValueKind.Bool) bytes[off].toInt() == TAG_TRUE else null

    fun asLong(): Long? {
        if (kind() != ValueKind.Int) return null
        val raw = valueLeb(bytes, bytes.size, ValueCursor(off + 1))
        if (valueLebBad) return null
        return (raw ushr 1) xor -(raw and 1)
    }

    fun asDouble(): Double? {
        if (kind() != ValueKind.Float || bytes.size - off < 9) return null
        var bits = 0L
        for (i in 0 until 8) bits = bits or ((bytes[off + 1 + i].toLong() and 0xFF) shl (8 * i))
        return Double.fromBits(bits)
    }

    private fun payload(kind: ValueKind): ByteArray? {
        if (kind() != kind) return null
        val pos = ValueCursor(off + 1)
        val n = valueLeb(bytes, bytes.size, pos)
        if (valueLebBad || !valueFits(n, bytes.size - pos.at)) return null
        return bytes.copyOfRange(pos.at, pos.at + n.toInt())
    }

    /** The text of a string value (invalid UTF-8 becomes U+FFFD), or null for another kind. */
    fun asString(): String? = payload(ValueKind.String)?.decodeToString()

    /** A copy of the raw UTF-8 payload of a string value. */
    fun asStringBytes(): ByteArray? = payload(ValueKind.String)

    /** A copy of the payload of a bytes value. */
    fun asBytes(): ByteArray? = payload(ValueKind.Bytes)

    /** Items of a list or entries of a map, -1 for any other value. */
    fun count(): Int {
        val k = kind()
        if (k != ValueKind.List && k != ValueKind.Map) return -1
        val n = valueLeb(bytes, bytes.size, ValueCursor(off + 1))
        return if (valueLebBad) -1 else n.toInt()
    }

    fun iter(): ValueIter {
        val k = kind()
        val pos = ValueCursor(off + 1)
        var left = 0L
        if (k == ValueKind.List || k == ValueKind.Map) {
            val n = valueLeb(bytes, bytes.size, pos)
            left = if (valueLebBad) 0 else n
        }
        return ValueIter(bytes, pos.at, left, k == ValueKind.Map)
    }

    fun at(index: Int): Value? {
        if (kind() != ValueKind.List || index < 0) return null
        val it = iter()
        var i = 0
        while (it.next()) {
            if (i == index) return it.item
            i++
        }
        return null
    }

    fun find(name: String): Value? {
        if (kind() != ValueKind.Map) return null
        val wanted = name.encodeToByteArray()
        val it = iter()
        while (it.next()) {
            if (it.keyBytes.contentEquals(wanted)) return it.item
        }
        return null
    }
}

const val CALL_TOO_LARGE = -100

/** The outcome of callOut: `error` is 0 or a negative import result (CALL_TOO_LARGE when nothing fit). */
class OutResult(val error: Int, val bytes: ByteArray) {
    /** The answer validated as a document, or null. */
    fun value(): Value? = if (error == 0) Value.open(bytes) else null
}

private fun valueCopyOut(at: Int, n: Int): ByteArray {
    val p = Pointer(at.toUInt())
    return ByteArray(n) { (p + it).loadByte() }
}

/** Calls an import that fills (out, cap) and answers with the length it needs: with `initial` bytes, then once more
 * with exactly the needed size when the answer did not fit. The receiver allocates any other arguments. */
fun callOut(initial: Int = 256, call: MemoryAllocator.(Int, Int) -> Int): OutResult {
    var cap = if (initial < 1) 1 else initial
    var retried = false
    while (true) {
        var need = -1
        var failure = 0
        var data: ByteArray? = null
        withScopedMemoryAllocator { a ->
            val out = a.allocate(cap).address.toInt()
            val n = a.call(out, cap)
            if (n < 0) failure = n else if (n > cap) need = n else data = valueCopyOut(out, n)
        }
        if (failure != 0) return OutResult(failure, ByteArray(0))
        val answer = data
        if (answer != null) return OutResult(0, answer)
        if (retried || need > VALUE_MAX_BYTES) return OutResult(CALL_TOO_LARGE, ByteArray(0))
        cap = need
        retried = true
    }
}
