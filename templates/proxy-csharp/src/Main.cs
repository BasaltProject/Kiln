using Basalt.Proxy.Core;
using Basalt.Proxy.Values;
using LegacyPlayers = global::Basalt.Proxy.Players.Players;
using SdkCommands = global::Basalt.Proxy.Commands.Commands;
using SdkConfig = global::Basalt.Proxy.Config.Config;
using SdkData = global::Basalt.Proxy.Data.Data;
using SdkEvents = global::Basalt.Proxy.Events.Events;
using SdkLogger = global::Basalt.Proxy.Log.Logger;
using SdkPlayers = global::Basalt.Proxy.Players.Players;
using SdkTasks = global::Basalt.Proxy.Tasks.Tasks;

namespace Basalt.Proxy.Example;

public static class Plugin
{
    static void OnJoin(int ctx)
    {
        int player = SdkEvents.EventPlayer(ctx);
        if (player == 0)
        {
            return;
        }
        long visits = SdkData.DataGetI64(player, "visits", 0) + 1;
        SdkData.DataSetI64(player, "visits", visits);
        SdkPlayers.Send(player, "@welcome", new Args().Add("name", SdkPlayers.PlayerName(player) ?? "").Add("visits", visits.ToString()));
        LegacyPlayers.PlayerTitle(player, "<gold>Welcome", "<gray>Have fun");
    }

    static void Hello(int ctx)
    {
        int player = SdkEvents.EventPlayer(ctx);
        string server = player == 0 ? "console" : LegacyPlayers.PlayerServer(player) ?? "";
        SdkCommands.CommandReply(ctx, "@hello", new Args().Add("name", SdkCommands.CommandSourceName(ctx) ?? "").Add("server", server));
    }

    public static void Init()
    {
        SdkLogger.LogInfo("starting up");
        SdkEvents.On("player.join", Priority.Normal, OnJoin);
        SdkCommands.Command("@ID@ hello", "", Hello);

        int period = (int)SdkConfig.ConfigI64("announce-period", 1200);
        SdkTasks.Every(period, period, ctx =>
        {
            foreach (int player in SdkPlayers.PlayersOnline())
            {
                SdkPlayers.Send(player, "@announce");
            }
        });
    }
}
