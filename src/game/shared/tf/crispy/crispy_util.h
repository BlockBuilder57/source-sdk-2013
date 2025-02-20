#ifndef CRISPY_UTIL_H
#define CRISPY_UTIL_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include <string.h>
#include <type_traits>

#include "utlvector.h"
#include "util_shared.h"
#include "shareddefs.h"

#define GET_LOCAL_TFPLAYER(varname, strict) \
	CTFPlayer* ##varname = ToTFPlayer(UTIL_GetCommandClient()); \
	if(!##varname && strict) \
	{ \
		Warning(#varname " is null, couldn't find command client\n"); \
		return; \
	}
#define CHECK_ARGC(actual, op, desired) \
	if (!(actual op desired)) \
	{ \
		Warning("Argument count is not " #op #desired ", (it's %d)\n", actual); \
		return; \
	}
#define PLAYER_BY_COMMANDARG(varname, arg) \
	CBasePlayer* ##varname = UTIL_PlayerByCommandArg(arg); \
	if (!##varname) \
	{ \
		Warning("No target found for %s\n", arg); \
		return; \
	}
#define TFPLAYERS_BY_COMMANDARG(varname, arg) \
	UTIL_PlayersByCommandArg(##varname, arg); \
	if (##varname.IsEmpty()) \
	{ \
		Warning("No targets found for %s\n", arg); \
		return; \
	}

// Helper for concommand arguments.
// Returns TEAM_INVALID if no team was found/specified.
// Currently accepts, in descending priority for each team:
//  - red, blu[e], spec[tator[s]]
//  - value then default of mp_tournament_*teamname
//  - team index
int UTIL_TeamByArg(const char* name, int defaultTeam = TEAM_INVALID);

// Helper for concommand arguments.
// Returns an empty vector if no players were found
// Currently accepts, in descending priority:
//  - !me, !self, or !player for current player
//  - !picker for players under crosshair
//  - !all for all players
//  - !bots for all bots
//  - !humans for all humans
//  - ![team] through UTIL_TeamByArg
//  - Any of the above selectors with #[class] appened, ie !all#scout
//  - Multiple partial name matches
//  - Anything UTIL_PlayerByCommandArg accepts, but the match will be singular
template<typename T, typename = std::enable_if<std::is_base_of_v<CBasePlayer, T>, T>>
int UTIL_PlayersByCommandArg(CUtlVector<T> &players, const char* arg, bool shouldAppend = false)
{
	if ( !shouldAppend )
	{
		players.RemoveAll();
	}

	size_t nLength = V_strlen( arg );
	if ( nLength < 1 )
		return 0;

	bool bAllButFirstNumbers = true;
	for (size_t idx = 1; bAllButFirstNumbers && idx < nLength; idx++)
	{
		bAllButFirstNumbers = V_isdigit(arg[idx]);
	}

	int bFoundCount = 0;

	if (!bFoundCount && arg[0] == '!')
	{
		CUtlVector<char*, CUtlMemory<char*, int> > outStrings;

		// split the argument by #
		// !me will become ["me"]
		// !all#scout will become ["all", "scout"]
		V_SplitString(&arg[1], "#", outStrings);

		const char* specifier = outStrings[0];
		const char* className = outStrings[0];

		if (outStrings.Count() > 1)
			className = outStrings[1];

		//DevMsg("arg[1] == \"%s\"\n    specifier is \"%s\", className is \"%s\"\n", &arg[1], specifier, className);
		
		if (!bFoundCount && (V_stricmp(specifier, "me") == 0 || V_stricmp(specifier, "self") == 0 || V_stricmp(specifier, "player") == 0))
		{
			T pPlayer = reinterpret_cast<T>(UTIL_GetCommandClient());
			if (pPlayer && players.Find(pPlayer) == -1)
				players.AddToTail(pPlayer);

			bFoundCount = players.Count();
		}
		else if (!bFoundCount && V_stricmp(specifier, "picker") == 0)
		{
			extern CBaseEntity *FindPickerEntity( CBasePlayer *pPlayer );

			CBaseEntity* pPicked = FindPickerEntity(UTIL_GetCommandClient());

			if (pPicked && pPicked->IsPlayer())
			{
				T pPlayer = reinterpret_cast<T>(pPicked);
				if (pPlayer && players.Find(pPlayer) == -1)
					players.AddToTail(pPlayer);
			}

			bFoundCount = players.Count();
		}
		else if (!bFoundCount && V_stricmp(specifier, "all") == 0)
		{
			bFoundCount = CollectPlayers(&players);
		}
		else if (!bFoundCount && V_stricmp(specifier, "bots") == 0)
		{
			CUtlVector<T> playerVector;
			CollectPlayers(&playerVector);

			for (int i = 0; i < playerVector.Count(); ++i)
				if (playerVector[i]->IsFakeClient())
					players.AddToTail(playerVector[i]);

			bFoundCount = players.Count();
		}
		else if (!bFoundCount && (V_stricmp(specifier, "humans") == 0 || V_stricmp(specifier, "players") == 0))
		{
			bFoundCount = CollectHumanPlayers(&players);
		}
		else if (!bFoundCount && !bAllButFirstNumbers)
		{
			int iTeamNum = TEAM_INVALID;

			iTeamNum = UTIL_TeamByArg(specifier);

			if (iTeamNum > 0)
				bFoundCount = CollectPlayers(&players, iTeamNum);
		}

		if (bFoundCount > 0 && V_stricmp(specifier, className) != 0)
		{
			int classIndex = GetClassIndexFromString(className);

			for (int i = players.Count()-1; i >= 0; i--)
			{
				CTFPlayer* pTarget = static_cast<CTFPlayer*>(players[i]);
				if (pTarget && classIndex != pTarget->GetPlayerClass()->GetClassIndex())
					players.FastRemove(i);
			}
		}
	}

	// search for partial names
	if (!bFoundCount)
	{
		UTIL_PlayersByPartialName(players, arg);
		bFoundCount = players.Count();
	}

	// if we haven't found anything, attempt a fallback to the singular command arg
	if (!bFoundCount || !players.Count())
	{
		T pPlayer = (T)UTIL_PlayerByCommandArg(arg);
		if (pPlayer && players.Find(pPlayer) == -1)
			players.AddToTail(pPlayer);
	}

	return bFoundCount;
}

// Finds players with this substring
template<typename T, typename = std::enable_if<std::is_base_of_v<CBasePlayer, T>, T>>
int UTIL_PlayersByPartialName(CUtlVector<T> &players, const char* name, bool shouldAppend = false)
{
	if ( !shouldAppend )
	{
		players.RemoveAll();
	}

	if ( !name || !name[0] )
		return 0;

	int bFoundCount = 0;
	for (int i = 1; i<=gpGlobals->maxClients; i++ )
	{
		T pPlayer = (T)UTIL_PlayerByIndex( i );
		if ( !pPlayer )
			continue;

#ifndef CLIENT_DLL
		if ( !pPlayer->IsConnected() )
			continue;
#endif

		if ( Q_stristr( pPlayer->GetPlayerName(), name ) )
		{
			if (players.Find(pPlayer) == -1)
			{
				players.AddToTail(pPlayer);
				bFoundCount++;
			}
		}
	}

	return bFoundCount;
}

// Minecraft-style coordinate helpers.
// Currently supports:
// - Normal coordinates (0, 256, -512)
//     Simply sets vec to the given values.
// - Pos-relative coordinates ($0 $32 $-64) (equivalent to ~)
//     Relative to vec's value. Each axis value is added to vec.
// - View-relative coordinates (^256 ^ ^)
//     Relative to vec's value. Each value is a multiplier for forward, right, and up vectors from a passed CBasePlayer's eye vectors. These multiplied vectors are then added onto vec.
// - Raycasted coordinates: (+ + +32)
//     Relative to the result of a world raycast. Each axis value is added to vec.
// Any of these types can be appended with *<n> to add a random value, [-n, n].
void UTIL_MCCoordNotation(CBasePlayer* pPlayer, const char* argx, const char* argy, const char* argz, Vector* vec);

#endif // CRISPY_UTIL_H
