/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Quezlcoatl_LoadGraphics = Object32_LoadGraphics
 *   Quezlcoatl_DamagePlayerOnContact = Enemy_DamagePlayerOnContact
 *   QuezlcoatlSegment_DamagePlayerOnContact = Enemy_DamagePlayerOnContact */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

extern u8 sQuezlcoatlWhiteDragonGfx[];
extern u8 sQuezlcoatlWhiteDragonPalette[];
extern u8 sQuezlcoatlWhiteDragonSpriteSheet[];
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
 * @brief 786B0 | Same as Object32_LoadGraphics.
 */
u8 Quezlcoatl_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sQuezlcoatlWhiteDragonGfx, 0);

    if (gfx < 0)
        return 0;
    entity->unk_510.part8.basePalette = ObjPalette_FindOrLoadVariant(sQuezlcoatlWhiteDragonPalette, 1);
    Sprite_Attach(entity, sQuezlcoatlWhiteDragonSpriteSheet, gfx, entity->unk_510.part8.basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 786FC | Same as Enemy_DamagePlayerOnContact.
 */
void Quezlcoatl_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}

/**
 * @brief 78728 | Same as Enemy_DamagePlayerOnContact.
 */
void QuezlcoatlSegment_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}
