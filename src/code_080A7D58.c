/* Sky Fish: normal movement and death sequence. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void EntityMove(struct EwramData_EntityData *);
extern void EntityAccelerateAndMove(struct EwramData_EntityData *, s32);
extern s32 Entity_GetVisibilityPattern(s32);
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern s32 EntityIsOutsideScreen(struct EwramData_EntityData *, s32);
extern void SkyFish_SpawnTrail(struct EwramData_EntityData *, s32, s32, s32, s32);
extern void SkyFish_SpawnRisingTrail(struct EwramData_EntityData *, s32, s32, s32, s32);
extern struct AnimScript sSkyFishFlyAnimScript;

/* Fast traversal; SkillChronomageUse sets and decrements the global 0x4BE
 * countdown. While active it switches this fish to a slower animation/movement
 * state, rather than simply stopping its updates. */
void SkyFish_UpdateFly(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        if ((s8)entity->unk_4FC.part8.unk_4FD != 0)
            entity->posX.whole = 0x1000000;
        else
            entity->posX.whole = -0x100000;
        entity->posY.whole = (RandomNumberGenerator() % 0x200000) + 0x200000;
        entity->flipX = Entity_IsTargetRight(entity);
        if (gEwramData->gameState.timeStopTimer != 0)
            entity->deleteRequested = 1;
        else if (entity->unk_4FC.part16.unk_4FE[0]-- <= 1)
        {
            entity->subState++;
            entity->hidden = 0;
            if (entity->flipX)
                entity->velX.whole = 0x1A0000;
            else
                entity->velX.whole = -0x1A0000;
        }
        break;
    case 1:
        {
            s32 x = entity->posX.whole + (RandomNumberGenerator() & 0xFFFFF) - 0x80000;
            s32 y = entity->posY.whole + (RandomNumberGenerator() & 0xFFFFF) - 0x80000;
            EntityMove(entity);
            SkyFish_SpawnTrail(entity, x, y, entity->spriteLayer + 1, 1);
            if (gEwramData->gameState.timeStopTimer != 0)
            {
                entity->subState++;
                entity->unk_4FC.part8.unk_4FC = 0;
                Anim_StartIndexed(entity, 0, 0, 1);
                if (entity->flipX)
                    entity->velX.whole = 0x10000;
                else
                    entity->velX.whole = -0x10000;
            }
            else if (EntityIsOutsideScreen(entity, 6))
                entity->deleteRequested = 1;
        }
        break;
    case 2:
        EntityMove(entity);
        if (gEwramData->gameState.timeStopTimer == 0)
        {
            Anim_Start(entity, &sSkyFishFlyAnimScript, 0, 0);
            entity->subState = 1;
            entity->unk_4FC.part8.unk_4FC = 1;
            if (entity->flipX)
                entity->velX.whole = 0x1A0000;
            else
                entity->velX.whole = -0x1A0000;
        }
        break;
    }
}

/* Fall, blink, emit a small effect every eight frames, then end. */
void SkyFish_UpdateDeath(struct EwramData_EntityData *entity)
{
    if (entity->subState == 0)
    {
        entity->subState++;
        entity->velX.whole = 0;
        entity->velY.whole = 0;
        entity->accelX.whole = 0;
        entity->accelY.whole = 0x2000;
    }
    EntityAccelerateAndMove(entity, 0x40000);
    entity->hidden = Entity_GetVisibilityPattern(entity->timer);
    if ((entity->timer & 7) == 0)
    {
        s32 x = entity->posX.whole + (RandomNumberGenerator() % 0x80000) - 0x40000;
        s32 y = entity->posY.whole + (RandomNumberGenerator() % 0x100000) - 0x80000;
        SkyFish_SpawnRisingTrail(entity, x, y, entity->spriteLayer + 1, 0);
    }
    if ((s8)--entity->timer <= 0)
        entity->deleteRequested = 1;
}
