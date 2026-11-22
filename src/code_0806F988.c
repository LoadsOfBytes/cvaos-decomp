/* Initialize Killer Fish, choosing its ground or submerged setup. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
extern u8 KillerFish_LoadGraphics(struct EwramData_EntityData *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void Entity_SnapToGround(struct EwramData_EntityData *);
extern void Entity_EnableBoxes(struct EwramData_EntityData *, void *, void *);
extern void KillerFish_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void KillerFish_OnHit(struct EwramData_EntityData *);
extern void KillerFish_SpawnSwarmFish(struct EwramData_EntityData *);
extern s32 Tile_Classify(s16, s16);
static inline void FillParts(struct EwramData_EntityData *entity)
{
    s32 index;
    for (index = 0; index < (s16)entity->unk_514.part16.spawnParam1; index++)
        KillerFish_SpawnSwarmFish(entity);
}
void EnemyKillerFishCreate(struct EwramData_EntityData *entity)
{
    struct BgLayerState *background = &gEwramData->bgInfo[1];
    if (!KillerFish_LoadGraphics(entity))
    {
        EntityDelete(entity);
        goto finished;
    }
    Anim_StartIndexed(entity, 1, 0, 1);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    if (gEwramData->gameState.bestiaryPreview)
        entity->state = 5;
    else
    {
        Entity_EnableBoxes(entity, KillerFish_DamagePlayerOnContact, KillerFish_OnHit);
        if (Tile_Classify((s16)(background->xPos.part16.integer + entity->posX.part16.integer),
                          (s16)(background->yPos.part16.integer + entity->posY.part16.integer)) != 3)
        {
            Entity_SnapToGround(entity);
            entity->subState = 99;
            entity->flipY = 1;
            entity->posY.part16.integer -= 8;
        }
        else
        {
            FillParts(entity);
        }
        entity->state = 0;
    }
finished:
    return;
}

