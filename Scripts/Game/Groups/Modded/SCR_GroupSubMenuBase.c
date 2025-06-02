// Left side of group tab
// Updates Groups and sorts them by frequency
// Show hide settings button 
modded class SCR_GroupSubMenuBase{

	static array<SCR_AIGroup> SortGroupsByFrequency(array<SCR_AIGroup> groups)
	{
	    int count = groups.Count();
	
	    // Bubble Sort Algorithm
	    for (int i = 0; i < count - 1; i++)
	    {
	        for (int j = 0; j < count - i - 1; j++)
	        {
	            // Compare frequencies and swap if needed
	            if (groups.Get(j).GetRadioFrequency() > groups.Get(j + 1).GetRadioFrequency())
	            {
	                // Swap elements
	                SCR_AIGroup temp = groups.Get(j);
	                groups.Set(j, groups.Get(j + 1));
	                groups.Set(j + 1, temp);
	            }
	        }
	    }
	
	    return groups;
	}
	
	override protected void UpdateGroups(SCR_PlayerControllerGroupComponent playerGroupController)
	{
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return;
		
		SCR_Faction playerFaction = SCR_Faction.Cast(factionManager.GetLocalPlayerFaction());
		if (!playerFaction)
			return;
		
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager)
			return;
		
		if (!playerGroupController)
			return;
		
		//no need to check playerFaction for null, because groups are not enabled for players without faction
		m_AddGroupButton.SetEnabled(groupManager.CanCreateNewGroup(playerFaction));
		Widget children = m_wGridWidget.GetChildren();
		while (children)
		{
			m_wGridWidget.RemoveChild(children);
			children = m_wGridWidget.GetChildren();
		}
		
		bool showSettingsButton = playerGroupController.IsPlayerLeaderOwnGroup() && groupManager.CanPlayersChangeAttributes();
		int selectedGroupID = playerGroupController.GetSelectedGroupID();
		
		SetNavigationButtonVisibile(m_GroupSettingsButton, showSettingsButton);
		
		array<SCR_AIGroup> playableGroups = groupManager.GetPlayableGroupsByFaction(playerFaction);
		if (!playableGroups)
			return;
				
		if (playableGroups.IsIndexValid(selectedGroupID) && m_wMenuRoot)
		{
			ImageWidget privateIcon = ImageWidget.Cast(m_wMenuRoot.FindAnyWidget("PrivateIconDetail"));
			if (privateIcon)
				privateIcon.SetVisible(playableGroups[selectedGroupID].IsPrivate());
		}
		
		// ------------------------------------------------------------------------------------ //
		// Sort groups and resulting UI display order. Sort by frequency.
	    array<SCR_AIGroup> reversedArray = SortGroupsByFrequency(playableGroups);
		playableGroups = reversedArray;
		// ------------------------------------------------------------------------------------ //
		
		int groupCount = playableGroups.Count();
		for (int i = 0; i < groupCount; i++)
		{ 
			Widget groupTile = GetGame().GetWorkspace().CreateWidgets(m_ButtonLayout, m_wGridWidget);	
			if (!groupTile)
				continue;
					
			ButtonWidget buttonWidget = ButtonWidget.Cast(groupTile.FindAnyWidget("Button"));
			if (!buttonWidget)
				continue;
			
			SCR_GroupTileButton buttonComponent = SCR_GroupTileButton.Cast(buttonWidget.FindHandler(SCR_GroupTileButton));
			if (buttonComponent)
			{
				buttonComponent.SetGroupID(playableGroups[i].GetGroupID());
				buttonComponent.SetGroupFaction(playerFaction);
				buttonComponent.SetJoinGroupButton(m_JoinGroupButton);
				buttonComponent.InitiateGroupTile();				
				if (playerGroupController.GetGroupID() == -1 && i == 0 && playerGroupController.GetSelectedGroupID() < 0)
					GetGame().GetCallqueue().CallLater(buttonComponent.RefreshPlayers, 1, false);
				if (selectedGroupID < 0 && playableGroups[i].GetGroupID() == playerGroupController.GetGroupID() )
					GetGame().GetCallqueue().CallLater(buttonComponent.RefreshPlayers, 1, false);
				if (selectedGroupID == playableGroups[i].GetGroupID())
					GetGame().GetCallqueue().CallLater(buttonComponent.RefreshPlayers, 1, false);
			}
		}
	}

}