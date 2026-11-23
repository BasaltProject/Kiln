package org.basaltmc.server.services;

import org.basaltmc.server.core.*;
import org.basaltmc.server.internal.*;
import org.basaltmc.server.values.Documents;
import org.basaltmc.server.values.Documents.OutResult;
import org.basaltmc.server.values.Documents.Value;
import org.teavm.interop.Address;

/** Server area: permissions, dynamic commands, broadcast, files, storage, services and scheduler completions. */
public final class RuntimeServices {
    private RuntimeServices() {}

    public static final int BROADCAST_PLAYERS = 1;
    public static final int BROADCAST_CONSOLE = 2;
    public static final int PROVIDE_CHECK = 1;
    public static final int PROVIDE_META = 2;

    private static final int BUFFER = 1024;
    private static final byte[] NONE = new byte[0];

    /** An answer: `code` is 0 or a negative import result, `value` the document when there is one, else null. */
    public static final class Reply {
        public final int code;
        public final Value value;

        private Reply(int code, Value value) {
            this.code = code;
            this.value = value;
        }
    }

    private interface Fill {
        int call(Address out, int cap);
    }

    private static Reply reply(Fill fill) {
        OutResult result = Documents.callOut(BUFFER, fill::call);
        if (result.error != Documents.OK) {
            return new Reply(result.error, null);
        }
        Value value = Value.open(result.bytes);
        return new Reply(value == null ? Documents.INVALID : Documents.OK, value);
    }

    private static String text(int capacity, Fill fill) {
        OutResult result = Documents.callOut(capacity, fill::call);
        return result.error == Documents.OK ? Native.text(result.bytes, result.bytes.length) : null;
    }

    private static byte[] bytes(byte[] data) {
        return data == null ? NONE : data;
    }

    private static int token(Handler handler) {
        return handler == null ? -1 : Callbacks.tokenFor(handler);
    }

    public static int permissionCheck(String subject, String node) {
        byte[] s = Native.utf8(subject);
        byte[] n = Native.utf8(node);
        return RawServer.rawPermissionCheck(Native.at(s), s.length, Native.at(n), n.length);
    }

    public static int permissionHas(int player, String node) {
        byte[] n = Native.utf8(node);
        return RawServer.rawPermissionHas(player, Native.at(n), n.length);
    }

    private static int permissionSet(int player, String node, int mode) {
        byte[] n = Native.utf8(node);
        return RawServer.rawPermissionSet(player, Native.at(n), n.length, mode);
    }

    public static int permissionGrant(int player, String node) { return permissionSet(player, node, 1); }

    public static int permissionDeny(int player, String node) { return permissionSet(player, node, 0); }

    public static int permissionDrop(int player, String node) { return permissionSet(player, node, -1); }

    /** The meta value of a subject, or null when it has none. */
    public static String permissionMeta(String subject, String key) {
        byte[] s = Native.utf8(subject);
        byte[] k = Native.utf8(key);
        return text(128, (out, cap) ->
                RawServer.rawPermissionMeta(Native.at(s), s.length, Native.at(k), k.length, out, cap));
    }

    /** Registers a provider; the handler answers with permissionAnswer or permissionAnswerMeta inside its call. */
    public static int permissionProvide(int priority, int kinds, Handler handler) {
        return RawServer.rawPermissionProvide(priority, kinds, Callbacks.tokenFor(handler));
    }

    public static int permissionProviderRemove(int provider) { return RawServer.rawPermissionProviderRemove(provider); }

    public static int permissionAnswer(int ctx, int answer) { return RawServer.rawPermissionAnswer(ctx, answer); }

    public static int permissionAnswerMeta(int ctx, String text) {
        byte[] t = Native.utf8(text);
        return RawServer.rawPermissionAnswerMeta(ctx, Native.at(t), t.length);
    }

    public static int permissionChanged() { return RawServer.rawPermissionChanged(); }

    /** A field of the provider query (`kind`, `name`, `uuid`, `console`, `node`, `world`) as text, or null. */
    public static String permissionQuery(int ctx, String field) {
        Value value = valueResult(ctx);
        Value found = value == null ? null : value.find(field);
        return found == null ? null : found.asString();
    }

    /** The answer of a completed request, valid inside its callback, or null. */
    public static Value valueResult(int ctx) {
        return Documents.callOut(BUFFER, (out, cap) -> RawCoreValues.rawValueResult(ctx, out, cap)).value();
    }

    public static int requestCancel(int request) { return RawCoreValues.rawRequestCancel(request); }

    /** Registers a command; `complete` may be null. */
    public static int command(String spec, String permission, Handler handler, Handler complete) {
        byte[] s = Native.utf8(spec);
        byte[] p = Native.utf8(permission);
        int main = Callbacks.tokenFor(handler);
        return RawServer.rawCommandRegisterEx(Native.at(s), s.length, Native.at(p), p.length, main, token(complete));
    }

    public static int command(String spec, String permission, Handler handler) {
        return command(spec, permission, handler, null);
    }

    public static int commandAlias(String name, String alias) {
        byte[] n = Native.utf8(name);
        byte[] a = Native.utf8(alias);
        return RawServer.rawCommandAlias(Native.at(n), n.length, Native.at(a), a.length);
    }

    public static int commandUnregister(String name) {
        byte[] n = Native.utf8(name);
        return RawServer.rawCommandUnregister(Native.at(n), n.length);
    }

    /** Runs a command line as the console (player 0) or as a player. */
    public static int commandRun(String line, int player) {
        byte[] l = Native.utf8(line);
        return RawServer.rawCommandRun(Native.at(l), l.length, player);
    }

    public static String completeInput(int ctx) {
        return text(256, (out, cap) -> RawServer.rawCommandCompleteInput(ctx, out, cap));
    }

    public static int completeAdd(int ctx, String candidate) {
        byte[] c = Native.utf8(candidate);
        return RawServer.rawCommandCompleteAdd(ctx, Native.at(c), c.length);
    }

    /** The float argument `name`, or null when the command line did not give it. */
    public static Double commandArgF64(int ctx, String name) {
        byte[] n = Native.utf8(name);
        double[] out = new double[1];
        return RawServer.rawCommandArgF64(ctx, Native.at(n), n.length, Address.ofData(out)) == 0
                ? Double.valueOf(out[0]) : null;
    }

    /** The bool argument `name`, or null when the command line did not give it. */
    public static Boolean commandArgBool(int ctx, String name) {
        byte[] n = Native.utf8(name);
        int[] out = new int[1];
        return RawServer.rawCommandArgBool(ctx, Native.at(n), n.length, Address.ofData(out)) == 0
                ? Boolean.valueOf(out[0] != 0) : null;
    }

    /** Returns how many players were reached; `args` may be null. */
    public static int broadcast(String text, Args args, int flags) {
        byte[] t = Native.utf8(text);
        byte[] a = args == null ? NONE : args.packed();
        return RawServer.rawServerBroadcast(Native.at(t), t.length, Native.at(a), a.length, flags);
    }

    /** Expands placeholders for a player (0 for none); null when the host refused. */
    public static String placeholderFill(String text, int player) {
        byte[] t = Native.utf8(text);
        return text(256, (out, cap) -> RawServer.rawPlaceholderFill(Native.at(t), t.length, player, out, cap));
    }

    public static Reply info() { return reply((out, cap) -> RawServer.rawServerInfo(out, cap)); }

    /** Runs the admin operation `op`; `args` is a finished document or null. */
    public static Reply call(String op, byte[] args) {
        byte[] o = Native.utf8(op);
        byte[] a = bytes(args);
        return reply((out, cap) -> RawServer.rawServerCall(Native.at(o), o.length, Native.at(a), a.length, out, cap));
    }

    public static int profileLookup(String name, Handler handler) {
        byte[] n = Native.utf8(name);
        return RawServer.rawProfileLookup(Native.at(n), n.length, Callbacks.tokenFor(handler));
    }

    /** The answer of a completion callback (a wait, a lookup, a service call). */
    public static Reply result(int ctx) {
        return reply((out, cap) -> RawCoreValues.rawValueResult(ctx, out, cap));
    }

    /** Reads a packaged or plugin file into `out`; the result is the file size, which may exceed out.length. */
    public static int fileRead(String path, byte[] out) {
        byte[] p = Native.utf8(path);
        return RawServer.rawFilesRead(Native.at(p), p.length, Native.at(out), out.length);
    }

    public static int fileWrite(String path, String data) {
        byte[] p = Native.utf8(path);
        byte[] d = Native.utf8(data);
        return RawServer.rawFilesWrite(Native.at(p), p.length, Native.at(d), d.length);
    }

    public static boolean fileExists(String path) {
        byte[] p = Native.utf8(path);
        return RawServer.rawFilesExists(Native.at(p), p.length) == 1;
    }

    public static int fileDelete(String path, boolean recursive) {
        byte[] p = Native.utf8(path);
        return RawServer.rawFilesDelete(Native.at(p), p.length, recursive ? 1 : 0);
    }

    public static Reply fileList(String prefix) {
        byte[] p = Native.utf8(prefix);
        return reply((out, cap) -> RawServer.rawFilesList(Native.at(p), p.length, out, cap));
    }

    public static Reply fileStat(String path) {
        byte[] p = Native.utf8(path);
        return reply((out, cap) -> RawServer.rawFilesStat(Native.at(p), p.length, out, cap));
    }

    public static Reply fileWalk(String path) {
        byte[] p = Native.utf8(path);
        return reply((out, cap) -> RawServer.rawFilesWalk(Native.at(p), p.length, out, cap));
    }

    private static int storeSet(String key, byte[] doc) {
        byte[] k = Native.utf8(key);
        return doc == null ? Documents.INVALID : RawServer.rawStoreSet(Native.at(k), k.length, Native.at(doc), doc.length);
    }

    public static int storeSet(String key, long value) {
        return storeSet(key, new Documents.Writer().integer(value).finish());
    }

    public static int storeSet(String key, String value) {
        return storeSet(key, new Documents.Writer().string(value).finish());
    }

    public static Reply storeGet(String key) {
        byte[] k = Native.utf8(key);
        return reply((out, cap) -> RawServer.rawStoreGet(Native.at(k), k.length, out, cap));
    }

    public static int dataWrite(String name, byte[] doc) {
        byte[] n = Native.utf8(name);
        byte[] d = bytes(doc);
        return RawServer.rawDataWrite(Native.at(n), n.length, Native.at(d), d.length);
    }

    public static Reply dataRead(String name) {
        byte[] n = Native.utf8(name);
        return reply((out, cap) -> RawServer.rawDataRead(Native.at(n), n.length, out, cap));
    }

    /** One value of the plugin config; an empty key answers the whole table. */
    public static Reply configValue(String key) {
        byte[] k = Native.utf8(key);
        return reply((out, cap) -> RawServer.rawConfigValue(Native.at(k), k.length, out, cap));
    }

    public static int serviceRegister(String name, Handler handler) {
        byte[] n = Native.utf8(name);
        return RawServer.rawServiceRegister(Native.at(n), n.length, Callbacks.tokenFor(handler));
    }

    /** Answers the service call being handled; `doc` is a finished document, or null for null. */
    public static int serviceReply(int ctx, byte[] doc) {
        byte[] d = bytes(doc);
        return RawServer.rawServiceReply(ctx, Native.at(d), d.length);
    }

    public static int serviceRemove(int service) { return RawServer.rawServiceRemove(service); }

    public static boolean serviceHas(String name) {
        byte[] n = Native.utf8(name);
        return RawServer.rawServiceHas(Native.at(n), n.length) == 1;
    }

    public static Reply serviceList() { return reply((out, cap) -> RawServer.rawServiceList(out, cap)); }

    public static int serviceCall(String name, byte[] args, int timeout, Handler handler) {
        byte[] n = Native.utf8(name);
        byte[] a = bytes(args);
        return RawServer.rawServiceCall(Native.at(n), n.length, Native.at(a), a.length, timeout,
                Callbacks.tokenFor(handler));
    }

    public static int wait(int ticks, Handler handler) {
        return RawServer.rawTaskWait(ticks, Callbacks.tokenFor(handler));
    }

    public static int at(String world, double x, double z, Handler handler) {
        byte[] w = Native.utf8(world);
        return RawServer.rawTaskAt(Native.at(w), w.length, x, z, Callbacks.tokenFor(handler));
    }
}
