/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Triton_LoadGraphics = Object25_LoadGraphics
 *   Triton_DamagePlayerOnContact = Enemy_DamagePlayerOnContact
 *   TritonEffect_Update = GolemIdleEffect_Update */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern u8 sTritonGfx[];
extern u8 sTritonPalette[];
extern u8 sTritonSpriteSheet[];
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
 * @brief 87D24 | Same as Object25_LoadGraphics.
 */
u8 Triton_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sTritonGfx, 0);

    if (gfx < 0)
        return 0;
    entity->unk_510.part8.basePalette = ObjPalette_FindOrLoadVariant(sTritonPalette, 0);
    Sprite_Attach(entity, sTritonSpriteSheet, gfx, entity->unk_510.part8.basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 87D70 | Same as Enemy_DamagePlayerOnContact.
 */
void Triton_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}

/**
 * @brief 87D9C | Same as GolemIdleEffect_Update.
 */
void TritonEffect_Update(struct EwramData_EntityData *entity)
{
    Anim_Update(entity);
    if (entity->animDone)
        EntityDelete(entity);
}
