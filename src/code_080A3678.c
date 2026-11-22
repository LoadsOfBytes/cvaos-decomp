#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern s32 Entity_AimVelocityAtPoint(struct EwramData_EntityData *, s32, s32, s32);
extern s32 Entity_AimAccelerationAtPoint(struct EwramData_EntityData *, s32, s32, s32);

#define OFFSET_X(e) ((e)->unk_4F8.whole)
#define OFFSET_Y(e) ((e)->unk_4FC.whole)
#define SPREAD(e) ((e)->unk_504.whole)
#define MASH(e) (((u8 *)&(e)->unk_500)[1])

/* Chaos soul grab: holds the player, who can mash buttons to break free. */
void ChaosSpark_Update(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    struct EwramData_EntityData *orb = entity->unk_4F4.unk_4F4_entity;
    struct EwramData *ewram = gEwramData;
    struct BgLayerState *camera = &ewram->bgInfo[1];
    struct EwramData_EntityData *target;
    s32 x, y;
    s32 state = entity->state;

    if (state == 0)
    {
        if (orb == NULL || orb->updateFunc == NULL
            || orb->unk_518.part8.enemyId != entity->unk_518.part8.enemyId)
            goto check;
        if (entity->stage == 0)
        {
            if (SPREAD(entity) != 0)
            {
                SPREAD(entity) -= 0x40000;
                if (SPREAD(entity) <= 0)
                {
                    SPREAD(entity) = state;
                    entity->stage++;
                    entity->timer = 30;
                    PlaySong(0x141);
                }
            }
        }
        else if (entity->stage == 1)
        {
            if ((s8)entity->timer-- <= 0)
            {
                entity->stage = state;
                entity->state = 1;
                entity->timer = 0x78;
                target = ewram->playerState.enemyTarget;
                x = target->posX.whole + camera->xPos.whole;
                y = target->posY.whole + camera->yPos.whole - 0x100000;
                Entity_AimVelocityAtPoint(entity, x, y, 0x4000);
                entity->unk_500.part8.unk_500 = state;
                return;
            }
        }
        entity->posX.whole = orb->posX.whole + OFFSET_X(entity);
        entity->posY.whole = orb->posY.whole + OFFSET_Y(entity);
        Anim_Update(entity);
    }
    else if (state == 1)
    {
        entity->scrollWithBg = 1;
        if (entity->timer != 0)
        {
            entity->timer--;
            target = ewram->playerState.enemyTarget;
            x = target->posX.whole + camera->xPos.whole;
            y = target->posY.whole + camera->yPos.whole - 0x100000;
            Entity_AimAccelerationAtPoint(entity, x, y, 0x2000);
        }
        EntityAccelerateAndMove(entity, 0x30000);
        Anim_Update(entity);
        if (EntityIsOutsideScreen(entity, 4))
            goto delete;
        if (entity->unk_500.part8.unk_500 == 1)
        {
            entity->state = 2;
            entity->subState = 0;
            entity->timer = 0x90;
            MASH(entity) = 0;
            OFFSET_X(entity) = gEwramData->playerState.enemyTarget->posX.whole - entity->posX.whole;
            OFFSET_Y(entity) = gEwramData->playerState.enemyTarget->posY.whole - entity->posY.whole;
            entity->spriteLayer = 9;
        }
        Hit_CheckWithPlayer(collision, entity);
    }
    else if (state == 2)
    {
        if ((entity->timer & 7) == 0)
        {
            struct EwramData_EntityData *child;
            s32 x2 = entity->posX.whole + RandomNumberGenerator() % 0x60000 - 0x30000;
            child = Effect_CreateFromTable(x2, entity->posY.whole + RandomNumberGenerator() % 0x60000 - 0x30000,
                                 0, -0x2000, 7, 6);
            if (child != NULL)
            {
                x = orb->posX.whole + camera->xPos.whole;
                y = orb->posY.whole + camera->yPos.whole;
                Entity_AimAccelerationAtPoint(child, x, y, 0x1000);
            }
        }
        Player_ConsumeMP(1);
        if ((u8)(entity->timer % 0x24) == 0)
            PlaySong(0x142);
        ewram = gEwramData;
        entity->posX.whole = ewram->playerState.enemyTarget->posX.whole - OFFSET_X(entity);
        entity->posY.whole = ewram->playerState.enemyTarget->posY.whole - OFFSET_Y(entity);
        if ((ewram->inputData.playerNewInput & 0x3FF) && MASH(entity) <= 99)
            MASH(entity)++;
        if (SPREAD(entity) <= 0x40000)
            SPREAD(entity) += 0x10000;
        /* Released when time runs out or after 20 mashed inputs. */
        if ((u8)--entity->timer <= 1)
            goto release;
        if (MASH(entity) <= 19)
            goto check;
        goto release;
    }
    else if (state == 3)
    {
        if (SPREAD(entity) > 0x1000000)
        {
        delete:
            EntityDelete(entity);
            return;
        }
        SPREAD(entity) += 0x60000;
    }
check:
    if (orb->state == 10)
    {
    release:
        entity->state = 3;
    }
}
