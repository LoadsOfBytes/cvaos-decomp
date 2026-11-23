/* Flying enemy: approaches a random target near Target_GetX/Target_GetY and re-targets. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "code/code_080211F0.h"
#include "structs/ewram.h"
extern s32 Target_IsInRect(s16, s16, s16, s16);
extern s32 Target_GetX(struct EwramData_EntityData *);
extern s32 Target_GetY(struct EwramData_EntityData *);
extern void EntityDampVelocityX(struct EwramData_EntityData *, s32);

static inline void SetTarget(struct EwramData_EntityData *entity, struct BgLayerState *bg, s32 r, s32 dy)
{
    s32 x = Target_GetX(entity);
    s32 bx = bg->xPos.whole;
    if (!(RandomNumberGenerator() & 1))
        entity->ext.ectoplasm.pointX = x - r + bx;
    else
        entity->ext.ectoplasm.pointX = x + r + bx;
    x = Target_GetY(entity) - RandomNumberGenerator() % 0x300000;
    x += bg->yPos.whole + dy;
    entity->ext.ectoplasm.pointY = x;
}

void Ectoplasm_UpdateFloat(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    s32 state = entity->subState;
    s32 d;
    s32 dy;
    s32 speed;

    switch (state)
    {
    case 0:
        { s32 bx_ = bg->xPos.whole; entity->posX.whole = entity->ext.ectoplasm.pointX - bx_; }
        dy = entity->ext.ectoplasm.pointY - (entity->posY.whole + bg->yPos.whole);
        entity->posY.whole += dy + Sin(entity->ext.ectoplasm.bobPhase + 0x4000) * 3;
        entity->ext.ectoplasm.bobPhase += 0x200;
        entity->timer++;
        if (Target_IsInRect(entity->posX.part16.integer - 0x5A, entity->posY.part16.integer - 0x64, 0xB4, 0xC8))
        {
            entity->subState++;
            entity->timer = 0;
        }
        break;
    case 1:
        if (!entity->ext.ectoplasm.retarget)
        {
            entity->ext.ectoplasm.retarget = 1;
            { s32 r = RandomNumberGenerator() % 0x300000 + 0x100000;
            SetTarget(entity, bg, r, -0x100000); }
        }
        else
        {
            entity->ext.ectoplasm.retarget = 0;
            { s32 r = RandomNumberGenerator() % 0x300000 + 0x300000;
            SetTarget(entity, bg, r, -0x200000); }
        }
        entity->subState++;
        entity->timer = 0;
    case 2:
        d = entity->ext.ectoplasm.pointX - (entity->posX.whole + bg->xPos.whole);
        dy = entity->ext.ectoplasm.pointY - (entity->posY.whole + bg->yPos.whole);
        if (d > 0x40000)
            entity->accelX.whole = 0x400;
        else if (d < -0x40000)
            entity->accelX.whole = -0x400;
        else
            entity->accelX.whole = 0;
        if (dy > 0x40000)
            entity->accelY.whole = 0x400;
        else if (dy < -0x40000)
            entity->accelY.whole = -0x400;
        else
            entity->accelY.whole = 0;
        EntityDampVelocityX(entity, 0x20);
        EntityDampVelocityY(entity, 0x20);
        speed = 0x10000;
        EntityAccelerateAndMove(entity, speed);
        if (Player_GetFocusedActionFlags() & 0x360)
        {
            EntityDampVelocityX(entity, 0x20);
            EntityDampVelocityY(entity, 0x20);
            EntityAccelerateAndMove(entity, speed);
        }
        if ((d < 0 ? -d : d) <= 0x3FFFF && (dy < 0 ? -dy : dy) <= 0x3FFFF)
            entity->subState = 1;
        if (entity->timer++ > 0x77)
            entity->subState = 1;
        break;
    }
}
