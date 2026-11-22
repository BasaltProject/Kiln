package org.basaltmc.proxy.network;
import org.basaltmc.proxy.core.*;
import org.basaltmc.proxy.internal.*;
import org.basaltmc.proxy.core.*;

import org.teavm.interop.Address;

/** Proxy network area (abi/areas/proxy_network.h): messaging channels, raw packets and antibot control. */
public final class Network {
    private Network() {}

    private interface Fetch {
        int call(Address out, int cap);
    }

    private static int lastError;

    /** The negative ABI code of the last value call that returned null. */
    public static int lastError() { return lastError; }

    /** The encoded value a call answered, or null when it failed (see lastError). */
    private static byte[] fetch(Fetch call) {
        int cap = 256;
        while (true) {
            byte[] out = new byte[cap];
            int n = call.call(Native.at(out), cap);
            if (n < 0) {
                lastError = n;
                return null;
            }
            if (n <= cap) {
                byte[] value = new byte[n];
                System.arraycopy(out, 0, value, 0, n);
                return value;
            }
            cap = n;
        }
    }

    public static int channelRegister(String name) {
        byte[] n = Native.utf8(name);
        return RawProxyNetwork.rawNetworkChannelRegister(Native.at(n), n.length);
    }

    public static int channelUnregister(String name) {
        byte[] n = Native.utf8(name);
        return RawProxyNetwork.rawNetworkChannelUnregister(Native.at(n), n.length);
    }

    /** Observes the messages of the plugin's channels; an empty filter takes all of them. */
    public static int messageOn(String filter, Handler handler) {
        byte[] f = Native.utf8(filter);
        return RawProxyNetwork.rawNetworkMessageOn(Native.at(f), f.length, Callbacks.tokenFor(handler));
    }

    /** The encoded map of the message (player_name, player_uuid, channel, data, source, server), null on failure. */
    public static byte[] messageGet(int ctx) {
        return fetch((out, cap) -> RawProxyNetwork.rawNetworkMessageGet(ctx, out, cap));
    }

    /** Sends `data` on a channel; towardClient false goes to the player's backend, true to the client. */
    public static int messageSend(int player, String channel, byte[] data, boolean towardClient) {
        byte[] c = Native.utf8(channel);
        return RawProxyNetwork.rawNetworkMessageSend(player, Native.at(c), c.length, Native.at(data), data.length,
                towardClient ? 1 : 0);
    }

    public static byte[] packetResolve(String name) {
        byte[] n = Native.utf8(name);
        return fetch((out, cap) -> RawProxyNetwork.rawPacketResolve(Native.at(n), n.length, out, cap));
    }

    public static byte[] packetNames() {
        return fetch((out, cap) -> RawProxyNetwork.rawPacketNames(out, cap));
    }

    /** A decider listener; opts is an encoded value map (priority, fail) or empty. Returns the listener. */
    public static int packetOn(String name, byte[] opts, Handler handler) {
        byte[] n = Native.utf8(name);
        return RawProxyNetwork.rawPacketOn(Native.at(n), n.length, Native.at(opts), opts.length,
                Callbacks.tokenFor(handler));
    }

    /** An observer listener: it sees the final packet and cannot cancel or change it. */
    public static int packetWatch(String name, byte[] opts, Handler handler) {
        byte[] n = Native.utf8(name);
        return RawProxyNetwork.rawPacketWatch(Native.at(n), n.length, Native.at(opts), opts.length,
                Callbacks.tokenFor(handler));
    }

    public static int packetRemove(int listener) { return RawProxyNetwork.rawPacketRemove(listener); }

    public static byte[] packetGet(int ctx) {
        return fetch((out, cap) -> RawProxyNetwork.rawPacketGet(ctx, out, cap));
    }

    public static byte[] packetBody(int ctx) {
        return fetch((out, cap) -> RawProxyNetwork.rawPacketBody(ctx, out, cap));
    }

    public static int packetSetBody(int ctx, byte[] body) {
        return RawProxyNetwork.rawPacketSetBody(ctx, Native.at(body), body.length);
    }

    public static int packetCancel(int ctx) { return RawProxyNetwork.rawPacketCancel(ctx); }

    public static int packetInject(int ctx, String name, byte[] body, boolean intercept) {
        byte[] n = Native.utf8(name);
        return RawProxyNetwork.rawPacketInject(ctx, Native.at(n), n.length, Native.at(body), body.length,
                intercept ? 1 : 0);
    }

    public static int packetSend(int player, String name, byte[] body) {
        byte[] n = Native.utf8(name);
        return RawProxyNetwork.rawPacketSend(player, Native.at(n), n.length, Native.at(body), body.length);
    }

    public static int packetReceive(int player, String name, byte[] body) {
        byte[] n = Native.utf8(name);
        return RawProxyNetwork.rawPacketReceive(player, Native.at(n), n.length, Native.at(body), body.length);
    }

    public static int antibotVerify(String name) {
        byte[] n = Native.utf8(name);
        return RawProxyNetwork.rawAntibotVerify(Native.at(n), n.length);
    }

    public static int antibotUnverify(String name) {
        byte[] n = Native.utf8(name);
        return RawProxyNetwork.rawAntibotUnverify(Native.at(n), n.length);
    }

    public static int antibotIsVerified(String name) {
        byte[] n = Native.utf8(name);
        return RawProxyNetwork.rawAntibotIsVerified(Native.at(n), n.length);
    }

    public static int antibotBan(String address) {
        byte[] a = Native.utf8(address);
        return RawProxyNetwork.rawAntibotBan(Native.at(a), a.length);
    }

    public static int antibotUnban(String address) {
        byte[] a = Native.utf8(address);
        return RawProxyNetwork.rawAntibotUnban(Native.at(a), a.length);
    }

    public static int antibotUnderAttack() { return RawProxyNetwork.rawAntibotUnderAttack(); }

    public static int antibotOn(Handler handler) { return RawProxyNetwork.rawAntibotOn(Callbacks.tokenFor(handler)); }

    public static byte[] antibotEventGet(int ctx) {
        return fetch((out, cap) -> RawProxyNetwork.rawAntibotEventGet(ctx, out, cap));
    }
    public static String[] serversList() { return org.basaltmc.proxy.internal.Proxy.serversList(); }
    public static String serverAddress(String name) { return org.basaltmc.proxy.internal.Proxy.serverAddress(name); }
    public static int[] playersOn(String server) { return org.basaltmc.proxy.internal.Proxy.playersOn(server); }
}
