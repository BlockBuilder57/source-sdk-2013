#include "cbase.h"
#include "tf/crispy/crispyfortress.h"
#include "tf/crispy/crispy_util.h"
#include "tf/tf_player.h"
#include "tf_gamerules.h"
#include "tf_dropped_weapon.h"
#include "econ_entity_creation.h"

//extern void BotGenerateAndWearItem( CTFPlayer *pBot, const char *itemName );

class CCrispyAttributeIterator : public CEconItemSpecificAttributeIterator
{
public:
	CCrispyAttributeIterator() : m_vecAttributes({}) {}

	bool OnIterateAttributeValue(const CEconItemAttributeDefinition* pAttrDef, attrib_value_t value) OVERRIDE
	{
		attrib_iterator_value_t attrVal { pAttrDef };
		attrVal.m_value = value;
		m_vecAttributes.AddToTail(attrVal);

		return true;
	}

	bool OnIterateAttributeValue(const CEconItemAttributeDefinition* pAttrDef, const CAttribute_String& value) OVERRIDE
	{
		attrib_iterator_value_t attrVal{ pAttrDef };
		attrVal.m_strValue = &value;
		m_vecAttributes.AddToTail(attrVal); 
		
		return true;
	}

	struct attrib_iterator_value_t
	{
		const CEconItemAttributeDefinition* m_pAttrDef = nullptr;
		attrib_value_t m_value = 0;
		const CAttribute_String* m_strValue = nullptr;
	};

	static int __cdecl SortComparator(const attrib_iterator_value_t* a, const attrib_iterator_value_t* b)
	{
		attrib_definition_index_t aidx = a->m_pAttrDef->GetDefinitionIndex();
		attrib_definition_index_t bidx = b->m_pAttrDef->GetDefinitionIndex();
		
		if (aidx < bidx)
			return -1;
		else if (aidx == bidx)
			return 0;
		return 1;
	}

	CUtlVector<attrib_iterator_value_t> m_vecAttributes;
};

struct CCFEconTools : public ICrispyPlugin
{
	bool CreateAndGiveItem(CEconItemDefinition* pData, CTFPlayer* pTFPlayer, const char* pszTranslatedEnt, bool switchTo = false, bool removeExisting = true);
	bool GiveAttribute(CEconItemAttributeDefinition* pAttrib, CAttributeList* pAttrList, float flValue);
	void ListAttributes(CCrispyAttributeIterator* it);

	CON_COMMAND_MEMBER_F(CCFEconTools, "cf_econ_giveitem", CC_GiveItem_f, "Directly gives an item to the player or specified player(s).\nUsage: cf_econ_giveitem <item index> [player match] [remove existing weapon] [entity name override]", FCVAR_CHEAT);
	CON_COMMAND_MEMBER_F(CCFEconTools, "cf_econ_dropweapon", CC_DropWeapon_f, "Drops a weapon out of your player.\nUsage: cf_econ_dropweapon <item index>", FCVAR_CHEAT);
	CON_COMMAND_MEMBER_F(CCFEconTools, "cf_econ_trytaunt", CC_TryTaunt_f, "Taunts with the specified taunt name.\nUsage: cf_econ_trytaunt <item index> [player match]", FCVAR_NONE);

	CON_COMMAND_MEMBER_F(CCFEconTools, "cf_econ_removeweapons", CC_RemoveWeapons_f, "Removes all weapons from the player or specified player(s).\nUsage: cf_econ_removeweapons [player match]", FCVAR_CHEAT);

	CON_COMMAND_MEMBER_F(CCFEconTools, "cf_econ_addwepattrib", CC_AddWeaponAttribute_f, "Adds a runtime attribute on the player or specified player(s) current weapon.\nUsage: cf_econ_addwepattrib <attrib index> <value> [player match]", FCVAR_CHEAT);
	CON_COMMAND_MEMBER_F(CCFEconTools, "cf_econ_clearwepattribs", CC_ClearWeaponAttributes_f, "Clears runtime attributes on the player or specified player(s) current weapon.\nUsage: cf_econ_clearwepattribs [player match]", FCVAR_CHEAT);
	CON_COMMAND_MEMBER_F(CCFEconTools, "cf_econ_listwepattribs", CC_ListWeaponAttributes_f, "Lists attributes on the player or specified player(s) current weapon.\nUsage: cf_econ_listwepattribs [player match]", FCVAR_CHEAT);

	CON_COMMAND_MEMBER_F(CCFEconTools, "cf_econ_addplrattrib", CC_AddPlayerAttribute_f, "Adds a runtime attribute on the player or specified player(s).\nUsage: cf_econ_addplrattrib <attrib index> <value> [length] [player match]", FCVAR_CHEAT);
	CON_COMMAND_MEMBER_F(CCFEconTools, "cf_econ_clearplrattribs", CC_ClearPlayerAttributes_f, "Clears runtime attributes on the player or specified player(s).\nUsage: cf_econ_clearplrattribs [player match]", FCVAR_CHEAT);
	CON_COMMAND_MEMBER_F(CCFEconTools, "cf_econ_listplrattribs", CC_ListPlayerAttributes_f, "Lists attributes on the player or specified player(s).\nUsage: cf_econ_listplrattribs [player match]", FCVAR_CHEAT);

	CON_COMMAND_MEMBER_F(CCFEconTools, "cf_econ_generate_definition", CC_GenerateDefinition_f, "Generates an items_game.txt definition for the currently held weapon.", FCVAR_NONE);
	CON_COMMAND_MEMBER_F(CCFEconTools, "cf_econ_count_loadout_slots", CC_CountLoadoutSlots_f, "Counts the number of item definitions in each loadout slot.", FCVAR_NONE);
};

bool CCFEconTools::CreateAndGiveItem(CEconItemDefinition* pData, CTFPlayer* pTFPlayer, const char* pszTranslatedEnt, bool switchTo/* = false*/, bool removeExisting/* = true*/)
{
	CBaseEntity* pItem = ItemGeneration()->SpawnItem(pData->GetDefinitionIndex(), pTFPlayer->GetAbsOrigin(), vec3_angle, 57, AE_UNIQUE, pszTranslatedEnt/*, pTFPlayer->GetPlayerClass()->GetClassIndex()*/);
	if (pItem)
	{
		// If it's a weapon, remove the current one, and give us this one.
		CEconEntity* pEconEnt = static_cast<CEconEntity*>(pItem);
		if (!pEconEnt)
		{
			Warning("pEconEnt is null...?\n");
			return false;
		}

		//DevMsg("%s giving item, def slot is %d and class slot is %d\n", __FUNCTION__, pEconEnt->GetAttributeContainer()->GetItem()->GetStaticData()->GetDefaultLoadoutSlot(), pEconEnt->GetAttributeContainer()->GetItem()->GetStaticData()->GetLoadoutSlot(pTFPlayer->GetPlayerClass()->GetClassIndex()));

		int iItemSlot = pEconEnt->GetAttributeContainer()->GetItem()->GetStaticData()->GetLoadoutSlot(pTFPlayer->GetPlayerClass()->GetClassIndex());
		if (iItemSlot < 0)
			iItemSlot = pEconEnt->GetAttributeContainer()->GetItem()->GetStaticData()->GetDefaultLoadoutSlot();

		CBaseEntity* pExistingSlotEntity = pTFPlayer->GetEntityForLoadoutSlot(iItemSlot);

		if (removeExisting && pExistingSlotEntity)
		{
			if (pExistingSlotEntity->IsWearable())
			{
				CEconWearable* pWearable = pTFPlayer->GetWearable(iItemSlot);
				if (pWearable)
				{
					pTFPlayer->RemoveWearable(pWearable);
					DevMsg("removed existing wearable\n");
				}
			}
			else if (pExistingSlotEntity->IsBaseCombatWeapon())
			{
				CBaseCombatWeapon *pExistingWpn = dynamic_cast<CBaseCombatWeapon *>(pExistingSlotEntity);
				if (pExistingWpn)
				{
					pTFPlayer->Weapon_Detach(pExistingWpn);
					UTIL_Remove(pExistingWpn);
					DevMsg("deatched and removed existing combat weapon\n");
				}
			}
			else
			{
				UTIL_Remove(pExistingSlotEntity);
				DevMsg("removed existing something or other\n");
			}
		}

		// these need item ids
		// also has the (good!) side effect of sorta owning the given weapon
		CSteamID playerSteamID;
		pTFPlayer->GetSteamID(&playerSteamID);
		pEconEnt->GetAttributeContainer()->GetItem()->SetOverrideAccountID(playerSteamID.GetAccountID());

		DispatchSpawn(pItem);
		pEconEnt->GiveTo(pTFPlayer);

		pTFPlayer->PostInventoryApplication();

		if (switchTo)
		{
			CBaseCombatWeapon *pWpn = dynamic_cast<CBaseCombatWeapon *>(pItem);
			if (pWpn && pTFPlayer->Weapon_CanSwitchTo(pWpn))
				pTFPlayer->Weapon_Switch(pWpn);
		}

		return true;
	}
	
	return false;
}
bool CCFEconTools::GiveAttribute(CEconItemAttributeDefinition *pAttrib, CAttributeList *pAttrList, float flValue)
{
	if (pAttrib)
	{
		if (pAttrList)
		{
			pAttrList->SetRuntimeAttributeValue(pAttrib, flValue);
			return true;
		}
		return false;
	}

	return false;
}
void CCFEconTools::ListAttributes(CCrispyAttributeIterator *it)
{
	it->m_vecAttributes.Sort(&CCrispyAttributeIterator::SortComparator);

	FOR_EACH_VEC(it->m_vecAttributes, i)
	{
		CCrispyAttributeIterator::attrib_iterator_value_t* val = &it->m_vecAttributes[i];

		bool lyingLittleFloat = false;
		if (!val->m_strValue)
		{
			// some attributes are reported incorrectly, so let's check for common float values (1, -1, 0.5, -0.5)
			// should match 0x(3|B)F(0|8)00000
			if ((val->m_value & 0x3F000000) >= 0x3F000000)
				lyingLittleFloat = true;
		}

		if (val->m_strValue)
			Msg("    %s (id %d): %s\n", val->m_pAttrDef->GetDefinitionName(), val->m_pAttrDef->GetDefinitionIndex(), val->m_strValue->value().c_str());
		else if (val->m_pAttrDef->IsStoredAsFloat() || lyingLittleFloat)
			Msg("    %s (id %d): %f\n", val->m_pAttrDef->GetDefinitionName(), val->m_pAttrDef->GetDefinitionIndex(), *((float*)&val->m_value));
		else
			Msg("    %s (id %d): %d\n", val->m_pAttrDef->GetDefinitionName(), val->m_pAttrDef->GetDefinitionIndex(), val->m_value);
	}
}

void CCFEconTools::CC_GiveItem_f(const CCommand &args)
{
	CHECK_ARGC(args.ArgC(), >, 1);

	CUtlVector<CTFPlayer*> targets;

	GET_LOCAL_TFPLAYER(pTFPlayer, false);
	if (pTFPlayer)
		targets.AddToTail(pTFPlayer);

	if (args.ArgC() > 2)
		TFPLAYERS_BY_COMMANDARG(targets, args[2]);

	CEconItemDefinition* pData;

	char* end;
	int iIndex = V_strtoi64(args[1], &end, 10);

	if (args[1] == end)
		pData = ItemSystem()->GetStaticDataForItemByName(args[1]);
	else
		pData = ItemSystem()->GetStaticDataForItemByDefIndex(iIndex);

	if (!pData || pData->GetDefinitionIndex() < 0)
	{
		Warning("Failed to get item \"%s\", doesn't seem to exist?\n", args[1]);
		return;
	}

	bool removeExisting = true;
	if (args.ArgC() > 3)
		removeExisting = atoi(args[3]);

	const char* pszTranslatedEnt;
	if (args.ArgC() > 4)
		pszTranslatedEnt = args[4];
	else
		pszTranslatedEnt = TranslateWeaponEntForClass(pData->GetItemClass(), pTFPlayer->GetPlayerClass()->GetClassIndex());

	FOR_EACH_VEC( targets, i )
	{
		CTFPlayer* pTarget = targets[i];
		if (!pTarget)
			continue;

		if (CreateAndGiveItem(pData, pTarget, pszTranslatedEnt, true, removeExisting))
			Msg("Gave %s (index %d) to %s\n", pData->GetItemBaseName(), pData->GetDefinitionIndex(), pTarget->GetPlayerName());
		else
			Warning("Failed to create item %s\n", args[1]);
	}
}
void CCFEconTools::CC_DropWeapon_f(const CCommand &args)
{
	GET_LOCAL_TFPLAYER(pTFPlayer, true);
	CHECK_ARGC(args.ArgC(), >, 1);

	CEconItemDefinition* pData;

	char* end;
	int iIndex = V_strtoi64(args[1], &end, 10);

	if (args[1] == end)
		pData = ItemSystem()->GetStaticDataForItemByName(args[1]);
	else
		pData = ItemSystem()->GetStaticDataForItemByDefIndex(iIndex);

	if (!pData || pData->GetDefinitionIndex() < 0)
	{
		Warning("Failed to get item %s, doesn't seem to exist?\n", args[1]);
		return;
	}

	CTFWeaponBase *pActiveWeapon = pTFPlayer->GetActiveTFWeapon();
	if (!pActiveWeapon)
		return;

	Vector vecPackOrigin = pTFPlayer->GetAbsOrigin();
	QAngle vecPackAngles = vec3_angle;
	if (!pTFPlayer->CalculateAmmoPackPositionAndAngles(pActiveWeapon, vecPackOrigin, vecPackAngles))
		return;

	CBaseEntity* pItem = ItemGeneration()->SpawnItem(pData->GetDefinitionIndex(), vecPackOrigin, vecPackAngles, 57, AE_UNIQUE, NULL/*, pTFPlayer->GetPlayerClass()->GetClassIndex()*/);
	if (pItem)
	{
		CTFWeaponBase* pWeapon = static_cast<CTFWeaponBase*>(pItem);
		CEconItemView* pDroppedItem = static_cast<CEconEntity*>(pItem)->GetAttributeContainer()->GetItem();

		CTFDroppedWeapon* pDroppedWeapon = CTFDroppedWeapon::Create(pTFPlayer, vecPackOrigin, vecPackAngles, pWeapon->GetWorldModel(), pDroppedItem);
		if (pDroppedWeapon)
		{
			pDroppedWeapon->InitDroppedWeapon(pTFPlayer, pWeapon, false);
			Msg("Dropped a %s (index %d)\n", pData->GetItemBaseName(), pData->GetDefinitionIndex());
		}
	}
}
void CCFEconTools::CC_TryTaunt_f(const CCommand &args)
{
	CHECK_ARGC(args.ArgC(), >, 1);

	CUtlVector<CTFPlayer*> targets;

	GET_LOCAL_TFPLAYER(pTFPlayer, false);
	if (pTFPlayer)
		targets.AddToTail(pTFPlayer);

	if (args.ArgC() > 2 && sv_cheats->GetBool())
		TFPLAYERS_BY_COMMANDARG(targets, args[2]);

	CEconItemDefinition* pData;

	char* end;
	int iIndex = V_strtoi64(args[1], &end, 10);

	if (args[1] == end)
		pData = ItemSystem()->GetStaticDataForItemByName(args[1]);
	else
		pData = ItemSystem()->GetStaticDataForItemByDefIndex(iIndex);

	if (!pData || pData->GetDefinitionIndex() < 0)
	{
		Warning("Failed to get item \"%s\", doesn't seem to exist?\n", args[1]);
		return;
	}

	static CEconItemView item;
	item.SetItemDefIndex( pData->GetDefinitionIndex() );
	item.SetItemQuality( pData->GetQuality() );
	item.SetInitialized( true );

	FOR_EACH_VEC( targets, i )
	{
		CTFPlayer* pTarget = targets[i];
		if (!pTarget)
			continue;

		pTarget->PlayTauntSceneFromItem(&item);
	}
}

void CCFEconTools::CC_RemoveWeapons_f(const CCommand &args)
{
	CUtlVector<CTFPlayer*> targets;

	GET_LOCAL_TFPLAYER(pTFPlayer, false);
	if (pTFPlayer)
		targets.AddToTail(pTFPlayer);

	if (args.ArgC() > 1)
		TFPLAYERS_BY_COMMANDARG(targets, args[1]);

	FOR_EACH_VEC( targets, i )
	{
		CTFPlayer* pTarget = targets[i];
		if (!pTarget)
			continue;

		pTarget->RemoveAllObjects(true);
		pTarget->RemoveAllOwnedEntitiesFromWorld(true);
		pTarget->RemoveAllWeapons();
	}
}

void CCFEconTools::CC_AddWeaponAttribute_f(const CCommand &args)
{
	CHECK_ARGC(args.ArgC(), >, 2);

	CUtlVector<CTFPlayer*> targets;

	GET_LOCAL_TFPLAYER(pTFPlayer, false);
	if (pTFPlayer)
		targets.AddToTail(pTFPlayer);

	CEconItemAttributeDefinition *pAttrib;

	char* end;
	int iIndex = V_strtoi64(args[1], &end, 10);

	if (args[1] == end)
		pAttrib = ItemSystem()->GetStaticDataForAttributeByName(args[1]);
	else
		pAttrib = ItemSystem()->GetStaticDataForAttributeByDefIndex(iIndex);

	float flVal = V_atof(args[2]);

	if (!pAttrib->IsStoredAsFloat())
		*(int*)&flVal = (int)flVal; // convert to integer but store in float

	if (args.ArgC() > 3)
		TFPLAYERS_BY_COMMANDARG(targets, args[3]);

	FOR_EACH_VEC( targets, i )
	{
		CTFPlayer* pTarget = targets[i];
		if (!pTarget)
			return;

		CTFWeaponBase *pActiveWeapon = pTarget->GetActiveTFWeapon();
		if (!pActiveWeapon)
			return;

		if (GiveAttribute(pAttrib, pActiveWeapon->GetAttributeList(), flVal))
			Msg("%s's current weapon was given attribute \"%s\" (index %d) with value %f\n", pTarget->GetPlayerName(), pAttrib->GetDefinitionName(), pAttrib->GetDefinitionIndex(), flVal);
		else
			Warning("Invalid attribute %s!\n", args[1]);
	}
}
void CCFEconTools::CC_ClearWeaponAttributes_f(const CCommand &args)
{
	CUtlVector<CTFPlayer*> targets;

	GET_LOCAL_TFPLAYER(pTFPlayer, false);
	if (pTFPlayer)
		targets.AddToTail(pTFPlayer);

	if (args.ArgC() > 1)
		TFPLAYERS_BY_COMMANDARG(targets, args[1]);

	FOR_EACH_VEC(targets, i)
	{
		CTFPlayer* pTarget = targets[i];
		if (!pTarget)
			return;

		CTFWeaponBase *pActiveWeapon = pTarget->GetActiveTFWeapon();
		if (!pActiveWeapon)
			return;

		pActiveWeapon->GetAttributeList()->DestroyAllAttributes();

		Msg("Removed all runtime attributes from %s's current weapon\n", pTarget->GetPlayerName());
	}
}
void CCFEconTools::CC_ListWeaponAttributes_f(const CCommand& args)
{
	CUtlVector<CTFPlayer*> targets;

	GET_LOCAL_TFPLAYER(pTFPlayer, false);
	if (pTFPlayer)
		targets.AddToTail(pTFPlayer);

	if (args.ArgC() > 1)
		TFPLAYERS_BY_COMMANDARG(targets, args[1]);

	FOR_EACH_VEC(targets, i)
	{
		CTFPlayer* pTarget = targets[i];
		if (!pTarget)
			return;

		CTFWeaponBase* pActiveWeapon = pTarget->GetActiveTFWeapon();
		if (!pActiveWeapon)
			return;

		CEconItemView* pItemView = pActiveWeapon->GetAttributeContainer()->GetItem();
		if (!pItemView)
			return;

		const GameItemDefinition_t* pItemDef = pItemView->GetItemDefinition();

		Msg("%s is holding \"%s\" (id %d):\n", pTarget->GetPlayerName(), pItemDef->GetDefinitionName(), pItemDef->GetDefinitionIndex());

		CCrispyAttributeIterator it;
		pItemView->IterateAttributes(&it);
		CCFEconTools::ListAttributes(&it);
	}
}

void CCFEconTools::CC_AddPlayerAttribute_f(const CCommand &args)
{
	CHECK_ARGC(args.ArgC(), >, 2);

	CUtlVector<CTFPlayer*> targets;

	GET_LOCAL_TFPLAYER(pTFPlayer, false);
	if (pTFPlayer)
		targets.AddToTail(pTFPlayer);

	CEconItemAttributeDefinition *pAttrib;

	char* end;
	int iIndex = V_strtoi64(args[1], &end, 10);

	if (args[1] == end)
		pAttrib = ItemSystem()->GetStaticDataForAttributeByName(args[1]);
	else
		pAttrib = ItemSystem()->GetStaticDataForAttributeByDefIndex(iIndex);

	float flVal = V_atof(args[2]);
	float flDuration = -2.0f; // -2.0 is the uninitialized signal
	if (args.ArgC() > 3)
		flDuration = V_atof(args[3]);

	if (args.ArgC() > 4)
		TFPLAYERS_BY_COMMANDARG(targets, args[4]);

	FOR_EACH_VEC( targets, i )
	{
		CTFPlayer* pTarget = targets[i];
		if (!pTarget)
			return;

		CTFWeaponBase *pActiveWeapon = pTarget->GetActiveTFWeapon();
		if (!pActiveWeapon)
			return;

		// tf_player has this I guess?
		// good for duration, bad for no error checking
		if (flDuration != -2.0f)
			pTFPlayer->AddCustomAttribute(pAttrib->GetDefinitionName(), flVal, flDuration);
		else
		{
			if (GiveAttribute(pAttrib, pTarget->GetAttributeList(), flVal))
				Msg("%s was given attribute \"%s\" (index %d) with value %f\n", pTarget->GetPlayerName(), pAttrib->GetDefinitionName(), pAttrib->GetDefinitionIndex(), flVal);
			else
				Warning("Invalid attribute %s!\n", args[1]);
		}
	}
}
void CCFEconTools::CC_ClearPlayerAttributes_f(const CCommand &args)
{
	CUtlVector<CTFPlayer*> targets;

	GET_LOCAL_TFPLAYER(pTFPlayer, false);
	if (pTFPlayer)
		targets.AddToTail(pTFPlayer);

	if (args.ArgC() > 1)
		TFPLAYERS_BY_COMMANDARG(targets, args[1]);

	FOR_EACH_VEC(targets, i)
	{
		CTFPlayer* pTarget = targets[i];
		if (!pTarget)
			return;

		pTarget->GetAttributeList()->DestroyAllAttributes();

		Msg("Removed all runtime attributes from %s\n", pTarget->GetPlayerName());
	}
}
void CCFEconTools::CC_ListPlayerAttributes_f(const CCommand& args)
{
	CUtlVector<CTFPlayer*> targets;

	GET_LOCAL_TFPLAYER(pTFPlayer, false);
	if (pTFPlayer)
		targets.AddToTail(pTFPlayer);

	if (args.ArgC() > 1)
		TFPLAYERS_BY_COMMANDARG(targets, args[1]);

	FOR_EACH_VEC(targets, i)
	{
		CTFPlayer* pTarget = targets[i];
		if (!pTarget)
			return;

		Msg("%s has the following attributes:\n", pTarget->GetPlayerName());

		CCrispyAttributeIterator it;
		pTarget->GetAttributeList()->IterateAttributes(&it);
		CCFEconTools::ListAttributes(&it);
	}
}

void CCFEconTools::CC_GenerateDefinition_f(const CCommand &args)
{
	GET_LOCAL_TFPLAYER(pTFPlayer, true);

	CTFWeaponBase *pActiveWeapon = pTFPlayer->GetActiveTFWeapon();
	if (!pActiveWeapon)
		return;

	CEconItemView* pEconView = pActiveWeapon->GetAttributeContainer()->GetItem();
	CEconItemDefinition* pData = ItemSystem()->GetStaticDataForItemByDefIndex(pActiveWeapon->GetAttributeContainer()->GetItem()->GetItemDefinition()->GetDefinitionIndex());
	const GameItemDefinition_t* pDef = pEconView->GetItemDefinition();

	KeyValues* kv = new KeyValues("crispyweapon");
	kv->SetString("name", "The Code To Get Weapon Names (check playerstatus)");
	kv->SetString("first_sale_date", "2007/10/10"); // tf2 release date

	kv->SetString("item_class", pData->GetItemClass());
	kv->SetString("item_name", pData->GetItemBaseName());
	kv->SetString("item_type_name", pData->GetItemTypeName());
	kv->SetString("propername", pData->HasProperName() ? "0" : "1");
	kv->SetString("item_slot", GetItemSchema()->GetLoadoutStrings(EQUIP_TYPE_CLASS)[pDef->GetDefaultLoadoutSlot()]);
	kv->SetString("item_quality", "normal");

	KeyValues* kvAttribs = new KeyValues("attributes");
	kv->AddSubKey(kvAttribs);
	
	for (int i = 0; i < pActiveWeapon->GetAttributeList()->GetNumAttributes(); i++)
	{
		CEconItemAttribute* pAttribute = pActiveWeapon->GetAttributeList()->GetAttribute(i);

		KeyValues* kvAttribute = new KeyValues(pAttribute->GetStaticData()->GetDefinitionName());
		kv->AddSubKey(kvAttribs);

		kvAttribute->SetString("attribute_class", pAttribute->GetStaticData()->GetAttributeClass());
		kvAttribute->SetFloat("value", pActiveWeapon->GetAttributeList()->GetRuntimeAttributeValue(pAttribute->GetStaticData()));
	}

	CKeyValuesDumpContextAsDevMsg dump(0);
	if (kv->Dump(&dump))
	{
		const char* szText = "";
		dump.KvWriteText(szText);
	}

	kvAttribs->deleteThis();
	kv->deleteThis();
}
void CCFEconTools::CC_CountLoadoutSlots_f(const CCommand &args)
{
	const char* textLoadoutPositions[]
	{
		"LOADOUT_POSITION_PRIMARY",
		"LOADOUT_POSITION_SECONDARY",
		"LOADOUT_POSITION_MELEE",
		"LOADOUT_POSITION_UTILITY",
		"LOADOUT_POSITION_BUILDING",
		"LOADOUT_POSITION_PDA",
		"LOADOUT_POSITION_PDA2",

		// Wearables. If you add new wearable slots, make sure you add them to IsWearableSlot() below this.
		"LOADOUT_POSITION_HEAD",
		"LOADOUT_POSITION_MISC",

		// other
		"LOADOUT_POSITION_ACTION",

		// More wearables, yay!
		"LOADOUT_POSITION_MISC2",

		// taunts
		"LOADOUT_POSITION_TAUNT",
		"LOADOUT_POSITION_TAUNT2",
		"LOADOUT_POSITION_TAUNT3",
		"LOADOUT_POSITION_TAUNT4",
		"LOADOUT_POSITION_TAUNT5",
		"LOADOUT_POSITION_TAUNT6",
		"LOADOUT_POSITION_TAUNT7",
		"LOADOUT_POSITION_TAUNT8",
	};

	int counts[CLASS_LOADOUT_POSITION_COUNT];
	memset(&counts[0], 0, sizeof(counts));

	const CEconItemSchema::ItemDefinitionMap_t& mapItemDefs = ItemSystem()->GetItemSchema()->GetItemDefinitionMap();
	FOR_EACH_MAP_FAST( mapItemDefs, i )
	{
		const CTFItemDefinition *pItemDef = dynamic_cast< const CTFItemDefinition * >( mapItemDefs[i] );

		if ( pItemDef )
		{
			loadout_positions_t defaultSlot = (loadout_positions_t)pItemDef->GetDefaultLoadoutSlot();

			if (defaultSlot >= 0 && defaultSlot < CLASS_LOADOUT_POSITION_COUNT)
				counts[defaultSlot]++;

			//if (i < 60)
			//	Warning("adding %s to %s\n", pItemDef->GetDefinitionName(), textLoadoutPositions[defaultSlot]);
		}
	}

	for (int i = 0; i < sizeof(counts)/sizeof(int); i++)
	{
		Msg("%-30s: %d item(s)\n", textLoadoutPositions[i], counts[i]);
	}
}

CF_LINK_PLUGIN(CCFEconTools, "EconTools");