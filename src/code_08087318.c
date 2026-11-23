#include <string.h>
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "syscalls.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
extern s32 Entity_IsTimeStopped(struct EwramData_EntityData *);
extern s32 Target_GetX(struct EwramData_EntityData *);
extern s32 Target_GetY(struct EwramData_EntityData *);
extern s32 Entity_GetVisibilityPattern(s32);
extern void Effect_SpawnAt(s32, s32, s32);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void EntityMove(struct EwramData_EntityData *);
extern void TritonSpear_SpawnEffect(struct EwramData_EntityData *, s32, s32);
extern s8 sTritonSpearOffsets[32];
#define PART_ANGLE(e) (e->ext.tritonSpear.angle)
#define PART_TARGET_ANGLE(e) ((e)->accelX.whole)
#define PIXEL(v) ((v) < 0 ? -((-(v)) >> 16) : (v) >> 16)
/* Triton spear update. Angle targets are derived from a point 24 pixels below the spear. */
void TritonSpear_Update(struct EwramData_EntityData *spear)
{
    struct EwramData_EntityData *owner = spear->ext.tritonSpear.owner;
    s32 bounds[4];
    s8 offsets[32];
    struct HitBoxRecord box;
    s32 x, y;
    s32 angle;

    memcpy(offsets, sTritonSpearOffsets, sizeof(offsets));
    if (!owner) return;
    if (!owner->updateFunc || owner->unk_518.part8.enemyId != 0x43) goto fail;
    if (spear->state == 0 && owner->state == 2)
    {
        spear->state = 1;
        spear->scrollWithBg = 1;
        spear->timer = 41;
    }
restart:
    if (spear->state == 0)
    {
        switch (spear->subState)
        {
        case 0:
        {
            s32 ownerX = owner->posX.whole;
            spear->posX.whole = ownerX;
            spear->posY.whole = owner->posY.whole + (offsets[owner->animFrame] << 16);
            if (owner->flipX)
            {
                if (owner->animFrame == 13) spear->posX.whole = ownerX + 0xC0000;
                else spear->posX.whole = ownerX + 0x140000;
            }
            else
            {
                if (owner->animFrame == 13) spear->posX.whole = ownerX - 0xB0000;
                else spear->posX.whole = ownerX - 0x130000;
            }
            if (*(u8 *)&owner->ext.triton.thrust)
            {
                spear->subState = 1;
                spear->timer = 0;
                spear->scrollWithBg = 1;
            }
            break;
        }
        case 1:
            if (!Entity_IsTimeStopped(owner))
            {
                spear->posY.whole -= 0x18000;
                if (spear->posY.whole <= 0x200000)
                {
                    spear->subState++;
                    spear->timer = 0;
                    PlaySong(0x86);
                }
            }
            break;
        case 2:
            if (!Entity_IsTimeStopped(owner))
            {
                if (owner->flipX)
                {
                    PART_ANGLE(spear) -= 0x1000;
                    if (PART_ANGLE(spear) <= 0xC000)
                    {
                        PlaySong(0x86);
                        PART_ANGLE(spear) += 0x10000;
                        if (++spear->timer > 2)
                        {
                            spear->subState++;
                            spear->timer = 0;
                            x = Target_GetX(spear) - spear->posX.whole;
                            {
                                s32 target = Target_GetY(spear);
                                s32 base = spear->posY.whole + 0x180000;
                                y = target - base;
                            }
                            angle = (u16)ArcTan2(x < 0 ? (s16)(-((-x) >> 16)) : x >> 16,
                                                 y < 0 ? (s16)(-((-y) >> 16)) : y >> 16);
                            PART_TARGET_ANGLE(spear) = angle + 0x10000;
                        }
                    }
                }
                else
                {
                    PART_ANGLE(spear) += 0x1000;
                    if (PART_ANGLE(spear) > 0x2BFFF)
                    {
                        PlaySong(0x86);
                        PART_ANGLE(spear) -= 0x10000;
                        if (++spear->timer > 2)
                        {
                            spear->subState++;
                            spear->timer = 0;
                            x = Target_GetX(spear) - spear->posX.whole;
                            {
                                s32 target = Target_GetY(spear);
                                s32 base = spear->posY.whole + 0x180000;
                                y = target - base;
                            }
                            angle = (u16)ArcTan2(x < 0 ? (s16)(-((-x) >> 16)) : x >> 16,
                                                 y < 0 ? (s16)(-((-y) >> 16)) : y >> 16);
                            PART_TARGET_ANGLE(spear) = angle + 0x20000;
                        }
                    }
                }
            }
            break;
        case 3:
            if (!Entity_IsTimeStopped(owner))
            {
                if (owner->flipX)
                {
                    PART_ANGLE(spear) -= 0x1000;
                    if (PART_ANGLE(spear) <= PART_TARGET_ANGLE(spear)) { PART_ANGLE(spear) = PART_TARGET_ANGLE(spear); spear->subState++; }
                }
                else
                {
                    PART_ANGLE(spear) += 0x1000;
                    if (PART_ANGLE(spear) >= PART_TARGET_ANGLE(spear))
                    {

                        PART_ANGLE(spear) = PART_TARGET_ANGLE(spear);
                        spear->subState++;
                    }
                }
            }
            break;
        case 4:
            if (!Entity_IsTimeStopped(owner))
            {
                if (++spear->timer == 11)
                {
                    angle = PART_ANGLE(spear);
                    x = Sin(angle + 0x4000);
                    y = Sin(angle);
                    angle = 0xB0000;
                    x = FixedMultiply(x, angle);
                    y = FixedMultiply(y, angle);
                    TritonSpear_SpawnEffect(spear, x, y);
                }
                if (spear->timer > 59)
                {
                    spear->subState++;
                    angle = PART_ANGLE(spear);
                    x = Sin(angle + 0x4000);
                    y = Sin(angle);
                    angle = 0x40000;
                    spear->velX.whole = FixedMultiply(x, angle);
                    spear->velY.whole = FixedMultiply(y, angle);
                    PlaySong(0x85);
                }
            }
            break;
        case 5:
            if (!Entity_IsTimeStopped(owner))
            {
                EntityMove(spear);
                if (EntityIsOutsideScreen(spear, 2))
                {
                    s32 ownerX;
                    spear->subState++;
                    ownerX = owner->posX.whole;
                    spear->posX.whole = ownerX;
                    if (owner->flipX) spear->posX.whole = ownerX + 0x140000;
                    else spear->posX.whole = ownerX - 0x130000;
                    spear->posY.whole = 0xF00000;
                    PART_ANGLE(spear) = 0x1C000;
                }
            }
            break;
        case 6:
            if (!Entity_IsTimeStopped(owner))
            {
                s32 ownerX = owner->posX.whole;
                spear->posX.whole = ownerX;
                spear->posX.whole = ownerX + (owner->flipX ? 0x140000 : -0x130000);
                spear->posY.whole -= 0x18000;
                if (owner->posY.whole - 0x300000 >= spear->posY.whole)
                {
                    /* Keep the shared zero to reproduce the original stores. */
                    s32 zero = 0;
                    *(u8 *)&spear->ext.triton.thrust = 1;
                    *(u8 *)&owner->ext.triton.thrust = zero;
                    spear->subState = zero;
                    spear->scrollWithBg = 0;
                    goto restart;
                }
            }
            break;
        }
    }
    angle = PART_ANGLE(spear);
    x = Sin(angle + 0x4000);
    y = Sin(angle);
    angle = 0xC0000;
    x = FixedMultiply(x, angle);
    y = FixedMultiply(y, angle);
    if (!gEwramData->gameState.bestiaryPreview)
    {
        box.x = PIXEL(x) - 3;
        box.y = PIXEL(y) - 3;
        box.width = 6;
        box.height = 6;
        HitBox_SetAttack(spear, &box);
    }
    /* Reuse angle here: a separate expression changes agbcc register allocation. */
    angle = PART_ANGLE(spear) - 0xC000;
    ObjAffine_SetBiased(angle, 0x10000, 0x10000, spear->flipX, spear->flipY, spear->affineIndex);
    if (spear->state == 1)
    {
        spear->hidden = Entity_GetVisibilityPattern(spear->timer--);
        if (owner->state == 3)
        {
            spear->state = 2;
            spear->timer = 4;
        }
    }
    if (spear->state == 2)
    {
        if ((s8)spear->timer-- == 0)
        {
            Effect_SpawnAt(spear->posX.whole, spear->posY.whole, spear->spriteLayer - 1);
            spear->deleteRequested = 1;
            ObjAffine_Free(spear->affineIndex);
        }
    }
    if (spear->state == 0 && !gEwramData->gameState.bestiaryPreview)
        Hit_CheckWithPlayer(bounds, spear);
    return;
fail:
    EntityDelete(spear);
}
