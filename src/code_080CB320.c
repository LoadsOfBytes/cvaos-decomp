/* Alternate Beam Skeleton fragment update. */
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s8 ObjPalette_FindOrLoad(void *palette);
extern s32 Entity_GetTargetDirection(struct EwramData_EntityData *entity);
extern u8 sBeamSkeletonPalette[];
extern u8 sBeamSkeletonSpriteSheet[];
extern s8 sBeamSkeletonDeathPieceOffsets[][6];
#define ABS(x) ((x) < 0 ? -(x) : (x))
#define INDEX(e) ((e)->unk_4F4.part8.unk_4F5)
static inline s32 TooLow(s32 y) { return y > 170; }

/**
 * @brief Unreferenced per-piece update that sets up a Beam Skeleton death piece
 * from sBeamSkeletonDeathPieceOffsets (the table of BeamSkeleton_UpdateDeath) on the
 * owner's graphics and lets it fly.
 */
void BeamSkeletonDeathPiece_UpdateUnused(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = &gEwramData->entityData[entity->unk_4F4.part8.unk_4F6];
    switch (entity->state)
    {
        case 0:
        {
            s32 graphics;
            s32 x;
            u16 *y;
            void *sprite;
            entity->spriteLayer = owner->spriteLayer;
            entity->scrollWithBg = 1;
            sprite = sBeamSkeletonSpriteSheet;
            graphics = Sprite_GetTileSlot(owner);
            Sprite_Attach(entity,sprite,graphics,ObjPalette_FindOrLoad(sBeamSkeletonPalette));
            entity->drawFunc = (u32 *)Sprite_DrawEntity;
            entity->posX.whole = owner->posX.whole;
            entity->posY.whole = owner->posY.whole;
            x = sBeamSkeletonDeathPieceOffsets[INDEX(entity)][0];
            y = &entity->posY.part16.integer;
            *y = sBeamSkeletonDeathPieceOffsets[INDEX(entity)][1] + *y + 16;
            entity->animFrame = sBeamSkeletonDeathPieceOffsets[INDEX(entity)][2];
            entity->velX.whole = (u8)sBeamSkeletonDeathPieceOffsets[INDEX(entity)][3] << 12;
            entity->velY.whole = -(sBeamSkeletonDeathPieceOffsets[INDEX(entity)][4] << 12) - 16;
            entity->timer = sBeamSkeletonDeathPieceOffsets[INDEX(entity)][5];
            entity->flipX = owner->flipX;
            if ((s8)Entity_GetTargetDirection(owner) > 0)
            {
                entity->velX.whole = -ABS(entity->velX.whole);
                x = -ABS(x);
            }
            entity->posX.part16.integer += x;
            entity->state++;
            break;
        }
        case 1:
        {
            s32 done = 0;
            if (--entity->timer == 0)
                done = 1;
            if (TooLow((s16)entity->posY.part16.integer))
                done = 2;
            if (done)
            {
                owner->unk_4FC.part8.unk_4FC--;
                entity->deleteRequested = 1;
            }
            break;
        }
    }
    entity->posX.whole += entity->velX.whole;
    entity->posY.whole += entity->velY.whole;
    entity->velY.whole += 0x3000;
}
