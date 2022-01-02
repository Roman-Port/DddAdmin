using System;
using System.Collections.Generic;
using System.Text;

namespace LibDddAdminTransport
{
    public class GameMessage
    {
        public GameMessage()
        {

        }

        public static GameMessage Deserialize(byte[] buffer)
        {
            return Deserialize(buffer, 0, buffer.Length);
        }

        private static GameMessage Deserialize(byte[] buffer, int offset, int length)
        {
            //Create
            GameMessage msg = new GameMessage();

            //Read properties until the end is reached
            int end = offset + length;
            while (offset < end)
            {
                //Decode header
                GameMessageKey key = (GameMessageKey)BitConverter.ToUInt16(buffer, offset); offset += 2;
                GameMessageKey type = (GameMessageKey)BitConverter.ToUInt16(buffer, offset); offset += 2;
                int propLen = BitConverter.ToInt32(buffer, offset); offset += 4;

                //Convert to desired object
                switch (type)
                {
                    case GameMessageKey.DDDTYPE_SHORT:
                        if (propLen != 2)
                            throw new Exception("Error decoding message; Invalid length for data type.");
                        msg.PutShort(key, BitConverter.ToInt16(buffer, offset));
                        break;
                    case GameMessageKey.DDDTYPE_INT:
                        if (propLen != 4)
                            throw new Exception("Error decoding message; Invalid length for data type.");
                        msg.PutInt(key, BitConverter.ToInt32(buffer, offset));
                        break;
                    case GameMessageKey.DDDTYPE_FLOAT:
                        if (propLen != 4)
                            throw new Exception("Error decoding message; Invalid length for data type.");
                        msg.PutFloat(key, BitConverter.ToSingle(buffer, offset));
                        break;
                    case GameMessageKey.DDDTYPE_STRING:
                        msg.PutString(key, Encoding.UTF8.GetString(buffer, offset, propLen));
                        break;
                    case GameMessageKey.DDDTYPE_MESSAGE:
                        msg.PutMessage(key, Deserialize(buffer, offset, propLen));
                        break;
                    case GameMessageKey.DDDTYPE_MESSAGE_ARRAY:
                        msg.PutMessageArray(key, DeserializeArray(buffer, offset, propLen));
                        break;
                    default:
                        throw new Exception("Error decoding message; Unknown type: " + type);
                }

                //Advance
                offset += propLen;
            }

            //Make sure we land right on the dot
            if (offset != end)
                throw new Exception("Parsing bug; Read too far!");

            return msg;
        }

        private static List<GameMessage> DeserializeArray(byte[] buffer, int offset, int length)
        {
            //Query length and create
            int end = offset + length;
            int count = BitConverter.ToInt32(buffer, offset); offset += 4;
            List<GameMessage> result = new List<GameMessage>(count);

            //Deserialize each
            for (int i = 0; i < count; i++)
            {
                //Read length
                int propLen = BitConverter.ToInt32(buffer, offset); offset += 4;

                //Deserialize
                result.Add(Deserialize(buffer, offset, propLen));

                //Advance
                offset += propLen;
            }

            //Make sure we land right on the dot
            if (offset != end)
                throw new Exception("Parsing bug; Read too far!");

            return result;
        }

        private Dictionary<GameMessageKey, object> values = new Dictionary<GameMessageKey, object>();

        public IReadOnlyDictionary<GameMessageKey, object> RawValues => values;

        public bool HasValue(GameMessageKey key)
        {
            return values.ContainsKey(key);
        }

        public void PutShort(GameMessageKey key, short value)
        {
            values.Add(key, value);
        }

        public void PutInt(GameMessageKey key, int value)
        {
            values.Add(key, value);
        }

        public void PutFloat(GameMessageKey key, float value)
        {
            values.Add(key, value);
        }

        public void PutString(GameMessageKey key, string value)
        {
            values.Add(key, value);
        }

        public void PutMessage(GameMessageKey key, GameMessage value)
        {
            values.Add(key, value);
        }

        public void PutMessageArray(GameMessageKey key, List<GameMessage> value)
        {
            values.Add(key, value);
        }

        public short GetShort(GameMessageKey key)
        {
            return Get<short>(key);
        }

        public int GetInt(GameMessageKey key)
        {
            return Get<int>(key);
        }

        public float GetFloat(GameMessageKey key)
        {
            return Get<float>(key);
        }

        public string GetString(GameMessageKey key)
        {
            return Get<string>(key);
        }

        public GameMessage GetMessage(GameMessageKey key)
        {
            return Get<GameMessage>(key);
        }

        public List<GameMessage> GetMessageArray(GameMessageKey key)
        {
            return Get<List<GameMessage>>(key);
        }

        private T Get<T>(GameMessageKey key)
        {
            //Fetch from dict
            object value;
            if (!values.TryGetValue(key, out value))
                throw new KeyNotFoundException($"Could not find {key} in message.");

            //Get the type and make sure it's what we expect
            if (typeof(T) != value.GetType())
                throw new Exception($"Key \"{key}\" exists in message, but it was not a matching type.");

            return (T)value;
        }

        public void Dump(StringBuilder builder)
        {
            Dump(builder, 0);
        }

        private void Dump(StringBuilder builder, int depth, string prefix = "")
        {
            //Create padding
            string padding = "";
            for (int i = 0; i < depth; i++)
                padding += "    ";

            //Write title
            builder.AppendLine(padding + prefix + "{");

            //Add each
            foreach (var p in values)
            {
                builder.AppendLine(padding + "    " + $"[{p.Key} ({GetItemType(p.Value)})] => {p.Value}");
                if (p.Value is GameMessage)
                    ((GameMessage)p.Value).Dump(builder, depth + 1);
                else if (p.Value is List<GameMessage>)
                {
                    int index = 0;
                    builder.AppendLine(padding + "    [");
                    foreach (var pp in (List<GameMessage>)p.Value)
                        pp.Dump(builder, depth + 2, $"[{index++}] ");
                    builder.AppendLine(padding + "    ]");
                }
            }

            //Write footer
            builder.AppendLine(padding + "}");
        }

        private GameMessageKey GetItemType(object o)
        {
            if (o.GetType() == typeof(short))
                return GameMessageKey.DDDTYPE_SHORT;
            if (o.GetType() == typeof(int))
                return GameMessageKey.DDDTYPE_INT;
            if (o.GetType() == typeof(float))
                return GameMessageKey.DDDTYPE_FLOAT;
            if (o.GetType() == typeof(string))
                return GameMessageKey.DDDTYPE_STRING;
            if (o.GetType() == typeof(GameMessage))
                return GameMessageKey.DDDTYPE_MESSAGE;
            if (o.GetType() == typeof(List<GameMessage>))
                return GameMessageKey.DDDTYPE_MESSAGE_ARRAY;
            throw new Exception("Invalid network type.");
        }
    }
}
