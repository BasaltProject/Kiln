package org.basaltmc.proxy.tasks;
import org.basaltmc.proxy.core.*;
import static org.basaltmc.proxy.core.Level.*;
import static org.basaltmc.proxy.core.Priority.*;
import static org.basaltmc.proxy.internal.Native.*;
import static org.basaltmc.proxy.internal.RawCore.*;
import static org.basaltmc.proxy.internal.Callbacks.*;

public abstract class SharedTasks {
    public static int after(int delay, Handler handler) { return rawTaskAfter(delay, tokenFor(handler)); }

    public static int every(int delay, int period, Handler handler) {
        return rawTaskEvery(delay, period, tokenFor(handler));
    }

    public static int taskCancel(int task) { return rawTaskCancel(task); }
}
