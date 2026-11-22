package org.basaltmc.proxy.players;
import org.teavm.interop.Address;
import org.basaltmc.proxy.core.*;
import static org.basaltmc.proxy.core.Level.*;
import static org.basaltmc.proxy.core.Priority.*;
import static org.basaltmc.proxy.internal.Native.*;
import static org.basaltmc.proxy.internal.RawCore.*;
import static org.basaltmc.proxy.internal.Callbacks.*;

public abstract class SharedPlayers {
    public static int playerByName(String name) {
        byte[] n = utf8(name);
        return rawPlayerByName(at(n), n.length);
    }

    public static int[] playersOnline() {
        int cap = 64;
        while (true) {
            int[] out = new int[cap];
            int n = rawPlayersOnline(Address.ofData(out), cap);
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

    public static String playerName(int player) { return fetch((out, cap) -> rawPlayerName(player, out, cap)); }

    public static int send(int player, String text) { return send(player, text, null); }

    public static int send(int player, String text, Args args) {
        byte[] t = utf8(text);
        byte[] a = packed(args);
        return rawPlayerSend(player, at(t), t.length, at(a), a.length);
    }

    public static int actionBar(int player, String text, Args args) {
        byte[] t = utf8(text);
        byte[] a = packed(args);
        return rawPlayerActionBar(player, at(t), t.length, at(a), a.length);
    }
}
