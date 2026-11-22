package org.basaltmc.server.tasks;
import org.basaltmc.server.core.*;
import static org.basaltmc.server.core.Level.*;
import static org.basaltmc.server.core.Priority.*;
import static org.basaltmc.server.internal.Native.*;
import static org.basaltmc.server.internal.RawCore.*;
import static org.basaltmc.server.internal.Callbacks.*;

public abstract class SharedTasks {
    public static int after(int delay, Handler handler) { return rawTaskAfter(delay, tokenFor(handler)); }

    public static int every(int delay, int period, Handler handler) {
        return rawTaskEvery(delay, period, tokenFor(handler));
    }

    public static int taskCancel(int task) { return rawTaskCancel(task); }
}
