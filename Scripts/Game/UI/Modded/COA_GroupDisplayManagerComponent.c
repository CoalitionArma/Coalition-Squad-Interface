[ComponentEditorProps(category: "GameScripted/Callsign", description: "")]
class COA_GroupDisplayManagerComponentClass: SCR_BaseGameModeComponentClass {};

class COA_GroupDisplayManagerComponent : SCR_BaseGameModeComponent
{	
	protected ref map<string,string> m_mPlayerMap = new map <string,string>;
	protected ref map<string,string> m_mAuthorityPlayerMap = new map <string,string>;
	
	//------------------------------------------------------------------------------------------------

	// override/static functions

	//------------------------------------------------------------------------------------------------

	static COA_GroupDisplayManagerComponent GetInstance() 
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (gameMode)
			return COA_GroupDisplayManagerComponent.Cast(gameMode.FindComponent(COA_GroupDisplayManagerComponent));
		else
			return null;
	}
	
	//------------------------------------------------------------------------------------------------

	// Functions to store and replicate a map in which we store custom player values

	//------------------------------------------------------------------------------------------------
	
	void UpdateAuthorityPlayerMap(int playerID, string write, string value)
	{		
		if (!Replication.IsServer()) return;
				
		string key = string.Format("%1 %2", playerID, write);
		string valueStr = m_mAuthorityPlayerMap.Get(key);
		
   		if(valueStr == value) return;
							
		m_mAuthorityPlayerMap.Set(key, value);
		EncodeMapThenBroadcast(m_mAuthorityPlayerMap);
	}
	
	string ReturnMapValue(string key, int playerID)
	{	
		string checkKey = string.Format("%1 %2", playerID, key);
		return m_mPlayerMap.Get(checkKey);
	}
	
	//------------------------------------------------------------------------------------------------

	// Functions for replication
	
	//------------------------------------------------------------------------------------------------
	
	protected void EncodeMapThenBroadcast(map<string,string> mapToEncode)
	{	
		if (!Replication.IsServer()) return;
		
		string localMapValueStr = "";
		
		// Encode mapToEncode into a string so we can replicate it.
		for (int i = 0; i<mapToEncode.Count(); i++)
		{
			string key = mapToEncode.GetKey(i);
			string value = mapToEncode.Get(key);
			
			string mapValueString = string.Format("%1!%2!", key, value);
			string localMapStr = string.Format("%1=%2", localMapValueStr, mapValueString);
			localMapValueStr = localMapStr;
		};
		
		DecodeMap(localMapValueStr);
		Rpc(DecodeMap, localMapValueStr);
	}
	
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void DecodeMap(string mapStr)
	{	
		array<string> mapArray = {};
		mapStr.Split("=", mapArray, true);
		foreach (string keyAndValue : mapArray)
		{
			array<string> keyAndValueArray = {};
			keyAndValue.Split("!", keyAndValueArray, false);
			m_mPlayerMap.Set(keyAndValueArray[0], keyAndValueArray[1]);
		};
	}
}
