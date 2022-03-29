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

        public static GameMessage FromBuffer(byte[] buffer, int offset = 0)
        {
            GameMessage msg = new GameMessage();
            msg.Deserialize(buffer, offset);
            return msg;
        }

        private static GameMessage[] ArrayFromBuffer(byte[] buffer, int offset)
        {
            //Get the number of items
            int count = BitConverter.ToInt32(buffer, offset); offset += 4;

            //Make array and process
            GameMessage[] msgs = new GameMessage[count];
            for (int i = 0; i < msgs.Length; i++)
            {
                msgs[i] = new GameMessage();
                offset = msgs[i].Deserialize(buffer, offset);
            }
            return msgs;
        }

        private Dictionary<GameMessageKey, object> values = new Dictionary<GameMessageKey, object>();

        public int SerializedLength
        {
            get
            {
                int length = 2;
                foreach (var p in values)
                {
                    length += 2 + 2 + 4;
                    length += GetItemSize(p.Value);
                }
                return length;
            }
        }

        public int Serialize(byte[] buffer, int offset)
        {
            //Write the number of properties
            BitConverter.GetBytes((ushort)values.Count).CopyTo(buffer, offset); offset += 2;

            //Write each property
            foreach (var p in values)
            {
                //Calculate size
                int size = GetItemSize(p.Value);

                //Write header
                BitConverter.GetBytes((ushort)p.Key).CopyTo(buffer, offset); offset += 2;
                BitConverter.GetBytes((ushort)GetItemType(p.Value)).CopyTo(buffer, offset); offset += 2;
                BitConverter.GetBytes((uint)size).CopyTo(buffer, offset); offset += 4;

                //Convert
                if (p.Value is short valueShort)
                    BitConverter.GetBytes(valueShort).CopyTo(buffer, offset);
                else if (p.Value is int valueInt)
                    BitConverter.GetBytes(valueInt).CopyTo(buffer, offset);
                else if (p.Value is long valueLong)
                    BitConverter.GetBytes(valueLong).CopyTo(buffer, offset);
                else if (p.Value is float valueFloat)
                    BitConverter.GetBytes(valueFloat).CopyTo(buffer, offset);
                else if (p.Value is string valueString)
                    Encoding.UTF8.GetBytes(valueString).CopyTo(buffer, offset);
                else if (p.Value is GameMessage valueObject)
                    valueObject.Serialize(buffer, offset);
                else if (p.Value is GameMessage[] valueObjectArr)
                    SerializeArray(valueObjectArr, buffer, offset);
                else
                    throw new Exception("This type cannot be serialized.");

                //Advance
                offset += size;
            }
            return offset;
        }

        private static void SerializeArray(GameMessage[] messages, byte[] buffer, int offset)
        {
            //Write the number of items
            BitConverter.GetBytes((ushort)messages.Length).CopyTo(buffer, offset); offset += 4;

            //Serialize each
            foreach (var m in messages)
                offset = m.Serialize(buffer, offset);
        }

        public int Deserialize(byte[] buffer, int offset)
        {
            //Read the number of properties
            int propCount = BitConverter.ToUInt16(buffer, offset); offset += 2;
            
            //Read all of these
            for (int i = 0; i < propCount; i++)
            {
                //Decode header
                GameMessageKey key = (GameMessageKey)BitConverter.ToUInt16(buffer, offset); offset += 2;
                GameMessageKey type = (GameMessageKey)BitConverter.ToUInt16(buffer, offset); offset += 2;
                int propLen = BitConverter.ToInt32(buffer, offset); offset += 4;

                //Convert to desired object
                switch (type)
                {
                    case GameMessageKey.TYPE_INT16:
                        if (propLen != 2)
                            throw new Exception("Error decoding message; Invalid length for data type.");
                        PutShort(key, BitConverter.ToInt16(buffer, offset));
                        break;
                    case GameMessageKey.TYPE_INT32:
                        if (propLen != 4)
                            throw new Exception("Error decoding message; Invalid length for data type.");
                        PutInt(key, BitConverter.ToInt32(buffer, offset));
                        break;
                    case GameMessageKey.TYPE_INT64:
                        if (propLen != 8)
                            throw new Exception("Error decoding message; Invalid length for data type.");
                        PutLong(key, BitConverter.ToInt64(buffer, offset));
                        break;
                    case GameMessageKey.TYPE_FLOAT:
                        if (propLen != 4)
                            throw new Exception("Error decoding message; Invalid length for data type.");
                        PutFloat(key, BitConverter.ToSingle(buffer, offset));
                        break;
                    case GameMessageKey.TYPE_STRING:
                        PutString(key, Encoding.UTF8.GetString(buffer, offset, propLen));
                        break;
                    case GameMessageKey.TYPE_OBJECT:
                        PutMessage(key, FromBuffer(buffer, offset));
                        break;
                    case GameMessageKey.TYPE_OBJECT_ARR:
                        PutMessageArray(key, ArrayFromBuffer(buffer, offset));
                        break;
                    default:
                        throw new Exception("Error decoding message; Unknown type: " + type);
                }

                //Advance
                offset += propLen;
            }

            return offset;
        }

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

        public void PutLong(GameMessageKey key, long value)
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

        public void PutMessageArray(GameMessageKey key, GameMessage[] value)
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

        public long GetLong(GameMessageKey key)
        {
            return Get<long>(key);
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

        public GameMessage[] GetMessageArray(GameMessageKey key)
        {
            return Get<GameMessage[]>(key);
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
                return GameMessageKey.TYPE_INT16;
            if (o.GetType() == typeof(int))
                return GameMessageKey.TYPE_INT32;
            if (o.GetType() == typeof(long))
                return GameMessageKey.TYPE_INT64;
            if (o.GetType() == typeof(float))
                return GameMessageKey.TYPE_FLOAT;
            if (o.GetType() == typeof(string))
                return GameMessageKey.TYPE_STRING;
            if (o.GetType() == typeof(GameMessage))
                return GameMessageKey.TYPE_OBJECT;
            if (o.GetType() == typeof(GameMessage[]))
                return GameMessageKey.TYPE_OBJECT_ARR;
            throw new Exception("Invalid network type.");
        }

        private int GetItemSize(object o)
        {
            if (o.GetType() == typeof(short))
                return 2;
            if (o.GetType() == typeof(int))
                return 4;
            if (o.GetType() == typeof(long))
                return 8;
            if (o.GetType() == typeof(float))
                return 4;
            if (o.GetType() == typeof(string))
                return (o as string).Length;
            if (o.GetType() == typeof(GameMessage))
                return (o as GameMessage).SerializedLength;
            if (o.GetType() == typeof(GameMessage[]))
            {
                GameMessage[] msgs = (o as GameMessage[]);
                int len = 4;
                foreach (var m in msgs)
                    len += m.SerializedLength;
                return len;
            }
            throw new Exception("Invalid network type.");
        }
    }
}
