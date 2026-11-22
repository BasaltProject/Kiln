using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using Basalt.Proxy.Core;
using Basalt.Proxy.Internal;
using Basalt.Proxy.Values;
using static Basalt.Proxy.Internal.Native;
using static Basalt.Proxy.Core.Level;
using static Basalt.Proxy.Core.Priority;
namespace Basalt.Proxy.Log;

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
