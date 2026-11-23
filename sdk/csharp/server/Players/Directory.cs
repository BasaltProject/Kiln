using Basalt.Server.Core;
using Basalt.Server.Internal;
using Basalt.Server.Values;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using static Basalt.Server.Core.Level;
using static Basalt.Server.Core.Priority;
using static Basalt.Server.Internal.Native;

namespace Basalt.Server.Players;

public static unsafe partial class Directory {
    public static int Particle(int player, string id, int count, double spread)
    {
        byte[] i = Utf8(id);
        fixed (byte* p = i) return RawPlayerParticle(player, p, i.Length, count, spread);
    }

    public static int Sound(int player, string id, float volume, float pitch)
    {
        byte[] i = Utf8(id);
        fixed (byte* p = i) return RawPlayerSound(player, p, i.Length, volume, pitch);
    }
}
