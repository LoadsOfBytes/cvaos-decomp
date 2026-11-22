/* Death burst and create of the enemy at 0x080BB664..0x080BB7E3 (named
 * EnemyKillerMantleCreate upstream). */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern void PlaySong(u16 song);
extern struct EwramData_EntityData *Effect_SpawnShared(s32 x, s32 y, s32 script, s32 palette);
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *entity, u8 *resource, void *palette, u8 variant, void *sheet);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *entity);
extern s32 Hit_EnableContact(struct EwramData_EntityData *entity, s32 type, void *onHit, s32 onHurt);
extern s32 HitBox_SetAttackRaw(struct EwramData_EntityData *entity, s32 box);
extern s32 HitBox_SetBodyRaw(struct EwramData_EntityData *entity, s32 box);
extern struct EwramData_EntityData *Entity_CreateObject(u32 *updateFunc);
extern void Enemy_OnContactMask1(struct EwramData_EntityData *entity);
extern void KillerMantle_OnHit(struct EwramData_EntityData *param_0, s32 param_1, u16 *param_2);
extern void KillerMantlePart_Update(struct EwramData_EntityData *entity);

/* Graphics of this enemy (EnemyKillerMantleCreate). */
extern u8 sKillerMantleGfx[];
extern u8 sKillerMantlePalette[];
extern u8 sKillerMantleSpriteSheet[];

/**
 * @brief BB664 | Death burst: sprite 6 and song 0x6D, then every fourth
 * frame a fragment (Effect_SpawnShared, layer 0x14, mirrored vertically) flying
 * in a random direction and slowing down by 1/24 of its speed. Returns 1
 * once the animation ended.
 */
s32 KillerMantle_UpdateDeath(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *effect;
    s16 angle;
    s32 timer;

    switch (entity->subState)
    {
        case 0:
            Anim_StartIndexed(entity, 6, 0, 0);
            entity->subState = 1;
            PlaySong(0x6D);
        case 1:
            timer = entity->timer + 1;
            entity->timer = timer;
            if (!(timer & 3))
            {
                effect = Effect_SpawnShared(entity->posX.whole, entity->posY.whole, 22, 8);
                if (effect != NULL)
                {
                    angle = RandomNumberGenerator();
                    effect->flipY = 1;
                    effect->velX.whole = Sin(angle + 0x4000);
                    effect->velY.whole = Sin(angle);
                    effect->accelX.whole = effect->velX.whole / 24;
                    effect->accelY.whole = effect->velY.whole / 24;
                    effect->spriteLayer = 0x14;
                }
            }
            if (entity->animFlags & 4)
                return 1;
            break;
    }
    return 0;
}

/**
 * @brief BB70C | Create: load the graphics (delete the enemy when that
 * fails), hit boxes, layer 0x13, sprite 0; state 4 when bestiaryPreview is set,
 * otherwise box callbacks (Enemy_OnContactMask1 / KillerMantle_OnHit), boxes, its part
 * KillerMantlePart_Update and state 0.
 */
void EnemyKillerMantleCreate(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child;

    if (!Entity_LoadGraphics(entity, sKillerMantleGfx, sKillerMantlePalette, 0, sKillerMantleSpriteSheet))
    {
        EntityDelete(entity);
        return;
    }
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer = 0x13;
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 4;
        return;
    }
    Hit_EnableContact(entity, 8, Enemy_OnContactMask1, (s32)KillerMantle_OnHit);
    HitBox_SetBodyRaw(entity, 0x1010F8F8);
    HitBox_SetAttackRaw(entity, 0x1414F6EC);
    entity->hit.flags |= 2;
    child = Entity_CreateObject((u32 *)KillerMantlePart_Update);
    if (child != NULL)
    {
        child->scrollWithBg = 1;
        child->unk_4F8.unk_4F8_entity = entity;
    }
    entity->scrollWithBg = 1;
    entity->state = 0;
}
