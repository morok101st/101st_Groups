// Each TileButton represents a group header including buttons to modify the group attributes
// decide if edit buttons are shown
//------------------------------------------------------------------------------------------------
modded class SCR_GroupTileButton{
	
	override void SetupJoinGroupButton()
	{
		SCR_PlayerControllerGroupComponent playerGroupController = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!playerGroupController)
			return;

		SCR_AIGroup group;
		if (playerGroupController.GetSelectedGroupID() == -1)
		{
			group = m_GroupManager.FindGroup(m_iGroupID);
		}
		else
		{
			group = m_GroupManager.FindGroup(playerGroupController.GetSelectedGroupID());
		}

		if (!group)
			return;

		m_JoinGroupButton.m_OnActivated.Clear();
		
		// -----------------------------------------------------------------------------------------------
		// Make all groups be joinable by Gamemaster
		// Used to join locked and emty predefined groups
		PlayerController playerController = GetGame().GetPlayerController();
		int playerID = playerController.GetPlayerId();
		
		bool isAdmin = SCR_Global.IsAdmin(playerID);
		
		// Check if group is locked & player is gamemaster
		if (group.IsPrivate() && !isAdmin)
		{
			// Usual behaviour, ask for invite in locked groups
			m_JoinGroupButton.SetLabel(REQUEST_JOIN_GROUP);
			m_JoinGroupButton.m_OnActivated.Insert(SCR_GroupSubMenu.RequestJoinPrivateGroup);
		}
		else if (isAdmin)
		{
			// Admin may join any time (if group not full etc. see below line 60)
			m_JoinGroupButton.SetLabel(JOIN_GROUP);
			m_JoinGroupButton.m_OnActivated.Insert(SCR_GroupSubMenu.JoinSelectedGroup);
			//m_JoinGroupButton.m_OnActivated.Insert(SCR_GroupSubMenu.GetGroupLeadPrivateGroup);			
		}
		else
		{
			// unlocked groups, behave as usual
			m_JoinGroupButton.SetLabel(JOIN_GROUP);
			m_JoinGroupButton.m_OnActivated.Insert(SCR_GroupSubMenu.JoinSelectedGroup);
		}
		// -----------------------------------------------------------------------------------------------
		
		array<int> denied = {};
		group.GetDeniedRequesters(denied);
		array<int> requesters = {};
		group.GetRequesterIDs(requesters);

		bool requesterDenied = denied.Contains(playerGroupController.GetPlayerID());
		bool requestSent = requesters.Contains(playerGroupController.GetPlayerID());

		// Enable & Disable button when
		if (
		// my request was denied 
				requesterDenied || 	
		// my request was send, but not yet accepted/rejected
				requestSent ||		
		// no group exists
				!m_GroupManager.FindGroup(m_iGroupID) ||	
		// I am not allowed at all (bcs I'm already in the group)
				!playerGroupController.CanPlayerJoinGroup( SCR_PlayerController.GetLocalPlayerId() ,m_GroupManager.FindGroup(m_iGroupID))
		)
			{
				m_JoinGroupButton.SetEnabled(false);
			}
		else
			m_JoinGroupButton.SetEnabled(true);
	}
	
	//------------------------------------------------------------------------------------------------
	// Change how Groups with custom name are displayed.
	override void InitiateGroupTile()
	{
		m_GroupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!m_GroupManager)
			return;

		m_GroupComponent = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!m_GroupComponent)
			return;

		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!playerController)
			return;


		SCR_AIGroup group = m_GroupManager.FindGroup(m_iGroupID);
		if (!group)
			return;

		group.GetOnPlayerLeaderChanged().Insert(SetupJoinGroupButton);
		group.GetOnJoinPrivateGroupRequest().Insert(SetupJoinGroupButton);
		
		//workaround for issues with playerControllerGroupsComponent, needs to be reworked with this whole script mess
		SCR_GroupSubMenu.Init();
		SCR_GroupSubMenu.GetOnJoingGroupRequestSent().Insert(SetupJoinGroupButton);

		m_BaseTaskManager = GetTaskManager();
		if (m_BaseTaskManager)
			m_BaseTaskManager.s_OnTaskUpdate.Insert(RefreshPlayers);

		group.GetOnMemberStateChange().Insert(RefreshPlayers);

		RichTextWidget squadName = RichTextWidget.Cast(GetRootWidget().FindAnyWidget("Callsign"));
		if (squadName)
		{
			string groupName = group.GetCustomNameWithOriginal();
			Print("-- InitiateGroupTile -- GetCustomNameWithOriginal");
			Print(groupName);
			string company, platoon, squad, character, format;
			group.GetCallsigns(company, platoon, squad, character, format);

			if (groupName.IsEmpty() || group.GetNameAuthorID() != 0 && !playerController.CanViewContentCreatedBy(group.GetNameAuthorID()))
			{
				Print("-- InitiateGroupTile -- if statement");
				if (group.GetCustomName()) 
				{
					squadName.SetText(group.GetCustomName());
				}
				else 
				{
					squadName.SetTextFormat(format, company, platoon, squad, character);
				}
			}
			else
			{
				
				//Print("-- InitiateGroupTile -- else statement");
				//Print(groupName);
				company = WidgetManager.Translate(company);
				if (group.GetCustomName()) 
				{
					squadName.SetText(group.GetCustomName());
				}
				// -----------------------------------------
				// Display Original Name if no custom Name is given
				// if (!group.GetCustomName())
				else squadName.SetText(" (" + string.Format(format, company, platoon, squad, character) + ")");

				//groupName = group.GetCustomName() + " (" + string.Format(format, company, platoon, squad, character) + ")";
				//squadName.SetText(groupName);
				
				// Only display custom name, no '(original name appendix)'
				// squadName.SetText(group.GetCustomName());
			}
			Print("-- InitiateGroupTile -- groupCustomName");
			Print(group.GetCustomName());
		}

		RichTextWidget frequency = RichTextWidget.Cast(GetRootWidget().FindAnyWidget("Frequency"));
		if (frequency)
			frequency.SetText(""+group.GetRadioFrequency()*0.001 + " #AR-VON_FrequencyUnits_MHz");

		RichTextWidget playerCount = RichTextWidget.Cast(GetRootWidget().FindAnyWidget("PlayerCount"));
		if (playerCount)
			playerCount.SetText(group.GetPlayerCount().ToString() + "/" + group.GetMaxMembers());

		if (group.IsFull())
			SetGroupInfoColor(m_GroupFullColor);

		ImageWidget background = ImageWidget.Cast(GetRootWidget().FindAnyWidget("Background"));

		if (background)
			background.SetVisible(m_GroupComponent.GetSelectedGroupID() == m_iGroupID);

		if (group.IsPlayerInGroup(playerController.GetPlayerId()))
			squadName.SetColor(m_PlayerNameSelfColor);

		ImageWidget privateIcon = ImageWidget.Cast(GetRootWidget().FindAnyWidget("PrivateIcon"));
		if (privateIcon)
			privateIcon.SetVisible(group.IsPrivate());

		if (!m_ParentSubMenu)
			FindParentMenu();
	}
	
	//------------------------------------------------------------------------------------------------
	override void RefreshPlayers()
	{
		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!m_GroupFaction || !m_GroupManager || !playerManager)
			return;

		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!playerController)
			return;

		SCR_AIGroup group = m_GroupManager.FindGroup(m_iGroupID);
		if (!group)
			return;

		SetupSelectGroupFlagButton(group);

		if (m_ParentSubMenu == null)
			FindParentMenu();

		m_aPlayerComponentsList.Clear();

		VerticalLayoutWidget playerList = VerticalLayoutWidget.Cast(m_ParentSubMenu.GetRootWidget().FindAnyWidget("PlayerList"));
		if (!playerList)
			return;

		VerticalLayoutWidget leaderList = VerticalLayoutWidget.Cast(m_ParentSubMenu.GetRootWidget().FindAnyWidget("Leader"));
		if (!leaderList)
			return;

		RichTextWidget squadName = RichTextWidget.Cast(m_ParentSubMenu.GetRootWidget().FindAnyWidget("GroupDetailCallsign"));
		if (!squadName)
			return;

		RichTextWidget description = RichTextWidget.Cast(m_ParentSubMenu.GetRootWidget().FindAnyWidget("Description"));
		if (!description)
			return;

		RichTextWidget frequency = RichTextWidget.Cast(m_ParentSubMenu.GetRootWidget().FindAnyWidget("GroupDetailFrequency"));
		if (!frequency)
			return;

		SCR_PlayerControllerGroupComponent s_PlayerGroupController = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!s_PlayerGroupController)
			return;

		SocialComponent socialComp = SocialComponent.Cast(playerController.FindComponent(SocialComponent));
		if (!socialComp)
			return;		
		
		if (!s_PlayerGroupController.CanPlayerJoinGroup(playerController.GetPlayerId(), m_GroupManager.FindGroup(m_iGroupID)))
			m_JoinGroupButton.SetEnabled(false);
		else
			m_JoinGroupButton.SetEnabled(true);

		string groupName = group.GetCustomName();
		string company, platoon, squad, character, format;
		group.GetCallsigns(company, platoon, squad, character, format);
		
		Print("-- RefreshPlayers -- 1 -- groupName");
		Print(groupName);
		
		if (groupName.IsEmpty() || group.GetNameAuthorID() != 0 && socialComp.IsRestricted(group.GetNameAuthorID(), EUserInteraction.UserGeneratedContent))
		{
			Print("-- RefreshPlayers -- 1a -- groupName");
			if (group.GetCustomName())
			{
			    Print("-- RefreshPlayers -- 1a1 -- using customName");
			    Print(group.GetCustomName());
			    squadName.SetText(group.GetCustomName());
			}
			else squadName.SetTextFormat(format, company, platoon, squad, character);
		}
		else
		{
			
			Print("-- RefreshPlayers -- 1b --");
			// company = WidgetManager.Translate(company);
			// -----------------------------------------
			// groupName = group.GetCustomName() + " (" + string.Format(format, company, platoon, squad, character) + ")";
			// squadName.SetText(groupName);
			
			// Only display custom name, no (original name appendix)
			squadName.SetText(group.GetCustomName());
		}

		if (!group.GetCustomDescription().IsEmpty())
			description.SetText(group.GetCustomDescription());
		else
			description.SetText(string.Empty);

		CheckLeaderOptions();

		frequency.SetText(""+group.GetRadioFrequency()*0.001 + " #AR-VON_FrequencyUnits_MHz");

		Widget children = playerList.GetChildren();
		while (children)
		{
			playerList.RemoveChild(children);
			children = playerList.GetChildren();
		}

		children = leaderList.GetChildren();
		while (children)
		{
			leaderList.RemoveChild(children);
			children = leaderList.GetChildren();
		}

		array<int> playerIDs = group.GetPlayerIDs();
		Widget playerTile;

		int leaderID = group.GetLeaderID();
		if (leaderID >= 0)
		{
			playerTile = GetGame().GetWorkspace().CreateWidgets(m_textLayout, leaderList);
			SetupPlayerTile(playerTile, leaderID);
		}

		foreach (int playerID : playerIDs)
		{
			if (playerID == leaderID)
				continue;

			playerTile = GetGame().GetWorkspace().CreateWidgets(m_textLayout, playerList);
			SetupPlayerTile(playerTile, playerID);
		}

		SetupJoinGroupButton();

		GetGame().GetWorkspace().SetFocusedWidget(GetRootWidget());
		
		Print("-- RefreshPlayers -- groupName 2");
		Print(groupName);
	}
	
}
