using Basalt.Proxy.Core;
using Basalt.Proxy.Internal;
using Basalt.Proxy.Values;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using static Basalt.Proxy.Core.Level;
using static Basalt.Proxy.Core.Priority;
using static Basalt.Proxy.Internal.Native;

namespace Basalt.Proxy.Tasks;

public static unsafe partial class Scheduler {
    public static int After(int delay, Handler handler) => RawTaskAfter(delay, TokenFor(handler));

    public static int Every(int delay, int period, Handler handler) => RawTaskEvery(delay, period, TokenFor(handler));

    public static int TaskCancel(int task) => RawTaskCancel(task);
}
