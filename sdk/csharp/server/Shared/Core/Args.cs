using Basalt.Server.Core;
using Basalt.Server.Internal;
using Basalt.Server.Values;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using static Basalt.Server.Core.Level;
using static Basalt.Server.Core.Priority;
using static Basalt.Server.Internal.Native;

namespace Basalt.Server.Core;

public sealed class Args
    {
        readonly List<byte> body = new();
        int count;

        public Args Add(string key, string value)
        {
            count++;
            foreach (string part in new[] { key, value })
            {
                byte[] data = Utf8(part);
                for (int shift = 0; shift < 32; shift += 8)
                {
                    body.Add((byte)(data.Length >> shift));
                }
                body.AddRange(data);
            }
            return this;
        }

        internal byte[] Packed()
        {
            if (count == 0)
            {
                return null;
            }
            byte[] output = new byte[4 + body.Count];
            for (int i = 0; i < 4; i++)
            {
                output[i] = (byte)(count >> (8 * i));
            }
            body.CopyTo(output, 4);
            return output;
        }
    }
