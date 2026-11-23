/* Child entity that follows its owner: copies the owner flags and runs a
 * timed rise/wait state machine (wait tables sUnk_08119580..). */
#include "code_08009A0.h"
#include "graham_entity.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"
extern s32 GrahamPart_UpdatePaletteSequence(struct EwramData_EntityData *, s32);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *);

struct Wait16 { s32 t[16]; };
struct Wait8 { s32 t[8]; };
extern struct Wait16 sUnk_08119580;
extern struct Wait8 sUnk_081195C0;
extern struct Wait8 sUnk_081195E0;

/**
 * @brief Mirror owner flags and run palette entry, table-timed waits and
 * a 16-step vertical motion pattern with effects and contact checks.
 * Preserve table copies, halfword event reads and pinned registers.
 */
void GrahamSidePart_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    struct Wait16 rise = sUnk_08119580;
    s32 m = 0x10;
    struct Wait8 waitA = sUnk_081195C0;
    struct Wait8 waitB = sUnk_081195E0;
    u32 mode = entity->state;
    s32 x, y, dx, r;

    if (mode == 0xD0)
    {
        Anim_Update(entity);
        return;
    }
    if (owner)
    {
        register s32 zero asm("r6"); /* FAKEMATCH: zero in r6 */
        register u32 flags asm("r1"); /* FAKEMATCH: flags in r1 */
        if (owner->enemyId != entity->enemyId)
        {
            entity->deleteRequested = 1;
            return;
        }
        
        flags = (u16)owner->ext.graham.flags;
        zero = 0;
        entity->ext.graham.flags = flags;
        if ((flags & m) && mode <= 0xDF)
        {
            entity->state = 0xE0;
            entity->stage = zero;
            entity->subState = zero;
        }
    }
    {
        s32 state = entity->state;
        switch (state)
        {
        case 0:
            if (GrahamPart_UpdatePaletteSequence(entity, 1))
            {
                entity->state = 1;
                entity->stage = state;
                entity->subState = state;
            }
            break;
        case 1:
            switch (entity->subState)
            {
            case 0:
                if (entity->stage == 0)
                {
                    if (entity->animScriptIndex)
                        Anim_StartIndexed(entity, 0, 0, 1);
                    entity->timer = 0x80;
                    entity->stage++;
                }
                goto wait;
            case 1:
            {
                if (entity->ext.grahamPart.side)
                    entity->timer = waitA.t[entity->ext.grahamPart.step];
                else
                    entity->timer = waitB.t[entity->ext.grahamPart.step];
                if (++entity->ext.grahamPart.step != 1 && entity->ext.grahamPart.step > 7)
                {
                    entity->ext.grahamPart.step = 0;
                    entity->ext.grahamPart.side = 1 - entity->ext.grahamPart.side;
                }
                entity->subState++;
                entity->stage = 0;
            }
            case 2:
            wait:
                if (--entity->timer == 0xFF)
                {
                    entity->subState++;
                    entity->stage = 0;
                }
                break;
            case 3:
            {
                u16 busy = *(u16 *)&owner->animStep;
                if (busy == 0 && owner)
                {
                    entity->state = 2;
                    entity->stage = busy;
                    entity->subState = busy;
                    if (entity->animScriptIndex != 1)
                        Anim_StartIndexed(entity, 1, 0, 0);
                }
                break;
            }
            }
            break;
        case 2:
            switch (entity->subState)
            {
            case 0:
                entity->velY.whole = 0x10000;
                entity->stage = 0;
                entity->subState++;
                entity->ext.grahamPart.flash = 16;
            case 1:
                entity->timer = rise.t[entity->stage] - 1;
                entity->subState++;
            case 2:
                if (entity->ext.grahamPart.flash)
                {
                    if ((s16)(entity->ext.grahamPart.flash % 3) == 0)
                    {
                        dx = -0x380000;
                        if (!entity->flipX)
                            dx = -dx;
                        r = RandomNumberGenerator();
                        x = entity->posX.whole + dx;
                        x += -0x40000;
                        x += r & 0x7FFFF;
                        r = RandomNumberGenerator();
                        y = entity->posY.whole;
                        y += -0x40000;
                        y += r & 0x7FFFF;
                        Effect_CreateFromTable(x, y, 0, -0x2000, 4, 9);
                    }
                    entity->ext.grahamPart.flash--;
                }
                if (--entity->timer == 0xFF)
                {
                    entity->posY.whole += entity->velY.whole;
                    if ((u8)++entity->stage == m)
                    {
                        entity->ext.grahamPart.flash = 0;
                        if (entity->animScriptIndex)
                            Anim_StartIndexed(entity, 0, 0, 1);
                        entity->state = 1;
                        entity->stage = 0;
                        entity->subState = 1;
                    }
                    else
                    {
                        if ((entity->stage % 4) == 0)
                            entity->velY.whole = -entity->velY.whole;
                        entity->subState--;
                    }
                }
                break;
            }
            break;
        case 0xE0:
            if (entity->ext.graham.flags & 0x20)
                entity->objMode = 1;
            return;
        }
    }
    if (entity->state)
    {
        Entity_AnimateWithBoxes(entity);
        Enemy_CheckHitPlayer(entity);
    }
}
