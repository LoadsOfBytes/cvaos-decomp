/* Legion (state == 1): rises, bounces around the room, sinks back. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

struct LegionShell {
    s32 state;
    s32 actionState;
    s32 radiusSet;
    s32 unk_C;
    s32 angleOffset;
    s32 timer;
};

struct LegionState {
    u32 unk_0;
    s32 rotationAngle;
    s32 angularVelocity;
    s32 roomX;
    s32 roomY;
    s32 bobPhase;
    s32 shellActionTimer;
    s32 unk_1C[3];
    struct LegionShell shells[3];
};

extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern s32 Entity_IsTargetBelow(struct EwramData_EntityData *);
extern s32 Entity_MoveWithFilteredBoxCollision(struct EwramData_EntityData *, s32, s32, s32, s32, s32, s32);
extern s32 Entity_MoveWithBoxCollision(struct EwramData_EntityData *, s32, s32, s32, s32, s32, s32);
extern void EntityDampVelocityX(struct EwramData_EntityData *, s32);
extern void EntityDampVelocityY(struct EwramData_EntityData *, s32);
extern s32 Legion_GetPathEdge(s32 x, s32 y, s32 size);
extern u8 sLegionAnimScript0[];
#define DIR (*(u8 *)&entity->ext.raw.unk_500.part8.unk_501)
#define ANIM(P) Anim_Start(entity, (struct AnimScript *)(sLegionAnimScript0 + (P)), 2, 1)
#define BOUNCE(R) \
    if ((R) & 0xF00) \
    { \
        s32 t = entity->velX.whole; \
        entity->velX.whole = entity->velY.whole; \
        entity->velY.whole = t; \
        t = entity->accelX.whole; \
        entity->accelX.whole = entity->accelY.whole; \
        entity->accelY.whole = t; \
    } \
    else if ((R) & 3) \
    { \
        entity->velX.whole = -entity->velX.whole; \
        entity->accelX.whole = -entity->accelX.whole; \
    } \
    else if ((R) & 0xC) \
    { \
        entity->velY.whole = -entity->velY.whole; \
        entity->accelY.whole = -entity->accelY.whole; \
    }

void Legion_UpdateBounce(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    struct LegionState *data = (struct LegionState *)entity->ext.legion.data.value;
    s32 r;

    switch (entity->subState)
    {
    case 0:
        data->angularVelocity += 8;
        if (entity->timer < 200)
            entity->timer++;
        if (entity->timer == 30)
            ANIM(0x14);
        else if (entity->timer == 60)
            ANIM(0x28);
        else if (entity->timer == 90)
            ANIM(0x3C);
        else if (entity->timer == 120)
            ANIM(0x50);
        else if (entity->timer == 150)
            ANIM(0x64);
        if (data->angularVelocity < 0x800)
            break;
        data->angularVelocity = 0x800;
        if (data->shells[0].actionState != 0 || data->shells[1].actionState != 0 || data->shells[2].actionState != 0)
            break;
        data->shells[0].state = 0;
        data->shells[1].state = 0;
        data->shells[2].state = 0;
        entity->subState++;
        entity->timer = 250;
        entity->velX.whole = 0;
        entity->velY.whole = 0;
        if (Entity_IsTargetRight(entity))
            entity->accelX.whole = 0x800;
        else
            entity->accelX.whole = -0x800;
        if (Entity_IsTargetBelow(entity))
            entity->accelY.whole = 0x800;
        else
            entity->accelY.whole = -0x800;
        break;
    case 1:
        entity->posX.whole = data->roomX - bg->xPos.whole;
        entity->posY.whole = data->roomY - bg->yPos.whole;
        r = Entity_MoveWithFilteredBoxCollision(entity, 0, -16, -16, 32, 32, 0x20000);
        data->roomX = entity->posX.whole + bg->xPos.whole;
        data->roomY = entity->posY.whole + bg->yPos.whole;
        BOUNCE(r)
        if (--entity->timer < 2)
        {
            entity->subState++;
            entity->timer = 0;
            data->shells[0].state = 2;
            data->shells[1].state = 2;
            data->shells[2].state = 2;
            data->shells[0].actionState = 2;
            data->shells[1].actionState = 2;
            data->shells[2].actionState = 2;
            entity->accelX.whole = 0;
            entity->accelY.whole = 0;
        }
        break;
    case 2:
        EntityDampVelocityX(entity, 0x10);
        EntityDampVelocityY(entity, 0x10);
        entity->posX.whole = data->roomX - bg->xPos.whole;
        entity->posY.whole = data->roomY - bg->yPos.whole;
        r = Entity_MoveWithBoxCollision(entity, 0, -16, -16, 32, 32, 0x20000);
        data->roomX = entity->posX.whole + bg->xPos.whole;
        data->roomY = entity->posY.whole + bg->yPos.whole;
        BOUNCE(r)
        data->angularVelocity -= 8;
        if (entity->timer < 200)
            entity->timer++;
        if (entity->timer == 30)
            ANIM(0x50);
        else if (entity->timer == 60)
            ANIM(0x3C);
        else if (entity->timer == 90)
            ANIM(0x28);
        else if (entity->timer == 120)
            ANIM(0x14);
        else if (entity->timer == 150)
            ANIM(0);
        if (data->angularVelocity > 0x100)
            break;
        data->angularVelocity = 0x100;
        if (data->shells[0].actionState != 0 || data->shells[1].actionState != 0 || data->shells[2].actionState != 0)
            break;
        data->shells[0].state = 0;
        data->shells[1].state = 0;
        data->shells[2].state = 0;
        {
            u8 two = 2;
            u32 dir;
            entity->subState = two;
            entity->state = 0;
            entity->velX.whole = 0;
            entity->velY.whole = 0;
            entity->accelX.whole = 0;
            entity->accelY.whole = 0;
            entity->posX.whole = data->roomX - bg->xPos.whole;
            entity->posY.whole = data->roomY - bg->yPos.whole;
            r = Legion_GetPathEdge(entity->posX.whole + bg->xPos.whole, entity->posY.whole + bg->yPos.whole, 0x300000);
            DIR = r;
            dir = (u8)r;
            if (dir == 0)
            {
                if (Entity_IsTargetRight(entity))
                    entity->ext.legion.circleDir = two;
                else
                    entity->ext.legion.circleDir = 1;
            }
            else if (dir == 2)
            {
                if (Entity_IsTargetRight(entity))
                    entity->ext.legion.circleDir = 1;
                else
                    entity->ext.legion.circleDir = dir;
            }
            entity->ext.legion.retargetTimer = (RandomNumberGenerator() % 64) + 0x3C;
            data->shellActionTimer = (RandomNumberGenerator() % 128) + 0x78;
        }
        break;
    }
    data->shells[0].angleOffset += 0x87;
    data->shells[1].angleOffset -= 0x76;
    data->shells[2].angleOffset += 0xC3;
    data->rotationAngle += data->angularVelocity;
    ObjAffine_Set(data->rotationAngle, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
    if (entity->ext.legion.flags & 2)
    {
        data->bobPhase += 0x180;
        entity->posX.whole = data->roomX - bg->xPos.whole;
        entity->posY.whole = data->roomY - bg->yPos.whole + Sin(data->bobPhase) * 3;
    }
    else
    {
        entity->posX.whole = data->roomX - bg->xPos.whole;
        entity->posY.whole = data->roomY - bg->yPos.whole;
    }
}
