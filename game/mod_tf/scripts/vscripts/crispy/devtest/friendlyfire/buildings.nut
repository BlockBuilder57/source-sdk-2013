::ff.MakeOrReplaceSentry <- function(name, pos, team, upgrade)
{
	local sentry = Entities.FindByName(null, name);
	if (Entities.FindByName(null, name) != null)
	{
		sentry.Kill();
	}

	sentry = SpawnEntityFromTable("obj_sentrygun",
	{
		targetname = name,
		origin = pos,
		teamnum = team,
		defaultupgrade = upgrade
	});

	return sentry;
}

::ff.MakeOrReplaceTeleporter <- function(name, pos, team, upgrade, type)
{
	local tele = Entities.FindByName(null, name);
	if (Entities.FindByName(null, name) != null)
	{
		tele.Kill();
	}

	tele = SpawnEntityFromTable("obj_teleporter",
	{
		targetname = name,
		origin = pos,
		teamnum = team,
		defaultupgrade = upgrade,
		teleporterType = type
	});

	return tele;
}

::ff.MakeOrReplaceDispenser <- function(name, pos, team, upgrade)
{
	local dispenser = Entities.FindByName(null, name);
	if (Entities.FindByName(null, name) != null)
	{
		dispenser.Kill();
	}

	dispenser = SpawnEntityFromTable("obj_dispenser",
	{
		targetname = name,
		origin = pos,
		teamnum = team,
		defaultupgrade = upgrade
	});

	return dispenser;
}

// script ff.tf_obj_sentrygun(2)
// script ff.sentry.SetBuilder(PlayerInstanceFromIndex(1))

::ff.tf_obj_sentrygun <- function(level)
{
	QuotaFakeClients(1);

	local pos_sentry = Vector(512, 0, 0)
	local pos_engineer = Vector(450, 0, 32)
	local rot_eyeangles = QAngle(0, 0, 0)

	const SENTRY_NAME = "ff_sentry"

	QuotaBots[0].SetupQuotaBot(pos_engineer, rot_eyeangles, Constants.ETFTeam.TF_TEAM_RED, Constants.ETFClass.TF_CLASS_ENGINEER)

	local sentry = ff.MakeOrReplaceSentry(SENTRY_NAME, pos_sentry, Constants.ETFTeam.TF_TEAM_RED, 2)
	sentry.SetBuilder(QuotaBots[0]);

	::ff.sentry <- sentry
}

::ff.tf_obj_teleporter <- function()
{
	QuotaFakeClients(1);

	local pos_tele_entrance = Vector(256, -384, 0)
	#local pos_tele_exit = Vector(256, 384, 0)
	local pos_tele_exit = Vector(256, -256, 0)
	local pos_tele_exit2 = Vector(1024, 384, 0)
	local pos_engineer = Vector(256, 384, 32)
	local rot_eyeangles = QAngle(0, -90, 0)

	const ENTRANCE_NAME = "ff_tele_entrance"
	const EXIT_NAME = "ff_tele_exit"
	const EXIT2_NAME = "ff_tele_exit2"

	QuotaBots[0].SetupQuotaBot(pos_engineer, rot_eyeangles, Constants.ETFTeam.TF_TEAM_RED, Constants.ETFClass.TF_CLASS_ENGINEER)

	local tele_entrance = ff.MakeOrReplaceTeleporter(ENTRANCE_NAME, pos_tele_entrance, Constants.ETFTeam.TF_TEAM_RED, 0, 1);
	tele_entrance.SetBuilder(QuotaBots[0]);

	local tele_exit = ff.MakeOrReplaceTeleporter(EXIT_NAME, pos_tele_exit, Constants.ETFTeam.TF_TEAM_RED, 0, 2);
	tele_exit.SetBuilder(QuotaBots[0]);

	local tele_exit2 = ff.MakeOrReplaceTeleporter(EXIT2_NAME, pos_tele_exit2, Constants.ETFTeam.TF_TEAM_RED, 0, 2);
	tele_exit2.SetBuilder(QuotaBots[0]);

	::ff.tele_entrance <- tele_entrance
	::ff.tele_exit <- tele_exit
	::ff.tele_exit2 <- tele_exit2

	// ff.tele_entrance.SetMatchingTeleporter(ff.tele_exit)
	// ff.tele_exit.SetMatchingTeleporter(ff.tele_entrance)

	// ff.tele_entrance.SetMatchingTeleporter(ff.tele_exit2)

	// ff.tele_entrance.SetBuilder(Entities.FindPlayerByArgs("#1"))

	//printl("Remember tf_teleporter_telefrag_teammates");
}

::ff.tf_obj_dispenser <- function()
{
	QuotaFakeClients(1);

	local pos_dispenser = Vector(256, 0, 0)
	local pos_engineer = Vector(256+64, 0, 32)
	local rot_eyeangles = QAngle(0, 180, 0)

	const DISPENSER_NAME = "ff_dispenser"

	QuotaBots[0].SetupQuotaBot(pos_engineer, rot_eyeangles, Constants.ETFTeam.TF_TEAM_RED, Constants.ETFClass.TF_CLASS_ENGINEER)

	local dispenser = ff.MakeOrReplaceDispenser(DISPENSER_NAME, pos_dispenser, Constants.ETFTeam.TF_TEAM_RED, 2);
	dispenser.SetBuilder(QuotaBots[0]);

	::ff.dispenser <- dispenser
}

::ff.engienest <- function()
{
	QuotaFakeClients(1);

	local pos_sentry = Vector(0, 2048, 16)
	local pos_dispenser = Vector(512, 2048+128, 16)
	local pos_tele_entr = Vector(0, 0, 0)
	local pos_tele_exit = Vector(512, 2048+256+64, 16)
	local pos_engineer = Vector(512, 2048+128+64, 32)
	local rot_eyeangles = QAngle(0, 180, 0)

	const SENTRY_NAME = "ff_sentry"
	const DISPENSER_NAME = "ff_dispenser"
	const ENTR_NAME = "ff_tele_entrance"
	const EXIT_NAME = "ff_tele_exit"

	QuotaBots[0].SetupQuotaBot(pos_engineer, rot_eyeangles, Constants.ETFTeam.TF_TEAM_RED, Constants.ETFClass.TF_CLASS_ENGINEER)

	local sentry = ff.MakeOrReplaceSentry(SENTRY_NAME, pos_sentry, Constants.ETFTeam.TF_TEAM_RED, 2)
	sentry.SetBuilder(QuotaBots[0]);

	local dispenser = ff.MakeOrReplaceDispenser(DISPENSER_NAME, pos_dispenser, Constants.ETFTeam.TF_TEAM_RED, 2);
	dispenser.SetBuilder(QuotaBots[0]);

	local tele_entr = ff.MakeOrReplaceTeleporter(ENTR_NAME, pos_tele_entr, Constants.ETFTeam.TF_TEAM_RED, 2, 1);
	tele_entr.SetBuilder(QuotaBots[0]);

	local tele_exit = ff.MakeOrReplaceTeleporter(EXIT_NAME, pos_tele_exit, Constants.ETFTeam.TF_TEAM_RED, 2, 2);
	tele_exit.SetBuilder(QuotaBots[0]);

	::ff.sentry <- sentry
	::ff.dispenser <- dispenser
	::ff.tele_entr <- tele_entr
	::ff.tele_exit <- tele_exit
}