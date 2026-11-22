#include "code_08009A0.h"
/* This call site consumes the complete return register, as in the ROM. */
extern s32 Collision_GetGroundOffset(s32, s32);
#include "structs/ewram.h"
extern s32 RandomSigned(s32);
extern s32 Entity_IsInRect(struct EwramData_EntityData *, s32, s32, s32, s32);

/* Ballistic fragment: bounce once against room geometry, then fall away. */
static inline s32 FragmentHitsGround(struct EwramData_EntityData *entity)
{
    s32 x = (s16)entity->posX.part16.integer;
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    s32 y;
    x += (s16)bg->xPos.part16.integer;
    y = entity->posY.part16.integer;
    y += (s16)bg->yPos.part16.integer;
    return Collision_GetGroundOffset(x, y + 4);
}
void MinotaurAxePiece_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
    case 0:
        entity->velX.whole = RandomSigned(0x20000) - 0x8000;
        if (entity->flipX)
            entity->velX.whole = -entity->velX.whole;
        entity->velY.whole = -0x10000 - ((RandomNumberGenerator() & 0x1FFF) << 4);
        entity->state++;
        break;
    case 1:
        if (FragmentHitsGround(entity))
        {
            entity->animFrame = (RandomNumberGenerator() & 1) + 64;
            entity->velY.whole = -(entity->velY.whole >> 1);
            entity->state++;
        }
        else
        {
            entity->posX.whole += entity->velX.whole;
            entity->posY.whole += entity->velY.whole;
            entity->velY.whole += 0x3000;
        }
        break;
    case 2:
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
        entity->velY.whole += 0x3000;
        break;
    }
    if (!Entity_IsInRect(entity, -16, -16, 272, 192))
        entity->deleteRequested = 1;
}
