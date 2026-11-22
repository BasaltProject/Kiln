package org.basaltmc.server.data;
import java.util.ArrayList;
import java.util.List;
import org.basaltmc.server.core.*;
import static org.basaltmc.server.internal.Native.*;
import static org.basaltmc.server.internal.RawServerCore.*;

public final class Data extends SharedData {

    public static final class TopEntry {
        public final String name;
        public final long value;

        TopEntry(String name, long value) {
            this.name = name;
            this.value = value;
        }
    }

    /** The `limit` highest values of a numeric data key, best first. */

    public static List<TopEntry> storeTop(String key, int limit) {
        byte[] k = utf8(key);
        int cap = 1024;
        while (true) {
            byte[] out = new byte[cap];
            int n = rawStoreTop(at(k), k.length, limit, at(out), cap);
            if (n < 0) {
                return new ArrayList<>();
            }
            if (n > cap) {
                cap = n;
                continue;
            }
            List<TopEntry> rows = new ArrayList<>();
            int pos = 0;
            while (pos + 4 <= n) {
                int len = le32(out, pos);
                pos += 4;
                if (pos + len + 8 > n) {
                    break;
                }
                String name = text(slice(out, pos, len), len);
                pos += len;
                rows.add(new TopEntry(name, le64(out, pos)));
                pos += 8;
            }
            return rows;
        }
    }

}
