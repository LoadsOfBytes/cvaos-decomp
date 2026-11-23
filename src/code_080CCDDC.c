/* Bael death: shaking head, slowing rotation, smoke and final fragments. */
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include <string.h>
extern void Effect_SpawnRandom(s32 x,s32 y,s32 scaleX,s32 scaleY,s32 layer);
extern void Effect_SpawnAt(s32 x,s32 y,s32 layer);
extern s32 Entity_Walk(struct EwramData_EntityData *entity,s32 speed,void *profile,s32 flags);
extern void Bael_MoveAndRotate(struct EwramData_EntityData *entity,s32 mode);
extern s16 sBaelWalkProfile[];
extern s16 sBaelPositions[8][2];
#define SPEED(e) (e->ext.wheel.speed)
#define ANGLE(e) (e->ext.wheel.angle)
#define SHAKE_X(e) (e->ext.wheelDeath.shake[0])
#define SHAKE_Y(e) (e->ext.wheelDeath.shake[1])

/**
 * @brief Shake the linked head, slow rotation, emit effects, delete the head, then free the affine slot after the final fragment burst.
 */
void Bael_UpdateDeath(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *part = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    s32 oldX = entity->posX.whole;
    s32 oldY = entity->posY.whole;
    switch (entity->subState)
    {
        case 0:
            entity->hit.flags |= 2;
            PlaySong(0x6E);
            part->drawSkip = 1;
            part->palette = part->unk_510.part8.basePalette;
            part->state++;
            entity->timer = 0x60;
            entity->palette = entity->unk_510.part8.basePalette;
            entity->subState++;
            if ((s32)SPEED(entity) >= 0)
                SPEED(entity) = 0x20000;
            else
                SPEED(entity) = -0x20000;
        case 1:
            if ((u8)((u32)entity->timer % 7) == 0)
                Effect_SpawnRandom(part->posX.whole,part->posY.whole,0x100000,0x100000,part->spriteLayer-1);
            if ((u8)((u32)entity->timer % 3) == 0)
            {
                SHAKE_X(part) = 1;
                SHAKE_Y(part) = 1;
                part->posX.part16.integer += 1;
                part->posY.part16.integer += SHAKE_Y(part);
            }
            else
            {
                s32 zero;
                part->posX.part16.integer -= SHAKE_X(part);
                part->posY.part16.integer -= SHAKE_Y(part);
                zero = 0;
                if (SHAKE_X(part) != 0)
                    SHAKE_X(part) = zero;
                if (SHAKE_Y(part) != 0)
                    SHAKE_Y(part) = zero;
            }
            if ((s32)SPEED(entity) >= 0)
            {
                SPEED(entity) -= 0x1C0;
                if ((s32)SPEED(entity) <= 0x6FFF)
                    SPEED(entity) = 0x7000;
            }
            else
            {
                SPEED(entity) += 0x1C0;
                if ((s32)SPEED(entity) > -0x7000)
                    SPEED(entity) = -0x7000;
            }
            ANGLE(entity) += (s32)SPEED(entity) / 64;
            ObjAffine_Set(ANGLE(entity),0x10000,0x10000,0,0,entity->affineIndex);
            Entity_Walk(entity,0,sBaelWalkProfile,2);
            part->posX.whole += entity->posX.whole-oldX;
            part->posY.whole += entity->posY.whole-oldY;
            if (--entity->timer == 0xFF)
            {
                part->deleteRequested = 1;
                entity->timer = 0x80;
                entity->subState++;
                entity->drawSkip = 1;
            }
            break;
        case 2:
            if (((u32)entity->timer % 2) == 0)
            {
                u32 timer;
                Effect_SpawnRandom(oldX,oldY,0x200000,0x200000,entity->spriteLayer-1);
                timer = entity->timer;
                if ((u8)(timer % 20) == 0 && timer > 28)
                    PlaySong(0x6E);
            }
            Bael_MoveAndRotate(entity,1);
            if (--entity->timer == 0xFF)
                entity->subState++;
            break;
        case 3:
        {
            s16 positions[8][2];
            s32 i;
            memcpy(positions,sBaelPositions,sizeof(positions));
            for (i=0; i<8; i++)
                Effect_SpawnAt(entity->posX.whole + (positions[i][0]<<16),entity->posY.whole + (positions[i][1]<<16) + 0x40000,entity->spriteLayer-1);
            PlaySong(0x6E);
            ObjAffine_Free(entity->affineIndex);
            entity->deleteRequested = 1;
            break;
        }
    }
}



