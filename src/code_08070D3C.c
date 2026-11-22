/* Initialize Merman's hidden, waiting or jumping entry mode. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
extern u8 Merman_LoadGraphics(struct EwramData_EntityData *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void Entity_SnapToGround(struct EwramData_EntityData *);
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern void Entity_EnableBoxes(struct EwramData_EntityData *, void *, void *);
extern void Enemy_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void Merman_OnHit(struct EwramData_EntityData *);
void EnemyMermanCreate(struct EwramData_EntityData *entity)
{
    struct BgLayerState *background = &gEwramData->bgInfo[1];
    if (!Merman_LoadGraphics(entity))
    {
        EntityDelete(entity);
        goto finished;
    }
    Anim_StartIndexed(entity, 4, 0, 1);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 4;
        goto finished;
    }
    Entity_EnableBoxes(entity, Enemy_DamagePlayerOnContact, Merman_OnHit);
    {
        s32 mode = entity->unk_514.part16.spawnParam1;
        if (mode != 0)
        {
            entity->state = 3;
            entity->hidden = 1;
            entity->hit.flags |= 4;
            entity->hit.flags |= 2;
            entity->unk_500.part16.unk_500 = entity->posX.part16.integer + background->xPos.part16.integer;
            entity->unk_500.part16.unk_502 = entity->posY.part16.integer + background->yPos.part16.integer;
        }
        else
        {
            entity->state = mode;
            entity->timer = 60;
            if (entity->unk_514.part16.spawnParam0 == 0)
            {
                Entity_SnapToGround(entity);
                entity->subState = 10;
                entity->accelX.whole = mode;
                entity->velY.whole = 0x20000;
                entity->accelY.whole = 0x1800;
                entity->unk_4F8.part8.unk_4F8 = mode;
                entity->unk_4F8.part8.unk_4F9 = mode;
                entity->unk_4F8.part8.unk_4FA = mode;
                entity->unk_4F8.part8.unk_4FB = mode;
                Anim_StartIndexed(entity, 2, 0, 1);
                entity->flipX = Entity_IsTargetRight(entity);
                if (entity->flipX) entity->velX.whole = 0x8000;
                else entity->velX.whole = -0x8000;
                entity->timer = 0;
                entity->unk_504.part8.unk_504 = 1;
            }
            else entity->unk_504.part8.unk_504 = mode;
        }
    }
finished:
    return;
}
