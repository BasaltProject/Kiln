using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using Basalt.Server.Core;
using Basalt.Server.Internal;
using Basalt.Server.Values;
using static Basalt.Server.Internal.Native;
using static Basalt.Server.Core.Level;
using static Basalt.Server.Core.Priority;
namespace Basalt.Server.Log;

public static unsafe partial class Logger {
    public static void Log(int level, string msg)
    {
        byte[] m = Utf8(msg);
        fixed (byte* p = m) RawLog(level, p, m.Length);
    }

    public static void LogDebug(string msg) => Log(Debug, msg);

    public static void LogInfo(string msg) => Log(Info, msg);

    public static void LogWarn(string msg) => Log(Warn, msg);

    public static void LogError(string msg) => Log(Error, msg);
}
