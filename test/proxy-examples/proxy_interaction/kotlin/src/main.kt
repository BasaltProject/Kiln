import org.basaltmc.proxy.commands.command
import org.basaltmc.proxy.commands.commandReply
import org.basaltmc.proxy.core.Args
import org.basaltmc.proxy.core.Ctx
import org.basaltmc.proxy.core.Priority
import org.basaltmc.proxy.events.eventPlayer
import org.basaltmc.proxy.events.on
import org.basaltmc.proxy.interaction.DialogButton
import org.basaltmc.proxy.interaction.cookieRequest
import org.basaltmc.proxy.interaction.cookieResult
import org.basaltmc.proxy.interaction.cookieSet
import org.basaltmc.proxy.interaction.dialogClear
import org.basaltmc.proxy.interaction.dialogShow
import org.basaltmc.proxy.interaction.playerAddress
import org.basaltmc.proxy.interaction.playerDataGetF64
import org.basaltmc.proxy.interaction.playerDataSetF64
import org.basaltmc.proxy.interaction.playerPort
import org.basaltmc.proxy.interaction.playerTransfer
import org.basaltmc.proxy.interaction.playersBroadcast
import org.basaltmc.proxy.interaction.playersByUuid
import org.basaltmc.proxy.interaction.playersCount
import org.basaltmc.proxy.interaction.resourcePackClear
import org.basaltmc.proxy.interaction.resourcePackPush
import org.basaltmc.proxy.interaction.resourcePackRemove
import org.basaltmc.proxy.interaction.valueResult
import org.basaltmc.proxy.interaction.watchResourcePacks
import org.basaltmc.proxy.interaction.watchSettings
import org.basaltmc.proxy.log.logInfo
import org.basaltmc.proxy.players.playerUuid
import org.basaltmc.proxy.tasks.after
import org.basaltmc.proxy.values.Value

var seen = 0

fun say(text: String) = logInfo(text)

fun ok(result: Int) = result >= 0

fun textOf(root: Value?, key: String): String = root?.find(key)?.asString() ?: ""

fun onPackStatus(ctx: Ctx) {
    val event = valueResult(ctx)
    if (event == null) {
        say("pack event unreadable")
        return
    }
    say("pack status:")
    say(textOf(event, "status"))
}

fun onSettings(ctx: Ctx) {
    val event = valueResult(ctx)
    if (event == null) {
        say("settings event unreadable")
        return
    }
    say("settings locale:")
    say(textOf(event.find("settings"), "locale"))
}

fun onCookie(ctx: Ctx) {
    val cookie = cookieResult(ctx)
    if (cookie == null) {
        say("cookie absent")
        return
    }
    say("cookie received:")
    say(cookie.decodeToString())
}

fun onDialogClick(ctx: Ctx) {
    say("dialog clicked")
    commandReply(ctx, "<green>dialog clicked")
}

fun onTransfer(ctx: Ctx) {
    val player = eventPlayer(ctx)
    say(if (ok(playerTransfer(player, "127.0.0.1", 25970))) "transfer sent" else "transfer failed")
}

fun onCleanup(ctx: Ctx) {
    val player = eventPlayer(ctx)
    val removed = ok(resourcePackRemove(player, "demo"))
    val cleared = ok(resourcePackClear(player))
    val closed = ok(dialogClear(player))
    say(if (removed && cleared && closed) "cleanup ok" else "cleanup failed")
}

fun onStale(ctx: Ctx) {
    say(if (dialogClear(seen) == -2) "stale after quit" else "stale not detected")
}

fun onQuit(ctx: Ctx) {
    after(20, ::onStale)
}

fun onJoin(ctx: Ctx) {
    val player = eventPlayer(ctx)
    seen = player
    say(if (playersCount() >= 1) "count ok" else "count failed")
    say(if (playersCount("lobby") >= 1) "count lobby ok" else "count lobby failed")
    say(if (playersCount("nowhere") == 0) "count nowhere ok" else "count nowhere failed")
    val address = playerAddress(player)
    say(if (!address.isNullOrEmpty() && playerPort(player) > 0) "address ok" else "address failed")
    val uuid = playerUuid(player) ?: ""
    say(if (playersByUuid(uuid) == player) "by uuid ok" else "by uuid failed")
    say(if (playersByUuid("00000000-0000-0000-0000-000000000000") == 0) "by uuid none ok" else "by uuid none failed")

    val buttons = listOf(DialogButton("Accept", "pidialog"), DialogButton("Decline", "pidialog"))
    say(if (ok(dialogShow(player, "multi_action", "<gold>Interaction", "<gray>Pick one", buttons))) "dialog ok" else "dialog failed")
    say(if (dialogShow(player, "bogus", "x", null, emptyList()) == -3) "bad kind invalid" else "bad kind accepted")

    val id = resourcePackPush(player, "https://example.invalid/pack.zip", prompt = "<gold>Test pack", id = "demo")
    if (id != null) {
        say("pack pushed:")
        say(id)
    } else {
        say("pack failed")
    }
    val bad = resourcePackPush(player, "https://example.invalid/pack.zip", hash = "nothex")
    say(if (bad == null) "bad hash invalid" else "bad hash accepted")

    say(if (ok(cookieSet(player, "basalt:demo", "abcd".encodeToByteArray()))) "cookie stored" else "cookie store failed")
    say(if (cookieRequest(player, "basalt:demo", ::onCookie) > 0) "cookie requested" else "cookie request failed")

    say(if (ok(playerDataSetF64(player, "ratio", 2.5))) "float stored" else "float store failed")
    say(if (playerDataGetF64(player, "ratio", 0.0) == 2.5) "float read ok" else "float read failed")
    say(if (playerDataGetF64(player, "missing", 7.5) == 7.5) "float default ok" else "float default failed")

    say(if (ok(playersBroadcast("<green>Interaction hello {who}", Args().add("who", "everyone")))) "broadcast ok" else "broadcast failed")
    say(if (playerTransfer(player, "", 25565) == -3) "bad host invalid" else "bad host accepted")
}

fun init() {
    on("player.join", Priority.NORMAL, ::onJoin)
    on("player.quit", Priority.NORMAL, ::onQuit)
    watchResourcePacks(::onPackStatus)
    watchSettings(::onSettings)
    command("pidialog", "", ::onDialogClick)
    command("pitransfer", "", ::onTransfer)
    command("picleanup", "", ::onCleanup)
    say("interaction ready")
}
