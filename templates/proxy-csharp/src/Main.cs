using Basalt.Proxy.Commands;
using Basalt.Proxy.Config;
using Basalt.Proxy.Core;
using Basalt.Proxy.Data;
using Basalt.Proxy.Events;
using Basalt.Proxy.Log;
using Basalt.Proxy.Players;
using Basalt.Proxy.Tasks;
using Basalt.Proxy.Values;

namespace Basalt.Proxy.Example;

public static class Plugin
{
    static void OnJoin(int ctx)
    {
        int player = EventBus.EventPlayer(ctx);
        if (player == 0)
        {
            return;
        }
        long visits = PlayerStore.DataGetI64(player, "visits", 0) + 1;
        PlayerStore.DataSetI64(player, "visits", visits);
        Directory.Send(player, "@welcome", new Args().Add("name", Directory.PlayerName(player) ?? "").Add("visits", visits.ToString()));
        Directory.PlayerTitle(player, "<gold>Welcome", "<gray>Have fun");
    }

    static void Hello(int ctx)
    {
        int player = EventBus.EventPlayer(ctx);
        string server = player == 0 ? "console" : Directory.PlayerServer(player) ?? "";
        Sender.Reply(ctx, "@hello", new Args().Add("name", Sender.Name(ctx) ?? "").Add("server", server));
    }

    public static void Init()
    {
        Logger.LogInfo("starting up");
        EventBus.On("player.join", Priority.Normal, OnJoin);
        Registry.Register("@ID@ hello", "", Hello);

        int period = (int)Settings.ConfigI64("announce-period", 1200);
        Scheduler.Every(period, period, ctx =>
        {
            foreach (int player in Directory.PlayersOnline())
            {
                Directory.Send(player, "@announce");
            }
        });
    }
}
