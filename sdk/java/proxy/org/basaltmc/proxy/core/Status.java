package org.basaltmc.proxy.core;

/** Results of the host calls: OK or a negative error code. */
public final class Status {
    private Status() {}

    public static final int OK = 0;
    public static final int ABSENT = -1;
    public static final int STALE = -2;
    public static final int INVALID = -3;
    public static final int DENIED = -4;
}
