#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern s8 Entity_GetTargetDirection(struct EwramData_EntityData *);
extern void Effect_UpdateSpriteDebris(struct EwramData_EntityData *);
extern u8 sSkeletonSpriteSheet[], sSkeletonWaiterSkeletonPalette[];
extern const s8 sSkeletonDeathPieceOffsets[][6];

/* Hide the owner and spawn up to seven debris pieces from its sprite. */
/**
 * @brief Hide the owner and create up to seven table-configured sprite pieces, tracking each in the owner count.
 */
void Skeleton_SpawnDeathPieces(struct EwramData_EntityData *owner)
{
    s32 i, x, graphics;
    struct EwramData_EntityData *child;
    owner->hidden = 1;
    if ((owner->ext.skeleton.flags & 1) || (owner->ext.skeleton.flags & 2))
        i = 0;
    else
        i = 1;
    for (; i <= 6; i++)
    {
        child = EntityCreateInRange(0x49, 0xCF, (u32 *)Effect_UpdateSpriteDebris);
        if (child != NULL)
        {
            child->ext.skeletonPiece.ownerIndex = ENTITY_INDEX(owner);
            child->ext.skeletonPiece.index = i;
            owner->ext.skeletonDeath.pieces++;
            child->spriteLayer = owner->spriteLayer;
            child->scrollWithBg = 1;
            graphics = Sprite_GetTileSlot(owner);
            Sprite_Attach(child, sSkeletonSpriteSheet, graphics, ObjPalette_FindOrLoad(sSkeletonWaiterSkeletonPalette));
            child->drawFunc = (u32 *)Sprite_DrawEntity;
            child->posX.whole = owner->posX.whole;
            child->posY.whole = owner->posY.whole;
            x = sSkeletonDeathPieceOffsets[child->ext.skeletonPiece.index][0];
            child->posY.part16.integer += sSkeletonDeathPieceOffsets[child->ext.skeletonPiece.index][1];
            child->animFrame = sSkeletonDeathPieceOffsets[child->ext.skeletonPiece.index][2];
            child->velX.whole = (u8)sSkeletonDeathPieceOffsets[child->ext.skeletonPiece.index][3] << 12;
            child->velY.whole = -(sSkeletonDeathPieceOffsets[child->ext.skeletonPiece.index][4] << 12) - 16;
            child->accelY.whole = 0x3000;
            child->timer = sSkeletonDeathPieceOffsets[child->ext.skeletonPiece.index][5];
            child->flipX = owner->flipX;
            if (Entity_GetTargetDirection(owner) > 0)
            {
                child->velX.whole = -(child->velX.whole < 0 ? -child->velX.whole : child->velX.whole);
                x = -(x < 0 ? -x : x);
            }
            child->posX.part16.integer += x;
        }
    }
}

