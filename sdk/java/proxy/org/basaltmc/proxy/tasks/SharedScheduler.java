package org.basaltmc.proxy.tasks;

import org.basaltmc.proxy.core.*;

import static org.basaltmc.proxy.core.Level.*;
import static org.basaltmc.proxy.core.Priority.*;
import static org.basaltmc.proxy.internal.Callbacks.*;
import static org.basaltmc.proxy.internal.Native.*;
import static org.basaltmc.proxy.internal.RawCore.*;

abstract class SharedScheduler {
    public static int after(int delay, Handler handler) { return rawTaskAfter(delay, tokenFor(handler)); }

    public static int every(int delay, int period, Handler handler) {
        return rawTaskEvery(delay, period, tokenFor(handler));
    }

    public static int taskCancel(int task) { return rawTaskCancel(task); }
}
