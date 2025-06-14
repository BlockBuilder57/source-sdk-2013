//========= Copyright Valve Corporation, All rights reserved. ============//

#include "cbase.h"
#include "tf_point_weapon_mimic.h"

#ifdef GAME_DLL
#include "ndebugoverlay.h"
#endif

#include "tf_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( tf_point_weapon_mimic, CTFPointWeaponMimic );

IMPLEMENT_NETWORKCLASS_ALIASED( TFPointWeaponMimic, DT_TFPointWeaponMimic )

BEGIN_NETWORK_TABLE( CTFPointWeaponMimic, DT_TFPointWeaponMimic )
END_NETWORK_TABLE()

#ifdef GAME_DLL
// Data Description
BEGIN_DATADESC( CTFPointWeaponMimic )

	// Keyfields
	DEFINE_KEYFIELD( m_nWeaponType, FIELD_INTEGER, "WeaponType" ),
	DEFINE_KEYFIELD( m_pzsFireSound, FIELD_SOUNDNAME, "FireSound" ),
	DEFINE_KEYFIELD( m_pzsFireParticles, FIELD_STRING, "ParticleEffect" ),
	DEFINE_KEYFIELD( m_pzsModelOverride, FIELD_MODELNAME, "ModelOverride" ),
	DEFINE_KEYFIELD( m_flModelScale, FIELD_FLOAT, "ModelScale" ),
	DEFINE_KEYFIELD( m_flSpeedMin, FIELD_FLOAT, "SpeedMin" ),
	DEFINE_KEYFIELD( m_flSpeedMax, FIELD_FLOAT, "SpeedMax" ),
	DEFINE_KEYFIELD( m_flDamage, FIELD_FLOAT, "Damage" ),
	DEFINE_KEYFIELD( m_flSplashRadius, FIELD_FLOAT, "SplashRadius" ),
	DEFINE_KEYFIELD( m_flSpreadAngle, FIELD_FLOAT, "SpreadAngle" ),
	DEFINE_KEYFIELD( m_bCrits, FIELD_BOOLEAN, "Crits" ),
	DEFINE_KEYFIELD( m_flFireRate, FIELD_FLOAT, "FireRate" ),
	DEFINE_KEYFIELD( m_nBurstSize, FIELD_INTEGER, "BurstSize" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "FireOnce", InputFireOnce ),
	DEFINE_INPUTFUNC( FIELD_INTEGER, "FireMultiple", InputFireMultiple ),
	DEFINE_INPUTFUNC( FIELD_INTEGER, "QueueMultiple", InputQueueMultiple ),
	DEFINE_INPUTFUNC( FIELD_VOID, "DetonateStickies", InputDetonateStickies ),

	DEFINE_THINKFUNC( MimicThink ),

END_DATADESC()

BEGIN_ENT_SCRIPTDESC( CTFPointWeaponMimic, CBaseEntity , "TF Weapon Mimic" )
	DEFINE_SCRIPTFUNC( Fire, "Fire the mimic weapon" )
	DEFINE_SCRIPTFUNC_NAMED( ScriptFireMultiple, "FireMultiple", "Fire the mimic weapon a number of times at once" )
	DEFINE_SCRIPTFUNC_NAMED( ScriptQueueMultiple, "QueueMultiple", "Queue a number of fires of the mimic weapon" )

	DEFINE_SCRIPTFUNC( FireRocket, "Fire a rocket" )
	DEFINE_SCRIPTFUNC( FireGrenade, "Fire a grenade" )
	DEFINE_SCRIPTFUNC( FireArrow, "Fire an arrow" )
	DEFINE_SCRIPTFUNC( FireStickyGrenade, "Fire a stickybomb" )
	DEFINE_SCRIPTFUNC( FireHitscan, "Fire a hitscan bullet" )

	DEFINE_SCRIPTFUNC( DetonateStickies, "Detonate stickybombs" )

	DEFINE_SCRIPTFUNC_NAMED( ScriptSetModelScale, "SetModelScale", "Sets the scale of the projectiles" )
	DEFINE_SCRIPTFUNC_NAMED( ScriptSetMinSpeed, "SetMinSpeed", "Sets the minimum speed range of the projectiles" )
	DEFINE_SCRIPTFUNC_NAMED( ScriptSetMaxSpeed, "SetMaxSpeed", "Sets the maximum speed range of the projectiles" )
	DEFINE_SCRIPTFUNC_NAMED( ScriptSetDamage, "SetDamage", "Sets the base damage of the projectiles" )
	DEFINE_SCRIPTFUNC_NAMED( ScriptSetSplashRadius, "SetSplashRadius", "Sets the splash radius of the projectiles" )
	DEFINE_SCRIPTFUNC_NAMED( ScriptSetSpreadAngle, "SetSpreadAngle", "Sets the spread angle of the projectiles" )
	DEFINE_SCRIPTFUNC_NAMED( ScriptSetForceCrits, "SetForceCrits", "Decides whether projectiles should be crits" )
END_SCRIPTDESC();
#endif

CTFPointWeaponMimic::CTFPointWeaponMimic()
{
	// This entity was designed for MvM, and was hardcoded to fire blue projectiles
	// To avoid breaking old maps, let's act as blue by default
	// The keyfield should (intentionally) clobber this later, if it exists.
	ChangeTeam( TF_TEAM_BLUE );

#ifdef GAME_DLL
	m_flModelScale = 1.0f;
	m_flSpeedMin = m_flSpeedMax = 1000.0f;
	m_flDamage = 75.0f;
	m_flSplashRadius = 50.0f;
	m_flSpreadAngle = 0.0f;
	m_bCrits = false;
	m_flFireRate = 0.5f;
	m_nBurstSize = 1;
	m_nBurstLeft = 0;
#endif

	SetThink( &CTFPointWeaponMimic::MimicThink );
	SetNextThink( gpGlobals->curtime + 0.1f );
}

#ifdef GAME_DLL
int CTFPointWeaponMimic::DrawDebugTextOverlays()
{
	int text_offset = BaseClass::DrawDebugTextOverlays();

	if ( m_debugOverlays & OVERLAY_TEXT_BIT ) 
	{
		static const char *pszWeaponTypes[] =
		{
			"Rocket",
			"Grenade",
			"Arrow",
			"Sticky Grenade",
			"Hitscan",

			"<UNKNOWN>",
		};

		char tempstr[255];

		Q_snprintf( tempstr, sizeof(tempstr), "Team: %s (%d)", g_aTeamNames[ Clamp(GetTeamNumber(), 0, (int)TF_TEAM_COUNT) ], GetTeamNumber() );
		EntityText( text_offset, tempstr, 0 );
		text_offset++;

		Q_snprintf( tempstr, sizeof(tempstr), "Weapon type: %s", pszWeaponTypes[ Clamp(m_nWeaponType.Get(), 0, (int)WEAPON_TYPES) ] );
		EntityText( text_offset, tempstr, 0 );
		text_offset++;

		Q_snprintf( tempstr, sizeof(tempstr), "Until next fire: %.2f", GetNextThink() - gpGlobals->curtime );
		EntityText( text_offset, tempstr, 0 );
		text_offset++;

		Q_snprintf( tempstr, sizeof(tempstr), "Burst: %d/%d", m_nBurstLeft, m_nBurstSize.Get());
		EntityText( text_offset, tempstr, 0 );
		text_offset++;
	}

	return text_offset;
}

void CTFPointWeaponMimic::DrawDebugGeometryOverlays()
{
	if ( m_debugOverlays & OVERLAY_TEXT_BIT )
	{
		Vector vForward, vUp;
		AngleVectors( GetAbsAngles(), &vForward, NULL, &vUp);

		NDebugOverlay::Axis( GetAbsOrigin(), GetAbsAngles(), 8.f, false, 0 );
		NDebugOverlay::Circle( GetAbsOrigin(), GetAbsAngles(), 8.f, 255, 255, 255, 0, false, 0 );
		NDebugOverlay::VertArrow( GetAbsOrigin(), GetAbsOrigin() + ( vForward * 16.f ), 2.f, 255, 255, 255, 0, false, 0);
	}

	BaseClass::DrawDebugGeometryOverlays();
}
#endif


void CTFPointWeaponMimic::Spawn()
{
	BaseClass::Spawn();

#ifdef GAME_DLL
	if( m_pzsModelOverride )
	{
		PrecacheModel( m_pzsModelOverride );
	}
#endif
}

void CTFPointWeaponMimic::MimicThink()
{
	SetNextThink( gpGlobals->curtime + m_flFireRate );

#ifdef GAME_DLL
	if ( m_nBurstLeft > 0 )
	{
		m_nBurstLeft--;
		Fire();
	}
#endif
}


#ifdef GAME_DLL
void CTFPointWeaponMimic::InputFireOnce( inputdata_t& inputdata )
{
	Fire();
}

void CTFPointWeaponMimic::InputFireMultiple( inputdata_t& inputdata )
{
	int nNumFires = Max( 1, abs(inputdata.value.Int()) );

	while( nNumFires-- )
	{
		Fire();
	}
}

void CTFPointWeaponMimic::InputQueueMultiple( inputdata_t& inputdata )
{
	if ( inputdata.value.FieldType() == FIELD_INTEGER && inputdata.value.Int() >= 0 )
	{
		m_nBurstSize = Max( 1, abs(inputdata.value.Int()) );
	}
	
	m_nBurstLeft = m_nBurstSize;
}

void CTFPointWeaponMimic::InputDetonateStickies( inputdata_t &inputdata )
{
	DetonateStickies();
}

void CTFPointWeaponMimic::DetonateStickies()
{
	int count = m_Pipebombs.Count();

	for ( int i = 0; i < count; i++ )
	{
		CTFGrenadePipebombProjectile *pTemp = m_Pipebombs[i];
		if ( pTemp )
		{
			//This guy will die soon enough.
			if ( pTemp->IsEffectActive( EF_NODRAW ) )
				continue;
	
			pTemp->Detonate();
		}
	}

	m_Pipebombs.Purge();
}


void CTFPointWeaponMimic::ScriptFireMultiple( int times /* = 1 */ )
{
	int nNumFires = Max( 1, abs(times) );

	while( nNumFires-- )
	{
		Fire();
	}
}

void CTFPointWeaponMimic::ScriptQueueMultiple( int times /* = 1 */ )
{
	if ( time >= 0 )
	{
		m_nBurstSize = Max( 1, abs(times) );
	}
	
	m_nBurstLeft = m_nBurstSize;
}


void CTFPointWeaponMimic::Fire()
{
	Assert( m_nWeaponType >= 0 && m_nWeaponType < WEAPON_TYPES );

	switch( m_nWeaponType )
	{
	case WEAPON_STANDARD_ROCKET:
		FireRocket();
		break;
	case WEAPON_STANDARD_GRENADE:
		FireGrenade();
		break;
	case WEAPON_STANDARD_ARROW:
		FireArrow();
		break;
	case WEAPON_STICKY_GRENADE:
		FireStickyGrenade();
		break;
	case WEAPON_HITSCAN:
		FireHitscan();
		break;
	}
}

void CTFPointWeaponMimic::FireRocket()
{
	CTFProjectile_Rocket *pProjectile = CTFProjectile_Rocket::Create( this, GetAbsOrigin(), GetFiringAngles(), this, NULL);

	if ( pProjectile )
	{
		if( m_pzsModelOverride )
		{
			pProjectile->SetModel( m_pzsModelOverride );
		}

		pProjectile->SetCritical( m_bCrits );
		pProjectile->SetDamage( m_flDamage );
		Vector vVelocity = pProjectile->GetAbsVelocity().Normalized() * GetSpeed();
		pProjectile->SetAbsVelocity( vVelocity );	
		pProjectile->SetupInitialTransmittedGrenadeVelocity( vVelocity );
		pProjectile->SetCollisionGroup( TFCOLLISION_GROUP_ROCKET_BUT_NOT_WITH_OTHER_ROCKETS );
	}
}

void CTFPointWeaponMimic::FireGrenade()
{
	QAngle vFireAngles = GetFiringAngles();
	Vector vForward, vUp;
	AngleVectors( vFireAngles, &vForward, NULL, &vUp );
	Vector vVelocity( vForward * GetSpeed() );

	CTFGrenadePipebombProjectile *pGrenade = static_cast<CTFGrenadePipebombProjectile*>( CBaseEntity::CreateNoSpawn( "tf_projectile_pipe", GetAbsOrigin(), vFireAngles, this ) );
	if ( pGrenade )
	{
		DispatchSpawn( pGrenade );
		if( m_pzsModelOverride )
		{
			pGrenade->SetModel( m_pzsModelOverride );
		}
		pGrenade->InitGrenade( vVelocity, AngularImpulse( 600, random->RandomInt( -1200, 1200 ), 0 ), NULL, m_flDamage, m_flSplashRadius );
		pGrenade->SetDetonateTimerLength( 2.f );
		pGrenade->SetModelScale( m_flModelScale );
		pGrenade->SetCollisionGroup( TFCOLLISION_GROUP_ROCKETS );  // we want to use collision_group_rockets so we don't ever collide with players
		pGrenade->SetDamage( m_flDamage );
		pGrenade->SetFullDamage( m_flDamage );
		pGrenade->SetDamageRadius( m_flSplashRadius );
		pGrenade->SetCritical( m_bCrits );
		vVelocity = pGrenade->GetAbsVelocity().Normalized() * GetSpeed();
		pGrenade->SetAbsVelocity( vVelocity );	
		pGrenade->SetupInitialTransmittedGrenadeVelocity( vVelocity );
	}
}

void CTFPointWeaponMimic::FireArrow()
{
	CTFProjectile_Arrow *pProjectile = CTFProjectile_Arrow::Create( GetAbsOrigin(), GetFiringAngles(), 2000, 0.7f, TF_PROJECTILE_ARROW, this, NULL );

	if ( pProjectile )
	{
		if( m_pzsModelOverride )
		{
			pProjectile->SetModel( m_pzsModelOverride );
		}
		pProjectile->ChangeTeam( GetTeamNumber() );
		pProjectile->SetCritical( m_bCrits );
		pProjectile->SetDamage( m_flDamage );
		Vector vVelocity = pProjectile->GetAbsVelocity().Normalized() * GetSpeed();
		pProjectile->SetAbsVelocity( vVelocity );	
		pProjectile->SetupInitialTransmittedGrenadeVelocity( vVelocity );
		pProjectile->SetCollisionGroup( TFCOLLISION_GROUP_ROCKET_BUT_NOT_WITH_OTHER_ROCKETS );
	}
}

void CTFPointWeaponMimic::FireStickyGrenade()
{
	QAngle vFireAngles = GetFiringAngles();
	Vector vForward, vUp;
	AngleVectors( vFireAngles, &vForward, NULL, &vUp );
	Vector vVelocity( vForward * GetSpeed() );

	CTFGrenadePipebombProjectile *pGrenade = static_cast<CTFGrenadePipebombProjectile*>( CBaseEntity::CreateNoSpawn( "tf_projectile_pipe", GetAbsOrigin(), vFireAngles, this ) );
	if ( pGrenade )
	{
		pGrenade->m_bDefensiveBomb = true;

		pGrenade->SetPipebombMode( TF_GL_MODE_REMOTE_DETONATE );
		pGrenade->SetModelScale( m_flModelScale );
		pGrenade->SetCollisionGroup( TFCOLLISION_GROUP_ROCKETS );  // we want to use collision_group_rockets so we don't ever collide with players
		pGrenade->SetCanTakeDamage( false );
		DispatchSpawn( pGrenade );
		if( m_pzsModelOverride )
		{
			pGrenade->SetModel( m_pzsModelOverride );
		}
		else
		{
			pGrenade->SetModel( "models/weapons/w_models/w_stickybomb_d.mdl" );
		}

		pGrenade->InitGrenade( vVelocity, AngularImpulse( 600, random->RandomInt( -1200, 1200 ), 0 ), NULL, m_flDamage, m_flSplashRadius );
		vVelocity = pGrenade->GetAbsVelocity().Normalized() * GetSpeed();
		pGrenade->SetAbsVelocity( vVelocity );	
		pGrenade->SetupInitialTransmittedGrenadeVelocity( vVelocity );

		pGrenade->SetDamage( m_flDamage );
		pGrenade->SetFullDamage( m_flDamage );
		pGrenade->SetDamageRadius( m_flSplashRadius );
		pGrenade->SetCritical( m_bCrits );
		pGrenade->ChangeTeam( GetTeamNumber() );
		pGrenade->m_nSkin = 1;

		m_Pipebombs.AddToTail( pGrenade );
	}
}

void CTFPointWeaponMimic::FireHitscan()
{
	FireBulletsInfo_t info;

	info.m_vecSrc = GetAbsOrigin();
	AngleVectors(GetFiringAngles(), &info.m_vecDirShooting);
	info.m_iTracerFreq = 1;
	info.m_iShots = 1;
	info.m_iAmmoType = TF_AMMO_PRIMARY;
	info.m_pAttacker = this;
	info.m_vecSpread = vec3_origin; // handled by GetFiringAngles
	info.m_flDistance = MAX_TRACE_LENGTH;
	info.m_flDamage = m_flDamage;
	info.m_flDamageForceScale = 0.0f;
	info.m_nFlags = FIRE_BULLETS_FIRST_SHOT_ACCURATE | FIRE_BULLETS_ALLOW_WATER_SURFACE_IMPACTS;

	FireBullets( info );

	//MakeTracer();
}

QAngle CTFPointWeaponMimic::GetFiringAngles() const
{
	// No spread?  Straight along our angles, then
	QAngle angles = GetAbsAngles();
	if( m_flSpreadAngle == 0 )
		return angles;

	Vector vForward, vRight, vUp;
	AngleVectors( angles, &vForward, &vRight, &vUp );

	// Rotate around up by half the spread input, then rotate around the original forward by +-180
	float flHalfSpread = m_flSpreadAngle / 2.f;
	VMatrix mtxRotateAroundUp		= SetupMatrixAxisRot( vUp,		RandomFloat( -flHalfSpread, flHalfSpread ) );
	VMatrix mtxRotateAroundForward	= SetupMatrixAxisRot( vForward,	RandomFloat( -180, 180 ) );

	// Rotate forward
	VMatrix mtxSpreadRot;
	MatrixMultiply( mtxRotateAroundForward, mtxRotateAroundUp, mtxSpreadRot );
	vForward = mtxSpreadRot * vForward;

	// Back to angles
	VectorAngles( vForward, vUp, angles );

	return angles;

}

float CTFPointWeaponMimic::GetSpeed() const
{
	return RandomFloat( m_flSpeedMin, m_flSpeedMax );
}

void CTFPointWeaponMimic::ModifyFireBulletsDamage( CTakeDamageInfo *dmgInfo )
{
	dmgInfo->SetCritType( m_bCrits ? CTakeDamageInfo::ECritType::CRIT_FULL : CTakeDamageInfo::ECritType::CRIT_NONE );
}

#endif 