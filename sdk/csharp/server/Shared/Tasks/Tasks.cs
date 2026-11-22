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
namespace Basalt.Server.Tasks;

public static unsafe partial class Tasks {
    public static int After(int delay, Handler handler) => RawTaskAfter(delay, TokenFor(handler));

    public static int Every(int delay, int period, Handler handler) => RawTaskEvery(delay, period, TokenFor(handler));

    public static int TaskCancel(int task) => RawTaskCancel(task);
}
