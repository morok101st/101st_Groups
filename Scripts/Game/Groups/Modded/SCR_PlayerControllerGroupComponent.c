modded class SCR_PlayerControllerGroupComponent
{

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	override void RPC_AskCreateGroup()
	{
		Print("--  RPC_AskCreateGroup");
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;
		
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return;
		
		Faction faction = factionManager.GetPlayerFaction(GetPlayerID());
		if (!faction)
			return;
		
		SCR_Faction scrFaction = SCR_Faction.Cast(faction);
		if(!scrFaction)
			return;
		

		// We _DO NOT_ check if there is any empty group already for our faction
		//Print("We check if there is any empty group already for our faction");
		//if (groupsManager.TryFindEmptyGroup(faction))
			//return;
		

		// We _DO NOT_ check if other then predefined group can be created
		//Print("We check if other then predefined group can be created");
		if(scrFaction.GetCanCreateOnlyPredefinedGroups())
			return;
		
		// No empty group found, we allow creation of new group	
		//Print("Creation of new group");	
		SCR_AIGroup newGroup = groupsManager.CreateNewPlayableGroup(faction);
		
		// No new group was created, return
		//Print("No new group was created, return");	
		if (!newGroup)
			return;
		
		// New group sucessfully created
		// Print("New group sucessfully created");	
		// The player should be automatically added/moved to it
		RPC_AskJoinGroup(newGroup.GetGroupID());
		Print("--  RPC_AskCreateGroup -- group.CustomName");
		Print(newGroup.GetCustomName());
	}
	
		//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] groupID
	//! \param[in] name
	//! \param[in] authorID
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	override void RPC_AskSetCustomName(int groupID, string name, int authorID)
	{
		
		Print("-- RPC_AskSetCustomName");
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;
		
		SCR_AIGroup group = groupsManager.FindGroup(groupID);
		if (!group)
			return;
		
		if (group.GetKeepAsCreated()){
			Print("Group is flagged as KeepAsCreated, no changes");
		}
		else {
			Print("-- RPC_AskSetCustomName, set CustomName");
			group.SetCustomName(name, authorID);
		}
		Print("-- RPC_AskSetCustomName, group.CustomName");
		Print(group.GetCustomName());
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] groupID
	//! \param[in] desc
	//! \param[in] authorID
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	override void RPC_AskSetCustomDescription(int groupID, string desc, int authorID)
	{
		Print("-- RPC_AskSetCustomDescription");
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;
		
		SCR_AIGroup group = groupsManager.FindGroup(groupID);
		if (!group)
			return;
		
		if (group.GetKeepAsCreated())
		{
			Print("Group is flagged as KeepAsCreated, no changes");
		}
		else {
			Print("-- SetCustomDescription");
			Print(desc);
			group.SetCustomDescription(desc, authorID);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] groupID
	//! \param[in] maxMembers
	override void RequestSetGroupMaxMembers(int groupID, int maxMembers)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;
		
		SCR_AIGroup group = groupsManager.FindGroup(groupID);
		if (!group)
			return;
		
		// ---------------------------------
		if (group.GetKeepAsCreated())
			return;		
		// ---------------------------------
		
		if (group.GetMaxMembers() == maxMembers || maxMembers < 0)
			return;
		
		Rpc(RPC_AskSetGroupMaxMembers, groupID, maxMembers);
	}
	 
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] groupID
	//! \param[in] flagIndex
	//! \param[in] isFromImageset
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	override void RPC_AskSetGroupFlag(int groupID, int flagIndex, bool isFromImageset)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;
		
		SCR_AIGroup group = groupsManager.FindGroup(groupID);
		if (!group)
			return;
		
		// ---------------------------------
		if (group.GetKeepAsCreated())
			return;
		// ---------------------------------
		
		group.SetGroupFlag(flagIndex, isFromImageset);		
	}
	
	
}