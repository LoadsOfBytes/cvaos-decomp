/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Biphron_LoadGraphics = Object25_LoadGraphics
 *   Biphron_DamagePlayerOnContact = Enemy_DamagePlayerOnContact */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern s32 Enemy_PreUpdate(struct EwramData_EntityData *entity);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *entity);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *entity);
extern void Biphron_UpdateWalk(struct EwramData_EntityData *entity);
extern void Biphron_UpdateAttack(struct EwramData_EntityData *entity);
extern void Biphron_UpdateDeath(struct EwramData_EntityData *entity);
extern u8 sBiphronGfx[];
extern u8 sBiphronPalette[];
extern u8 sBiphronSpriteSheet[];
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
 * @brief 9A3D8 | Same as Object25_LoadGraphics.
 */
u8 Biphron_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sBiphronGfx, 0);

    if (gfx < 0)
        return 0;
    entity->unk_510.part8.basePalette = ObjPalette_FindOrLoadVariant(sBiphronPalette, 0);
    Sprite_Attach(entity, sBiphronSpriteSheet, gfx, entity->unk_510.part8.basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 9A424 | Same as Enemy_DamagePlayerOnContact.
 */
void Biphron_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}

/**
 * @brief 9A450 | Biphron update: state 0..2 run Biphron_UpdateWalk / Biphron_UpdateAttack /
 * Biphron_UpdateDeath then movement (Entity_AnimateWithBoxes, Enemy_CheckHitPlayer); state 3 only
 * Anim_Update. Nothing while Enemy_PreUpdate reports the entity inactive. */
void EnemyBiphronUpdate(struct EwramData_EntityData *entity)
{
    u8 unused[0x30]; /* stack frame reserved by the original */

    if (Enemy_PreUpdate(entity))
        goto finished;
    switch (entity->state)
    {
        case 0:
            Biphron_UpdateWalk(entity);
            break;
        case 1:
            Biphron_UpdateAttack(entity);
            break;
        case 2:
            Biphron_UpdateDeath(entity);
            break;
        case 3:
            Anim_Update(entity);
            goto finished;
    }
    Entity_AnimateWithBoxes(entity);
    Enemy_CheckHitPlayer(entity);
finished:
    return;
}
