::TF_CLASSES <- ["undef", "scout", "sniper", "soldier", "demoman", "medic", "heavyweapons", "pyro", "spy", "engineer", "civilian"]
::CTFPlayer.ForceSetClass <- function(classname) {
	/*local vecPos = this.GetOrigin();
	local vecAng = this.EyeAngles();
	local vecVel = this.GetAbsVelocity();

	this.SetPlayerClass(classIndex);
	NetProps.SetPropInt(this, "m_Shared.m_iDesiredPlayerClass", classIndex);
	this.ForceRegenerateAndRespawn();

	this.Teleport(true, vecPos, true, vecAng, true, vecVel);*/

	if (typeof(classname) != "string") classname = TF_CLASSES[classname];
	local userid = NetProps.GetPropIntArray(Entities.FindByClassname(null, "tf_player_manager"), "m_iUserID", this.entindex())
	SendToServerConsole("cf_setclass " + classname + " #" + userid);
}

/*::CTFBot.ForceSetTeam <- function(team)
{
	local vecPos = this.GetOrigin();
	local vecAng = this.EyeAngles();
	local vecVel = this.GetAbsVelocity();

	this.ForceChangeTeam(team, true);
	NetProps.SetPropInt(this, "m_iTeamNum", team);

	local cosmetic = null
	while (cosmetic = Entities.FindByClassname(cosmetic, "tf_wearable"))
	{
		if (cosmetic.GetOwner() == this)
		{
			cosmetic.SetTeam(team)
		}
	}

	this.Teleport(true, vecPos, true, vecAng, true, vecVel);
}*/

::CTFPlayer.ForceSetTeam <- function(team)
{
	/*local vecPos = this.GetOrigin();
	local vecAng = this.EyeAngles();
	local vecVel = this.GetAbsVelocity();

	NetProps.SetPropInt(this, "m_bIsCoaching", 1);
	this.ForceChangeTeam(team, true);
	NetProps.SetPropInt(this, "m_bIsCoaching", 0);

	local cosmetic = null
	while (cosmetic = Entities.FindByClassname(cosmetic, "tf_wearable"))
	{
		if (cosmetic.GetOwner() == this)
		{
			cosmetic.SetTeam(team)
		}
	}

	this.Teleport(true, vecPos, true, vecAng, true, vecVel);*/

	local userid = NetProps.GetPropIntArray(Entities.FindByClassname(null, "tf_player_manager"), "m_iUserID", this.entindex())
	SendToServerConsole("cf_setteam " + team + " #" + userid);
}

#local player = GetListenServerHost();
#player.ForceSetClass(Constants.ETFClass.TF_CLASS_PYRO)
#player.ForceSetTeam(Constants.ETFTeam.TF_TEAM_RED)

printl("Loaded player extensions")