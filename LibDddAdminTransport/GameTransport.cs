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

            //Create server
            server = new Socket(SocketType.Stream, ProtocolType.Tcp);
            server.Bind(endpoint);
            server.Listen(4);

            //Create worker
            worker = new Thread(WorkerThread);
        }

        public const int NET_HEADER_LENGTH = 12;

        public const ushort CURRENT_PROTOCOL_VERSION = 2;
        public const ushort CURRENT_OPCODE_VERSION = 1;

        private IGameTransportLogger logger;
        private Socket server;
        private Socket sock;
        private Thread worker;
        private Dictionary<GamePacketEndpoint, Action<GameMessage>> bindings = new Dictionary<GamePacketEndpoint, Action<GameMessage>>();
        private bool connected;

        public event Action<bool> OnStatusChanged;

        public bool Connected
        {
            get => connected;
            private set
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

        public void SendMessage(GamePacketEndpoint endpoint, GameMessage message)
        {
            //Serialize
            byte[] buffer = new byte[NET_HEADER_LENGTH + message.SerializedLength];
            EncodePacketHeader(buffer, (uint)buffer.Length, CURRENT_PROTOCOL_VERSION, CURRENT_OPCODE_VERSION, endpoint);
            message.Serialize(buffer, NET_HEADER_LENGTH);

            //Deliver
            lock (sock)
                sock.Send(buffer);
        }

        public void Listen()
        {
            server.Listen(4);
            worker.Start();
        }

        private void WorkerThread()
        {
            while (true)
            {
                try
                {
                    //Wait for connection
                    logger.LogInfo("WorkerThread", $"Awaiting connection from game server...");
                    sock = server.Accept();
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
                        sock = null;
                    }
                    catch
                    {

                    }
                }
            }
        }

        private void WorkerServiceLoop(Socket sock)
        {
            //Read and decode the packet header
            DecodePacketHeader(ReadNetBytes(sock, NET_HEADER_LENGTH), out uint packetLength, out ushort protocolVersion, out ushort opcodeVersion, out GamePacketEndpoint endpoint);

            //Check protocol and opcode versions
            if (protocolVersion != CURRENT_PROTOCOL_VERSION)
                throw new Exception($"Server is not running a compatible protocol version. Server={protocolVersion}, Client={CURRENT_PROTOCOL_VERSION}.");
            if (opcodeVersion != CURRENT_OPCODE_VERSION)
                throw new Exception($"Server is not running a compatible protocol version. Server={opcodeVersion}, Client={CURRENT_OPCODE_VERSION}.");

            //Receive payload
            byte[] payload = ReadNetBytes(sock, (int)(packetLength - NET_HEADER_LENGTH));

            //Decode message
            GameMessage msg = GameMessage.FromBuffer(payload);

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

        private void EncodePacketHeader(byte[] buffer, uint packetLength, ushort protocolVersion, ushort opcodeVersion, GamePacketEndpoint endpoint)
        {
            BitConverter.GetBytes(packetLength).CopyTo(buffer, 0);
            BitConverter.GetBytes(protocolVersion).CopyTo(buffer, 4);
            BitConverter.GetBytes(opcodeVersion).CopyTo(buffer, 6);
            BitConverter.GetBytes((ushort)endpoint).CopyTo(buffer, 8);
            //reserved at 10
        }

        private void DecodePacketHeader(byte[] buffer, out uint packetLength, out ushort protocolVersion, out ushort opcodeVersion, out GamePacketEndpoint endpoint)
        {
            packetLength = BitConverter.ToUInt32(buffer, 0);
            protocolVersion = BitConverter.ToUInt16(buffer, 4);
            opcodeVersion = BitConverter.ToUInt16(buffer, 6);
            endpoint = (GamePacketEndpoint)BitConverter.ToUInt16(buffer, 8);
            //reserved at 10
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
