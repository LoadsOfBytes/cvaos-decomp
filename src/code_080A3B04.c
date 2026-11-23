/* ChaosSoulShot_Update: Chaos soul shot update (created by ChaosOrb_SpawnSoulShot). */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

struct ChaosData {
    s32 graphics;
    s32 palette;
    s32 palette2;
};

extern u8 sChaosSpriteSheet[];
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Afterimage_Create(struct EwramData_EntityData *, s32, s32, s32);
extern s32 Entity_AimVelocityAtPoint(struct EwramData_EntityData *, s32, s32, s32);
extern void ChaosSoulShot_DamagePlayerOnContact(struct EwramData_EntityData *);

#define OFFSET_X(e) (e->ext.chaosShot.offsetX)
#define OFFSET_Y(e) (e->ext.chaosShot.offsetY)
#define SPREAD(e) (e->ext.chaosSoulShot.spread)


/**
 * @brief Soul shot update: fly toward the target (sounds 0x148 / 0x78), animate and
 * delete off screen or when its sequence ends.
 */
void ChaosSoulShot_Update(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    struct EwramData_EntityData *orb = entity->ext.chaosChild.link;
    struct EwramData *ewram = gEwramData;
    struct BgLayerState *camera = &ewram->bgInfo[1];
    s32 x, y;
    s32 state = entity->state;

    if (state == 0)
    {
        if (orb == NULL || orb->updateFunc == NULL
            || orb->enemyId != entity->enemyId)
            return;
        if (entity->stage == 0)
        {
            if (entity->animDone)
            {
                entity->stage++;
                entity->timer = 30;
                Anim_StartIndexed(entity, 3, 2, 1);
            }
        }
        else if (entity->stage == 1)
        {
            if ((s8)entity->timer-- <= 0)
            {
                struct EwramData_EntityData *target;
                s32 speed;

                entity->stage = state;
                entity->state = 1;
                target = ewram->playerState.enemyTarget;
                x = target->posX.whole + camera->xPos.whole;
                y = target->posY.whole + camera->yPos.whole - 0x100000;
                speed = 0x30000;
                Entity_AimVelocityAtPoint(entity, x, y, speed);
                if (entity->flipX)
                {
                    if (entity->velX.whole >= 0) goto sound;
                }
                else if (entity->velX.whole <= 0)
                    goto sound;
                entity->velX.whole = state;
                if (entity->velY.whole <= 0x2FFFF)
                    entity->velY.whole = speed;
            sound:
                PlaySong(0x148);
                return;
            }
        }
        entity->posX.whole = orb->posX.whole + OFFSET_X(entity);
        entity->posY.whole = orb->posY.whole + OFFSET_Y(entity);
        Anim_Update(entity);
    }
    else if (state == 1)
    {
        if (entity->stage == 0)
        {
            entity->stage++;
            entity->scrollWithBg = 1;
            Afterimage_Create(entity, 0, 12, 0x24);
            Afterimage_Create(entity, 0, 6, 0x24);
        }
        EntityAccelerateAndMove(entity, 0x60000);
        Anim_Update(entity);
        if (EntityIsOutsideScreen(entity, 4))
        {
            entity->state++;
            entity->subState = 0;
            entity->posX.whole = 0x800000 - camera->xPos.whole;
            entity->posY.whole = -0x300000;
            entity->timer = 30;
            entity->hp = 0;
        }
        else
            Hit_CheckWithPlayer(collision, entity);
    }
    else if (entity->state == 2)
    {
        if (entity->subState == 0)
        {
            s32 posX;
            s32 posY;
            s32 vx, vy;

            x = orb->posX.whole - entity->posX.whole;
            posX = entity->posX.whole;
            y = orb->posY.whole - entity->posY.whole;
            posY = entity->posY.whole;
            vx = x / entity->timer;
            entity->velX.whole = vx;
            vy = y / entity->timer;
            entity->velY.whole = vy;
            entity->posX.whole = posX + vx;
            entity->posY.whole = posY + vy;
            if (vx >= 0) entity->flipX = 1;
            else entity->flipX = 0;
            if (--entity->timer == 0)
            {
                entity->subState++;
                Anim_StartIndexed(entity, 2, 2, 0);
                PlaySong(0x78);
            }
        }
        else if (entity->subState == 1)
        {
            entity->posX.whole = orb->posX.whole;
            entity->posY.whole = orb->posY.whole;
            if (entity->animDone)
            {
                orb->ext.chaosOrb.shotDone = 1;
                EntityDelete(entity);
                return;
            }
        }
        Anim_Update(entity);
    }
}
