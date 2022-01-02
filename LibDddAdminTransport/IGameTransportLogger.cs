using System;
using System.Collections.Generic;
using System.Text;

namespace LibDddAdminTransport
{
    public interface IGameTransportLogger
    {
        void LogInfo(string topic, string message);
        void LogWarn(string topic, string message);
        void LogError(string topic, string message);
    }
}
