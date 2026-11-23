using Basalt.Proxy.Core;
using Basalt.Proxy.Internal;

namespace Basalt.Proxy.Network;

public static partial class Network {
    public static string[] ServersList() => ProxyNative.ServersList();
    public static string ServerAddress(string name) => ProxyNative.ServerAddress(name);
    public static int[] PlayersOn(string server) => ProxyNative.PlayersOn(server);
}
