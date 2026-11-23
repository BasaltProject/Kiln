package org.basaltmc.proxy.values;

import java.util.ArrayList;
import org.basaltmc.proxy.core.*;
import org.basaltmc.proxy.internal.*;
import org.basaltmc.proxy.values.Documents;
import org.teavm.interop.Address;

/** Value builder and reader over the byte format of basalt_values.h. The writer grows a heap buffer (an optional limit
 * acts as a fixed capacity); the reader works in place on a validated document and never throws on malformed input. */
public final class Documents {
    private Documents() {}

    public static final int MAX_BYTES = 1 << 20;
    public static final int MAX_DEPTH = 16;
    public static final int MAX_NODES = 65536;
    public static final int MAX_KEY_BYTES = 256;

    public static final int ABSENT = -1;
    public static final int NULL = 0;
    public static final int BOOL = 1;
    public static final int INT = 2;
    public static final int FLOAT = 3;
    public static final int STRING = 4;
    public static final int BYTES = 5;
    public static final int LIST = 6;
    public static final int MAP = 7;

    public static final int OK = 0;
    public static final int INVALID = -3;
    public static final int OVERFLOW = -5;
    public static final int TOO_LARGE = -100;

    private static final int TAG_NULL = 0;
    private static final int TAG_FALSE = 1;
    private static final int TAG_TRUE = 2;
    private static final int TAG_INT = 3;
    private static final int TAG_FLOAT = 4;
    private static final int TAG_STRING = 5;
    private static final int TAG_BYTES = 6;
    private static final int TAG_LIST = 7;
    private static final int TAG_MAP = 8;

    private static byte[] utf8(String text) {
        int length = text.length();
        byte[] out = new byte[length * 3];
        int at = 0;
        for (int i = 0; i < length; i++) {
            int c = text.charAt(i);
            if (c >= 0xD800 && c <= 0xDBFF && i + 1 < length) {
                int low = text.charAt(i + 1);
                if (low >= 0xDC00 && low <= 0xDFFF) {
                    c = 0x10000 + ((c - 0xD800) << 10) + (low - 0xDC00);
                    i++;
                }
            }
            if (c < 0x80) {
                out[at++] = (byte) c;
            } else if (c < 0x800) {
                out[at++] = (byte) (0xC0 | (c >> 6));
                out[at++] = (byte) (0x80 | (c & 0x3F));
            } else if (c < 0x10000) {
                out[at++] = (byte) (0xE0 | (c >> 12));
                out[at++] = (byte) (0x80 | ((c >> 6) & 0x3F));
                out[at++] = (byte) (0x80 | (c & 0x3F));
            } else {
                out[at++] = (byte) (0xF0 | (c >> 18));
                out[at++] = (byte) (0x80 | ((c >> 12) & 0x3F));
                out[at++] = (byte) (0x80 | ((c >> 6) & 0x3F));
                out[at++] = (byte) (0x80 | (c & 0x3F));
            }
        }
        byte[] exact = new byte[at];
        System.arraycopy(out, 0, exact, 0, at);
        return exact;
    }

    private static String text(byte[] data, int from, int length) {
        char[] chars = new char[length];
        int count = 0;
        int i = from;
        int end = from + length;
        while (i < end) {
            int b = data[i] & 0xFF;
            int c;
            int extra;
            if (b < 0x80) {
                c = b;
                extra = 0;
            } else if (b >= 0xF0) {
                c = b & 0x07;
                extra = 3;
            } else if (b >= 0xE0) {
                c = b & 0x0F;
                extra = 2;
            } else if (b >= 0xC0) {
                c = b & 0x1F;
                extra = 1;
            } else {
                chars[count++] = '�';
                i++;
                continue;
            }
            i++;
            boolean broken = false;
            for (int k = 0; k < extra; k++) {
                if (i >= end || (data[i] & 0xC0) != 0x80) {
                    broken = true;
                    break;
                }
                c = (c << 6) | (data[i] & 0x3F);
                i++;
            }
            if (broken || c > 0x10FFFF || (c >= 0xD800 && c <= 0xDFFF)) {
                chars[count++] = '�';
            } else if (c >= 0x10000) {
                c -= 0x10000;
                chars[count++] = (char) (0xD800 + (c >> 10));
                chars[count++] = (char) (0xDC00 + (c & 0x3FF));
            } else {
                chars[count++] = (char) c;
            }
        }
        return new String(chars, 0, count);
    }

    private static final class Frame {
        int start;
        int count;
        boolean isMap;
        boolean pendingKey;
    }

    /** Builds one document. A failure is sticky: later calls do nothing and finish() returns null. */
    public static final class Writer {
        private byte[] buf = new byte[64];
        private int len;
        private final int limit;
        private int nodes;
        private final ArrayList<Frame> frames = new ArrayList<>();
        private int failure = OK;

        public Writer() {
            this(MAX_BYTES);
        }

        /** Fails with OVERFLOW once the document would pass `limit` bytes. */
        public Writer(int limit) {
            this.limit = Math.min(limit, MAX_BYTES);
        }

        /** OK while healthy, else INVALID or OVERFLOW. */
        public int error() {
            return failure;
        }

        public boolean ok() {
            return failure == OK;
        }

        private void fail(int reason) {
            if (failure == OK) {
                failure = reason;
            }
        }

        private boolean room(int extra) {
            if (failure != OK) {
                return false;
            }
            if (extra < 0 || extra > limit - len) {
                fail(OVERFLOW);
                return false;
            }
            int need = len + extra;
            if (need > buf.length) {
                int size = buf.length * 2;
                while (size < need) {
                    size *= 2;
                }
                byte[] grown = new byte[size];
                System.arraycopy(buf, 0, grown, 0, len);
                buf = grown;
            }
            return true;
        }

        private void put(int b) {
            if (room(1)) {
                buf[len++] = (byte) b;
            }
        }

        private void leb(long value) {
            while ((value & ~0x7FL) != 0) {
                put((int) (value & 0x7F) | 0x80);
                value >>>= 7;
            }
            put((int) value);
        }

        private void raw(byte[] data) {
            if (room(data.length)) {
                System.arraycopy(data, 0, buf, len, data.length);
                len += data.length;
            }
        }

        private boolean begin() {
            if (failure != OK) {
                return false;
            }
            int n = frames.size();
            if (n == 0) {
                if (nodes > 0) {
                    fail(INVALID);
                    return false;
                }
            } else {
                Frame top = frames.get(n - 1);
                if (top.isMap) {
                    if (!top.pendingKey) {
                        fail(INVALID);
                        return false;
                    }
                    top.pendingKey = false;
                } else {
                    top.count++;
                }
            }
            nodes++;
            if (n > MAX_DEPTH || nodes > MAX_NODES) {
                fail(INVALID);
                return false;
            }
            return true;
        }

        public Writer nul() {
            if (begin()) {
                put(TAG_NULL);
            }
            return this;
        }

        public Writer bool(boolean value) {
            if (begin()) {
                put(value ? TAG_TRUE : TAG_FALSE);
            }
            return this;
        }

        public Writer integer(long value) {
            if (begin()) {
                put(TAG_INT);
                leb((value << 1) ^ (value >> 63));
            }
            return this;
        }

        public Writer number(double value) {
            if (begin()) {
                put(TAG_FLOAT);
                long bits = Double.doubleToRawLongBits(value);
                for (int shift = 0; shift < 64; shift += 8) {
                    put((int) (bits >>> shift) & 0xFF);
                }
            }
            return this;
        }

        private void blob(int tag, byte[] data) {
            if (begin()) {
                put(tag);
                leb(data.length);
                raw(data);
            }
        }

        public Writer string(String text) {
            blob(TAG_STRING, utf8(text));
            return this;
        }

        public Writer bytes(byte[] data) {
            blob(TAG_BYTES, data);
            return this;
        }

        private void open(boolean isMap) {
            if (begin()) {
                put(isMap ? TAG_MAP : TAG_LIST);
                if (failure != OK) {
                    return;
                }
                Frame frame = new Frame();
                frame.start = len;
                frame.isMap = isMap;
                frames.add(frame);
            }
        }

        public Writer beginList() {
            open(false);
            return this;
        }

        public Writer beginMap() {
            open(true);
            return this;
        }

        /** Closes the innermost list or map, inserting its count in front of the items. */
        public Writer end() {
            if (failure != OK) {
                return this;
            }
            int n = frames.size();
            if (n == 0 || (frames.get(n - 1).isMap && frames.get(n - 1).pendingKey)) {
                fail(INVALID);
                return this;
            }
            Frame top = frames.remove(n - 1);
            int size = 1;
            for (long v = top.count; v >= 0x80; v >>>= 7) {
                size++;
            }
            if (!room(size)) {
                return this;
            }
            System.arraycopy(buf, top.start, buf, top.start + size, len - top.start);
            long c = top.count;
            int at = top.start;
            while (c >= 0x80) {
                buf[at++] = (byte) ((c & 0x7F) | 0x80);
                c >>>= 7;
            }
            buf[at] = (byte) c;
            len += size;
            return this;
        }

        /** The next key of the open map; the value follows. */
        public Writer key(String name) {
            if (failure != OK) {
                return this;
            }
            int n = frames.size();
            byte[] encoded = utf8(name);
            if (n == 0 || !frames.get(n - 1).isMap || frames.get(n - 1).pendingKey
                    || encoded.length > MAX_KEY_BYTES) {
                fail(INVALID);
                return this;
            }
            leb(encoded.length);
            raw(encoded);
            frames.get(n - 1).count++;
            frames.get(n - 1).pendingKey = true;
            return this;
        }

        /** A copy of the finished document, or null when the writer failed or a container is still open. */
        public byte[] finish() {
            if (failure != OK) {
                return null;
            }
            if (!frames.isEmpty() || nodes == 0) {
                failure = INVALID;
                return null;
            }
            byte[] out = new byte[len];
            System.arraycopy(buf, 0, out, 0, len);
            return out;
        }
    }

    // A varint reader that advances a one-element position array; `bad` says whether the last read failed.
    private static boolean bad;

    private static long leb(byte[] b, int end, int[] pos) {
        long result = 0;
        int at = pos[0];
        bad = false;
        for (int shift = 0; shift < 70; shift += 7) {
            if (at >= end) {
                bad = true;
                return 0;
            }
            int c = b[at++] & 0xFF;
            if (shift == 63 && c > 1) {
                bad = true;
                return 0;
            }
            result |= (long) (c & 0x7F) << shift;
            if ((c & 0x80) == 0) {
                pos[0] = at;
                return result;
            }
        }
        bad = true;
        return 0;
    }

    private static boolean fits(long n, int room) {
        return n >= 0 && n <= room;
    }

    private static int skip(byte[] b, int end, int start) {
        if (start < 0 || start >= end) {
            return -1;
        }
        int tag = b[start];
        int[] pos = {start + 1};
        switch (tag) {
            case TAG_INT:
                leb(b, end, pos);
                return bad ? -1 : pos[0];
            case TAG_FLOAT:
                return end - pos[0] >= 8 ? pos[0] + 8 : -1;
            case TAG_STRING:
            case TAG_BYTES: {
                long n = leb(b, end, pos);
                return !bad && fits(n, end - pos[0]) ? pos[0] + (int) n : -1;
            }
            case TAG_LIST:
            case TAG_MAP: {
                long n = leb(b, end, pos);
                if (bad) {
                    return -1;
                }
                for (; n > 0; n--) {
                    if (tag == TAG_MAP) {
                        long k = leb(b, end, pos);
                        if (bad || !fits(k, end - pos[0])) {
                            return -1;
                        }
                        pos[0] += (int) k;
                    }
                    int next = skip(b, end, pos[0]);
                    if (next < 0) {
                        return -1;
                    }
                    pos[0] = next;
                }
                return pos[0];
            }
            default:
                return pos[0];
        }
    }

    private static boolean same(byte[] b, int x, int y, int n) {
        for (int i = 0; i < n; i++) {
            if (b[x + i] != b[y + i]) {
                return false;
            }
        }
        return true;
    }

    private static int walk(byte[] b, int end, int start, int depth, int[] nodes) {
        if (depth > MAX_DEPTH) {
            return -1;
        }
        nodes[0]++;
        if (nodes[0] > MAX_NODES || start >= end) {
            return -1;
        }
        int tag = b[start];
        int[] pos = {start + 1};
        switch (tag) {
            case TAG_NULL:
            case TAG_FALSE:
            case TAG_TRUE:
                return pos[0];
            case TAG_INT:
                leb(b, end, pos);
                return bad ? -1 : pos[0];
            case TAG_FLOAT:
                return end - pos[0] >= 8 ? pos[0] + 8 : -1;
            case TAG_STRING:
            case TAG_BYTES: {
                long n = leb(b, end, pos);
                return !bad && fits(n, end - pos[0]) ? pos[0] + (int) n : -1;
            }
            case TAG_LIST: {
                long n = leb(b, end, pos);
                if (bad || !fits(n, end - pos[0])) {
                    return -1;
                }
                for (long i = 0; i < n; i++) {
                    int next = walk(b, end, pos[0], depth + 1, nodes);
                    if (next < 0) {
                        return -1;
                    }
                    pos[0] = next;
                }
                return pos[0];
            }
            case TAG_MAP: {
                long n = leb(b, end, pos);
                if (bad || !fits(n, end - pos[0])) {
                    return -1;
                }
                int first = pos[0];
                for (long i = 0; i < n; i++) {
                    long klen = leb(b, end, pos);
                    if (bad || klen > MAX_KEY_BYTES || !fits(klen, end - pos[0])) {
                        return -1;
                    }
                    int name = pos[0];
                    int[] scan = {first};
                    for (long j = 0; j < i; j++) {
                        long olen = leb(b, end, scan);
                        if (olen == klen && same(b, scan[0], name, (int) klen)) {
                            return -1;
                        }
                        int after = skip(b, end, scan[0] + (int) olen);
                        if (after < 0) {
                            return -1;
                        }
                        scan[0] = after;
                    }
                    int next = walk(b, end, name + (int) klen, depth + 1, nodes);
                    if (next < 0) {
                        return -1;
                    }
                    pos[0] = next;
                }
                return pos[0];
            }
            default:
                return -1;
        }
    }

    /** Iterates the items of a list or the entries of a map: call next(), then read item (and key for a map). */
    public static final class Iter {
        private final byte[] bytes;
        private int pos;
        private int left;
        private final boolean isMap;
        /** The current item after a true next(). */
        public Value item;
        /** The key of the current entry, "" for a list. */
        public String key = "";
        /** The raw UTF-8 bytes of the key. */
        public byte[] keyBytes = new byte[0];

        private Iter(byte[] bytes, int pos, int left, boolean isMap) {
            this.bytes = bytes;
            this.pos = pos;
            this.left = left;
            this.isMap = isMap;
        }

        public boolean next() {
            int end = bytes.length;
            if (left <= 0 || pos >= end) {
                return false;
            }
            int[] cursor = {pos};
            if (isMap) {
                long klen = leb(bytes, end, cursor);
                if (bad || !fits(klen, end - cursor[0])) {
                    left = 0;
                    return false;
                }
                keyBytes = new byte[(int) klen];
                System.arraycopy(bytes, cursor[0], keyBytes, 0, (int) klen);
                key = text(bytes, cursor[0], (int) klen);
                cursor[0] += (int) klen;
            }
            item = new Value(bytes, cursor[0]);
            int after = skip(bytes, end, cursor[0]);
            pos = after < 0 ? end : after;
            left--;
            return true;
        }
    }

    /** A position in a validated document. An absent value has kind() == ABSENT and answers every getter with its
     * fallback; check kind() first when a fallback would be ambiguous. */
    public static final class Value {
        private final byte[] bytes;
        private final int off;

        private Value(byte[] bytes, int off) {
            this.bytes = bytes;
            this.off = off;
        }

        /** Validates the whole document (tags, limits, duplicate keys, trailing bytes); null when malformed. */
        public static Value open(byte[] document) {
            if (document == null || document.length == 0 || document.length > MAX_BYTES) {
                return null;
            }
            int[] nodes = {0};
            return walk(document, document.length, 0, 0, nodes) == document.length ? new Value(document, 0) : null;
        }

        public int kind() {
            if (off < 0 || off >= bytes.length) {
                return ABSENT;
            }
            switch (bytes[off]) {
                case TAG_FALSE:
                case TAG_TRUE:
                    return BOOL;
                case TAG_INT:
                    return INT;
                case TAG_FLOAT:
                    return FLOAT;
                case TAG_STRING:
                    return STRING;
                case TAG_BYTES:
                    return BYTES;
                case TAG_LIST:
                    return LIST;
                case TAG_MAP:
                    return MAP;
                default:
                    return NULL;
            }
        }

        public boolean isNull() {
            return kind() == NULL;
        }

        public boolean present() {
            return kind() != ABSENT;
        }

        public boolean asBool(boolean fallback) {
            return kind() == BOOL ? bytes[off] == TAG_TRUE : fallback;
        }

        public long asLong(long fallback) {
            if (kind() != INT) {
                return fallback;
            }
            int[] pos = {off + 1};
            long raw = leb(bytes, bytes.length, pos);
            return bad ? fallback : (raw >>> 1) ^ -(raw & 1);
        }

        public double asDouble(double fallback) {
            if (kind() != FLOAT || bytes.length - off < 9) {
                return fallback;
            }
            long bits = 0;
            for (int i = 0; i < 8; i++) {
                bits |= (long) (bytes[off + 1 + i] & 0xFF) << (8 * i);
            }
            return Double.longBitsToDouble(bits);
        }

        private byte[] payload(int kind) {
            if (kind() != kind) {
                return null;
            }
            int[] pos = {off + 1};
            long n = leb(bytes, bytes.length, pos);
            if (bad || !fits(n, bytes.length - pos[0])) {
                return null;
            }
            byte[] out = new byte[(int) n];
            System.arraycopy(bytes, pos[0], out, 0, (int) n);
            return out;
        }

        /** The text of a string value (invalid UTF-8 becomes U+FFFD), or null for another kind. */
        public String asString() {
            byte[] data = payload(STRING);
            return data == null ? null : text(data, 0, data.length);
        }

        /** A copy of the raw UTF-8 payload of a string value. */
        public byte[] asStringBytes() {
            return payload(STRING);
        }

        /** A copy of the payload of a bytes value. */
        public byte[] asBytes() {
            return payload(BYTES);
        }

        /** Items of a list or entries of a map, -1 for any other value. */
        public int count() {
            int k = kind();
            if (k != LIST && k != MAP) {
                return -1;
            }
            int[] pos = {off + 1};
            long n = leb(bytes, bytes.length, pos);
            return bad ? -1 : (int) n;
        }

        public Iter iter() {
            int k = kind();
            int[] pos = {off + 1};
            int left = 0;
            if (k == LIST || k == MAP) {
                long n = leb(bytes, bytes.length, pos);
                left = bad ? 0 : (int) n;
            }
            return new Iter(bytes, pos[0], left, k == MAP);
        }

        public Value at(int index) {
            if (kind() != LIST || index < 0) {
                return null;
            }
            Iter it = iter();
            for (int i = 0; it.next(); i++) {
                if (i == index) {
                    return it.item;
                }
            }
            return null;
        }

        public Value find(String name) {
            if (kind() != MAP) {
                return null;
            }
            byte[] wanted = utf8(name);
            Iter it = iter();
            while (it.next()) {
                byte[] k = it.keyBytes;
                if (k.length != wanted.length) {
                    continue;
                }
                boolean equal = true;
                for (int i = 0; i < k.length; i++) {
                    if (k[i] != wanted[i]) {
                        equal = false;
                        break;
                    }
                }
                if (equal) {
                    return it.item;
                }
            }
            return null;
        }
    }

    /** An import that fills (out, cap) and answers with the length it needs. */
    public interface Out {
        int call(Address out, int cap);
    }

    /** The outcome of callOut: `error` is OK or a negative import result (TOO_LARGE when nothing fit). */
    public static final class OutResult {
        public int error;
        public byte[] bytes = new byte[0];

        /** The answer validated as a document, or null. */
        public Value value() {
            return error == OK ? Value.open(bytes) : null;
        }
    }

    /** Calls `call` with `initial` bytes, then once more with exactly the needed size when the answer did not fit. */
    public static OutResult callOut(int initial, Out call) {
        OutResult result = new OutResult();
        byte[] buf = new byte[Math.max(initial, 1)];
        int n = call.call(Address.ofData(buf), buf.length);
        if (n < 0) {
            result.error = n;
            return result;
        }
        if (n > buf.length) {
            if (n > MAX_BYTES) {
                result.error = TOO_LARGE;
                return result;
            }
            buf = new byte[n];
            n = call.call(Address.ofData(buf), buf.length);
            if (n < 0) {
                result.error = n;
                return result;
            }
            if (n > buf.length) {
                result.error = TOO_LARGE;
                return result;
            }
        }
        byte[] exact = new byte[n];
        System.arraycopy(buf, 0, exact, 0, n);
        result.bytes = exact;
        return result;
    }
}
