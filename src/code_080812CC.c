/* Golem part update: follows its skeleton joint, maintains collision,
 * then falls and spins after the parent enters state 2. */
#include "code_08009A0.h"
#include "syscalls.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern s32 Entity_GetVisibilityPattern(s32);
extern void Hit_CheckWithPlayerSharedHistory(void *, struct EwramData_EntityData *, struct EwramData_EntityData *);

struct SegmentBox {
    s8 x;
    s8 y;
    s8 w;
    s8 h;
};

#define PART(e) (*(s8 *)&e->ext.jointPart.index)   /* own part index */
#define NEXT_PART(e) (*(s8 *)&e->ext.jointPart.anchor)
#define ANGLE(e) ((e)->velX.whole)
#define SPIN(e) (e->ext.golemSegment.spin)
#define FALL_ANGLE(e) (e->ext.golemSegment.fallAngle)
#define TO_PIXELS(d) ((d) < 0 ? -(-(d) >> 16) : (d) >> 16)
/* parts 0-5 and 7-10 have an affine sprite */
#define HAS_AFFINE(i) (((i) >= 0 && (i) <= 5) || (i) == 7 || (i) == 8 || (i) == 9 || (i) == 10)

/* Body segment of a golem (the owner unk_500 holds 0x20-byte parts): follow
 * the owner's part PART, turn towards NEXT_PART (or take the part's angle),
 * hurt the player with a box at the rotated end when the owner's mask
 * unk_506 has the part's bit; when the owner dies, fall spinning and blink
 * out. */
void GolemSegment_Update(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    struct SegmentBox box;
    struct EwramData_EntityData *owner = entity->ext.jointPart.owner;

    /* Share these temporaries with the collision calculation: agbcc register allocation. */
    s32 dx, dy;

    if (entity->state == 0)
    {
        struct GolemPart *parts;


        if (owner == NULL || owner->updateFunc == NULL
            || owner->enemyId != entity->enemyId)
            goto die;
        parts = owner->ext.golem.parts;
        entity->palette = owner->palette;
        entity->flipX = owner->flipX;
        entity->timer++;
        entity->posX.whole = owner->posX.whole + parts[PART(entity)].offsetX;
        entity->posY.whole = owner->posY.whole + parts[PART(entity)].offsetY;

        if (PART(entity) == 0 || PART(entity) == 2 || PART(entity) == 7 || PART(entity) == 9)
        {
            if (owner->state != 0 || owner->subState <= 19)
            {
                dx = parts[NEXT_PART(entity)].offsetX;
                dx -= parts[PART(entity)].offsetX;
                dy = parts[NEXT_PART(entity)].offsetY;
                dy -= parts[PART(entity)].offsetY;
                ANGLE(entity) = ArcTan2(TO_PIXELS(dx), TO_PIXELS(dy)) - 0x4000;
            }
            else
                /* This branch implies owner state 0; reuse that known zero. */
                ANGLE(entity) = 0;
        }
        else if (PART(entity) == 1 || PART(entity) == 3 || PART(entity) == 4 || PART(entity) == 5 || PART(entity) == 8 || PART(entity) == 10)
            ANGLE(entity) = parts[PART(entity)].rotationAngle;
        if (owner->state == 2)
        {
            s32 delay;
            entity->state = 1;
            entity->subState = 0;
            SPIN(entity) = (RandomNumberGenerator() & 0xFF) - 0x80;
            FALL_ANGLE(entity) = ANGLE(entity);
            entity->accelX.whole = 0;
            entity->accelY.whole = 0x1000;
            ANGLE(entity) = 0;
            entity->velY.whole = 0;
            delay = RandomNumberGenerator() & 3;
            entity->timer = delay + 70;
            if (PART(entity) == 3 || PART(entity) == 10)
                entity->timer = delay + 50;
            else if (PART(entity) == 2 || PART(entity) == 9)
                entity->timer = delay + 60;
            return;
        }
        if ((owner->ext.joint.hurtMask >> PART(entity)) & 1)
        {
            dx = Sin(parts[PART(entity)].rotationAngle + 0x8000);
            dy = Sin(parts[PART(entity)].rotationAngle + 0x4000);
            if (entity->flipX)
                box.x = -(TO_PIXELS(dx << 4) + 4);
            else
                box.x = TO_PIXELS(dx << 4) - 4;
            box.y = TO_PIXELS(dy << 4) - 4;
            box.w = 8;
            box.h = 8;
            HitBox_SetAttack(entity, &box);
            HitBox_SetBody(entity, &box);
            entity->hit.flags &= ~4;
            entity->hit.flags &= ~2;
            Hit_CheckWithPlayerSharedHistory(collision, entity, owner);
        }
        else
        {
            entity->hit.flags |= 4;
            entity->hit.flags |= 2;
        }
        if (HAS_AFFINE(PART(entity)))
            ObjAffine_Set(ANGLE(entity), 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
        return;
    die:
        if (HAS_AFFINE(PART(entity)))
            ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
        return;
    }
    if (entity->subState == 0)
    {
        entity->subState++;
        entity->scrollWithBg = 1;
    }
    if (entity->timer <= 49)
    {
        EntityAccelerateAndMove(entity, 0x70000);
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
        FALL_ANGLE(entity) += SPIN(entity);
    }
    if (HAS_AFFINE(PART(entity)))
        ObjAffine_Set(FALL_ANGLE(entity), 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
    if ((s8)--entity->timer <= 0)
    {
        if (HAS_AFFINE(PART(entity)))
            ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
    }
}
