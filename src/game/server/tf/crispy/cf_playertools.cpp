#include "cbase.h"
#include "tf/crispy/crispyfortress.h"
#include "tf/crispy/crispy_util.h"
#include "tf/tf_player.h"
#include "tf_gamerules.h"
#include "tf_dropped_weapon.h"
#include "econ_entity_creation.h"
#include "movevars_shared.h"

struct CCFPlayerTools : public ICrispyPlugin
{
	// left to do:

	// force respawn cvar

	CON_COMMAND_MEMBER_F(CCFPlayerTools, "cf_goto", CC_GotoPlayer_f, "Teleports you to a player's position.\n\tUsage: cf_goto <player>", FCVAR_CHEAT);
	CON_COMMAND_MEMBER_F(CCFPlayerTools, "cf_bring", CC_BringPlayers_f, "Teleports the player or specified player(s) to your crosshair position, or another player's position.\n\tUsage: cf_bring <player match> [player match]", FCVAR_CHEAT);
	CON_COMMAND_MEMBER_F(CCFPlayerTools, "cf_tele", CC_TeleportPlayers_f, "Teleports the player or specified player(s) to the specified coordinates.\n\tUsage: cf_tele <player match> <x> <y> <z> [[pitch] [yaw] [roll]] [[vx] [vy] [vz]] [[dx] [dy] [dz]] [[dpitch] [dyaw] [droll]] [[dvx] [dvy] [dvz]]", FCVAR_CHEAT);
	CON_COMMAND_MEMBER_F(CCFPlayerTools, "cf_catapult", CC_CatapultPlayers_f, "Catapults the player or specified player(s) to the specified coordinates.\n\tUsage: cf_catapult <x> <y> <z> [player match] [force]", FCVAR_CHEAT);

	CON_COMMAND_MEMBER_F(CCFPlayerTools, "cf_setteam", CC_SetTeam_f, "Joins the player or specified player(s) to the specified team.\n\tUsage: cf_setteam <team> [player match]", FCVAR_CHEAT);
	CON_COMMAND_MEMBER_F(CCFPlayerTools, "cf_setclass", CC_SetClass_f, "Sets the player's or specified player(s)'s class to the specified class.\n\tUsage: cf_setclass <class> [player match]", FCVAR_CHEAT);

	CON_COMMAND_MEMBER_F(CCFPlayerTools, "cf_settargetdummy", CC_SetTargetDummy_f, "Sets or toggles the player or specified player(s) to be a targetdummy.\n\tUsage: cf_settargetdummy <-1, 0, or 1> [player match]", FCVAR_CHEAT);
	CON_COMMAND_MEMBER_F(CCFPlayerTools, "cf_sethealth", CC_SetHealth_f, "Sets the player's or specified player(s)'s hp to the specified value.\n\tUsage: cf_sethealth <health> [player match]", FCVAR_CHEAT);
	CON_COMMAND_MEMBER_F(CCFPlayerTools, "cf_setammo", CC_SetAmmo_f, "Sets the player's or specified player(s)'s ammo to the specified value.\nOverride types are: 1 - primary, 2 - secondary, 3 - metal, 4-6 - grenades\n\tUsage: cf_setammo <amount> [override type] [player match]", FCVAR_CHEAT);
	CON_COMMAND_MEMBER_F(CCFPlayerTools, "cf_setclip", CC_SetClip_f, "Sets the player's or specified player(s)'s current clip to the specified value.\n\tUsage: cf_setclip <amount> [player match]", FCVAR_CHEAT);

	CON_COMMAND_MEMBER_F(CCFPlayerTools, "cf_whack", CC_Whack_f, "Whacks the specified player(s) with the current weapon. If specified, a specific damage amount is used.\n\tUsage: cf_whack <player match> [damage]", FCVAR_CHEAT);
	CON_COMMAND_MEMBER_F(CCFPlayerTools, "cf_createragdoll", CC_CreateRagdoll_f, "Creates a ragdoll from the player or specified player(s).\n\tUsage: cf_createragdoll [player match]", FCVAR_CHEAT);

	CON_COMMAND_MEMBER_F(CCFPlayerTools, "cf_test_playersbycommandarg", CC_TestPlayersByCommandArg_f, "Tests the new UTIL_PlayersByCommandArg\n\tUsage: cf_test_playersbycommandarg <player match>", FCVAR_NONE);
};

void CCFPlayerTools::CC_GotoPlayer_f(const CCommand &args)
{
	CHECK_ARGC(args.ArgC(), >, 1);
	GET_LOCAL_TFPLAYER(pTFPlayer, true);

	PLAYER_BY_COMMANDARG(pPlayer, args[1]);

	Vector vecPos = pPlayer->GetAbsOrigin();
	QAngle vecAng = pPlayer->EyeAngles();
	Vector vecVel = pPlayer->GetAbsVelocity();

	pTFPlayer->Teleport(&vecPos, &vecAng, &vecVel);
}
void CCFPlayerTools::CC_BringPlayers_f(const CCommand &args)
{
	CHECK_ARGC(args.ArgC(), >, 1);

	CUtlVector<CTFPlayer*> targets;
	TFPLAYERS_BY_COMMANDARG(targets, args[1]);

	Vector vecPos = vec3_origin;
	QAngle vecAng = vec3_angle;
	Vector vecVel = vec3_origin;

	// self going to target
	if (args.ArgC() == 2)
	{
		GET_LOCAL_TFPLAYER(pTFPlayer, true);

		trace_t tr;
		Vector forward;
		pTFPlayer->EyeVectors(&forward);
		UTIL_TraceLine(pTFPlayer->EyePosition(), pTFPlayer->EyePosition() + forward * MAX_TRACE_LENGTH, MASK_SOLID, pTFPlayer, COLLISION_GROUP_PLAYER, &tr);

		vecPos = tr.endpos;
		vecAng = pTFPlayer->EyeAngles();
		vecVel = vec3_origin;
	}
	// multiple going to target
	else if (args.ArgC() > 2)
	{
		PLAYER_BY_COMMANDARG(pPlayer, args[2]);

		vecPos = pPlayer->GetAbsOrigin();
		vecAng = pPlayer->EyeAngles();
		vecVel = pPlayer->GetAbsVelocity();
	}
	
	FOR_EACH_VEC( targets, i )
	{
		CTFPlayer* pTarget = targets[i];
		if (pTarget)
			pTarget->Teleport(&vecPos, &vecAng, &vecVel);
	}
}
void CCFPlayerTools::CC_TeleportPlayers_f(const CCommand &args)
{
	CHECK_ARGC(args.ArgC(), >, 4);
	CUtlVector<CTFPlayer*> targets;
	TFPLAYERS_BY_COMMANDARG(targets, args[1]);

	Vector vecAbsPos = vec3_origin, vecDelPos = vec3_origin;
	QAngle vecAbsAng = vec3_angle,  vecDelAng = vec3_angle;
	Vector vecAbsVel = vec3_origin, vecDelVel = vec3_origin;

	GET_LOCAL_TFPLAYER(pTFPlayer, false);
	if (pTFPlayer)
	{
		vecAbsPos = pTFPlayer->GetAbsOrigin();
		vecAbsAng = pTFPlayer->EyeAngles();
		vecAbsVel = pTFPlayer->GetAbsVelocity();
	}
	
	FOR_EACH_VEC( targets, i )
	{
		CTFPlayer* pTarget = targets[i];
		if (pTarget)
		{
			// each argument is explicitly parsed multiple times here for randomization
			// also, we'll want to use the command client rather than the target for
			// the absolute vectors so traces and view-relative things work as expected
			
			// absolute pos 
			UTIL_MCCoordNotation(pTFPlayer, args[2], args[3], args[4], &vecAbsPos);

			// absolute angle (need to cast to a Vector)
			if (args.ArgC() > 7)
				UTIL_MCCoordNotation(pTFPlayer, args[5], args[6], args[7], reinterpret_cast<Vector*>(&vecAbsAng));
			// absolute velocity
			if (args.ArgC() > 10)
				UTIL_MCCoordNotation(pTFPlayer, args[8], args[9], args[10], &vecAbsVel);
			
			pTarget->Teleport(&vecAbsPos, &vecAbsAng, &vecAbsVel);

			vecDelPos = vecDelVel = vec3_origin;
			vecDelAng = vec3_angle;

			// delta position
			if (args.ArgC() > 13)
				UTIL_MCCoordNotation(pTarget, args[11], args[12], args[13], &vecDelPos);
			// delta angle
			if (args.ArgC() > 16)
				UTIL_MCCoordNotation(pTarget, args[14], args[15], args[16], reinterpret_cast<Vector*>(&vecDelAng));
			// delta velocity
			if (args.ArgC() > 19)
				UTIL_MCCoordNotation(pTarget, args[17], args[18], args[19], &vecDelVel);

			vecAbsPos += vecDelPos;
			vecAbsAng += vecDelAng;
			vecAbsVel += vecDelVel;
		}
	}
}
void CCFPlayerTools::CC_CatapultPlayers_f(const CCommand &args)
{
	CHECK_ARGC(args.ArgC(), >, 3);
	CUtlVector<CTFPlayer*> targets;

	GET_LOCAL_TFPLAYER(pTFPlayer, false);
	if (pTFPlayer)
		targets.AddToTail(pTFPlayer);

	// can match players after coordinate
	if (args.ArgC() > 4)
		TFPLAYERS_BY_COMMANDARG(targets, args[4]);
	
	FOR_EACH_VEC( targets, i )
	{
		CTFPlayer* pTarget = targets[i];
		if (pTarget)
		{
			Vector vecSourcePos = pTarget->GetAbsOrigin();
			Vector vecTargetPos = vecSourcePos;

			// target coordinate
			UTIL_MCCoordNotation(pTFPlayer, args[1], args[2], args[3], &vecTargetPos);

			// If victim is player, adjust target position so player's center will hit the target
			vecTargetPos.z -= 32.0f;

			float flSpeed = (vecSourcePos - vecTargetPos).Length();	// u/sec
			float flGravity = GetCurrentGravity();

			// set force explicitly if we have it
			if (args.ArgC() > 5)
				flSpeed = V_atof(args[5]);
	
			Vector vecVelocity = (vecTargetPos - vecSourcePos);

			// throw at a constant time
			float time = vecVelocity.Length() / flSpeed;
			vecVelocity = vecVelocity * (1.f / time); // CatapultLaunchVelocityMultiplier

			// adjust upward toss to compensate for gravity loss
			vecVelocity.z += flGravity * time * 0.5;

			// Send us flying
			if ( pTarget->GetFlags() & FL_ONGROUND )
			{
				pTarget->SetGroundEntity( NULL );
				pTarget->SetGroundChangeTime( gpGlobals->curtime + 0.5f );
			}

			pTarget->Teleport( NULL, NULL, &vecVelocity );
		}
	}
}

void CCFPlayerTools::CC_SetTeam_f(const CCommand &args)
{
	CHECK_ARGC(args.ArgC(), >, 1);
	int iTeamNum = UTIL_TeamByArg(args[1]);

	CUtlVector<CTFPlayer*> targets;

	GET_LOCAL_TFPLAYER(pTFPlayer, false);
	if (pTFPlayer)
		targets.AddToTail(pTFPlayer);

	if (args.ArgC() > 2)
		TFPLAYERS_BY_COMMANDARG(targets, args[2]);
	
	FOR_EACH_VEC( targets, i )
	{
		CTFPlayer* pTarget = targets[i];
		if (!pTarget)
			return;

		Vector vecPos = pTarget->GetAbsOrigin();
		QAngle vecAng = pTarget->EyeAngles();
		Vector vecVel = pTarget->GetAbsVelocity();

		if (iTeamNum > TEAM_INVALID)
		{
			pTarget->AllowInstantSpawn();
			pTarget->ChangeTeam(iTeamNum, false, true);
			pTarget->Teleport(&vecPos, &vecAng, &vecVel);
		}
	}
}
void CCFPlayerTools::CC_SetClass_f(const CCommand &args)
{
	CHECK_ARGC(args.ArgC(), >, 1);
	const char* className = args[1];

	CUtlVector<CTFPlayer*> targets;

	GET_LOCAL_TFPLAYER(pTFPlayer, false);
	if (pTFPlayer)
		targets.AddToTail(pTFPlayer);

	if (args.ArgC() > 2)
		TFPLAYERS_BY_COMMANDARG(targets, args[2]);
	
	FOR_EACH_VEC( targets, i )
	{
		CTFPlayer* pTarget = targets[i];
		if (!pTarget)
			return;

		Vector vecPos = pTarget->GetAbsOrigin();
		QAngle vecAng = pTarget->EyeAngles();
		Vector vecVel = pTarget->GetAbsVelocity();

		pTarget->AllowInstantSpawn();
		pTarget->HandleCommand_JoinClass(className);
		pTarget->Teleport(&vecPos, &vecAng, &vecVel);
	}
}

void CCFPlayerTools::CC_SetTargetDummy_f(const CCommand &args)
{
	CHECK_ARGC(args.ArgC(), >, 1);
	int targetdummy = V_atoi64(args[1]);

	CUtlVector<CTFPlayer*> targets;

	GET_LOCAL_TFPLAYER(pTFPlayer, false);
	if (pTFPlayer)
		targets.AddToTail(pTFPlayer);

	if (args.ArgC() > 2)
		TFPLAYERS_BY_COMMANDARG(targets, args[2]);

	FOR_EACH_VEC(targets, i)
	{
		CTFPlayer* pTarget = targets[i];
		if (!pTarget)
			return;

		if (targetdummy >= 0)
			pTarget->SetTargetDummy(targetdummy);
		else
			pTarget->SetTargetDummy(pTarget->GetTargetDummy());

		if (pTarget->GetTargetDummy())
		{
			ClientPrint(pTarget, HUD_PRINTTALK, "You're now a targetdummy!\n");
			Msg("%s is now a targetdummy.\n", pTarget->GetPlayerName());
		}
		else
		{
			ClientPrint(pTarget, HUD_PRINTTALK, "You are no longer a targetdummy.\n");
			Msg("%s is no longer a targetdummy.\n", pTarget->GetPlayerName());
		}
	}
}
void CCFPlayerTools::CC_SetHealth_f(const CCommand &args)
{
	CHECK_ARGC(args.ArgC(), >, 1);
	int iHealth = V_atoi64(args[1]);

	CUtlVector<CTFPlayer*> targets;

	GET_LOCAL_TFPLAYER(pTFPlayer, false);
	if (pTFPlayer)
		targets.AddToTail(pTFPlayer);

	if (args.ArgC() > 2)
		TFPLAYERS_BY_COMMANDARG(targets, args[2]);
	
	FOR_EACH_VEC( targets, i )
	{
		CTFPlayer* pTarget = targets[i];
		if (!pTarget)
			return;

		pTarget->SetHealth(iHealth);
	}
}
void CCFPlayerTools::CC_SetAmmo_f(const CCommand &args)
{
	CHECK_ARGC(args.ArgC(), >, 1);
	int iAmount = V_atoi64(args[1]);

	int iType = -1;
	if (args.ArgC() > 2)
		iType = V_atoi64(args[2]);

	CUtlVector<CTFPlayer*> targets;

	GET_LOCAL_TFPLAYER(pTFPlayer, false);
	if (pTFPlayer)
		targets.AddToTail(pTFPlayer);

	if (args.ArgC() > 3)
		TFPLAYERS_BY_COMMANDARG(targets, args[3]);
	
	FOR_EACH_VEC( targets, i )
	{
		CTFPlayer* pTarget = targets[i];
		if (!pTarget)
			return;

		CTFWeaponBase *pActiveWeapon = pTarget->GetActiveTFWeapon();
		if (pActiveWeapon)
		{
			if (iType == -1)
				iType = pActiveWeapon->GetPrimaryAmmoType();
		}

		Clamp(iType, 0, (int)TF_AMMO_COUNT);

		pTarget->SetAmmoCount(iAmount, iType);
	}
}
void CCFPlayerTools::CC_SetClip_f(const CCommand &args)
{
	CHECK_ARGC(args.ArgC(), >, 1);
	int iClip = V_atoi64(args[1]);

	CUtlVector<CTFPlayer*> targets;

	GET_LOCAL_TFPLAYER(pTFPlayer, false);
	if (pTFPlayer)
		targets.AddToTail(pTFPlayer);

	if (args.ArgC() > 2)
		TFPLAYERS_BY_COMMANDARG(targets, args[2]);
	
	FOR_EACH_VEC( targets, i )
	{
		CTFPlayer* pTarget = targets[i];
		if (!pTarget)
			return;

		CTFWeaponBase *pActiveWeapon = pTarget->GetActiveTFWeapon();
		if (!pActiveWeapon)
			return;

		if (pActiveWeapon->UsesClipsForAmmo1())
			pActiveWeapon->m_iClip1.Set(iClip);
		else
			pActiveWeapon->SetPrimaryAmmoCount(iClip);
	}
}

void CCFPlayerTools::CC_Whack_f(const CCommand &args)
{
	CHECK_ARGC(args.ArgC(), >, 1);

	CUtlVector<CTFPlayer*> targets;
	TFPLAYERS_BY_COMMANDARG(targets, args[1]);

	GET_LOCAL_TFPLAYER(pTFPlayer, false);

	int iDamage = 10000;
	if (args.ArgC() > 2)
		iDamage = V_atoi64(args[2]);
	
	FOR_EACH_VEC( targets, i )
	{
		CTFPlayer* pTarget = targets[i];
		if (!pTarget)
			return;

		CTakeDamageInfo info( pTarget, (pTFPlayer ? pTFPlayer : pTarget), iDamage, DMG_BULLET );
		info.SetInflictor( pTFPlayer->GetActiveTFWeapon() );
		pTarget->TakeDamage( info );
	}
}
void CCFPlayerTools::CC_CreateRagdoll_f(const CCommand &args)
{
	CUtlVector<CTFPlayer*> targets;

	GET_LOCAL_TFPLAYER(pTFPlayer, false);
	if (pTFPlayer)
		targets.AddToTail(pTFPlayer);

	if (args.ArgC() > 1)
		TFPLAYERS_BY_COMMANDARG(targets, args[1]);

	bool bGib = false, bBurning = false, bElectrocuted = false, bOnGround = true, bCloakedCorpse = false, bGoldRagdoll = false, bIceRagdoll = false, bBecomeAsh = false, bCritOnHardHit = false;

	if (args.ArgC() > 2)
		bGib = V_atoi64(args[2]);
	if (args.ArgC() > 3)
		bBurning = V_atoi64(args[3]);
	if (args.ArgC() > 4)
		bElectrocuted = V_atoi64(args[4]);
	if (args.ArgC() > 5)
		bOnGround = V_atoi64(args[5]);
	if (args.ArgC() > 6)
		bCloakedCorpse = V_atoi64(args[6]);
	if (args.ArgC() > 7)
		bGoldRagdoll = V_atoi64(args[7]);
	if (args.ArgC() > 8)
		bIceRagdoll = V_atoi64(args[8]);
	if (args.ArgC() > 9)
		bBecomeAsh = V_atoi64(args[9]);
	if (args.ArgC() > 10)
		bCritOnHardHit = V_atoi64(args[10]);

	FOR_EACH_VEC( targets, i )
	{
		CTFPlayer* pTarget = targets[i];
		if (!pTarget)
			return;

		pTarget->CreateCrispyRagdollEntity(bGib, bBurning, bElectrocuted, bOnGround, bCloakedCorpse, bGoldRagdoll, bIceRagdoll, bBecomeAsh, 0, bCritOnHardHit);
	}
}

void CCFPlayerTools::CC_TestPlayersByCommandArg_f(const CCommand &args)
{
	CHECK_ARGC(args.ArgC(), >, 1);

	Warning("~ UTIL_PlayerByCommandArg ~ (Valve standard)\n");

	CBasePlayer* pPlayer = UTIL_PlayerByCommandArg(args[1]);
	if (pPlayer)
		Msg("\tUTIL_PlayerByCommandArg returned %splayer %d named \"%s\", on team %d\n", pPlayer->IsFakeClient() ? "fake (bot) " : "", pPlayer->GetClientIndex(), pPlayer->GetPlayerName(), pPlayer->GetTeamNumber());
	else
		Msg("\tUTIL_PlayerByCommandArg returned null, either match was ambiguous (fuck you) or there is no such player\n");

	Warning("~ UTIL_PlayersByCommandArg ~ (Crispy Fortress)\n");

	CUtlVector<CBasePlayer*> targets;
	UTIL_PlayersByCommandArg(targets, args[1]);
	if (targets.IsEmpty())
		Msg("UTIL_PlayersByCommandArg was empty, so no players were found\n");

	FOR_EACH_VEC( targets, i )
	{
		pPlayer = targets[i];
		if (pPlayer)
			Msg("\tUTIL_PlayersByCommandArg returned %splayer %d named \"%s\", on team %d\n", pPlayer->IsFakeClient() ? "fake (bot) " : "", pPlayer->GetClientIndex(), pPlayer->GetPlayerName(), pPlayer->GetTeamNumber());
		else
			Msg("\tSomehow a null crept into UTIL_PlayersByCommandArg\n");
	}
}

CF_LINK_PLUGIN(CCFPlayerTools, "PlayerTools");