package org.basaltmc.proxy.internal;

import org.basaltmc.proxy.core.Args;
import org.teavm.interop.Address;

/** Strings, buffers and argument lists as the raw imports take them. Not part of the plugin API. */
public final class Native {
    private Native() {}

    private static final byte[] NONE = new byte[0];

    public static byte[] utf8(String text) {
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
        for (int i = 0; i < at; i++) {
            exact[i] = out[i];
        }
        return exact;
    }

    public static String text(byte[] data, int length) {
        char[] chars = new char[length];
        int count = 0;
        int i = 0;
        while (i < length) {
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
                c = 0xFFFD;
                extra = 0;
            }
            i++;
            while (extra > 0 && i < length) {
                c = (c << 6) | (data[i] & 0x3F);
                i++;
                extra--;
            }
            if (c >= 0x10000) {
                c -= 0x10000;
                chars[count++] = (char) (0xD800 + (c >> 10));
                chars[count++] = (char) (0xDC00 + (c & 0x3FF));
            } else {
                chars[count++] = (char) c;
            }
        }
        return new String(chars, 0, count);
    }

    public static Address at(byte[] data) {
        return data.length == 0 ? Address.fromInt(0) : Address.ofData(data);
    }

    public static byte[] packed(Args args) {
        return args == null ? NONE : args.packed();
    }

    public static byte[] orEmpty(byte[] data) {
        return data == null ? NONE : data;
    }

    /** An import that fills (out, cap) and answers with the length it needs. */
    public interface Fetch {
        int call(Address out, int cap);
    }

    public static String fetch(Fetch call) {
        return fetch(256, call);
    }

    /** The text an import wrote, growing the buffer until it fits; null on a negative result. */
    public static String fetch(int initial, Fetch call) {
        int cap = initial;
        while (true) {
            byte[] out = new byte[cap];
            int n = call.call(at(out), cap);
            if (n < 0) {
                return null;
            }
            if (n <= cap) {
                return text(out, n);
            }
            cap = n;
        }
    }

    /** The handles an import wrote into an int array, growing it until they fit; empty on a negative result. */
    public static int[] handles(Fetch call) {
        int cap = 64;
        while (true) {
            int[] out = new int[cap];
            int n = call.call(Address.ofData(out), cap);
            if (n < 0) {
                return new int[0];
            }
            if (n <= cap) {
                int[] exact = new int[n];
                for (int i = 0; i < n; i++) {
                    exact[i] = out[i];
                }
                return exact;
            }
            cap = n;
        }
    }

    public static int le32(byte[] data, int at) {
        return (data[at] & 0xFF) | ((data[at + 1] & 0xFF) << 8) | ((data[at + 2] & 0xFF) << 16)
                | ((data[at + 3] & 0xFF) << 24);
    }

    public static long le64(byte[] data, int at) {
        return (le32(data, at) & 0xFFFFFFFFL) | ((long) le32(data, at + 4) << 32);
    }

    public static byte[] slice(byte[] data, int from, int length) {
        byte[] out = new byte[length];
        for (int i = 0; i < length; i++) {
            out[i] = data[from + i];
        }
        return out;
    }
}
