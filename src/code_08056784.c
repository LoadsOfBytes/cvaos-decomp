/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Object21Update = Object28Update
 *   Object21_LoadGraphics = Object32_LoadGraphics
 *   Object21_OnPlayerContact = DevilPart_DamagePlayerOnContact */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "code/code_08040A38.h"
#include "code_080D73B8.h"

extern void Object21_UpdateState(struct EwramData_EntityData *entity);
extern void Hit_CheckWithPlayer(void *res, struct EwramData_EntityData *b);
extern u8 sObject21Object23Gfx[];
extern u8 sUnk_08209FD8[];
extern u8 sObject21Object23SpriteSheet[];
extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);

/**
 * @brief 56784 | Same as Object28Update.
 */
void Object21Update(struct EwramData_EntityData *entity)
{
    u32 collision[4];

    if (entity->state == 0)
        Object21_UpdateState(entity);
    Anim_Update(entity);
    Hit_CheckWithPlayer(collision, entity);
}

/**
 * @brief 567AC | Same as Object32_LoadGraphics.
 */
u8 Object21_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sObject21Object23Gfx, 0);

    if (gfx < 0)
        return 0;
    entity->unk_510.part8.basePalette = ObjPalette_FindOrLoadVariant(sUnk_08209FD8, 1);
    Sprite_Attach(entity, sObject21Object23SpriteSheet, gfx, entity->unk_510.part8.basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 567F8 | Same as DevilPart_DamagePlayerOnContact.
 */
void Object21_OnPlayerContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 1;
    contactDamage.unk_4 = 0;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}
