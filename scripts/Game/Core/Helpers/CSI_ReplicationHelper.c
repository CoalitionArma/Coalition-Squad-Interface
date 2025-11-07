class CSI_ReplicationHelper
{
	//------------------------------------------------------------------------------------------------
	static void WriteMapIntInt(ScriptBitWriter writer, map<int, int> mapToWrite)
	{
		int count = mapToWrite.Count();
		writer.WriteInt(count);
		for (int i = 0; i < count; i++)
		{
			writer.WriteInt(mapToWrite.GetKey(i));
			writer.WriteInt(mapToWrite.GetElement(i));
		}
	}
	
	//------------------------------------------------------------------------------------------------
	static void ReadMapIntInt(ScriptBitReader reader, map<int, int> mapToWrite)
	{
		int count;
		reader.ReadInt(count);
		for (int i = 0; i < count; i++)
		{
			int key;
			int value;
			reader.ReadInt(key);
			reader.ReadInt(value);

			mapToWrite.Insert(key, value);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	static void WriteMapIntBool(ScriptBitWriter writer, map<int, bool> mapToWrite)
	{
		int count = mapToWrite.Count();
		writer.WriteInt(count);
		for (int i = 0; i < count; i++)
		{
			writer.WriteInt(mapToWrite.GetKey(i));
			writer.WriteBool(mapToWrite.GetElement(i));
		}
	}
	
	//------------------------------------------------------------------------------------------------
	static void ReadMapIntBool(ScriptBitReader reader, map<int, bool> mapToWrite)
	{
		int count;
		reader.ReadInt(count);
		for (int i = 0; i < count; i++)
		{
			int key;
			bool value;
			reader.ReadInt(key);
			reader.ReadBool(value);

			mapToWrite.Insert(key, value);
		}
	}
}