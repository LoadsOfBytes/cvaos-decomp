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
    if ((owner->unk_4FC.part8.unk_4FE & 1) || (owner->unk_4FC.part8.unk_4FE & 2))
        i = 0;
    else
        i = 1;
    for (; i <= 6; i++)
    {
        child = EntityCreateInRange(0x49, 0xCF, (u32 *)Effect_UpdateSpriteDebris);
        if (child != NULL)
        {
            child->unk_4F8.part8.unk_4F8 = owner->unk_4F4.part8.unk_4F4;
            child->unk_4F4.part8.unk_4F5 = i;
            owner->unk_4FC.part8.unk_4FC++;
            child->spriteLayer = owner->spriteLayer;
            child->scrollWithBg = 1;
            graphics = Sprite_GetTileSlot(owner);
            Sprite_Attach(child, sSkeletonSpriteSheet, graphics, ObjPalette_FindOrLoad(sSkeletonWaiterSkeletonPalette));
            child->drawFunc = (u32 *)Sprite_DrawEntity;
            child->posX.whole = owner->posX.whole;
            child->posY.whole = owner->posY.whole;
            x = sSkeletonDeathPieceOffsets[child->unk_4F4.part8.unk_4F5][0];
            child->posY.part16.integer += sSkeletonDeathPieceOffsets[child->unk_4F4.part8.unk_4F5][1];
            child->animFrame = sSkeletonDeathPieceOffsets[child->unk_4F4.part8.unk_4F5][2];
            child->velX.whole = (u8)sSkeletonDeathPieceOffsets[child->unk_4F4.part8.unk_4F5][3] << 12;
            child->velY.whole = -(sSkeletonDeathPieceOffsets[child->unk_4F4.part8.unk_4F5][4] << 12) - 16;
            child->accelY.whole = 0x3000;
            child->timer = sSkeletonDeathPieceOffsets[child->unk_4F4.part8.unk_4F5][5];
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

