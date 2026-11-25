#ifndef ENTITY_SLOTS_CONSTANTS_H
#define ENTITY_SLOTS_CONSTANTS_H

/*
 * Slot ranges of gEwramData->entityData[] used with EntityCreateInRange
 * (first free slot from FIRST to LAST inclusive). Names follow what the
 * callers create in each range.
 */

#define ENTITY_SLOT_PLAYER              0x00 /* Player_Create (sPlayerUpdateFuncs) */
#define ENTITY_SLOT_LINK_PARTNER        0x01 /* LinkPartner_Update (range PLAYER..LINK_PARTNER) */
#define ENTITY_SLOT_WEAPON_FIRST        0x02 /* weapons, whips, souls attached to the player */
#define ENTITY_SLOT_WEAPON_LAST         0x10
#define ENTITY_SLOT_PLAYER_ICON         0x11 /* subweapon icon, yellow soul, link partner marker */
#define ENTITY_SLOT_HUD_FIRST           0x16 /* HP number, status icons */
#define ENTITY_SLOT_HUD_LAST            0x19
#define ENTITY_SLOT_BOSS_RUSH_TIMER     0x1A
#define ENTITY_SLOT_OBJECT_FIRST        0x1B /* room enemies/objects (RoomEntity_Spawn) and their attacks (Entity_CreateObject) */
#define ENTITY_SLOT_OBJECT_LAST         0x48
#define ENTITY_SLOT_EFFECT_FIRST        0x49 /* particles, afterimages, debris and other effects */
#define ENTITY_SLOT_EFFECT_LAST         0xCF
#define ENTITY_SLOT_RESERVED_FIRST      0xD0 /* updated even while paused (Entity_UpdateReserved): menu icons, bestiary preview objects */
#define ENTITY_SLOT_RESERVED_LAST       0xDF
#define ENTITY_SLOT_COUNT               0xE0

#endif /* ENTITY_SLOTS_CONSTANTS_H */
