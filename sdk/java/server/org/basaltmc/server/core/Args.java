package org.basaltmc.server.core;

import org.basaltmc.server.internal.Native;

/** Named arguments for text: add pairs, then pass them to a send, translate or reply call. */
public final class Args {
    private static final byte[] NONE = new byte[0];

    private byte[] body = new byte[64];
    private int size;
    private int count;

    public Args add(String key, String value) {
        count++;
        put(Native.utf8(key));
        put(Native.utf8(value));
        return this;
    }

    private void put(byte[] data) {
        reserve(4 + data.length);
        for (int shift = 0; shift < 32; shift += 8) {
            body[size++] = (byte) (data.length >> shift);
        }
        for (int i = 0; i < data.length; i++) {
            body[size++] = data[i];
        }
    }

    private void reserve(int more) {
        if (size + more > body.length) {
            byte[] grown = new byte[Math.max(body.length * 2, size + more)];
            for (int i = 0; i < size; i++) {
                grown[i] = body[i];
            }
            body = grown;
        }
    }

    /** The packed list the host takes: a count, then length-prefixed keys and values. */
    public byte[] packed() {
        if (count == 0) {
            return NONE;
        }
        byte[] out = new byte[4 + size];
        for (int i = 0; i < 4; i++) {
            out[i] = (byte) (count >> (8 * i));
        }
        for (int i = 0; i < size; i++) {
            out[4 + i] = body[i];
        }
        return out;
    }
}
