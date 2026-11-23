/* Waiter Skeleton walking/approach states and breakup animation. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void Entity_Walk(struct EwramData_EntityData *, s32, void *, s32);
extern s32 Target_IsInRect(s16, s16, s32, s32);
extern void Effect_UpdateSpriteDebris(struct EwramData_EntityData *);
extern u8 sWaiterSkeletonWalkProfile[], sWaiterSkeletonGfx[], sWaiterSkeletonSpriteSheet[];
void WaiterSkeleton_UpdateWalk(struct EwramData_EntityData *entity)
{
    s32 speed = 0x8000;
    switch (entity->subState)
    {
    case 0:
        Entity_Walk(entity, speed, sWaiterSkeletonWalkProfile, 12);
        if (Target_IsInRect(entity->posX.part16.integer - 100, entity->posY.part16.integer - 40, 200, 80))
        {
            Anim_StartIndexed(entity, 4, 0, 1);
            if ((RandomNumberGenerator() & 3) != 0)
                entity->subState = 1;
            else
            {
                entity->subState = 3;
                entity->timer = 20;
                Anim_StartIndexed(entity, 4, 0, 1);
            }
        }
        break;
    case 1:
        speed = 0x10000;
        Entity_Walk(entity, speed, sWaiterSkeletonWalkProfile, 12);
        if (Target_IsInRect(entity->posX.part16.integer - 80, entity->posY.part16.integer - 40, 160, 80))
        {
            Anim_StartIndexed(entity, 5, 0, 0);
            entity->subState = 2;
        }
        break;
    case 2:
        if (entity->animStep == 2)
            entity->ext.waiterSkeleton.flags |= 4;
        if (entity->animFlags & 4)
        {
            entity->ext.waiterSkeleton.flags &= ~5;
            entity->subState = 5;
            Anim_StartIndexed(entity, 1, 0, 1);
            entity->timer = (RandomNumberGenerator() & 15) + 60;
        }
        break;
    case 3:
        speed = 0x10000;
        Entity_Walk(entity, speed, sWaiterSkeletonWalkProfile, 12);
        if (entity->timer != 0)
            entity->timer--;
        else
        {
            entity->subState = 4;
            entity->timer = 50;
            Anim_StartIndexed(entity, 3, 0, 1);
        }
        break;
    case 4:
        Entity_Walk(entity, -speed, sWaiterSkeletonWalkProfile, 12);
        if (entity->timer != 0)
            entity->timer--;
        else
            entity->subState = 0;
        break;
    case 5:
        Entity_Walk(entity, -speed, sWaiterSkeletonWalkProfile, 12);
        if (entity->timer != 0)
            entity->timer--;
        else
        {
            entity->state = 0;
            entity->stage = 0;
            entity->subState = 0;
            Anim_StartIndexed(entity, 0, 0, 1);
        }
        break;
    }
}

/* Spawn fragments using frames 10..16 (or 11..16 without flag 2). */
s32 WaiterSkeleton_SpawnDeathPieces(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        {
            s32 frame;
            PlaySong(0x6B);
            frame = 10;
            if (!(entity->ext.waiterSkeleton.flags & 2))
                frame = 11;
            for (; frame <= 16; frame++)
            {
                struct EwramData_EntityData *part = EntityCreateInRange(0x49, 0xCF, (u32 *)Effect_UpdateSpriteDebris);
                if (part != NULL)
                {
                    s32 tiles;
                    part->posX.whole = entity->posX.whole;
                    part->posY.whole = entity->posY.whole;
                    part->velX.whole = ((RandomNumberGenerator() & 15) - 8) << 14;
                    part->velY.whole = (((RandomNumberGenerator() & 15) - 8) << 12) - 0x28000;
                    part->accelY.whole = 0x2000;
                    part->spriteLayer = entity->spriteLayer;
                    tiles = ObjGfx_Find(sWaiterSkeletonGfx);
                    Sprite_Attach(part, sWaiterSkeletonSpriteSheet, tiles, entity->unk_510.part8.basePalette);
                    part->flipX = entity->flipX;
                    part->drawFunc = (u32 *)Sprite_DrawEntity;
                    part->scrollWithBg = 1;
                    part->timer = (RandomNumberGenerator() & 15) + 40;
                    part->subState = 1;
                    Anim_StartIndexed(part, (u16)frame, 0, 1);
                    Anim_Update(part);
                }
            }
            entity->hidden = 1;
            entity->subState = 1;
        }
        break;
    case 1:
        return 1;
    }
    return 0;
}
