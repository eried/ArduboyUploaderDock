using System;

namespace SafeSerialPort
{
    internal class Utilities
    {
        internal static bool IsRunningOnMono()
        {
            return Type.GetType("Mono.Runtime") != null;
        }
    }
}