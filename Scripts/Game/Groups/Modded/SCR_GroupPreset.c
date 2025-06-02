[BaseContainerProps()]
modded class SCR_GroupPreset
{
	[Attribute(desc: "Group will stay locked even when last player leaves.")]
	protected bool m_bKeepLocked;
	
	[Attribute(desc: "Group will stay public. Wins over keepLocked")]
	protected bool m_bKeepPublic;
	
	[Attribute(desc: "Group will stay as created. No changes are allowed to description, flag and title and max group size.")]
	protected bool m_bKeepAsCreated;

	bool GetKeepPublic()
	{
		return m_bKeepPublic;
	}
	
	bool GetKeepLocked()
	{
		return m_bKeepLocked;
	}
	
	bool GetKeepAsCreated()
	{
			return m_bKeepAsCreated;
	}
	
	//------------------------------------------------------------------------------------------------	
	override void SetupGroup(SCR_AIGroup group)
	{
		group.SetCustomName(m_sGroupName, 0);
		group.SetRadioFrequency(m_iRadioFrequency);
		group.SetMaxGroupMembers(m_iGroupSize);
		group.SetPrivate(m_bIsPrivate);
		group.SetCustomDescription(m_sGroupDescription, 0);
		
		if (!m_sGroupFlag.IsEmpty())
			group.SetCustomGroupFlag(m_sGroupFlag);
		
		// Apply new flags from preset to new group
		group.SetKeepLocked(m_bKeepLocked);
		group.SetKeepPublic(m_bKeepPublic);
		group.SetKeepAsCreated(m_bKeepAsCreated);
		// -------------------------------------------
	}
	
}