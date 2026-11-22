/* LegionCorePart_Update: owner-attached effect - follow the owner, animate through a table of sprite parts and spawn particles on timer ticks. */
#include "code_08009A0.h"
#include "legion_entity.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern void *memcpy(void *dst, const void *src, unsigned long size);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *);
extern s32 Entity_GetVisibilityPattern(s32);
extern void Legion_SpawnFragment(struct EwramData_EntityData *owner, s32 x, s32 y, s32 vx, s32 vy, s32 graphics);

#define FRAME LEGION_PART_KIND(entity)
#define TICK (LEGION_PART_TICK(entity) & 0x1F)
#define FLAGS LEGION_BROKEN_CORES(owner)
#define SPAWN(vx, vy) Legion_SpawnFragment(entity, x, y, vx, vy, LEGION_PART_TILES(entity))

void LegionCorePart_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = LEGION_PART_OWNER(entity);
    u8 buffer[0x10];
    u32 b0 = 0x1934FFE4, b1 = 0x1524ECF5;
    u32 b4 = 0x1932E8E6, b5 = 0x1521FFF7;
    u32 b2 = 0x1522EBE8, b3 = 0x1932FFE8;
    u32 b6 = 0x1934E8E8, b7 = 0x1524FFE8;
    u32 *tbl[4][2] = { { &b0, &b1 }, { &b2, &b3 }, { &b4, &b5 }, { &b6, &b7 } };
    s32 x, y, vx;
    s32 counter;
    s32 vy = 0;

    u8 idx;

    counter = LEGION_PART_TICK(entity) + 1;
    LEGION_PART_TICK(entity) = counter;

    if (entity->state <= 9)
    {
        if (owner == NULL || owner->updateFunc == NULL || owner->unk_518.part8.enemyId != entity->unk_518.part8.enemyId)
            return;
        if (FRAME == 3)
        {
            if (entity->state == 0)
            {
                if ((u8)LEGION_INTRO_STEP(owner) == 7)
                {
                    entity->state++;
                    entity->subState = 0;
                }
            }
            else if (entity->state == 1)
            {
                switch (entity->subState)
                {
                case 0:
                    entity->velX.whole += 0x80;
                    entity->velY.whole += 0x80;
                    if (entity->velX.whole > 0xCFFF)
                    {
                        entity->velX.whole = 0xD000;
                        entity->velY.whole = 0xD000;
                        entity->subState++;
                    }
                    break;
                case 1:
                    entity->velX.whole -= 0x80;
                    entity->velY.whole -= 0x80;
                    if (entity->velX.whole <= 0xCC00)
                    {
                        entity->velX.whole = 0xCC00;
                        entity->velY.whole = 0xCC00;
                        entity->subState++;
                    }
                    break;
                case 2:
                    entity->velX.whole += 0x80;
                    entity->velY.whole += 0x80;
                    if (entity->velX.whole > 0xDFFF)
                    {
                        entity->velX.whole = 0xE000;
                        entity->velY.whole = 0xE000;
                        entity->subState++;
                    }
                    break;
                case 3:
                    entity->velX.whole -= 0x80;
                    entity->velY.whole -= 0x80;
                    if (entity->velX.whole <= 0xDC00)
                    {
                        entity->velX.whole = 0xDC00;
                        entity->velY.whole = 0xDC00;
                        entity->subState++;
                    }
                    break;
                case 4:
                    entity->velX.whole += 0x80;
                    entity->velY.whole += 0x80;
                    if (entity->velX.whole > 0xEFFF)
                    {
                        entity->velX.whole = 0xF000;
                        entity->velY.whole = 0xF000;
                        entity->subState++;
                    }
                    break;
                case 5:
                    entity->velX.whole -= 0x80;
                    entity->velY.whole -= 0x80;
                    if (entity->velX.whole <= 0xEC00)
                    {
                        entity->velX.whole = 0xEC00;
                        entity->velY.whole = 0xEC00;
                        entity->subState++;
                    }
                    break;
                case 6:
                    entity->velX.whole += 0x80;
                    entity->velY.whole += 0x80;
                    if (entity->velX.whole > 0xFFFF)
                    {
                        entity->velX.whole = 0x10000;
                        entity->velY.whole = 0x10000;
                        entity->subState = 0;
                        FRAME = 0x14;
                        entity->state = 0;
                        LEGION_INTRO_ARRIVED(owner)++;
                        entity->animFrame = 3;
                        entity->posX.whole = owner->posX.whole + LEGION_PART_OFFSET_X(entity);
                        entity->posY.whole = owner->posY.whole + LEGION_PART_OFFSET_Y(entity);
                        goto draw;
                    }
                    break;
                case 7:
                    break;
                }
            }
            entity->posX.whole = owner->posX.whole;
            entity->posY.whole = owner->posY.whole;
        draw:
            ObjAffine_SetBiased(0, entity->velX.whole, entity->velY.whole, entity->flipX, entity->flipY, entity->affineIndex);
            return;
        }
        if (entity->state != 0)
            return;
        idx = FRAME <= 4 ? FRAME : 3;
        if (!(*((u8 *)gEwramData + 0x8E) & 0x40))
        {
            if (counter & 1)
            {
                HitBox_SetBody(entity, tbl[idx][0]);
                HitBox_SetAttack(entity, tbl[idx][1]);
            }
            else
            {
                HitBox_SetBody(entity, tbl[idx][1]);
                HitBox_SetAttack(entity, tbl[idx][0]);
            }
        }
        entity->posX.whole = owner->posX.whole + LEGION_PART_OFFSET_X(entity);
        entity->posY.whole = owner->posY.whole + LEGION_PART_OFFSET_Y(entity);
        if (!(*((u8 *)gEwramData + 0x8E) & 0x40))
            Hit_CheckWithPlayer(buffer, entity);
        Enemy_UpdateHitFlash(entity);
        if (owner->state == 2)
        {
            entity->state = 0xB;
            entity->subState = 0;
            entity->unk_518.part16.hp = 0;
            entity->palette = entity->unk_510.part8.basePalette;
            entity->timer = 0x3C;
            entity->unk_510.part8.hitFlashTimer = 0;
        }
        if (FRAME == 0)
        {
            if ((FLAGS & 4) && (LEGION_PART_TICK(entity) & 0x1F) < 1)
            {
                x = RandomNumberGenerator() % 0x300000 + ({ s32 t = entity->posX.whole - 0x180000; t; });
                y = entity->posY.whole + 0x180000;
                vx = -(RandomNumberGenerator() & 0x7FFF);
                SPAWN(vx, vy);
            }
            if ((FLAGS & 2) && TICK == 4)
            {
                y = RandomNumberGenerator() % 0x300000 + ({ s32 t = entity->posY.whole - 0x180000; t; });
                x = entity->posX.whole + 0x180000;
                vx = (RandomNumberGenerator() & 0x3FFF) + 0x8000;
                SPAWN(vx, 0);
            }
        }
        else if (FRAME == 1)
        {
            if ((FLAGS & 8) && TICK == 8)
            {
                x = RandomNumberGenerator() % 0x300000 + ({ s32 t = entity->posX.whole - 0x180000; t; });
                y = entity->posY.whole + 0x180000;
                SPAWN(RandomNumberGenerator() & 0x7FFF, 0);
            }
            if ((FLAGS & 1) && TICK == 0xC)
            {
                y = RandomNumberGenerator() % 0x300000 + ({ s32 t = entity->posY.whole - 0x180000; t; });
                x = entity->posX.whole - 0x180000;
                vx = -0x8000 - (RandomNumberGenerator() & 0x3FFF);
                SPAWN(vx, 0);
            }
        }
        else if (FRAME == 2)
        {
            if ((FLAGS & 1) && TICK == 0x10)
            {
                x = RandomNumberGenerator() % 0x300000 + ({ s32 t = entity->posX.whole - 0x180000; t; });
                y = entity->posY.whole - 0x180000;
                vx = -(RandomNumberGenerator() & 0x7FFF);
                SPAWN(vx, -(RandomNumberGenerator() & 0x7FFF));
            }
            if ((FLAGS & 8) && TICK == 0x14)
            {
                y = RandomNumberGenerator() % 0x300000 + ({ s32 t = entity->posY.whole - 0x180000; t; });
                x = entity->posX.whole + 0x180000;
                vx = (RandomNumberGenerator() & 0x3FFF) + 0x8000;
                SPAWN(vx, 0);
            }
        }
        else
        {
            if ((FLAGS & 2) && TICK == 0x18)
            {
                x = RandomNumberGenerator() % 0x300000 + ({ s32 t = entity->posX.whole - 0x180000; t; });
                y = entity->posY.whole - 0x180000;
                vx = RandomNumberGenerator() & 0x7FFF;
                SPAWN(vx, -(RandomNumberGenerator() & 0x7FFF));
            }
            if ((FLAGS & 4) && TICK == 0x1C)
            {
                y = RandomNumberGenerator() % 0x300000 + ({ s32 t = entity->posY.whole - 0x180000; t; });
                x = entity->posX.whole - 0x180000;
                vx = -0x8000 - (RandomNumberGenerator() & 0x3FFF);
                SPAWN(vx, 0);
            }
        }
        return;
    }
    if (entity->subState == 0)
    {
        entity->subState++;
        if (FRAME == 0)
        {
            if (entity->state == 10)
                FLAGS |= 1;
            entity->velX.whole = -0x4000;
            entity->velY.whole = -0x10000;
        }
        else if (FRAME == 1)
        {
            if (entity->state == 10)
                FLAGS |= 2;
            entity->velX.whole = 0x4000;
            entity->velY.whole = -0x10000;
        }
        else if (FRAME == 2)
        {
            if (entity->state == 10)
                FLAGS |= 4;
            entity->velX.whole = -0x4000;
            entity->velY.whole = 0;
        }
        else
        {
            if (entity->state == 10)
                FLAGS |= 8;
            entity->velX.whole = 0x4000;
            entity->velY.whole = 0;
        }
        entity->posX.whole = owner->posX.whole + LEGION_PART_OFFSET_X(entity);
        entity->posY.whole = owner->posY.whole + LEGION_PART_OFFSET_Y(entity);
        entity->scrollWithBg = 1;
        entity->accelX.whole = 0;
        entity->accelY.whole = 0x1800;
    }
    else if (entity->subState == 1)
    {
        EntityAccelerateAndMove(entity, 0x40000);
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
        if ((s8)--entity->timer <= 0)
        {
            if (FRAME == 0x14)
                ObjAffine_Free(entity->affineIndex);
            EntityDelete(entity);
        }
    }
}
