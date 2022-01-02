using System;
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;

namespace LibDddAdminTransport
{
    public class GameTransport
    {
        public GameTransport(IGameTransportLogger logger, IPEndPoint endpoint)
        {
            //Set
            this.logger = logger;
            this.endpoint = endpoint;

            //Create worker
            worker = new Thread(WorkerThread);
        }

        public const byte CURRENT_PROTOCOL_VERSION = 1;

        private IGameTransportLogger logger;
        private IPEndPoint endpoint;
        private Thread worker;
        private Dictionary<GamePacketEndpoint, Action<GameMessage>> bindings = new Dictionary<GamePacketEndpoint, Action<GameMessage>>();
        private bool connected;

        public event Action<bool> OnStatusChanged;

        public bool Connected
        {
            get => connected;
            set
            {
                if (value == connected)
                    return;
                logger.LogInfo("ConnectedChange", $"Connection status changed to {(value ? "UP" : "DOWN")}.");
                connected = value;
                OnStatusChanged?.Invoke(value);
            }
        }

        public void Bind(GamePacketEndpoint endpoint, Action<GameMessage> callback)
        {
            bindings.Add(endpoint, callback);
        }

        public void Connect()
        {
            worker.Start();
        }

        private void WorkerThread()
        {
            while (true)
            {
                Socket sock = null;
                try
                {
                    //logger.Log
                    logger.LogInfo("WorkerThread", $"Attempting to connect to game server at {endpoint}...");

                    //Open socket to the server
                    sock = new Socket(SocketType.Stream, ProtocolType.Tcp);
                    sock.Connect(endpoint);

                    //logger.Log
                    logger.LogInfo("WorkerThread", "Successfully opened connection!");
                    Connected = true;

                    //Enter service loop
                    while (true)
                        WorkerServiceLoop(sock);
                }
                catch (Exception ex)
                {
                    //logger.Log
                    logger.LogWarn("WorkerThread", $"Lost connection to game server: {ex.Message}. Retrying shortly...");
                    Connected = false;

                    //Close socket if needed
                    try
                    {
                        if (sock != null)
                            sock.Close();
                    }
                    catch
                    {

                    }

                    //Wait
                    Thread.Sleep(5 * 1000);
                }
            }
        }

        private void WorkerServiceLoop(Socket sock)
        {
            //Read and decode the packet header
            DecodePacketHeader(ReadNetBytes(sock, 8), out byte protocolVersion, out GamePacketFlags flags, out GamePacketEndpoint endpoint, out int payloadLength);

            //Sanity check protocol version
            if (protocolVersion != CURRENT_PROTOCOL_VERSION)
                throw new Exception($"Server is not running a compatible protocol version. Server={protocolVersion}, Client={CURRENT_PROTOCOL_VERSION}.");

            //Warn on dropped packets
            if ((flags & GamePacketFlags.PACKETS_DROPPED) != 0)
                logger.LogWarn("WorkerServiceLoop", "Server dropped packets, we are not buffering enough! This will likely result in corrupted game state.");

            //Receive payload
            byte[] payload = ReadNetBytes(sock, payloadLength);

            //Decode message
            GameMessage msg = GameMessage.Deserialize(payload);

            //Find
            Action<GameMessage> callback = null;
            lock (bindings)
            {
                if (!bindings.TryGetValue(endpoint, out callback))
                {
                    logger.LogWarn("WorkerServiceLoop", $"Got message for endpoint \"{endpoint}\", but there was nothing bound to it! Dropping...");
                }
            }

            //Dispatch
            try
            {
                if (callback != null)
                    callback(msg);
            }
            catch (Exception ex)
            {
                logger.LogError("WorkerServiceLoop", $"Got exception handling {endpoint}: {ex.Message}{ex.StackTrace}");
            }
        }

        private void DecodePacketHeader(byte[] buffer, out byte protocolVersion, out GamePacketFlags flags, out GamePacketEndpoint endpoint, out int payloadLength)
        {
            protocolVersion = buffer[0];
            flags = (GamePacketFlags)buffer[1];
            endpoint = (GamePacketEndpoint)BitConverter.ToUInt16(buffer, 2);
            payloadLength = BitConverter.ToInt32(buffer, 4);
        }

        private byte[] ReadNetBytes(Socket sock, int length)
        {
            //Create buffer
            byte[] buffer = new byte[length];

            //Read
            int offset = 0;
            while (length > 0)
            {
                //Read
                int read = sock.Receive(buffer, offset, length, SocketFlags.None);

                //Check
                if (read == 0)
                    throw new Exception("Socket closed while expecting bytes.");

                //Update
                length -= read;
                offset += read;
            }

            return buffer;
        }
    }
}
