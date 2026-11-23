package org.basaltmc.proxy.hud;

import org.basaltmc.proxy.core.*;
import org.basaltmc.proxy.internal.*;
import org.basaltmc.proxy.values.Documents;

/** Proxy hud area (abi/areas/proxy_hud.h): boss bars, sidebar, tab list, tab rows, server links, chat completions.
 * A null optional text leaves that part unchanged; every call answers 0 or a negative code. */
public final class Displays {
    private Displays() {}

    private static final int INVALID = -3;

    private static void key(Documents.Writer w, String name, String text) {
        if (text != null) {
            w.key(name).string(text);
        }
    }

    public static int clearTitle(int player, boolean reset) {
        return RawProxyHud.rawHudClearTitle(player, reset ? 1 : 0);
    }

    public static int tabList(int player, String header, String footer) {
        Documents.Writer w = new Documents.Writer();
        w.beginMap();
        key(w, "header", header);
        key(w, "footer", footer);
        w.end();
        byte[] doc = w.finish();
        return doc == null ? INVALID : RawProxyHud.rawHudTabList(player, Native.at(doc), doc.length);
    }

    public static int bossbarShow(int player, String id, String text, double progress, String color, String overlay) {
        byte[] idb = Native.utf8(id);
        Documents.Writer w = new Documents.Writer();
        w.beginMap();
        key(w, "text", text);
        w.key("progress").number(progress);
        key(w, "color", color);
        key(w, "overlay", overlay);
        w.end();
        byte[] doc = w.finish();
        return doc == null ? INVALID : RawProxyHud.rawHudBossbarShow(player, Native.at(idb), idb.length, Native.at(doc), doc.length);
    }

    public static int bossbarHide(int player, String id) {
        byte[] idb = Native.utf8(id);
        return RawProxyHud.rawHudBossbarHide(player, Native.at(idb), idb.length);
    }

    public static int bossbarProgress(int player, String id, double progress) {
        byte[] idb = Native.utf8(id);
        return RawProxyHud.rawHudBossbarProgress(player, Native.at(idb), idb.length, progress);
    }

    public static int bossbarTitle(int player, String id, String text) {
        byte[] idb = Native.utf8(id);
        byte[] textb = Native.utf8(text);
        return RawProxyHud.rawHudBossbarTitle(player, Native.at(idb), idb.length, Native.at(textb), textb.length, Native.at(new byte[0]), 0);
    }

    public static int bossbarStyle(int player, String id, String color, String overlay) {
        byte[] idb = Native.utf8(id);
        byte[] colorb = Native.utf8(color);
        byte[] overlayb = Native.utf8(overlay);
        return RawProxyHud.rawHudBossbarStyle(player, Native.at(idb), idb.length, Native.at(colorb), colorb.length, Native.at(overlayb), overlayb.length);
    }

    public static int sidebar(int player, String title, String[] lines) {
        Documents.Writer w = new Documents.Writer();
        w.beginMap();
        key(w, "title", title);
        w.key("lines").beginList();
        for (String line : lines) {
            w.string(line);
        }
        w.end();
        w.end();
        byte[] doc = w.finish();
        return doc == null ? INVALID : RawProxyHud.rawHudSidebar(player, Native.at(doc), doc.length);
    }

    public static int sidebarClear(int player) {
        return RawProxyHud.rawHudSidebarClear(player);
    }

    public static int tabEntry(int player, String id, String name, String text, long latency) {
        byte[] idb = Native.utf8(id);
        Documents.Writer w = new Documents.Writer();
        w.beginMap();
        key(w, "name", name);
        key(w, "text", text);
        w.key("latency").integer(latency);
        w.end();
        byte[] doc = w.finish();
        return doc == null ? INVALID : RawProxyHud.rawHudTabEntry(player, Native.at(idb), idb.length, Native.at(doc), doc.length);
    }

    public static int tabEntryRemove(int player, String id) {
        byte[] idb = Native.utf8(id);
        return RawProxyHud.rawHudTabEntryRemove(player, Native.at(idb), idb.length);
    }

    /** Label then url for each link. */
    public static int serverLinks(int player, String[] pairs) {
        Documents.Writer w = new Documents.Writer();
        w.beginList();
        for (int i = 0; i + 1 < pairs.length; i += 2) {
            w.beginList().string(pairs[i]).string(pairs[i + 1]).end();
        }
        w.end();
        byte[] doc = w.finish();
        return doc == null ? INVALID : RawProxyHud.rawHudServerLinks(player, Native.at(doc), doc.length);
    }

    public static int chatCompletions(int player, String action, String[] words) {
        byte[] actionb = Native.utf8(action);
        Documents.Writer w = new Documents.Writer();
        w.beginList();
        for (String word : words) {
            w.string(word);
        }
        w.end();
        byte[] doc = w.finish();
        return doc == null ? INVALID : RawProxyHud.rawHudChatCompletions(player, Native.at(actionb), actionb.length, Native.at(doc), doc.length);
    }
}
