/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   ZombiePiece_Update = ZombieSoldierPiece_Update
 *   Zombie_LoadGraphics = Object25_LoadGraphics
 *   Zombie_DamagePlayerOnContact = Enemy_DamagePlayerOnContact */
#include "code_08009A0.h"
#include "zombie_entity.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

extern void EntityAccelerateAndMove(struct EwramData_EntityData *entity, s32 maxSpeed);
extern u8 sZombieGfx[];
extern u8 sZombiePalette[];
extern u8 sZombieSpriteSheet[];
extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);

static inline void Entity_DamagePlayerOnContact(struct EwramData_EntityData *entity, u16 attributes, u16 param_2, u16 reaction)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = attributes;
    contactDamage.unk_4 = param_2;
    contactDamage.reaction = reaction;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief 7B3E4 | Same as ZombieSoldierPiece_Update.
 */
void ZombiePiece_Update(struct EwramData_EntityData *entity)
{
    EntityAccelerateAndMove(entity, 0x80000);
    if (entity->posY.whole > ZOMBIE_PIECE_FLOOR_Y(entity))
        EntityDelete(entity);
}

/**
 * @brief 7B404 | Same as Object25_LoadGraphics.
 */
u8 Zombie_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sZombieGfx, 0);

    if (gfx < 0)
        return 0;
    entity->unk_510.part8.basePalette = ObjPalette_FindOrLoadVariant(sZombiePalette, 0);
    Sprite_Attach(entity, sZombieSpriteSheet, gfx, entity->unk_510.part8.basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 7B450 | Same as Enemy_DamagePlayerOnContact.
 */
void Zombie_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}
