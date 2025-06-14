//========= Copyright Valve Corporation, All rights reserved. ============//

#ifndef TF_POINT_WEAPON_MIMIC_H
#define TF_POINT_WEAPON_MIMIC_H
#ifdef _WIN32
#pragma once
#endif

#ifdef GAME_DLL
#include "tf_projectile_rocket.h"
#include "tf_projectile_arrow.h"
#include "tf_weapon_grenade_pipebomb.h"
#endif

#ifdef CLIENT_DLL
#define CTFPointWeaponMimic C_TFPointWeaponMimic
#endif

class CTFPointWeaponMimic : public CBaseEntity
{
	DECLARE_CLASS( CTFPointWeaponMimic, CBaseEntity );
	DECLARE_NETWORKCLASS();

public:
	CTFPointWeaponMimic();
	~CTFPointWeaponMimic() {}

	virtual void	Spawn( void );

#ifdef GAME_DLL
	int DrawDebugTextOverlays( void );
	void DrawDebugGeometryOverlays( void );
#endif

	void MimicThink();

#ifdef GAME_DLL
	DECLARE_DATADESC();
	DECLARE_ENT_SCRIPTDESC();

	void InputFireOnce( inputdata_t& inputdata );
	void InputFireMultiple( inputdata_t& inputdata );
	void InputQueueMultiple( inputdata_t& inputdata );
	void InputDetonateStickies( inputdata_t& inputdata );

	void ScriptFireMultiple( int times = 1 );
	void ScriptQueueMultiple( int times = 1 );
	void ScriptSetWeaponType( int type ) { m_nWeaponType = type; }
	void ScriptSetModelScale( float num ) { m_flModelScale = num; }
	void ScriptSetMinSpeed( float num ) { m_flSpeedMin = num; }
	void ScriptSetMaxSpeed( float num ) { m_flSpeedMax = num; }
	void ScriptSetDamage( float num ) { m_flDamage = num; }
	void ScriptSetSplashRadius( float num ) { m_flSplashRadius = num; }
	void ScriptSetSpreadAngle( float num ) { m_flSpreadAngle = num; }
	void ScriptSetForceCrits( bool crits ) { m_bCrits = crits; }
	void ScriptSetFireRate( float num ) { m_flFireRate = num; }
	void ScriptSetBurstSize( int size ) { m_nBurstSize = size; }

	void Fire();

	void FireRocket();
	void FireGrenade();
	void FireArrow();
	void FireStickyGrenade();
	void FireHitscan();

	void DetonateStickies();

private:
	virtual void ModifyFireBulletsDamage( CTakeDamageInfo* dmgInfo );

	enum eWeaponType
	{
		WEAPON_STANDARD_ROCKET,
		WEAPON_STANDARD_GRENADE,
		WEAPON_STANDARD_ARROW,
		WEAPON_STICKY_GRENADE,
		WEAPON_HITSCAN,

		WEAPON_TYPES
	};

	QAngle GetFiringAngles() const;
	float GetSpeed() const;

	int m_nBurstLeft;

	// Effects for firing
	const char* m_pzsFireSound;
	const char* m_pzsFireParticles;

	// Override/defaults for the projectile/bullets
	const char* m_pzsModelOverride;

	// List of active pipebombs
	typedef CHandle<CTFGrenadePipebombProjectile>	PipebombHandle;
	CUtlVector<PipebombHandle>		m_Pipebombs;
#endif

	CNetworkVar( int, m_nWeaponType );
	CNetworkVar( float, m_flModelScale );
	CNetworkVar( float, m_flSpeedMin );
	CNetworkVar( float, m_flSpeedMax );
	CNetworkVar( float, m_flDamage );
	CNetworkVar( float, m_flSplashRadius );
	CNetworkVar( float, m_flSpreadAngle );
	CNetworkVar( bool, m_bCrits );
	CNetworkVar( float, m_flFireRate );
	CNetworkVar( int, m_nBurstSize );
};

#endif	//TF_POINT_WEAPON_MIMIC_H
