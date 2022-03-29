using LibDddAdminTransport;
using System;
using System.Net;

namespace DddAdminTransportExample
{
    class Program
    {
        static void Main(string[] args)
        {
            Logger log = new Logger();
            GameTransport game = new GameTransport(log, new IPEndPoint(IPAddress.Any, 33434));
            game.Listen();
            while (true)
            {
                GameMessage msg = new GameMessage();
                msg.PutString(GameMessageKey.MAP_NAME, Console.ReadLine());
                game.SendMessage(GamePacketEndpoint.MAP_START, msg);
            }
        }

        class Logger : IGameTransportLogger
        {
            public void LogError(string topic, string message)
            {
                Log("STOP", topic, message);
            }

            public void LogInfo(string topic, string message)
            {
                Log("INFO", topic, message);
            }

            public void LogWarn(string topic, string message)
            {
                Log("WARN", topic, message);
            }

            private void Log(string level, string topic, string message)
            {
                Console.WriteLine($"[{level}/{topic}] {message}");
            }
        }
    }
}
