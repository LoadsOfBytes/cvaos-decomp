#include "code_08009A0.h"
#include "manticore_entity.h"
#include "code_08013960.h"
#include "code_08033CAC.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern s32 Manticore_LoadGraphics(struct EwramData_EntityData *entity);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *entity);
extern void Entity_SnapToGround(struct EwramData_EntityData *entity);
extern void Entity_EnableBoxes(struct EwramData_EntityData *entity, void *onHit, void *onHurt);
extern struct EwramData_EntityData *Manticore_CreateTailTip(struct EwramData_EntityData *entity);
extern struct EwramData_EntityData *Manticore_CreateTailSegment(struct EwramData_EntityData *entity, struct EwramData_EntityData *tail, s32 value, s32 index);
extern s32 Manticore_CreateOverlay(struct EwramData_EntityData *entity);
extern void Manticore_DamagePlayerOnContact(struct EwramData_EntityData *entity);
extern void Manticore_OnHit(struct EwramData_EntityData *entity);

/**
 * @brief 72854 | Manticore create: the boss version (unk_514) is deleted
 * once beaten (bossEventFlags bit 10) unless in the boss rush mode bossRushMode,
 * where it gets +1000 HP and +20 to attack; palette 0x11; the tail head
 * Manticore_CreateTailTip (unk_4F8) and six segments Manticore_CreateTailSegment linked through
 * unk_504; frozen time gives state 3, otherwise any failed allocation
 * deletes it. The boss version starts song 0x10 and the boss HP bar
 * (Player_SetSelectedTarget).
 */
void EnemyManticoreCreate(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *seg[6];
    s32 i;

    if (entity->unk_514.part16.spawnParam0 != 0 && !gEwramData->gameState.bossRushMode
        && (gEwramData->gameState.bossEventFlags & 0x400))
        goto delete;
    if ((u8)Manticore_LoadGraphics(entity) == 0)
        goto delete;
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer = 0x11;
    if (gEwramData->gameState.bossRushMode)
    {
        entity->unk_518.part16.hp += 1000;
        entity->attack += 20;
    }
    if (gEwramData->gameState.bestiaryPreview)
    {
        MANTICORE_TAIL_TIP(entity) = Manticore_CreateTailTip(entity);
        for (i = 0; i <= 5; i++)
            seg[i] = Manticore_CreateTailSegment(entity, MANTICORE_TAIL_TIP(entity), 0, i);
        for (i = 0; i <= 4; i++)
            MANTICORE_TAIL_NEXT(seg[i]) = seg[i + 1];
        MANTICORE_TAIL_NEXT(seg[5]) = MANTICORE_TAIL_TIP(entity);
        Manticore_CreateOverlay(entity);
        entity->state = 3;
        return;
    }
    Entity_EnableBoxes(entity, Manticore_DamagePlayerOnContact, Manticore_OnHit);
    Entity_SnapToGround(entity);
    MANTICORE_TAIL_TIP(entity) = Manticore_CreateTailTip(entity);
    if (MANTICORE_TAIL_TIP(entity) == NULL)
        goto delete;
    for (i = 0; i <= 5; i++)
    {
        seg[i] = Manticore_CreateTailSegment(entity, MANTICORE_TAIL_TIP(entity), 0, i);
        if (seg[i] == NULL)
            goto delete;
    }
    for (i = 0; i <= 4; i++)
        MANTICORE_TAIL_NEXT(seg[i]) = seg[i + 1];
    MANTICORE_TAIL_NEXT(seg[5]) = MANTICORE_TAIL_TIP(entity);
    if (Manticore_CreateOverlay(entity) == 0)
    {
    delete:
        EntityDelete(entity);
        return;
    }
    entity->state = 0;
    if (entity->unk_514.part16.spawnParam0 != 0)
    {
        Music_SetCurrent(0x10);
        Player_SetSelectedTarget(entity);
        gEwramData->gameState.stateFlags |= 2;
    }
}
