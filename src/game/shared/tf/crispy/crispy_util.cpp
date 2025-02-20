#include "cbase.h"
#include <stdarg.h>

// Helper for concommand arguments.
// Returns TEAM_INVALID if no team was found/specified.
// Currently accepts, in descending priority for each team:
//  - red, blu[e], spec[tator[s]], rand[om]
//  - value then default of mp_tournament_*teamname
//  - team index
int UTIL_TeamByArg(const char* name, int defaultTeam /*= TEAM_INVALID*/)
{
	int iTeamNum = defaultTeam;

	ConVarRef mp_tournament_redteamname("mp_tournament_redteamname");
	ConVarRef mp_tournament_blueteamname("mp_tournament_blueteamname");

	if (V_strnicmp(name, "red", 3) == 0 || (mp_tournament_redteamname.IsValid() && (V_stricmp(name, mp_tournament_redteamname.GetString()) == 0 || V_stricmp(name, mp_tournament_redteamname.GetDefault()) == 0)) || V_atoi(name) == TF_TEAM_RED)
	{
		iTeamNum = TF_TEAM_RED;
	}
	else if (V_strnicmp(name, "blu", 3) == 0 || (mp_tournament_blueteamname.IsValid() && (V_stricmp(name, mp_tournament_blueteamname.GetString()) == 0 || V_stricmp(name, mp_tournament_blueteamname.GetDefault()) == 0)) || V_atoi(name) == TF_TEAM_BLUE)
	{
		iTeamNum = TF_TEAM_BLUE;
	}
	// is this one needed...?
	else if (V_strnicmp(name, "spec", 4) == 0 || V_atoi(name) == TEAM_SPECTATOR)
	{
		iTeamNum = TEAM_SPECTATOR;
	}
	else if (V_strnicmp(name, "rand", 4) == 0)
	{
		iTeamNum = random->RandomInt(TF_TEAM_RED, TF_TEAM_BLUE);
	}

	return iTeamNum;
}

void ParseMCCoordNotation(const char* arg, float traceEndposComponent, float& vecComponent, float& relativeLength)
{
	if (arg)
	{
		switch (arg[0])
		{
		case '$': // (~ in MC) Position relative (to vector, usually player position)
			vecComponent += V_atof(arg + 1);
			break;
		case '^': // View relative
			relativeLength = V_atof(arg + 1);
			break;
		case '+': // Trace relative
			vecComponent = traceEndposComponent;
			if (arg + 1 != nullptr)
				vecComponent += V_atof(arg + 1);
			break;
		default:
			vecComponent = V_atof(arg);
			break;
		}

		// Handles random values by reading the number after a *
		const char* randValue = V_strnchr(arg, '*', V_strlen(arg));
		if (randValue)
		{
			float flRandValue = abs(V_atof(randValue + 1));
			vecComponent += RandomFloat(-flRandValue, flRandValue);
		}
	}
}

void UTIL_MCCoordNotation(CBasePlayer* pPlayer, const char* argx, const char* argy, const char* argz, Vector* vec)
{
	if (!vec)
	{
		Warning("UTIL_MCCoordNotation was passed a null vector\n");
		return;
	}

	float forwardMult = 0.f;
	float rightMult = 0.f;
	float upMult = 0.f;

	Vector forward = vec3_origin;
	Vector right = vec3_origin;
	Vector up = vec3_origin;
	if (pPlayer)
		pPlayer->EyeVectors( &forward, &right, &up );

	trace_t tr;
	if (pPlayer)
		UTIL_TraceLine(pPlayer->EyePosition(), pPlayer->EyePosition() + forward * MAX_TRACE_LENGTH, MASK_SOLID, pPlayer, COLLISION_GROUP_NONE, &tr);

	// ^ needs to be applied to the final vector
	// all the number does is determine the strength of the application (or lack thereof) of the vector

	//Msg("MCCoord: vec before parsing (%.2f %.2f %.2f)\n", vec->x, vec->y, vec->z);
	//Msg("MCCoord: taking input [%s %s %s]\n", argx, argy, argz);
	//if (pPlayer)
	//	Msg("MCCoord: trace ended at (%.2f %.2f %.2f)\n", tr.endpos.x, tr.endpos.y, tr.endpos.z);

	ParseMCCoordNotation(argx, tr.endpos.x, vec->x, forwardMult);
	ParseMCCoordNotation(argy, tr.endpos.y, vec->y, rightMult);
	ParseMCCoordNotation(argz, tr.endpos.z, vec->z, upMult);

	//Msg("MCCoord: vec after parsing (%.2f %.2f %.2f)\n", vec->x, vec->y, vec->z);

	//if constexpr(std::is_same_v<T, Vector>)
	{
		// needs to be done here, as these affect multiple components of the vector
		*vec = *vec + (forward * forwardMult);
		*vec = *vec + (right * rightMult);
		*vec = *vec + (up * upMult);
	}

	//Msg("MCCoord: vec after view relative? (%.2f %.2f %.2f)\n", vec->x, vec->y, vec->z);
}
