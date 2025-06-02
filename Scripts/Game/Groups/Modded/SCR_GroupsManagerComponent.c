modded class SCR_GroupsManagerComponent
{
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] newGroupFaction
	//! \return
	override bool CanCreateNewGroup(notnull Faction newGroupFaction)
	{
		SCR_Faction scrFaction = SCR_Faction.Cast(newGroupFaction);
		
		SCR_AIGroup playerGroup = GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId());
		
		// We DO_NOT_ disable creation of new group if player is the last in his group
		// since when player is in predefined group and only predefined groups are available, player wants to create new group to name it
		//if (playerGroup && playerGroup.GetPlayerCount() == 1)
		//	return false;

		// m_bNewGroupsAllowed is 1 per default
		if (!m_bNewGroupsAllowed)
			return false;
		
		FactionHolder factions = new FactionHolder();
		if (GetFreeFrequency(newGroupFaction) == -1)
			return false;
		
		//if (TryFindEmptyGroup(newGroupFaction))
		//	return false;
		
		// Check for Editor Component Property in Faction
		if (scrFaction.GetCanCreateOnlyPredefinedGroups())
			return false;

		return true;
	}

	//! \param[in] group
	//! \param[in] playerID
	override void OnGroupPlayerRemoved(SCR_AIGroup group, int playerID)
	{
		// This script should only run on the server
		if (IsProxy())
			return;
		
		// Is empty?
		if (group.GetPlayerCount() > 0)
			return;
		
		//Can this group exist empty?
		if (!group.GetDeleteIfNoPlayer())
		{
			if (group.IsPrivate())
				group.SetPrivate(false);
			// -------------------------------------------------
			// Lock group when "Lock when empty" flag is set in group preset
			if (group.GetKeepLocked())
				group.SetPrivate(true);
			// -------------------------------------------------
			return;
		}
		
		// Yes, can we delete it?
		array<SCR_AIGroup> playableGroups = GetPlayableGroupsByFaction(group.GetFaction());
		if (!playableGroups)
			return;				
		
		DeleteGroupDelayed(group);
	}
	
	override void CreatePredefinedGroups()
	{			
		//if(IsProxy()) // TO DO: Commented out coz of initial replication 
		//	return;
			
		FactionManager factionManager = GetGame().GetFactionManager();		
		if (!factionManager)
			return;
		
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager)
			return;
		
		array<Faction> factions = {};
		
		factionManager.GetFactionsList(factions);	
		
		foreach (Faction faction : factions)
		{	
			SCR_Faction scrFaction  = SCR_Faction.Cast(faction);
			if (!scrFaction)
				return;
			
			array<ref SCR_GroupPreset> groups = {};
			
			scrFaction.GetPredefinedGroups(groups);
		
			foreach (SCR_GroupPreset gr : groups)
			{
				SCR_AIGroup newGroup = CreateNewPlayableGroup(faction);
				if (!newGroup)
					continue;
				
				newGroup.SetCanDeleteIfNoPlayer(false);
				
				gr.SetupGroup(newGroup);
				
				if (newGroup.GetGroupFlag().IsEmpty())
					newGroup.SetGroupFlag(0, !scrFaction.GetGroupFlagImageSet().IsEmpty());

				// Save preset that created the group into it, so we can restore defaults when flag is set
				// TODO: move to SetupGroup
				newGroup.SetPreset(gr);
				Print("-- CreatePredefinedGroups -- Create group from preset - customName: ");
				Print(newGroup.GetCustomName());
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] faction
	//! \param[in] ownGroup
	//! \param[in] respectPrivate
	//! \return
	override SCR_AIGroup GetFirstNotFullForFaction(notnull Faction faction, SCR_AIGroup ownGroup = null, bool respectPrivate = true)
	// changed to respect private to true, do not consider private groups option to enter when this function is called (i.e. on group leave / kick)
	{
		Print("         GetFirstNotFullForFaction             ");
		SCR_AIGroup group;
		array<SCR_AIGroup> factionGroups = GetPlayableGroupsByFaction(faction);
		if (!factionGroups)
			return group;
		Print("------------- factiongroups ------------- ");
		// Sort factionGroups according to function used in fucking somewhere
		array<SCR_AIGroup> sortedArray = SCR_GroupSubMenuBase.SortGroupsByFrequency(factionGroups);
		Print(sortedArray);
		for (int i = 0, count = sortedArray.Count(); i < count; i++)
		{
			Print("Name");
			Print(sortedArray[i].GetCustomName());
			if (!sortedArray[i].IsFull() && sortedArray[i] != ownGroup && (!respectPrivate || !sortedArray[i].IsPrivate()))
			{
				group = sortedArray[i];
				Print("First not full group");
				Print(group.GetCustomName());
				break;
			}
		}
		
		return group;
	}
	
	// Returns array of AI_Groups that are sorted the very same way as in GetFirstNotFullForFaction
	// Filters out groups not suitable for joning
	// unused as of now
	array<SCR_AIGroup> sortedNotFullForFaction(Faction faction, SCR_AIGroup ownGroup = null)
	{
		array<SCR_AIGroup> sortedGroups = SCR_GroupSubMenuBase.SortGroupsByFrequency(GetPlayableGroupsByFaction(faction));
		array<SCR_AIGroup> outGroups = new array<SCR_AIGroup>();
		for (int i = 0, count = sortedGroups.Count(); i < count; i++)
		{
			Print("Name");
			Print(sortedGroups[i].GetCustomName());
			if (!sortedGroups[i].IsFull() && sortedGroups[i] != ownGroup && !sortedGroups[i].IsPrivate())
				outGroups.Insert(sortedGroups[i]);
		}

		return outGroups;
	}	
}