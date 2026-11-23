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

namespace Basalt.Server.Tasks;

public static unsafe partial class Scheduler {
    public static int After(int delay, Handler handler) => RawTaskAfter(delay, TokenFor(handler));

    public static int Every(int delay, int period, Handler handler) => RawTaskEvery(delay, period, TokenFor(handler));

    public static int TaskCancel(int task) => RawTaskCancel(task);
}
