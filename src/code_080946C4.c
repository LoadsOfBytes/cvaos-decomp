/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Succubus_LoadGraphics = Object25_LoadGraphics
 * plus Succubus_DamagePlayerOnContact (decompiled locally). */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern u8 sLilithSuccubusGfx[];
extern u8 sLilithSuccubusPalette[];
extern u8 sLilithSuccubusSpriteSheet[];
extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);

/**
 * @brief 946C4 | Same as Object25_LoadGraphics.
 */
u8 Succubus_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sLilithSuccubusGfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sLilithSuccubusPalette, 0);
    Sprite_Attach(entity, sLilithSuccubusSpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 94710 | When not lunging, Player_ApplyContactDamage with (attack, 0, 0, 0)
 * at the entity position; otherwise grab the player (grabbed = holding = 1,
 * escape presses = 0).
 */
void Succubus_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    if ((s8)entity->ext.succubus.lunging == 0)
    {
        contactDamage.attack = entity->attack;
        contactDamage.attributes = 0;
        contactDamage.unused = 0;
        contactDamage.reaction = 0;
        contactDamage.sourceX = entity->posX.whole;
        contactDamage.sourceY = entity->posY.whole;
        Player_ApplyContactDamage(&contactDamage);
    }
    else
    {
        entity->ext.succubus.grabbed = 1;
        entity->ext.succubus.holding = 1;
        entity->ext.succubus.escapePresses = 0;
    }
}
