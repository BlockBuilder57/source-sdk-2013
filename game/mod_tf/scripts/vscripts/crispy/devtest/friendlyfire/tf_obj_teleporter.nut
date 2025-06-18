::ff.tf_obj_teleporter <- function()
{
	QuotaFakeClients(1);

	local pos_tele_entrance = Vector(256, -384, 0)
	local pos_tele_exit = Vector(256, 384, 0)
	local pos_engineer = Vector(256, 384, 32)
	local rot_eyeangles = QAngle(0, -90, 0)

	const ENTRANCE_NAME = "ff_tele_entrance"
	const EXIT_NAME = "ff_tele_exit"

	QuotaBots[0].SetAbsOrigin(pos_engineer)
	QuotaBots[0].SnapEyeAngles(rot_eyeangles)
	QuotaBots[0].ValidateScriptScope()

	QuotaBots[0].ForceSetTeam(Constants.ETFTeam.TF_TEAM_RED)
	QuotaBots[0].ForceSetClass(Constants.ETFClass.TF_CLASS_ENGINEER)
	QuotaBots[0].Regenerate(true)

	local makeTele = function(name, pos, type)
	{
		local toRet = null;
		if (Entities.FindByName(null, name) == null)
		{
			toRet = SpawnEntityFromTable("obj_teleporter",
			{
				targetname = name,
				origin = pos,
				teamnum = Constants.ETFTeam.TF_TEAM_RED,
				defaultupgrade = 2,
				teleporterType = type
			});
		}
		else
		{
			toRet = Entities.FindByName(null, name);
		}

		return toRet;
	}

	local tele_entrance = makeTele(ENTRANCE_NAME, pos_tele_entrance, 1);
	tele_entrance.SetBuilder(QuotaBots[0]);

	local tele_exit = makeTele(EXIT_NAME, pos_tele_exit, 2);
	tele_exit.SetBuilder(QuotaBots[0]);

	local tele_exit2 = makeTele(EXIT_NAME + "_2", pos_tele_exit + Vector(768, 0, 0), 2);
	tele_exit2.SetBuilder(QuotaBots[0]);

	::ff.tele_entrance <- tele_entrance
	::ff.tele_exit <- tele_exit
	::ff.tele_exit2 <- tele_exit2

	//ff.tele_entrance.SetMatchingTeleporter(ff.tele_exit)
	//ff.tele_exit.SetMatchingTeleporter(ff.tele_entrance)

	// ff.tele_entrance.SetMatchingTeleporter(ff.tele_exit2)

	// ff.tele_entrance.SetBuilder(Entities.FindPlayerByArgs("#1"))

	//printl("Remember tf_teleporter_telefrag_teammates");
}