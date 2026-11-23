/* Beam Skeleton: seven fragments on death. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void Effect_UpdateSpriteDebris(struct EwramData_EntityData *entity);
extern s8 ObjPalette_FindOrLoad(void *palette);
extern s32 Entity_GetTargetDirection(struct EwramData_EntityData *entity);
extern u8 sBeamSkeletonPalette[];
extern u8 sBeamSkeletonSpriteSheet[];
extern s8 sBeamSkeletonDeathPieceOffsets[][6];
#define ABS(x) ((x) < 0 ? -(x) : (x))
#define INDEX(e) (e->ext.beamSkeletonPiece.index)

extern void BeamSkeleton_SpawnDeathPieces(struct EwramData_EntityData *entity);
extern void Enemy_DropRewards(struct EwramData_EntityData *entity);

/**
 * @brief Death state (0xFF): spawn the seven pieces once, drop rewards,
 * restore the palette, free the owner affine slot and request deletion.
 */
void BeamSkeleton_UpdateDeath(struct EwramData_EntityData *entity)
{
    entity->hidden = 1;
    switch (entity->subState)
    {
        case 0:
            BeamSkeleton_SpawnDeathPieces(entity);
            Enemy_DropRewards(entity);
            entity->palette = entity->unk_510.part8.basePalette;
            entity->timer = 50;
            entity->state = 0xFF;
            entity->subState++;
        case 1:
            ObjAffine_Free(entity->affineIndex);
            entity->deleteRequested = 1;
            break;
    }
}

/**
 * @brief Hide the owner and spawn up to seven table-configured sprite
 * fragments, counting them in the owner field and selecting direction
 * relative to the target.
 */
void BeamSkeleton_SpawnDeathPieces(struct EwramData_EntityData *entity)
{
    s32 i;
    entity->hidden = 1;
    for (i = 0; i < 7; i++)
    {
        struct EwramData_EntityData *part = EntityCreateInRange(0x49,0xCF,(u32 *)Effect_UpdateSpriteDebris);
        if (part != NULL)
        {
            s32 graphics;
            s32 x;
            part->ext.beamSkeletonPiece.ownerIndex = ENTITY_INDEX(entity);
            INDEX(part) = i;
            entity->ext.beamSkeleton.deathPieces++;
            part->spriteLayer = entity->spriteLayer;
            part->scrollWithBg = 1;
            graphics = Sprite_GetTileSlot(entity);
            Sprite_Attach(part,sBeamSkeletonSpriteSheet,graphics,ObjPalette_FindOrLoad(sBeamSkeletonPalette));
            part->drawFunc = (u32 *)Sprite_DrawEntity;
            part->posX.whole = entity->posX.whole;
            part->posY.whole = entity->posY.whole;
            x = sBeamSkeletonDeathPieceOffsets[INDEX(part)][0];
            part->posY.part16.integer += sBeamSkeletonDeathPieceOffsets[INDEX(part)][1];
            part->animFrame = sBeamSkeletonDeathPieceOffsets[INDEX(part)][2];
            part->velX.whole = (u8)sBeamSkeletonDeathPieceOffsets[INDEX(part)][3] << 12;
            part->velY.whole = -(sBeamSkeletonDeathPieceOffsets[INDEX(part)][4] << 12) - 16;
            part->accelY.whole = 0x3000;
            part->timer = sBeamSkeletonDeathPieceOffsets[INDEX(part)][5];
            part->flipX = entity->flipX;
            if ((s8)Entity_GetTargetDirection(entity) > 0)
            {
                part->velX.whole = -ABS(part->velX.whole);
                x = -ABS(x);
            }
            part->posX.part16.integer += x;
        }
    }
}
