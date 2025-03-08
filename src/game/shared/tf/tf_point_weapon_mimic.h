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
	DECLARE_DATADESC();
	DECLARE_ENT_SCRIPTDESC();

	void InputFireOnce( inputdata_t& inputdata );
	void InputFireMultiple( inputdata_t& inputdata );
	void InputDetonateStickies( inputdata_t& inputdata );

	void ScriptFireMultiple( int times = 1 );
	void ScriptSetModelScale( float num ) { m_flModelScale = num; }
	void ScriptSetMinSpeed( float num ) { m_flSpeedMin = num; }
	void ScriptSetMaxSpeed( float num ) { m_flSpeedMax = num; }
	void ScriptSetDamage( float num ) { m_flDamage = num; }
	void ScriptSetSplashRadius( float num ) { m_flSplashRadius = num; }
	void ScriptSetSpreadAngle( float num ) { m_flSpreadAngle = num; }
	void ScriptSetForceCrits( bool crits ) { m_bCrits = crits; }

	void Fire();

	void FireRocket();
	void FireGrenade();
	void FireArrow();
	void FireStickyGrenade();

	void DetonateStickies();

private:
	enum eWeaponType
	{
		WEAPON_STANDARD_ROCKET,
		WEAPON_STANDARD_GRENADE,
		WEAPON_STANDARD_ARROW,
		WEAPON_STICKY_GRENADE,

		WEAPON_TYPES
	};

	QAngle GetFiringAngles() const;
	float GetSpeed() const;

	int m_nWeaponType;
	bool m_bContinousFire;

	// Effects for firing
	const char* m_pzsFireSound;
	const char* m_pzsFireParticles;

	// Override/defaults for the projectile/bullets
	const char* m_pzsModelOverride;

	CNetworkVar( float, m_flModelScale );
	CNetworkVar( float, m_flSpeedMin );
	CNetworkVar( float, m_flSpeedMax );
	CNetworkVar( float, m_flDamage );
	CNetworkVar( float, m_flSplashRadius );
	CNetworkVar( float, m_flSpreadAngle );
	CNetworkVar( bool, m_bCrits );

	// List of active pipebombs
	typedef CHandle<CTFGrenadePipebombProjectile>	PipebombHandle;
	CUtlVector<PipebombHandle>		m_Pipebombs;
#endif
};

#endif	//TF_POINT_WEAPON_MIMIC_H
