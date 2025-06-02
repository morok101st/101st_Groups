modded class CSI_ClientComponent
{
	override void Owner_SetMaxGroupMembers(int playerID, int maxMembers)
	{
		Rpc(RpcAsk_SetMaxGroupMembers, playerID, maxMembers);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	override protected void RpcAsk_SetMaxGroupMembers(int playerID, int maxMembers)
	{
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		SCR_AIGroup playersGroup = groupManager.GetPlayerGroup(playerID);
		
		if (maxMembers < playersGroup.GetPlayerCount()) 
		maxMembers = playersGroup.GetPlayerCount();
		
		if (!playersGroup.GetKeepAsCreated()) //-changed-by-nte (added for mattys group flag)
			playersGroup.SetMaxMembers(maxMembers);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Owner_RemovePlayerFromGroup(int playerID)
	{
		Rpc(RpcAsk_RemovePlayerFromGroup, playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	override protected void RpcAsk_RemovePlayerFromGroup(int playerID)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		SCR_PlayerControllerGroupComponent playerGroupController = SCR_PlayerControllerGroupComponent.GetPlayerControllerComponent(playerID);
		SCR_AIGroup group = groupsManager.GetPlayerGroup(playerID);
		
		//-changed-by-nte (makes player join first available group instead of making a new one)
		SCR_AIGroup newGroup = groupsManager.GetFirstNotFullForFaction(group.GetFaction());
		//SCR_AIGroup newGroup = groupsManager.CreateNewPlayableGroup(group.GetFaction());
		
		if (!newGroup)
			return;
	playerGroupController.RequestJoinGroup(newGroup.GetGroupID());
	}
}