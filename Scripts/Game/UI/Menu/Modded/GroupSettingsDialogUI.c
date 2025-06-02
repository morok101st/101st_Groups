//------------------------------------------------------------------------------------------------
modded class GroupSettingsDialogUI
{
	
	protected SCR_EditBoxComponent m_Description;
	protected SCR_ComboBoxComponent m_GroupStatus;
	protected SCR_EditBoxComponent m_GroupName;
	protected SCR_EditBoxComponent m_GroupDescription;
	
	protected static ref SCR_ScriptPlatformRequestCallback m_CallbackGetPrivilege;
		
	protected SCR_PlayerControllerGroupComponent m_PlayerComponent;
	protected SCR_GroupsManagerComponent m_GroupsManager;
	protected SCR_AIGroup m_PlayerGroup;
	
	protected bool m_bHasPrivilege;

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		
		m_PlayerComponent = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!m_PlayerComponent)
			return;
		
		if (m_PlayerComponent.GetSelectedGroupID() == -1)
		{
			Close();
			return;
		}
		if (!m_CallbackGetPrivilege)
			m_CallbackGetPrivilege = new SCR_ScriptPlatformRequestCallback();
		
		m_CallbackGetPrivilege.m_OnResult.Insert(OnPrivilegeCallback);
		
		GetGame().GetPlatformService().GetPrivilegeAsync(UserPrivilege.USER_GEN_CONTENT, m_CallbackGetPrivilege);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		if (m_CallbackGetPrivilege)
			m_CallbackGetPrivilege.m_OnResult.Remove(OnPrivilegeCallback);
	}
	
	// Groups ------------------------------------------------------------------------------------------------
	override void OnPrivilegeCallback(UserPrivilege privilege, UserPrivilegeResult result)
	{
		
		m_GroupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!m_GroupsManager)
			return;
		
		m_PlayerGroup = m_GroupsManager.FindGroup(m_PlayerComponent.GetGroupID());
		if (!m_PlayerGroup)
			return;
		
		Widget w = GetRootWidget();
		if (!w)
			return;
		
		m_GroupStatus = SCR_ComboBoxComponent.GetComboBoxComponent("Type", w);
		if (!m_GroupStatus)
			return;
		
		m_GroupName = SCR_EditBoxComponent.GetEditBoxComponent("Name", w);
		if (!m_GroupName)
			return;
		
		m_GroupDescription = SCR_EditBoxComponent.GetEditBoxComponent("Description", w);
		if (!m_GroupDescription)
			return;
		
		Print("-- OnPrivilegeCallback -- ");
		if (result == UserPrivilegeResult.ALLOWED)
		{
			Print("-- 1a -- allowed");
			m_GroupName.SetEnabled(true);
			m_GroupName.SetValue(m_PlayerGroup.GetCustomName());
			
			m_GroupDescription.SetEnabled(true);
			m_GroupDescription.SetValue(m_PlayerGroup.GetCustomDescription());
			m_bHasPrivilege = true;
		}
		else
		{
			Print("-- 1b -- denied");
			m_GroupName.SetEnabled(false);
			m_GroupName.SetValue(WidgetManager.Translate("#AR-UserActionUnavailable"));
			m_GroupDescription.SetEnabled(false);
			m_GroupDescription.SetValue(WidgetManager.Translate("#AR-UserActionUnavailable"));
			m_bHasPrivilege = false;
		}
		
		SetupGroupStatusCombo();
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnConfirm()
	{
		
		Print("-- OnConfirm -- ");
		SCR_PlayerControllerGroupComponent groupController = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!groupController)
			return;
		
		if (m_bHasPrivilege)
		{
			Print("-- hasPrivilege -- ");
			int groupID = m_PlayerGroup.GetGroupID();
			groupController.RequestSetCustomGroupDescription(groupID, m_GroupDescription.GetValue());
			groupController.RequestSetCustomGroupName(groupID, m_GroupName.GetValue());
		}
		
		Close();
	}
};
