package org.basaltmc.server.core;

/** Event priorities, lowest runs first. */
public final class Priority {
    private Priority() {}

    public static final int LOWEST = 0;
    public static final int LOW = 1;
    public static final int NORMAL = 2;
    public static final int HIGH = 3;
    public static final int HIGHEST = 4;
}
