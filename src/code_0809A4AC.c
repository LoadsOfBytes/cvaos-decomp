#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *entity);
extern s32 Entity_IsTimeStopped(struct EwramData_EntityData *entity);
extern s32 Entity_GetVisibilityPattern(s32 step);
extern void Effect_SpawnRandom(s32 x, s32 y, s32 w, s32 h, s32 layer);
extern void Heap_Free(void *parts);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *entity);
extern void Stolas_UpdateMove(struct EwramData_EntityData *entity);
extern void Stolas_UpdateAttack(struct EwramData_EntityData *entity);
extern void Stolas_SpawnDebris(struct EwramData_EntityData *entity, s32 x, s32 y, s32 vx, s32 vy);
extern void Stolas_UpdatePreview(struct EwramData_EntityData *entity);

struct StolasPreviewAnimation {
    u16 stepCount;
    u16 type;
    u8 frame;
    u8 duration;
    u16 unk_6;
};

struct StolasBox {
    s32 x:8;
    s32 y:8;
    u32 w:8;
    u32 h:8;
};

extern const struct StolasPreviewAnimation sStolasPreviewAnimation;

void EnemyStolasUpdate(struct EwramData_EntityData *entity)
{
    u8 unused[48];
    struct StolasBox box;
    struct StolasPreviewAnimation anim = sStolasPreviewAnimation;
    s32 *parts;
    /* The signed address comparison below preserves the original ble. */
    s32 *cursor;
    struct EwramData_EntityData **slot;
    struct EwramData_EntityData *child;

    entity->unk_4FC.part8.unk_4FC = 0;
    parts = (s32 *)entity->unk_500.jointParts;
    entity->unk_508.part16.unk_50A = 3;
    cursor = parts;
    slot = (struct EwramData_EntityData **)(parts + 0x48);
    do {
        child = *slot;
        if (child != NULL && child->updateFunc != NULL && child->unk_518.part8.enemyId == cursor[0x4B]
            && (s16)entity->unk_518.part16.hp > 0)
            entity->unk_508.part16.unk_50A--;
        else
            *slot = NULL;
        cursor++;
        slot++;
    } while ((s32)cursor <= (s32)(parts + 2));
    if (entity->state != 2)
    {
        if (Entity_IsInRect(entity, -16, 0, 272, 176))
            Player_TrackEntity(entity);
        if (entity->state != 2)
            Enemy_UpdateHitFlash(entity);
    }
    if (Entity_IsTimeStopped(entity) && entity->state != 2)
        goto boxes;
    switch (entity->state)
    {
    case 0:
        Stolas_UpdateMove(entity);
        break;
    case 1:
        Stolas_UpdateAttack(entity);
        break;
    case 2:
        if ((entity->timer & 3) == 0)
            Effect_SpawnRandom(entity->posX.whole, entity->posY.whole - 0x260000, 0x100000, 0xC0000, entity->spriteLayer - 1);
        if ((entity->timer & 7) == 0)
        {
            s32 vx = RandomNumberGenerator() % 0x60000 - 0x30000;
            Stolas_SpawnDebris(entity, 0, -0x1D0000, vx, RandomNumberGenerator() % 0x60000 - 0x30000);
        }
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
        if ((s8)--entity->timer <= 0)
            entity->deleteRequested = 1;
        if (entity->deleteRequested && entity->unk_500.jointParts != NULL)
        {
            /* Reuse the allocation variable after the loop. In agbcc this
             * also preserves the distinct base and cursor registers. */
            parts = (s32 *)entity->unk_500.jointParts;
            Heap_Free(parts);
        }
        Anim_Update(entity);
        return;
    case 3:
        Stolas_UpdatePreview(entity);
        Anim_Update(entity);
        return;
    }
boxes:
    box.x = 0;
    box.y = -0x48;
    box.w = 0x0A;
    box.h = 0x48;
    HitBox_SetAttack(entity, &box);
    HitBox_SetBody(entity, &box);
    if (entity->unk_4FC.part8.unk_4FC & 1)
    {
        Anim_Start(entity, (struct AnimScript *)&anim, 0, 1);
        anim.frame = entity->unk_4FC.part8.unk_4FD;
        Anim_Update(entity);
    }
    else
        Anim_Update(entity);
    Enemy_CheckHitPlayer(entity);
}
