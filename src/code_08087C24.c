/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   TritonSpear_DamagePlayerOnContact = DevilPart_DamagePlayerOnContact */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

extern void TritonEffect_Update(struct EwramData_EntityData *);
extern u8 sTritonGfx[];
extern u8 sTritonSpriteSheet[];

/**
 * @brief 87C24 | Same as DevilPart_DamagePlayerOnContact.
 */
void TritonSpear_DamagePlayerOnContact(struct EwramData_EntityData *entity)
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

/* Homing shot contact: Player_ApplyContactDamage at the position and set
 * timer to 0xFA. */
void TritonHomingShot_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage effect;
    effect.attack = entity->attack;
    effect.attributes = 0;
    effect.unk_4 = 0;
    effect.reaction = 0;
    effect.sourceX = entity->posX.whole;
    effect.sourceY = entity->posY.whole;
    entity->timer = 0xFA;
    Player_ApplyContactDamage(&effect);
}
/* Only set timer to 0xFA (callbacks registered with TritonHomingShot_DamagePlayerOnContact). */
void TritonHomingShot_OnHit(struct EwramData_EntityData *entity)
{
    entity->timer = 0xFA;
}
/* Create the effect entity TritonEffect_Update at the owner + (x, y), one layer
 * below, facing the same way. */
void TritonSpear_SpawnEffect(struct EwramData_EntityData *owner, s32 x, s32 y)
{
    struct EwramData_EntityData *effect = EntityCreateInRange(0x49, 0xCF, (u32 *)TritonEffect_Update);
    if (effect != NULL)
    {
        s32 graphics;
        effect->posX.whole = owner->posX.whole + x;
        effect->posY.whole = owner->posY.whole + y;
        effect->spriteLayer = owner->spriteLayer - 1;
        graphics = ObjGfx_Find(sTritonGfx);
        Sprite_Attach(effect, sTritonSpriteSheet, graphics, 6);
        Anim_StartIndexed(effect, 9, 0, 0);
        Anim_Update(effect);
        effect->flipX = owner->flipX;
        effect->scrollWithBg = 1;
        effect->drawFunc = (u32 *)Sprite_DrawEntity;
    }
}
