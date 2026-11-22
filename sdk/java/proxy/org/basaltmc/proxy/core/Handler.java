package org.basaltmc.proxy.core;

/** A callback the host runs with the handle of the call (event, command, task, menu click, placeholder). */
@FunctionalInterface
public interface Handler {
    void run(int ctx);
}
