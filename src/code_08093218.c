#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern s32 Target_IsInRect(s16, s16, s16, s16);
extern s32 Enemy_IsDamaged(struct EwramData_EntityData *);
extern s32 Entity_MoveWithCollision(struct EwramData_EntityData *, s32, s32, s32, s32);
extern s32 Entity_AdjustSpeedForSlope(struct EwramData_EntityData *entity, s32 speed);
extern s32 Entity_GetHorizontalRoomEdge(struct EwramData_EntityData *, s32);
extern s32 Collision_IsWalkBlocked(s32 x, s32 y);

/**
 * @brief 93218 | Patrol AI: walks at 0x8000 (Entity_AdjustSpeedForSlope) while the
 * animation is outside frames 4-7 (stands still in them unless animTimer),
 * plays sound 0x85 on frame 4 and turns around on walls, ledges
 * (Collision_IsWalkBlocked one pixel ahead) or Entity_GetHorizontalRoomEdge; state 1 (after
 * Enemy_IsDamaged) always faces the player. In frame 1 with the player in the
 * 0x64 x 0x46 box it attacks (state = 1) with a 3/4 chance.
 */
void Curly_UpdateWalk(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    s32 res;
    s32 x;
    s32 y;
    u8 anim;

    switch (entity->subState)
    {
    case 0:
        if (Enemy_IsDamaged(entity))
        {
            entity->subState++;
            break;
        }
        if ((u8)(entity->animStep - 4) > 3)
        {
            if (!entity->animTimer)
            {
                if (entity->flipX)
                    entity->velX.whole = Entity_AdjustSpeedForSlope(entity, 0x8000);
                else
                    entity->velX.whole = Entity_AdjustSpeedForSlope(entity, -0x8000);
                if ((anim = entity->animStep) == 1
                    && Target_IsInRect(entity->posX.part16.integer - 0x32, entity->posY.part16.integer - 0x40, 0x64, 0x46)
                    && (RandomNumberGenerator() & 0x7F) <= 0x5F)
                {
                    entity->state = anim;
                    entity->subState = 0;
                    entity->timer = 0;
                    Anim_StartIndexed(entity, 1, 0, 0);
                    break;
                }
            }
        }
        else if (!entity->animTimer)
        {
            entity->velX.whole = 0;
        }
        if (*(u16 *)&entity->animStep == 4)
            PlaySong(0x85);
        res = Entity_MoveWithCollision(entity, 0x10000, -0x200000, 0x80000, 8);
        if (!(res & 4))
            break;
        x = (s16)entity->posX.part16.integer + (s16)bg->xPos.part16.integer;
        y = (s16)entity->posY.part16.integer + (s16)bg->yPos.part16.integer;
        if (entity->flipX)
            x++;
        else
            x--;
        if ((res & 3) || Collision_IsWalkBlocked(x, y) || Entity_GetHorizontalRoomEdge(entity, 1))
        {
            entity->flipX ^= 1;
            if (entity->flipX)
                entity->velX.whole = Entity_AdjustSpeedForSlope(entity, 0x8000);
            else
                entity->velX.whole = Entity_AdjustSpeedForSlope(entity, -0x8000);
        }
        break;
    case 1:
        if ((u8)(entity->animStep - 4) > 3)
        {
            if (!entity->animTimer)
            {
                entity->flipX = Entity_IsTargetRight(entity);
                if (entity->flipX)
                    entity->velX.whole = Entity_AdjustSpeedForSlope(entity, 0x8000);
                else
                    entity->velX.whole = Entity_AdjustSpeedForSlope(entity, -0x8000);
                if ((anim = entity->animStep) == 1
                    && Target_IsInRect(entity->posX.part16.integer - 0x32, entity->posY.part16.integer - 0x40, 0x64, 0x46)
                    && (RandomNumberGenerator() & 0x7F) <= 0x5F)
                {
                    entity->state = anim;
                    entity->subState = 0;
                    entity->timer = 0;
                    Anim_StartIndexed(entity, 1, 0, 0);
                    break;
                }
            }
        }
        else if (!entity->animTimer)
        {
            entity->velX.whole = 0;
        }
        if (*(u16 *)&entity->animStep == 4)
            PlaySong(0x85);
        Entity_MoveWithCollision(entity, 0x10000, -0x200000, 0x80000, 8);
        break;
    }
}
