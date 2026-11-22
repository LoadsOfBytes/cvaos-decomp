#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern s8 Entity_GetTargetDirection(struct EwramData_EntityData *);
extern u8 sSkeletonSpriteSheet[], sSkeletonWaiterSkeletonPalette[];

extern const s8 sSkeletonDeathPieceOffsets[][6];

static inline u8 BelowScreen(struct EwramData_EntityData *entity)
{
    s32 y = entity->posY.part16.integer; return y > 170;
}

/**
 * @brief Unreferenced per-piece update that sets up a Skeleton death piece from
 * sSkeletonDeathPieceOffsets (the table of Skeleton_SpawnDeathPieces) on the owner's
 * graphics and lets it fly.
 */
void SkeletonDeathPiece_UpdateUnused(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = &gEwramData->entityData[(u8)entity->unk_4F8.part8.unk_4F8];
    s32 x, graphics, reason, below;
    switch (entity->state)
    {
    case 0:
        entity->spriteLayer = owner->spriteLayer;
        entity->scrollWithBg = 1;
    {
        u8 *sprite = sSkeletonSpriteSheet;
        graphics = Sprite_GetTileSlot(owner);
        Sprite_Attach(entity, sprite, graphics, ObjPalette_FindOrLoad(sSkeletonWaiterSkeletonPalette));
    }
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->posX.whole = owner->posX.whole;
        entity->posY.whole = owner->posY.whole;
        x = sSkeletonDeathPieceOffsets[entity->unk_4F4.part8.unk_4F5][0];
        entity->posY.part16.integer += sSkeletonDeathPieceOffsets[entity->unk_4F4.part8.unk_4F5][1];
        entity->posY.part16.integer += 16;
        entity->animFrame = sSkeletonDeathPieceOffsets[entity->unk_4F4.part8.unk_4F5][2];
        entity->velX.whole = (u8)sSkeletonDeathPieceOffsets[entity->unk_4F4.part8.unk_4F5][3] << 12;
        entity->velY.whole = -(sSkeletonDeathPieceOffsets[entity->unk_4F4.part8.unk_4F5][4] << 12) - 16;
        entity->timer = sSkeletonDeathPieceOffsets[entity->unk_4F4.part8.unk_4F5][5];
        entity->flipX = owner->flipX;
        if (Entity_GetTargetDirection(owner) > 0)
        {
            entity->velX.whole = -(entity->velX.whole < 0 ? -entity->velX.whole : entity->velX.whole);
            x = -(x < 0 ? -x : x);
        }
        entity->posX.part16.integer += x;
        entity->state++;
        break;
    case 1:
        reason = --entity->timer == 0;
        below = BelowScreen(entity);
        if (below) reason = 2;
        if (reason != 0)
        {
            owner->unk_4FC.part8.unk_4FC--;
            entity->deleteRequested = 1;
        }
        break;
    }
    entity->posX.whole += entity->velX.whole;
    entity->posY.whole += entity->velY.whole;
    entity->velY.whole += 0x3000;
}



