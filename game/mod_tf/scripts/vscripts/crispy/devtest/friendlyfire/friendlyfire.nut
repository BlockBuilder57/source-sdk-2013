::ff <- {}

::ff.DelayedFireThink <- function()
{
	// wait to begin firing
	if (Time() < ThinkParams.fireAt) return -1;

	local wep = NetProps.GetPropEntity(ThinkParams.player, "m_hActiveWeapon");
	if (wep)
	{
		//NetProps.SetPropFloat(wep, ThinkParams.fireMode ? "m_flNextPrimaryAttack" : "m_flNextSecondaryAttack", 0);
		//wep.SetClip1(500);
		//wep.SetClip2(500);
		ThinkParams.fireMode ? wep.PrimaryAttack() : wep.SecondaryAttack();
	}

	if (Time() < ThinkParams.fireAt + ThinkParams.fireLength) return -1;
	else NetProps.SetPropString(ThinkParams.player, "m_iszScriptThinkFunction", "");
}

::ff.test1 <- function()
{
	foreach(i, bot in QuotaFakeClients(9))
	{
		if (bot == null) continue;
		#printl(bot)
		bot.ForceSetClass(i + 1);
		bot.ForceSetTeam("blue");
		bot.SetAbsOrigin(Vector(0, 0, 16) + (Vector(64, 0, 0) * i));
	}
}

::ff.test2 <- function()
{
	QuotaFakeClients(1);

	local src = Vector(600, 400, 400)
	local target = Vector(1269, -275, 16)
	local eyeangles = QAngle(-3.75, 120, 0)

	QuotaBots[0].SetupQuotaBot(target, eyeangles, -1, Constants.ETFClass.TF_CLASS_PYRO)

	Weaponry.FireRocket(src, target + Vector(0, 0, 64), 0)

	// rockets move at 1000ups
	local time = (src - target + Vector(0, 0, 64) + (QuotaBots[0].EyeAngles().Forward() * -200)).Length() / 1000

	local params = {
		player = QuotaBots[0],
		fireAt = Time() + time,
		fireLength = 0.0,
		fireMode = false
	};

	QuotaBots[0].GetScriptScope()["ThinkParams"] <- params
	QuotaBots[0].GetScriptScope()["Think"] <- ff.DelayedFireThink
	AddThinkToEnt(QuotaBots[0], "Think")
}

::ff.test3 <- function()
{
	QuotaFakeClients(1);

	local src = Vector(1024, 1024, 64)
	local target = Vector(1024, -384, 0)
	local eyeangles = QAngle(0, 90, 0)

	QuotaBots[0].SetupQuotaBot(target, eyeangles, Constants.ETFTeam.TF_TEAM_RED, Constants.ETFClass.TF_CLASS_HEAVYWEAPONS)

	Weaponry.FireHitscan(src, target + Vector(0, 0, 64), Constants.ETFTeam.TF_TEAM_RED)
}

IncludeScript("crispy/devtest/friendlyfire/buildings.nut")

SendToServerConsole("mp_friendlyfire 1");
printl("Loaded friendly fire testing script");