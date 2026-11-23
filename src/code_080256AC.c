#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
struct EffectGraphicsChoice { u8 pad[12]; u8 graphicsIndex; u8 pad_D[11]; };
struct GraphicsSet { void *graphics, *palette, *sprite; };
struct EffectPoint { s16 x, y; };
extern struct EffectGraphicsChoice sUnk_084F12C8[];
extern struct GraphicsSet sSkillGraphicsSets[];
extern struct EwramData_EntityData *RedSoul_CreateEntity(struct EwramData_EntityData *, u32 *);
extern void BoneSoulEffect_Update(struct EwramData_EntityData *);
extern void Sprite_GetPartCornerOffset(struct EwramData_EntityData *, s32, s16 *);
/**
 * @brief Create a red-soul effect (animation 5, update BoneSoulEffect_Update)
 * at the bone projectile.
 */
void BoneSoul_SpawnFlyingEffect(struct EwramData_EntityData *owner)
{
    struct EffectGraphicsChoice *info = &sUnk_084F12C8[(u16)owner->ext.redSoul.param];
    struct EwramData_EntityData *entity = RedSoul_CreateEntity(owner, (u32 *)BoneSoulEffect_Update);
    struct EffectPoint point;
    if (entity != NULL)
    {
        Sprite_GetPartCornerOffset(owner, 1, &point.x);
        Sprite_Attach(entity, sSkillGraphicsSets[info->graphicsIndex].sprite, gEwramData->playerState.redSoulGfx, 2);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 13;
        entity->flipX = owner->flipX;
        entity->posX.part16.integer = owner->posX.part16.integer + point.x / 2;
        entity->posY.part16.integer = owner->posY.part16.integer + point.y - 4;
        Anim_StartIndexed(entity, 5, 0, 1);
        Anim_Update(entity);
        entity->timer = 10;
    }
}
