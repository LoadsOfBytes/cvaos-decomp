/* Legion (state 0): circling movement and shell part spawning. */
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

extern s32 Legion_UpdateIntro(struct EwramData_EntityData *entity);
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern s32 Entity_IsTargetBelow(struct EwramData_EntityData *);
extern void EntityAccelerateX(struct EwramData_EntityData *, s32);
extern void EntityAccelerateY(struct EwramData_EntityData *, s32);
extern void Legion_SteerX(struct EwramData_EntityData *entity, s32 target);
extern void Legion_SteerY(struct EwramData_EntityData *entity, s32 target);
extern s32 Target_GetX(struct EwramData_EntityData *);
extern s32 Target_GetY(struct EwramData_EntityData *);
extern s32 Legion_GetPathEdge(s32 x, s32 y, s32 size);
extern struct EwramData_EntityData *Legion_CreateShellPart(struct EwramData_EntityData *entity, s32 center, s32 kind);
extern u8 sUnk_08528340[];
#define DIR (*(u8 *)&entity->ext.raw.unk_500.part8.unk_501)

void Legion_UpdateCircle(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    struct LegionState *data = (struct LegionState *)entity->ext.legion.data.value;
    struct EwramData_EntityData *part;
    struct LegionShell *shell;
    s32 i;
    s32 dir;

    switch (entity->subState)
    {
    case 0:
        if (Legion_UpdateIntro(entity))
        {
            entity->subState++;
            entity->ext.legion.flags = 4;
        }
        break;
    case 1:
        if (data->roomY <= 0x1900000)
        {
            data->roomY += 0x8000;
            break;
        }
        {
            u8 two = 2;
            entity->subState = two;
            entity->stage = 0;
            entity->scrollWithBg = 0;
            entity->velX.whole = 0;
            entity->velY.whole = 0;
            entity->accelY.whole = 0;
        }
        if (Entity_IsTargetRight(entity))
            entity->ext.legion.circleDir = 2;
        else
            entity->ext.legion.circleDir = 1;
        entity->ext.legion.retargetTimer = (RandomNumberGenerator() % 64) + 0x3C;
        break;
    case 2:
        if (entity->ext.legion.circleDir == 1)
        {
            switch (DIR)
            {
            case 0:
                entity->accelX.whole = -0x800;
                EntityAccelerateX(entity, 0x8000);
                data->roomX += entity->velX.whole;
                Legion_SteerY(entity, 0x1900000);
                if (data->roomX <= 0x700000)
                {
                    data->roomX = 0x700000;
                    DIR = 1;
                }
                break;
            case 1:
                entity->accelY.whole = -0x800;
                EntityAccelerateY(entity, 0x8000);
                data->roomY += entity->velY.whole;
                Legion_SteerX(entity, 0x700000);
                if (data->roomY <= 0x700000)
                {
                    data->roomY = 0x700000;
                    DIR = 2;
                }
                break;
            case 2:
                entity->accelX.whole = 0x800;
                EntityAccelerateX(entity, 0x8000);
                data->roomX += entity->velX.whole;
                Legion_SteerY(entity, 0x700000);
                if (data->roomX > 0x18FFFFF)
                {
                    data->roomX = 0x1900000;
                    DIR = 3;
                }
                break;
            case 3:
                entity->accelY.whole = 0x800;
                EntityAccelerateY(entity, 0x8000);
                data->roomY += entity->velY.whole;
                Legion_SteerX(entity, 0x1900000);
                if (data->roomY > 0x18FFFFF)
                {
                    data->roomY = 0x1900000;
                    DIR = 0;
                }
                break;
            }
        }
        else
        {
            switch (DIR)
            {
            case 0:
                entity->accelX.whole = 0x800;
                EntityAccelerateX(entity, 0x8000);
                data->roomX += entity->velX.whole;
                Legion_SteerY(entity, 0x1900000);
                if (data->roomX > 0x18FFFFF)
                {
                    data->roomX = 0x1900000;
                    DIR = 3;
                }
                break;
            case 1:
                entity->accelY.whole = 0x800;
                EntityAccelerateY(entity, 0x8000);
                data->roomY += entity->velY.whole;
                Legion_SteerX(entity, 0x700000);
                if (data->roomY > 0x18FFFFF)
                {
                    data->roomY = 0x1900000;
                    DIR = 0;
                }
                break;
            case 2:
                entity->accelX.whole = -0x800;
                EntityAccelerateX(entity, 0x8000);
                data->roomX += entity->velX.whole;
                Legion_SteerY(entity, 0x700000);
                if (data->roomX <= 0x700000)
                {
                    data->roomX = 0x700000;
                    DIR = 1;
                }
                break;
            case 3:
                entity->accelY.whole = -0x800;
                EntityAccelerateY(entity, 0x8000);
                data->roomY += entity->velY.whole;
                Legion_SteerX(entity, 0x1900000);
                if (data->roomY <= 0x700000)
                {
                    data->roomY = 0x700000;
                    DIR = 2;
                }
                break;
            }
        }
        if ((u8)entity->ext.legion.retargetTimer-- <= 1)
        {
            entity->ext.legion.retargetTimer = (RandomNumberGenerator() % 64) + 0x3C;
            dir = Legion_GetPathEdge(Target_GetX(entity) + bg->xPos.whole,
                               Target_GetY(entity) + bg->yPos.whole + -0x300000, 0x300000);
            switch (DIR)
            {
            case 0:
                if (dir == 1)
                    entity->ext.legion.circleDir = 1;
                else if (dir == 3)
                    entity->ext.legion.circleDir = 2;
                else if (dir == 0)
                {
                    if (Entity_IsTargetRight(entity))
                        entity->ext.legion.circleDir = 2;
                    else
                        entity->ext.legion.circleDir = 1;
                }
                break;
            case 1:
                if (dir == 2)
                    entity->ext.legion.circleDir = 1;
                else if (dir == 0)
                    entity->ext.legion.circleDir = 2;
                else if (dir == 1)
                {
                    if (Entity_IsTargetBelow(entity))
                        entity->ext.legion.circleDir = 2;
                    else
                        entity->ext.legion.circleDir = 1;
                }
                break;
            case 2:
                if (dir == 1)
                    entity->ext.legion.circleDir = 2;
                else if (dir == 3)
                    entity->ext.legion.circleDir = 1;
                else if (dir == 2)
                {
                    if (Entity_IsTargetRight(entity))
                        entity->ext.legion.circleDir = 1;
                    else
                        entity->ext.legion.circleDir = 2;
                }
                break;
            case 3:
                if (dir == 2)
                    entity->ext.legion.circleDir = 2;
                else if (dir == 0)
                    entity->ext.legion.circleDir = 1;
                else if (dir == 3)
                {
                    if (Entity_IsTargetBelow(entity))
                        entity->ext.legion.circleDir = 1;
                    else
                        entity->ext.legion.circleDir = 2;
                }
                break;
            }
        }
        if (!(entity->ext.legion.flags & 8) && (entity->ext.legion.brokenCores & 0xF))
        {
            part = entity;
            for (i = 0; i <= 5; i++)
            {
                part = Legion_CreateShellPart(entity, (s32)part, i);
                if (part != NULL)
                {
                    part->ext.legionShell.radius = 0;
                    part->ext.legionShell.angle = (i + 1) * 5000;
                    part->ext.legionShell.delay = (6 - i) * 6;
                    part->ext.legionShell.index = 0;
                    switch (i)
                    {
                    case 0:
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                        Anim_Start(part, (struct AnimScript *)(sUnk_08528340 + 0x8), 2, 1);
                        Anim_Update(part);
                        break;
                    case 5:
                        Anim_Start(part, (struct AnimScript *)(sUnk_08528340 + 0x1C), 2, 1);
                        Anim_Update(part);
                        break;
                    }
                }
                else
                {
                    data->shells[0].state = 1;
                    return;
                }
            }
            data->shells[0].radiusSet = 0;
            data->shells[0].state = 2;
            data->shells[0].actionState = 2;
            data->shells[0].timer = (RandomNumberGenerator() % 128) + 0x78;
            entity->ext.legion.flags |= 8;
        }
        if (!(entity->ext.legion.flags & 0x10) && (entity->ext.legion.brokenCores & 0xF) == 0xF)
        {
            part = entity;
            for (i = 0; i <= 5; i++)
            {
                part = Legion_CreateShellPart(entity, (s32)part, i);
                if (part != NULL)
                {
                    part->ext.legionShell.radius = 0;
                    part->ext.legionShell.angle = (i + 1) * 5000;
                    part->ext.legionShell.delay = (6 - i) * 6;
                    part->ext.legionShell.index = 1;
                    switch (i)
                    {
                    case 0:
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                        Anim_Start(part, (struct AnimScript *)(sUnk_08528340 + 0x8), 2, 1);
                        Anim_Update(part);
                        break;
                    case 5:
                        Anim_Start(part, (struct AnimScript *)(sUnk_08528340 + 0x1C), 2, 1);
                        Anim_Update(part);
                        break;
                    }
                }
                else
                {
                    data->shells[1].state = 1;
                    return;
                }
            }
            data->shells[1].radiusSet = 1;
            data->shells[1].state = 2;
            data->shells[1].actionState = 2;
            data->shells[1].timer = (RandomNumberGenerator() % 128) + 0x78;
            entity->ext.legion.flags |= 0x10;
        }
        if (!(entity->ext.legion.flags & 0x20) && (entity->ext.legion.brokenCores & 0xF) == 0xF)
        {
            part = entity;
            for (i = 0; i <= 5; i++)
            {
                part = Legion_CreateShellPart(entity, (s32)part, i);
                if (part != NULL)
                {
                    part->ext.legionShell.radius = 0;
                    part->ext.legionShell.angle = (i + 1) * 5000;
                    part->ext.legionShell.delay = (6 - i) * 6;
                    part->ext.legionShell.index = 2;
                    switch (i)
                    {
                    case 0:
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                        Anim_Start(part, (struct AnimScript *)(sUnk_08528340 + 0x8), 2, 1);
                        Anim_Update(part);
                        break;
                    case 5:
                        Anim_Start(part, (struct AnimScript *)(sUnk_08528340 + 0x1C), 2, 1);
                        Anim_Update(part);
                        break;
                    }
                }
                else
                {
                    data->shells[2].state = 1;
                    return;
                }
            }
            data->shells[0].radiusSet = 1;
            data->shells[2].radiusSet = 1;
            data->shells[2].state = 2;
            data->shells[2].actionState = 2;
            data->shells[2].timer = (RandomNumberGenerator() % 128) + 0x78;
            entity->ext.legion.flags |= 0x20;
        }
        if (data->shells[0].state == 2 && data->shells[0].actionState == 0)
            data->shells[0].state = 0;
        if (data->shells[1].state == 2 && data->shells[1].actionState == 0)
            data->shells[1].state = 0;
        if (data->shells[2].state == 2 && data->shells[2].actionState == 0)
            data->shells[2].state = 0;
        if (entity->stage == 0)
        {
            if ((entity->ext.legion.brokenCores & 0xF) == 0xF && data->shells[0].state == 0
                && data->shells[1].state == 0 && data->shells[2].state == 0)
            {
                if (data->shellActionTimer == 0)
                {
                    entity->stage = 1;
                    return;
                }
                data->shellActionTimer--;
            }
            for (i = 0, shell = data->shells; i <= 2; shell++, i++)
            {
                if (shell->state == 0)
                {
                    /* the spawned-flag test reuses dir (its own callee-saved register) */
                    if (i == 0)
                        dir = *(u8 *)&entity->ext.legion.flags & 8;
                    else if (i == 1)
                        dir = *(u8 *)&entity->ext.legion.flags & 0x10;
                    else
                        dir = *(u8 *)&entity->ext.legion.flags & 0x20;
                    if (dir && shell->timer != 0 && --shell->timer == 0)
                    {
                        shell->state = 4;
                        shell->actionState = 4;
                        shell->timer = 0x3C;
                    }
                }
                else if (shell->state == 4)
                {
                    if (shell->timer != 0)
                        shell->timer--;
                    else if (shell->actionState == 0)
                    {
                        shell->state = 0;
                        shell->timer = (RandomNumberGenerator() % 128) + 0x78;
                    }
                }
                if (shell->state != 4)
                {
                    switch (i)
                    {
                    case 0:
                        data->shells[0].angleOffset += 0x87;
                        break;
                    case 1:
                        data->shells[1].angleOffset -= 0x76;
                        break;
                    case 2:
                        data->shells[2].angleOffset += 0xC3;
                        break;
                    }
                }
            }
        }
        else
        {
            data->shells[0].state = 3;
            data->shells[1].state = 3;
            data->shells[2].state = 3;
            data->shells[0].actionState = 3;
            data->shells[1].actionState = 3;
            data->shells[2].actionState = 3;
            entity->state = 1;
            entity->subState = 0;
            entity->stage = 0;
            entity->timer = 0;
            data->angularVelocity = 0x100;
        }
        break;
    }
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
    if (entity->ext.legion.flags & 4)
    {
        data->rotationAngle += data->angularVelocity;
        ObjAffine_Set(data->rotationAngle, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
    }
}
