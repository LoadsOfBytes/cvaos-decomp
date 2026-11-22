/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   KillerFish_LoadGraphics = Object25_LoadGraphics
 *   KillerFish_DamagePlayerOnContact = Enemy_DamagePlayerOnContact */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern u8 sKillerFishGfx[];
extern u8 sKillerFishPalette[];
extern u8 sKillerFishSpriteSheet[];
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
 * @brief 70AC4 | Same as Object25_LoadGraphics.
 */
u8 KillerFish_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sKillerFishGfx, 0);

    if (gfx < 0)
        return 0;
    entity->unk_510.part8.basePalette = ObjPalette_FindOrLoadVariant(sKillerFishPalette, 0);
    Sprite_Attach(entity, sKillerFishSpriteSheet, gfx, entity->unk_510.part8.basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 70B10 | Same as Enemy_DamagePlayerOnContact.
 */
void KillerFish_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}
