/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   AluraUneAttack_DamagePlayerOnContact = Object28_OnPlayerContact
 *   AluraUne_LoadGraphics = Object25_LoadGraphics
 *   AluraUne_DamagePlayerOnContact = Object28_OnPlayerContact */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern u8 sAluraUneGfx[];
extern u8 sAluraUnePalette[];
extern u8 sAluraUneSpriteSheet[];
extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);

/**
 * @brief 7CEF0 | Same as Object28_OnPlayerContact.
 */
void AluraUneAttack_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 0;
    contactDamage.unused = 0;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief 7CF20 | Same as Object25_LoadGraphics.
 */
u8 AluraUne_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sAluraUneGfx, 0);

    if (gfx < 0)
        return 0;
    entity->unk_510.part8.basePalette = ObjPalette_FindOrLoadVariant(sAluraUnePalette, 0);
    Sprite_Attach(entity, sAluraUneSpriteSheet, gfx, entity->unk_510.part8.basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 7CF6C | Same as Object28_OnPlayerContact.
 */
void AluraUne_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 0;
    contactDamage.unused = 0;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}
