/* Headhunter attack follower, burst shots and segmented extending attack.
 * The affine tip extends and retracts along an angle with ten linked
 * segments. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

struct SingleFrameAnimation {
    u16 count;
    u16 format;
    u8 frame;
    u8 duration;
    u16 unk_6;
};
extern struct SingleFrameAnimation sHeadhunterFrameAnim;
extern struct SingleFrameAnimation sHeadhunterAttackTipFrameAnim;
extern struct SingleFrameAnimation sHeadhunterAttackSegmentFrameAnim;
extern u8 sHeadhunterSpriteSheet[];
extern void HeadhunterAttackFollower_Update(struct EwramData_EntityData *);
extern void HeadhunterBurstShot_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void HeadhunterAttackTip_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void HeadhunterAttackTip_OnHit(struct EwramData_EntityData *);
extern void HeadhunterAttackSegment_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void HeadhunterAttackSegment_OnHit(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Hit_CheckWithPlayerSharedHistory(void *, struct EwramData_EntityData *, struct EwramData_EntityData *);
extern s32 Sin(s32);
extern s32 FixedMultiply(s32, s32);
extern void EntityDampVelocityX(struct EwramData_EntityData *, s32);
extern void EntityDampVelocityY(struct EwramData_EntityData *, s32);
extern void EntityAccelerateAndMove(struct EwramData_EntityData *, s32);
void HeadhunterBurstShot_Update(struct EwramData_EntityData *entity);
void HeadhunterAttackTip_Update(struct EwramData_EntityData *head);
struct EwramData_EntityData *Headhunter_CreateAttackSegment(struct EwramData_EntityData *owner, struct EwramData_EntityData *parent, s32 index);
void HeadhunterAttackSegment_Update(struct EwramData_EntityData *segment);
#define BASE_X(e) ((e)->unk_4FC.whole) /* room coordinates */
#define BASE_Y(e) ((e)->unk_500.whole)
#define OFFSET_X(e) ((e)->unk_504.whole)
#define OFFSET_Y(e) ((e)->unk_4F4.whole)
#define ANGLE(e) ((e)->velX.whole)
#define SPEED(e) ((e)->accelX.whole)
#define SEGMENT_INDEX(e) (*(s8 *)&(e)->unk_504.part8.unk_504)

/* Create the follower effect HeadhunterAttackFollower_Update for `timer` frames. */
/**
 * @brief Create the animation-0x16 effect that follows the owner for
 * the supplied signed countdown, using palette variant 3.
 */
void Headhunter_SpawnAttackFollower(struct EwramData_EntityData *owner, s32 timer)
{
    struct EwramData_EntityData *effect = EntityCreateInRange(0x49, 0xCF, (u32 *)HeadhunterAttackFollower_Update);
    if (effect != NULL)
    {
        s32 graphics;
        effect->flipX = owner->flipX;
        effect->spriteLayer = owner->spriteLayer;
        graphics = owner->unk_500.whole;
        effect->unk_510.part8.basePalette = owner->unk_504.part8.unk_507;
        Sprite_Attach(effect, sHeadhunterSpriteSheet, graphics, effect->unk_510.part8.basePalette);
        Anim_StartIndexed(effect, 0x16, 2, 0);
        Anim_Update(effect);
        effect->timer = timer;
        effect->unk_4F8.unk_4F8_entity = owner;
        effect->scrollWithBg = 0;
        effect->drawFunc = (u32 *)Sprite_DrawEntity;
    }
}

/* Create the burst shot HeadhunterBurstShot_Update at the owner + (x, y). */
/**
 * @brief Create a burst shot at the owner-relative offset with supplied
 * velocity, an 8-by-8 attack box and its distinct contact callback.
 */
void Headhunter_SpawnBurstShot(struct EwramData_EntityData *owner, s32 x, s32 y, s32 vx, s32 vy)
{
    struct EwramData_EntityData *shot = EntityCreateInRange(0x49, 0xCF, (u32 *)HeadhunterBurstShot_Update);
    struct HitBoxRecord box = {-4, -4, 8, 8};
    if (shot != NULL)
    {
        s32 graphics;
        shot->posX.whole = owner->posX.whole + x;
        shot->posY.whole = owner->posY.whole + y;
        shot->velX.whole = vx;
        shot->velY.whole = vy;
        shot->spriteLayer = owner->spriteLayer - 1;
        graphics = owner->unk_500.whole;
        shot->unk_500.whole = graphics;
        shot->unk_510.part8.basePalette = owner->unk_504.part8.unk_507;
        Sprite_Attach(shot, sHeadhunterSpriteSheet, graphics, shot->unk_510.part8.basePalette);
        shot->unk_4FC.part8.unk_4FF = 0x54;
        Hit_Enable(shot, 8, HeadhunterBurstShot_DamagePlayerOnContact, 0);
        HitBox_SetAttack(shot, &box);
        shot->hit.flags |= 4;
        shot->hit.flags &= ~2;
        shot->attack = owner->attack;
        shot->unk_4F8.unk_4F8_entity = owner;
        shot->scrollWithBg = 1;
        shot->drawFunc = (u32 *)Sprite_DrawEntity;
    }
}

/* Burst shot: slow down, grow its hit box at 20 and 40 frames, vanish after 60. */
/**
 * @brief Damp and move the shot, grow the frame and attack box at
 * timers 20 and 40 and check contact until removal after timer 59.
 */
void HeadhunterBurstShot_Update(struct EwramData_EntityData *entity)
{
    s32 bounds[4];
    struct SingleFrameAnimation animation = sHeadhunterFrameAnim;
    struct HitBoxRecord box1 = {-7, -7, 14, 14};
    struct HitBoxRecord box2 = {-11, -11, 22, 22};
    EntityDampVelocityX(entity, 0x20);
    EntityDampVelocityY(entity, 0x20);
    entity->flipX = RandomNumberGenerator() % 2;
    entity->flipY = RandomNumberGenerator() % 2;
    EntityAccelerateAndMove(entity, 0x40000);
    if (entity->timer == 20)
    {
        entity->unk_4FC.part8.unk_4FF = 0x55;
        HitBox_SetAttack(entity, &box1);
    }
    if (entity->timer == 40)
    {
        entity->unk_4FC.part8.unk_4FF = 0x56;
        HitBox_SetAttack(entity, &box2);
    }
    Anim_Start(entity, (struct AnimScript *)&animation, 2, 1);
    animation.frame = entity->unk_4FC.part8.unk_4FF;
    Anim_Update(entity);
    if (entity->timer++ > 59)
        EntityDelete(entity);
    else
        Hit_CheckWithPlayer(bounds, entity);
}

/* Create the whip head at the owner + (x, y) towards `angle`, with ten segments. */
/**
 * @brief Create the affine attack tip and ten segments at the supplied
 * offset and angle. Preserve the existing partial-allocation cleanup
 * and owner substate advance if a segment cannot be created.
 */
void Headhunter_SpawnExtendingAttack(struct EwramData_EntityData *owner, s32 x, s32 y, s32 angle)
{
    struct EwramData_EntityData *head = EntityCreateInRange(0x49, 0xCF, (u32 *)HeadhunterAttackTip_Update);
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    struct HitBoxRecord box = {-1, -1, 3, 3};
    struct SingleFrameAnimation animation = sHeadhunterAttackTipFrameAnim;
    u16 slot;
    s32 graphics;
    s32 i;
    if (head == NULL) return;
    head->posX.whole = owner->posX.whole + x;
    head->posY.whole = owner->posY.whole + y;
    BASE_X(head) = owner->posX.whole + x + camera->xPos.whole;
    BASE_Y(head) = owner->posY.whole + y + camera->yPos.whole;
    OFFSET_X(head) = 0;
    OFFSET_Y(head) = 0;
    ANGLE(head) = angle;
    SPEED(head) = 0x40000;
    head->spriteLayer = owner->spriteLayer - 1;
    graphics = owner->unk_500.whole;
    head->unk_510.part8.basePalette = owner->unk_504.part8.unk_506;
    Sprite_Attach(head, sHeadhunterSpriteSheet, graphics, head->unk_510.part8.basePalette);
    Anim_Start(head, (struct AnimScript *)&animation, 2, 1);
    Anim_Update(head);
    if ((s32)((slot = ObjAffine_Alloc(1)) << 16) < 0)
    {
        EntityDelete(head);
        return;
    }
    head->affineIndex = slot;
    head->affine = 1;
    ObjAffine_SetBiased(0, 0x10000, 0x10000, head->flipX, head->flipY, head->affineIndex);
    Hit_Enable(head, 8, HeadhunterAttackTip_DamagePlayerOnContact, (s32)HeadhunterAttackTip_OnHit);
    HitBox_SetAttack(head, &box);
    HitBox_SetBody(head, &box);
    head->hit.flags &= ~4;
    head->hit.flags &= ~2;
    head->attack = owner->attack;
    head->unk_4F8.unk_4F8_entity = owner;
    head->scrollWithBg = 0;
    head->drawFunc = (u32 *)Sprite_DrawEntity;
    for (i = 0; i < 10; i++)
    {
        if (!Headhunter_CreateAttackSegment(owner, head, i))
        {
            ObjAffine_Free(head->affineIndex);
            EntityDelete(head);
            owner->subState++;
            return;
        }
    }
}

/* Whip head: wait, extend, slow down to reverse, retract, then finish. */
/**
 * @brief Wait, extend along the stored angle, decelerate through reversal
 * and retract. Advance the owner substate on return; free the affine
 * slot on finish or owner defeat. Share contact history with the boss.
 */
void HeadhunterAttackTip_Update(struct EwramData_EntityData *head)
{
    struct EwramData_EntityData *owner = head->unk_4F8.unk_4F8_entity;
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    s32 bounds[4];
    struct SingleFrameAnimation animation = sHeadhunterAttackTipFrameAnim;
    s32 x, y;
    if (owner->state == 2)
    {
        ObjAffine_Free(head->affineIndex);
        EntityDelete(head);
        return;
    }
    if (head->subState == 0)
    {
        head->posX.whole = BASE_X(head) + OFFSET_X(head) - camera->xPos.whole;
        head->posY.whole = BASE_Y(head) + OFFSET_Y(head) - camera->yPos.whole;
        if (head->timer++ > 14)
        {
            head->subState++;
            head->timer = 0;
            PlaySong(0x85);
        }
    }
    else if (head->subState == 1)
    {
        x = Sin(ANGLE(head) + 0x4000);
        y = Sin(ANGLE(head));
        x = FixedMultiply(x, SPEED(head));
        y = FixedMultiply(y, SPEED(head));
        OFFSET_X(head) += x;
        OFFSET_Y(head) += y;
        head->posX.whole = BASE_X(head) + OFFSET_X(head) - camera->xPos.whole;
        head->posY.whole = BASE_Y(head) + OFFSET_Y(head) - camera->yPos.whole;
        if (head->timer++ > 14)
        {
            head->subState++;
            head->timer = 0;
        }
    }
    else if (head->subState == 2)
    {
        SPEED(head) -= 0x8000;
        x = Sin(ANGLE(head) + 0x4000);
        y = Sin(ANGLE(head));
        x = FixedMultiply(x, SPEED(head));
        y = FixedMultiply(y, SPEED(head));
        OFFSET_X(head) += x;
        OFFSET_Y(head) += y;
        head->posX.whole = BASE_X(head) + OFFSET_X(head) - camera->xPos.whole;
        head->posY.whole = BASE_Y(head) + OFFSET_Y(head) - camera->yPos.whole;
        if (SPEED(head) <= -0x40000) head->subState++;
    }
    else if (head->subState == 3)
    {
        x = Sin(ANGLE(head) + 0x4000);
        y = Sin(ANGLE(head));
        x = FixedMultiply(x, SPEED(head));
        y = FixedMultiply(y, SPEED(head));
        OFFSET_X(head) += x;
        OFFSET_Y(head) += y;
        head->posX.whole = BASE_X(head) + OFFSET_X(head) - camera->xPos.whole;
        head->posY.whole = BASE_Y(head) + OFFSET_Y(head) - camera->yPos.whole;
        if ((OFFSET_X(head) < 0 ? -OFFSET_X(head) : OFFSET_X(head)) < 0x40000
            && (OFFSET_Y(head) < 0 ? -OFFSET_Y(head) : OFFSET_Y(head)) < 0x40000)
            head->subState++;
    }
    else if (head->subState == 4)
    {
        head->subState++;
        owner->subState++;
        head->hidden = 1;
        return;
    }
    else if (head->subState == 5)
    {
        ObjAffine_Free(head->affineIndex);
        EntityDelete(head);
        head->subState++;
        return;
    }
    Anim_Start(head, (struct AnimScript *)&animation, 2, 1);
    Anim_Update(head);
    ObjAffine_SetBiased(ANGLE(head), 0x10000, 0x10000, head->flipX, head->flipY, head->affineIndex);
    Hit_CheckWithPlayerSharedHistory(bounds, head, owner);
}

/* Create whip segment `index` of ten. */
/**
 * @brief Create one indexed affine segment linked to the attack tip
 * and boss, with both contact and forwarded-damage callbacks.
 * Return NULL on entity or affine allocation failure.
 */
struct EwramData_EntityData *Headhunter_CreateAttackSegment(struct EwramData_EntityData *owner, struct EwramData_EntityData *parent, s32 index)
{
    struct EwramData_EntityData *segment = EntityCreateInRange(0x49, 0xCF, (u32 *)HeadhunterAttackSegment_Update);
    struct HitBoxRecord box = {-1, -1, 3, 3};
    struct SingleFrameAnimation animation = sHeadhunterAttackSegmentFrameAnim;
    u16 slot;
    s32 graphics;
    if (segment == NULL) goto fail;
    segment->posX.whole = parent->posX.whole;
    segment->posY.whole = parent->posY.whole;
    SEGMENT_INDEX(segment) = index;
    segment->spriteLayer = parent->spriteLayer + 1;
    graphics = owner->unk_500.whole;
    segment->unk_510.part8.basePalette = owner->unk_504.part8.unk_506;
    Sprite_Attach(segment, sHeadhunterSpriteSheet, graphics, segment->unk_510.part8.basePalette);
    Anim_Start(segment, (struct AnimScript *)&animation, 2, 1);
    Anim_Update(segment);
    if ((s32)((slot = ObjAffine_Alloc(1)) << 16) < 0)
    {
        EntityDelete(segment);
        goto fail;
    }
    segment->affineIndex = slot;
    segment->affine = 1;
    ObjAffine_SetBiased(0, 0x10000, 0x10000, segment->flipX, segment->flipY, segment->affineIndex);
    Hit_Enable(segment, 8, HeadhunterAttackSegment_DamagePlayerOnContact, (s32)HeadhunterAttackSegment_OnHit);
    HitBox_SetAttack(segment, &box);
    HitBox_SetBody(segment, &box);
    segment->hit.flags &= ~4;
    segment->hit.flags &= ~2;
    segment->attack = owner->attack;
    segment->unk_4F8.unk_4F8_entity = owner;
    segment->unk_4FC.unk_4FC_entity = parent;
    segment->scrollWithBg = 0;
    segment->drawFunc = (u32 *)Sprite_DrawEntity;
    return segment;
fail:
    return NULL;
}

/* Whip segment: index/10 of the way from the base to the head. */
/**
 * @brief Place the segment at index/10 of the tip displacement from
 * its stored room-space base. Share boss contact history and free
 * the affine slot on boss defeat or tip substate 5.
 */
void HeadhunterAttackSegment_Update(struct EwramData_EntityData *segment)
{
    struct EwramData_EntityData *owner = segment->unk_4F8.unk_4F8_entity;
    struct EwramData_EntityData *parent = segment->unk_4FC.unk_4FC_entity;
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    s32 bounds[4];
    struct SingleFrameAnimation animation = sHeadhunterAttackSegmentFrameAnim;
    if (owner->state == 2 || parent->subState == 5)
    {
        ObjAffine_Free(segment->affineIndex);
        EntityDelete(segment);
        return;
    }
    segment->posX.whole = parent->unk_4FC.whole + parent->unk_504.whole * SEGMENT_INDEX(segment) / 10 - camera->xPos.whole;
    segment->posY.whole = parent->unk_500.whole + parent->unk_4F4.whole * SEGMENT_INDEX(segment) / 10 - camera->yPos.whole;
    Anim_Start(segment, (struct AnimScript *)&animation, 2, 1);
    Anim_Update(segment);
    ObjAffine_SetBiased(parent->velX.whole, 0x10000, 0x10000, segment->flipX, segment->flipY, segment->affineIndex);
    Hit_CheckWithPlayerSharedHistory(bounds, segment, owner);
}
