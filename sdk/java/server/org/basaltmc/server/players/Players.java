package org.basaltmc.server.players;
import org.basaltmc.server.core.*;
import static org.basaltmc.server.internal.Native.*;
import static org.basaltmc.server.internal.RawServerCore.*;

public final class Players extends SharedPlayers {

    public static int particle(int player, String id, int count, double spread) {
        byte[] i = utf8(id);
        return rawPlayerParticle(player, at(i), i.length, count, spread);
    }

    public static int sound(int player, String id, float volume, float pitch) {
        byte[] i = utf8(id);
        return rawPlayerSound(player, at(i), i.length, volume, pitch);
    }

}
