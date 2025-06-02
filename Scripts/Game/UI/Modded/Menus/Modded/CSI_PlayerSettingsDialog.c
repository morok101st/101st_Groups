modded class CSI_PlayerSettingsDialog
{
	override void OnMenuOpen()
	{
		//super.OnMenuOpen();

		m_wRoot = GetRootWidget();

		GetGame().GetInputManager().AddActionListener("MenuBack", EActionTrigger.DOWN, OnMenuBack);
		SCR_InputButtonComponent cancel = SCR_InputButtonComponent.Cast(m_wRoot.FindAnyWidget("Cancel").FindHandler(SCR_InputButtonComponent));
		cancel.m_OnClicked.Insert(OnMenuBack);

		m_ClientComponent = CSI_ClientComponent.GetInstance();

		// Get Global Player Controller and Group Manager.
		m_AuthorityComponent = CSI_AuthorityComponent.GetInstance();
		if (!m_AuthorityComponent || !m_ClientComponent) 
			return;

		m_wIconOveride = XComboBoxWidget.Cast(m_wRoot.FindAnyWidget("IconOveride"));
		m_wPlayerName = TextWidget.Cast(m_wRoot.FindAnyWidget("PlayerName"));
		m_wIcon = ImageWidget.Cast(m_wRoot.FindAnyWidget("Icon"));

		GetGame().GetCallqueue().CallLater(UpdatePlayerIcon, 215, true);
		GetGame().GetCallqueue().CallLater(UpdateIconOverride, 145);

		SCR_ModularButtonComponent confirmIOButton = SCR_ModularButtonComponent.Cast(m_wRoot.FindAnyWidget("ConfirmIOButton").FindHandler(SCR_ModularButtonComponent));
		SCR_ModularButtonComponent red = SCR_ModularButtonComponent.Cast(m_wRoot.FindAnyWidget("Red").FindHandler(SCR_ModularButtonComponent));
		SCR_ModularButtonComponent blue = SCR_ModularButtonComponent.Cast(m_wRoot.FindAnyWidget("Blue").FindHandler(SCR_ModularButtonComponent));
		SCR_ModularButtonComponent yellow = SCR_ModularButtonComponent.Cast(m_wRoot.FindAnyWidget("Yellow").FindHandler(SCR_ModularButtonComponent));
		SCR_ModularButtonComponent green = SCR_ModularButtonComponent.Cast(m_wRoot.FindAnyWidget("Green").FindHandler(SCR_ModularButtonComponent));
		SCR_ModularButtonComponent none = SCR_ModularButtonComponent.Cast(m_wRoot.FindAnyWidget("None").FindHandler(SCR_ModularButtonComponent));

		confirmIOButton.m_OnClicked.Insert(OnOverrideIconClicked);
		red.m_OnClicked.Insert(OnColorTeamClicked);
		blue.m_OnClicked.Insert(OnColorTeamClicked);
		yellow.m_OnClicked.Insert(OnColorTeamClicked);
		green.m_OnClicked.Insert(OnColorTeamClicked);
		none.m_OnClicked.Insert(OnColorTeamClicked);

		m_GroupsManagerComponent = SCR_GroupsManagerComponent.GetInstance();
		SCR_AIGroup openingPlayersGroup = m_GroupsManagerComponent.GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId());
		Print("-- CSI -- check for admin");
		// Show Advanced menu when logged in as admin or squad leader
		bool isAdmin = SCR_Global.IsAdmin(SCR_PlayerController.GetLocalPlayerId());
		if (openingPlayersGroup.IsPlayerLeader(SCR_PlayerController.GetLocalPlayerId()) || isAdmin) ShowAdvSettings();
	}
}