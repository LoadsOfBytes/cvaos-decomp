/* Death_UpdateDashAttack, decompiled from the original assembly. The first player
 * read goes through the inline Player() (offset constant reloaded as in the
 * target). */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"
#include "syscalls.h"

extern s32 Entity_AccelerateAlongAngle(struct EwramData_EntityData *entity, u16 angle, s32 speed, s32 target, s16 acceleration, u16 time);
extern void Afterimage_Create(struct EwramData_EntityData *, s32, s32, s32);

/* Move towards (tx, ty) (16.16) at a speed proportional to the distance;
 * velX/velY get the step taken. Returns the speed. */
static inline s32 MoveToward(struct EwramData_EntityData *entity, s32 tx, s32 ty)
{
    s32 x = entity->posX.whole;
    s32 y = entity->posY.whole;
    u32 dist = Sqrt(((x - tx) >> 16) * ((x - tx) >> 16) + ((y - ty) >> 16) * ((y - ty) >> 16)) << 16;
    u16 angle = ArcTan2(((tx < 0) ? -(-tx >> 16) : (tx >> 16)) - (s16)entity->posX.part16.integer,
                        ((ty < 0) ? -(-ty >> 16) : (ty >> 16)) - entity->posY.part16.integer);
    u32 speed = dist >> 4;
    Entity_AccelerateAlongAngle(entity, angle, speed, 0, 0, 0);
    entity->velX.whole = entity->posX.whole - x;
    entity->velY.whole = entity->posY.whole - y;
    return speed;
}

/* Cutscene step: fly to 0x78 px beside the player, then dash sideways,
 * spawn two effects and play the landing animation. */
static inline struct EwramData_EntityData *Player(void) { return gEwramData->playerState.player; }

/**
 * @brief Dash attack state (4): approach a stored point beside the player,
 * dash with afterimages and update the weapon pose through animation
 * steps; return to hover when the recovery animation finishes.
 */
void Death_UpdateDashAttack(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
    {
        s32 offset;
        {
            struct EwramData_EntityData *player = Player();
            s32 px = player->posX.whole;
            entity->flipX = (entity->posX.whole < px) ? 1 : 0;
        }
        entity->ext.death.dashStep = 2;
        Anim_StartIndexed(entity, 9, 2, 1);
        offset = 0x78;
        {
            struct EwramData *ewram = gEwramData;
            struct EwramData_EntityData **player = &ewram->playerState.player;
            struct BgLayerState *camera;
            {
                s32 px = (*player)->posX.whole;
                if (entity->posX.whole < px)
                    offset = -offset;
            }
            {
                s32 px = (s16)(*player)->posX.part16.integer;
                camera = &ewram->bgInfo[1];
                entity->ext.death.targetX = px + camera->xPos.part16.integer + offset;
            }
            {
                s32 py = (s16)(*player)->posY.part16.integer;
                entity->ext.death.targetY = py + camera->yPos.part16.integer;
            }
        }
        entity->subState = 1;
    }
    case 1:
    {
        struct BgLayerState *camera;
        if (MoveToward(entity, (entity->ext.death.targetX - (s16)(camera = &gEwramData->bgInfo[1])->xPos.part16.integer) << 16,
                       (entity->ext.death.targetY - (s16)camera->yPos.part16.integer) << 16) > 0x3FFF)
            break;
        Anim_StartIndexed(entity, 0xA, 2, 1);
        entity->subState = 2;
        {
            s32 vx = -0x50000;
            s32 ax;
            entity->velX.whole = vx;
            ax = 0xF00;
            entity->accelX.whole = ax;
            if (entity->flipX)
            {
                entity->velX.whole = -vx;
                entity->accelX.whole = -ax;
            }
        }
        entity->velY.whole = 0;
        entity->accelY.whole = 0;
        entity->timer = 0x10;
        entity->ext.death.dashStep = 3;
        Afterimage_Create(entity, 0, 0x10, 0x31);
        Afterimage_Create(entity, 0, 8, 0x30);
        PlaySong(0x87);
        break;
    }
    case 2:
        EntityAccelerateAndMove(entity, 0x50000);
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
        Anim_StartIndexed(entity, 0xB, 2, 0);
        entity->subState = 3;
        break;
    case 3:
        EntityAccelerateAndMove(entity, 0x50000);
        switch (entity->animStep)
        {
        case 1:
        case 5:
            entity->ext.death.dashStep = 1;
            break;
        case 3:
            if (entity->animTimer == 0)
                PlaySong(0x86);
            entity->ext.death.dashStep = 4;
            break;
        case 6:
            if (entity->animTimer == 0)
                PlaySong(0x85);
            entity->ext.death.dashStep = 5;
            break;
        }
        if (entity->animFlags & 4)
        {
            entity->flipX = 0;
            entity->ext.death.dashStep = 0;
            Anim_StartIndexed(entity, 7, 2, 1);
            entity->state = 3;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    }
}
