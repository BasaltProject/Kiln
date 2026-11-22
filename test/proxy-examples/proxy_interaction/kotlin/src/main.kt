import org.basaltmc.proxy.core.Ctx
import org.basaltmc.proxy.interaction.DialogButton
import org.basaltmc.proxy.values.Value

var seen = 0

fun say(text: String) = org.basaltmc.proxy.log.logInfo(text)

fun ok(result: Int) = result >= 0

fun textOf(root: Value?, key: String): String = root?.find(key)?.asString() ?: ""

fun onPackStatus(ctx: Ctx) {
    val event = org.basaltmc.proxy.interaction.valueResult(ctx)
    if (event == null) {
        say("pack event unreadable")
        return
    }
    say("pack status:")
    say(textOf(event, "status"))
}

fun onSettings(ctx: Ctx) {
    val event = org.basaltmc.proxy.interaction.valueResult(ctx)
    if (event == null) {
        say("settings event unreadable")
        return
    }
    say("settings locale:")
    say(textOf(event.find("settings"), "locale"))
}

fun onCookie(ctx: Ctx) {
    val cookie = org.basaltmc.proxy.interaction.cookieResult(ctx)
    if (cookie == null) {
        say("cookie absent")
        return
    }
    say("cookie received:")
    say(cookie.decodeToString())
}

fun onDialogClick(ctx: Ctx) {
    say("dialog clicked")
    org.basaltmc.proxy.commands.commandReply(ctx, "<green>dialog clicked")
}

fun onTransfer(ctx: Ctx) {
    val player = org.basaltmc.proxy.events.eventPlayer(ctx)
    say(if (ok(org.basaltmc.proxy.interaction.playerTransfer(player, "127.0.0.1", 25970))) "transfer sent" else "transfer failed")
}

fun onCleanup(ctx: Ctx) {
    val player = org.basaltmc.proxy.events.eventPlayer(ctx)
    val removed = ok(org.basaltmc.proxy.interaction.resourcePackRemove(player, "demo"))
    val cleared = ok(org.basaltmc.proxy.interaction.resourcePackClear(player))
    val closed = ok(org.basaltmc.proxy.interaction.dialogClear(player))
    say(if (removed && cleared && closed) "cleanup ok" else "cleanup failed")
}

fun onStale(ctx: Ctx) {
    say(if (org.basaltmc.proxy.interaction.dialogClear(seen) == -2) "stale after quit" else "stale not detected")
}

fun onQuit(ctx: Ctx) {
    org.basaltmc.proxy.tasks.after(20, ::onStale)
}

fun onJoin(ctx: Ctx) {
    val player = org.basaltmc.proxy.events.eventPlayer(ctx)
    seen = player
    say(if (org.basaltmc.proxy.interaction.playersCount() >= 1) "count ok" else "count failed")
    say(if (org.basaltmc.proxy.interaction.playersCount("lobby") >= 1) "count lobby ok" else "count lobby failed")
    say(if (org.basaltmc.proxy.interaction.playersCount("nowhere") == 0) "count nowhere ok" else "count nowhere failed")
    val address = org.basaltmc.proxy.interaction.playerAddress(player)
    say(if (!address.isNullOrEmpty() && org.basaltmc.proxy.interaction.playerPort(player) > 0) "address ok" else "address failed")
    val uuid = org.basaltmc.proxy.players.playerUuid(player) ?: ""
    say(if (org.basaltmc.proxy.interaction.playersByUuid(uuid) == player) "by uuid ok" else "by uuid failed")
    say(if (org.basaltmc.proxy.interaction.playersByUuid("00000000-0000-0000-0000-000000000000") == 0) "by uuid none ok" else "by uuid none failed")

    val buttons = listOf(DialogButton("Accept", "pidialog"), DialogButton("Decline", "pidialog"))
    say(if (ok(org.basaltmc.proxy.interaction.dialogShow(player, "multi_action", "<gold>Interaction", "<gray>Pick one", buttons))) "dialog ok" else "dialog failed")
    say(if (org.basaltmc.proxy.interaction.dialogShow(player, "bogus", "x", null, emptyList()) == -3) "bad kind invalid" else "bad kind accepted")

    val id = org.basaltmc.proxy.interaction.resourcePackPush(player, "https://example.invalid/pack.zip", prompt = "<gold>Test pack", id = "demo")
    if (id != null) {
        say("pack pushed:")
        say(id)
    } else {
        say("pack failed")
    }
    val bad = org.basaltmc.proxy.interaction.resourcePackPush(player, "https://example.invalid/pack.zip", hash = "nothex")
    say(if (bad == null) "bad hash invalid" else "bad hash accepted")

    say(if (ok(org.basaltmc.proxy.interaction.cookieSet(player, "basalt:demo", "abcd".encodeToByteArray()))) "cookie stored" else "cookie store failed")
    say(if (org.basaltmc.proxy.interaction.cookieRequest(player, "basalt:demo", ::onCookie) > 0) "cookie requested" else "cookie request failed")

    say(if (ok(org.basaltmc.proxy.interaction.playerDataSetF64(player, "ratio", 2.5))) "float stored" else "float store failed")
    say(if (org.basaltmc.proxy.interaction.playerDataGetF64(player, "ratio", 0.0) == 2.5) "float read ok" else "float read failed")
    say(if (org.basaltmc.proxy.interaction.playerDataGetF64(player, "missing", 7.5) == 7.5) "float default ok" else "float default failed")

    say(if (ok(org.basaltmc.proxy.interaction.playersBroadcast("<green>Interaction hello {who}", org.basaltmc.proxy.core.Args().add("who", "everyone")))) "broadcast ok" else "broadcast failed")
    say(if (org.basaltmc.proxy.interaction.playerTransfer(player, "", 25565) == -3) "bad host invalid" else "bad host accepted")
}

fun init() {
    org.basaltmc.proxy.events.on("player.join", org.basaltmc.proxy.core.Priority.NORMAL, ::onJoin)
    org.basaltmc.proxy.events.on("player.quit", org.basaltmc.proxy.core.Priority.NORMAL, ::onQuit)
    org.basaltmc.proxy.interaction.watchResourcePacks(::onPackStatus)
    org.basaltmc.proxy.interaction.watchSettings(::onSettings)
    org.basaltmc.proxy.commands.command("pidialog", "", ::onDialogClick)
    org.basaltmc.proxy.commands.command("pitransfer", "", ::onTransfer)
    org.basaltmc.proxy.commands.command("picleanup", "", ::onCleanup)
    say("interaction ready")
}
