#include "code_08009A0.h"
#include "legion_entity.h"
#include "code_08013960.h"
#include "code_08033CAC.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

struct LegionData {
    u32 unk_0;
    u32 rotationAngle;
    s32 angularVelocity;
    s32 roomX;
    s32 roomY;
    s32 bobPhase;
    s32 shellActionTimer;
};

extern void *Heap_Alloc(u16 size);
extern void Heap_Free(void *data);
extern s32 Legion_LoadGraphics(struct EwramData_EntityData *entity);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *entity);
extern s32 Legion_CreateCorePart(struct EwramData_EntityData *entity, s32 x, s32 y, s32 index);
extern s32 Legion_CreateCorpse(struct EwramData_EntityData *entity, s32 x, s32 y, s32 index);
extern void Legion_DamagePlayerOnContact(struct EwramData_EntityData *entity);
extern void Legion_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
extern u8 sLegionAnimScript0[];

/**
 * @brief 9C6BC | Legion create: with unk_514 set it is deleted once beaten
 * (bossEventFlags bit 5) unless in the boss rush mode; a 0x1C-byte state block in
 * unk_4F8, animation sLegionAnimScript0 with its own palette and identity
 * affine matrix, four core parts Legion_CreateCorePart (part 3 instead of 10 when
 * unk_514 is set and time runs); frozen time gives state 3, otherwise box
 * 0x2020F0F0 (boxes off) and song 0x10; without unk_514 the HP bar
 * (Player_SetSelectedTarget), with it five shell parts Legion_CreateCorpse; then unk_506 = 6
 * and the block starts at the room position with a random 120..247 timer.
 */
void EnemyLegionCreate(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    u32 box = 0x2020F0F0;
    struct LegionData *data;
    u16 palette;
    u8 frozen;

    if (entity->unk_514.part16.spawnParam0 != 0 && !gEwramData->gameState.bossRushMode
        && (gEwramData->gameState.bossEventFlags & 0x20))
        goto delete;
    LEGION_DATA(entity) = (s32)Heap_Alloc(0x1C);
    if (LEGION_DATA(entity) == 0)
        goto delete;
    if ((u8)Legion_LoadGraphics(entity) == 0)
    {
        if (LEGION_DATA(entity) != 0)
            Heap_Free((void *)LEGION_DATA(entity));
        goto delete;
    }
    Anim_Start(entity, (struct AnimScript *)sLegionAnimScript0, 2, 1);
    Anim_Update(entity);
    palette = ObjAffine_Alloc(1);
    if ((s16)palette < 0)
        goto delete;
    entity->affineIndex = palette;
    entity->affine = 1;
    ObjAffine_Set(0, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
    Enemy_ResetFromTemplate(entity);
    data = (struct LegionData *)LEGION_DATA(entity);
    if (!Legion_CreateCorePart(entity, -0x180000, 0x180000, 2))
        goto delete;
    if (gEwramData->gameState.bestiaryPreview || entity->unk_514.part16.spawnParam0 == 0)
    {
        if (!Legion_CreateCorePart(entity, 0x170000, 0x180000, 10))
        {
            EntityDelete(entity);
            return;
        }
    }
    else
    {
        if (!Legion_CreateCorePart(entity, 0x170000, 0x180000, 3))
            goto delete;
    }
    if (!Legion_CreateCorePart(entity, 0x180000, -0x180000, 1))
        goto delete;
    if (!Legion_CreateCorePart(entity, -0x180000, -0x180000, 0))
        goto delete;
    frozen = *((u8 *)gEwramData + 0x8E) & 0x40;
    if (frozen)
    {
        entity->state = 3;
        return;
    }
    Hit_Enable(entity, 8, Legion_DamagePlayerOnContact, (s32)Legion_OnHit);
    HitBox_SetBody(entity, &box);
    HitBox_SetAttack(entity, &box);
    entity->hit.flags &= 0xFB;
    entity->hit.flags &= 0xFD;
    Music_SetCurrent(0x10);
    if (entity->unk_514.part16.spawnParam0 == 0)
    {
        LEGION_FLAGS(entity) = 6;
        data->roomX = entity->posX.whole + bg->xPos.whole;
        data->roomY = entity->posY.whole + bg->yPos.whole;
        data->bobPhase = 0;
        data->angularVelocity = 0x100;
        data->shellActionTimer = (RandomNumberGenerator() & 0x7F) + 0x78;
        entity->state = 0;
        entity->subState = 1;
        Player_SetSelectedTarget(entity);
        gEwramData->gameState.stateFlags |= 2;
        return;
    }
    gEwramData->gameState.stateFlags |= 2;
    if (!Legion_CreateCorpse(entity, 0xA80000, 0x1BF0000, 2))
        goto delete;
    if (!Legion_CreateCorpse(entity, 0xD80000, 0x1BF0000, 0))
        goto delete;
    if (!Legion_CreateCorpse(entity, 0x1400000, 0x1BF0000, 1))
        goto delete;
    if (!Legion_CreateCorpse(entity, 0xBC0000, 0x1BF0000, 4))
        goto delete;
    if (!Legion_CreateCorpse(entity, 0x1190000, 0x1BF0000, 3))
    {
    delete:
        EntityDelete(entity);
        return;
    }
    LEGION_FLAGS(entity) = 6;
    data->roomX = entity->posX.whole + bg->xPos.whole;
    data->roomY = entity->posY.whole + bg->yPos.whole;
    data->bobPhase = 0;
    data->angularVelocity = 0x100;
    data->shellActionTimer = (RandomNumberGenerator() & 0x7F) + 0x78;
    entity->state = 0;
}
