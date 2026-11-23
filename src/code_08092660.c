/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   ZombieOfficerPiece_Update = ZombieSoldierPiece_Update
 *   ZombieOfficer_LoadGraphics = Object25_LoadGraphics
 *   ZombieOfficer_DamagePlayerOnContact = Enemy_DamagePlayerOnContact */
#include "code_08009A0.h"
#include "zombie_entity.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

extern void EntityAccelerateAndMove(struct EwramData_EntityData *entity, s32 maxSpeed);
extern u8 sZombieOfficerGfx[];
extern u8 sZombieOfficerPalette[];
extern u8 sZombieOfficerSpriteSheet[];
extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);

static inline void Entity_DamagePlayerOnContact(struct EwramData_EntityData *entity, u16 attributes, u16 unused, u16 reaction)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = attributes;
    contactDamage.unused = unused;
    contactDamage.reaction = reaction;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief 92660 | Same as ZombieSoldierPiece_Update.
 */
void ZombieOfficerPiece_Update(struct EwramData_EntityData *entity)
{
    EntityAccelerateAndMove(entity, 0x80000);
    if (entity->posY.whole > ZOMBIE_PIECE_FLOOR_Y(entity))
        EntityDelete(entity);
}

/**
 * @brief 92680 | Same as Object25_LoadGraphics.
 */
u8 ZombieOfficer_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sZombieOfficerGfx, 0);

    if (gfx < 0)
        return 0;
    entity->unk_510.part8.basePalette = ObjPalette_FindOrLoadVariant(sZombieOfficerPalette, 0);
    Sprite_Attach(entity, sZombieOfficerSpriteSheet, gfx, entity->unk_510.part8.basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 926CC | Same as Enemy_DamagePlayerOnContact.
 */
void ZombieOfficer_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}
