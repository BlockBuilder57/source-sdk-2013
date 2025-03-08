::Weaponry <- {}

const WEAPONRY_MIMIC_NAME = "weaponry_mimic"
if (Entities.FindByName(null, WEAPONRY_MIMIC_NAME) == null)
{
	::Weaponry.Mimic <- SpawnEntityFromTable("tf_point_weapon_mimic",
	{
		targetname = WEAPONRY_MIMIC_NAME
	})
}
else
{
	::Weaponry.Mimic <- Entities.FindByName(null, WEAPONRY_MIMIC_NAME)
}

::Weaponry.PointMimicAtTarget <- function(src, target)
{
	local dir = target - src
	dir.Norm()
	local rot = VectorAngles(dir)

	Weaponry.Mimic.SetAbsOrigin(src)
	Weaponry.Mimic.SetAbsAngles(rot)
}

::Weaponry.FireRocket <- function(src, target, team = 0)
{
	PointMimicAtTarget(src, target)

	Weaponry.Mimic.SetTeam(team)
	Weaponry.Mimic.FireRocket()
}

printl("Loaded weaponry")