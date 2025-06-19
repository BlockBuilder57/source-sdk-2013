// script ff.tf_obj_sentrygun(2)

::ff.tf_obj_sentrygun <- function(level)
{
	QuotaFakeClients(1);

	local pos_sentry = Vector(512, 0, 0)
	local pos_engineer = Vector(450, 0, 32)
	local rot_eyeangles = QAngle(0, 0, 0)

	const SENTRY_NAME = "ff_sentry"

	QuotaBots[0].SetAbsOrigin(pos_engineer)
	QuotaBots[0].SnapEyeAngles(rot_eyeangles)
	QuotaBots[0].ValidateScriptScope()

	QuotaBots[0].ForceSetTeam(Constants.ETFTeam.TF_TEAM_RED)
	QuotaBots[0].ForceSetClass(Constants.ETFClass.TF_CLASS_ENGINEER)
	QuotaBots[0].Regenerate(true)

	local sentry = Entities.FindByName(null, SENTRY_NAME);
	if (Entities.FindByName(null, SENTRY_NAME) != null)
	{
		sentry.Kill();
	}

	sentry = SpawnEntityFromTable("obj_sentrygun",
	{
		targetname = SENTRY_NAME,
		origin = pos_sentry,
		teamnum = Constants.ETFTeam.TF_TEAM_RED,
		defaultupgrade = level
	});

	sentry.SetBuilder(QuotaBots[0]);

	::ff.sentry <- sentry
}

// script ff.sentry.SetBuilder(PlayerInstanceFromIndex(1))