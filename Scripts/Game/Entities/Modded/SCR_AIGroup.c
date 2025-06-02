modded class SCR_AIGroup
{
	protected bool m_bKeepLocked = false;
	protected bool m_bKeepPublic = false;
	protected bool m_bKeepAsCreated = false;
	
	// protected bool m_bAlreadyRetried = false;
	
	protected SCR_GroupPreset m_gPreset;
	
	//------------------------------------------------------------------------------------------------
	//! 
	void SetKeepLocked(bool value)
	{
		m_bKeepLocked = value;
	}	
	
	//------------------------------------------------------------------------------------------------
	//! 
	bool GetKeepLocked()
	{
		return m_bKeepLocked;
	}	
	
	//------------------------------------------------------------------------------------------------
	//! 
	void SetKeepPublic(bool value)
	{
		m_bKeepPublic = value;
	}	
	
	//------------------------------------------------------------------------------------------------
	//! 
	bool GetKeepPublic()
	{
		return m_bKeepPublic;
	}	
	//------------------------------------------------------------------------------------------------
	//! 
	void SetKeepAsCreated(bool value)
	{
		m_bKeepAsCreated = value;
	}	
	
	//------------------------------------------------------------------------------------------------
	//! 
	bool GetKeepAsCreated()
	{
		return m_bKeepAsCreated;
	}
	
	//------------------------------------------------------------------------------------------------
	//! 
	void SetPreset(SCR_GroupPreset value)
	{
		m_gPreset = value;
	}	
	
	//------------------------------------------------------------------------------------------------
	//! 
	SCR_GroupPreset GetPreset()
	{
		return m_gPreset;
	}
	
	
	//------------------------------------------------------------------------------------------------
	//! Called on the server (authority)
	// Group cannot be changed to private if m_bKeepPublic is set
	// Group cannot be changed to public if m_bKeepLocked is set
	// will switch as usual when no flag is set
	override void SetPrivate(bool isPrivate)
	{
		if (m_bKeepPublic) { // cannot be changed to private
			RPC_SetPrivate(false);
			Rpc(RPC_SetPrivate, false);
		}else if (m_bKeepLocked){ // cannot be changed to public
			RPC_SetPrivate(true);
			Rpc(RPC_SetPrivate, true);
		}else { // default
			RPC_SetPrivate(isPrivate);
			Rpc(RPC_SetPrivate, isPrivate);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// unchanged
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	override void RPC_DoSetCustomName(string name, int authorID)
	{
		Print("-- RPC_DoSetCustomName -- name:");
		Print(name);
		if (name.IsEmpty())
		{
			Print("-- empty name --");
			m_sCustomName = name;
			m_iNameAuthorID = authorID;
			s_OnCustomNameChanged.Invoke(this);
			return;
		}
		
		array<string> textToFilter = {};
		
		m_ProfanityCallbackName = new SCR_ScriptProfanityFilterRequestCallback();
		textToFilter.Insert(name);
		m_ProfanityCallbackName.m_OnResult.Insert(OnNameFilteredCallback);

		if (!GetGame().GetPlatformService().FilterProfanityAsync(textToFilter, m_ProfanityCallbackName))
			OnNameFilteredCallback(textToFilter);
		Print("-- textToFilter --");
		Print(textToFilter);
		m_iNameAuthorID = authorID;
		
	}
	
	// unchanged	
	override void OnNameFilteredCallback(array<string> resultText)
	{
		//Print("-- OnNameFilteredCallback -- m_sCustomName:");
		m_sCustomName = resultText.Get(0);
		s_OnCustomNameChanged.Invoke(this);
		//Print(m_sCustomName);
	}

	// this returns empty strings when scenario is restarted?
	override string GetCustomName()
	{	
		Print("-- GetCustomName -- m_sCustomName:");
		Print(m_sCustomName);
		bool isRestricted;
		PlayerController pc = GetGame().GetPlayerController();
		if (!pc) {
			Print("-- GetCustomName -- NO PLAYER CONTROLLER! return empty string");
			// Retry
			//if (!m_bAlreadyRetried) 
			//{
			//	Print("-- GetCustomName -- retry --------------------------------------------");	
			//	GetGame().GetCallqueue().CallLater(GetCustomName, 1 * 1000);
			//	m_bAlreadyRetried = true;
			//}
			
			return string.Empty;
		}
		SocialComponent  socialComp = SocialComponent.Cast(pc.FindComponent(SocialComponent));
		if (!socialComp)
		{
			Print("-- GetCustomName -- NO SOCIAL COMPONENT! return empty string");
			return string.Empty;
		}
		
		if (m_iNameAuthorID > 0)
			isRestricted = socialComp.IsRestricted(m_iNameAuthorID, EUserInteraction.UserGeneratedContent);
		
		if (isRestricted || m_sCustomName.IsEmpty())
		{
			Print("-- GetCustomName -- isRestricted || m_sCustomName.IsEmpty()  return empty string");
			return string.Empty;
		}
		
		Print("-- GetCustomName -- returns m_sCustomName:");
		return m_sCustomName;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called on the server (authority)
	// Do not reset customName & description when leader leaves a predefined group
	override void RemovePlayer(int playerID)
	{
		Print("-- RemovePlayer --");
		if (!m_aPlayerIDs.Contains(playerID))
			return;
		
		//if player is last in group it doesnt matter as the group will get deleted
		if (playerID == m_iLeaderID  &&  GetPlayerCount() > 1)
		{
			Print("-- LEader Leaves non empty group --");
			// -- only do if not predefined group --
			if (!m_gPreset)
			// -----------------------------------------
			{
				Print("-- RemovePlayer --");
				SetCustomName("", 0);
				SetCustomDescription("", 0);
			}
		}
		
		RPC_DoRemovePlayer(playerID);
		Rpc(RPC_DoRemovePlayer, playerID);
		CheckForLeader(-1, false);
		RemovePlayerAgent(playerID);
		SCR_NotificationsComponent.SendToGroup(m_iGroupID, ENotification.GROUPS_PLAYER_LEFT, playerID);
	}
}