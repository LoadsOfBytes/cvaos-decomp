/* Legion shell part: orbits the core at a radius set by the shell state, fires, falls off. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
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

struct ShellRadii {
    s32 r[2][3];
};

extern void LegionShellPart_FireShot(struct EwramData_EntityData *owner, s32 dx, s32 dy, s32 unused, s32 angle);
extern s32 Entity_GetVisibilityPattern(s32);
extern const struct ShellRadii sLegionShellRadii;

#define KIND ((s8)entity->ext.legionShell.kind)
#define ANGLE entity->ext.legionShell.angle
#define RADIUS entity->ext.legionShell.radius

void LegionShellPart_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = entity->ext.legionPart.owner;
    struct EwramData_EntityData *center = entity->ext.legionShell.center;
    u8 unused[0x10];
    struct ShellRadii radii = sLegionShellRadii;
    struct LegionState *data = (struct LegionState *)owner->ext.legion.data.value;
    s32 k = (s8)entity->ext.legionShell.index * 6 + 10;
#define shell ((struct LegionShell *)((s32 *)data + k))
    s32 limit;
    s32 a;

    if (shell->state == 1)
        { EntityDelete(entity); return; }
    if (entity->state == 0)
    {
        if (owner == NULL || owner->updateFunc == NULL || owner->enemyId != 0x6C)
            { EntityDelete(entity); return; }
        if (shell->state != 4)
        {
            switch (entity->subState)
            {
            case 0:
                entity->subState = 1;
                entity->timer = 60;
                break;
            case 1:
                ANGLE += 500;
                if (ANGLE > 0x7800)
                {
                    ANGLE = 0x7800;
                    entity->subState++;
                }
                break;
            case 2:
                ANGLE -= 500;
                if (ANGLE < 0x800)
                {
                    ANGLE = 0x800;
                    entity->subState--;
                }
                break;
            }
            { s32 sel; s32 i = shell->radiusSet; if (KIND == 5) sel = 2; else sel = KIND != 0; limit = radii.r[i][sel]; }
            if (shell->state == 2)
            {
                entity->hidden = 0;
                if (RADIUS < limit)
                {
                    shell->actionState = 2;
                    RADIUS += limit / 120;
                    if (RADIUS >= limit)
                    {
                        RADIUS = limit;
                        shell->actionState = 0;
                    }
                }
            }
            if (shell->state == 3 && RADIUS > 0)
            {
                shell->actionState = 3;
                RADIUS -= limit / 120;
                if (RADIUS <= 0)
                {
                    RADIUS = 0;
                    shell->actionState = 0;
                    entity->hidden = 1;
                }
            }
        }
        else if (KIND == 5)
        {
            shell->actionState = 4;
            if (shell->timer == 1)
            {
                entity->stage = 0;
                entity->palette = entity->basePalette;
            }
            else if (shell->timer == 0)
            {
                switch (entity->stage)
                {
                case 0:
                    entity->stage = 1;
                    entity->timer = 60;
                    PlaySong(0x11F);
                    break;
                case 1:
                    if (entity->timer > 0x1D && !(entity->timer & 1))
                        LegionShellPart_FireShot(entity, 0, 0, data->unk_0, ANGLE + shell->angleOffset);
                    if (--entity->timer == 0)
                        shell->actionState = 0;
                    break;
                }
            }
            else if (Entity_GetVisibilityPattern(shell->timer) == 1)
                entity->palette = 4;
            else
                entity->palette = entity->basePalette;
        }
        if (KIND == 0)
            a = ANGLE - (ANGLE - 0x4000) / 4;
        else if (KIND == 1)
            a = ANGLE;
        else
            a = ANGLE + (ANGLE - 0x4000) / 4;
        a += shell->angleOffset;
        {
            s32 x, y;
            x = Sin(a + 0x4000);
            x = FixedMultiply(x, RADIUS);
            y = Sin(a);
            y = FixedMultiply(y, RADIUS);
            entity->posX.whole = center->posX.whole + x;
            entity->posY.whole = center->posY.whole + y;
        }
        if (KIND == 5)
            ObjAffine_SetBiased(ANGLE + ({ s32 t = shell->angleOffset - 0x4000; t; }), 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
        if (entity->state == 0 && owner->state == 2 && --entity->ext.legionShell.delay == 0)
        {
            entity->state = 1;
            entity->subState = 0;
            entity->stage = 0;
            entity->hidden = 0;
            entity->palette = entity->basePalette;
        }
        Anim_Update(entity);
        return;
    }
    if (entity->subState == 0)
    {
        entity->subState++;
        entity->scrollWithBg = 1;
        entity->velX.whole = 0;
        entity->accelX.whole = 0;
        entity->velY.whole = 0;
        entity->accelY.whole = 0x1800;
        entity->timer = 50;
    }
    else if (entity->subState == 1)
    {
        EntityAccelerateAndMove(entity, 0x40000);
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
        if ((s8)--entity->timer <= 0)
        {
            if (KIND == 5)
                ObjAffine_Free(entity->affineIndex);
            { EntityDelete(entity); return; }
        }
    }
    Anim_Update(entity);
    return;

}
