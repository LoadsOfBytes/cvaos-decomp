/* Triton attacks: homing shot and wave particles. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern void EntityMove(struct EwramData_EntityData *);
extern void EntitySetVelocityFromAngle(struct EwramData_EntityData *, s32, s32);
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern s32 Target_GetX(struct EwramData_EntityData *);
extern s32 Target_GetY(struct EwramData_EntityData *);
extern s32 Entity_GetVisibilityPattern(s32);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void TritonHomingShot_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void TritonHomingShot_OnHit(struct EwramData_EntityData *);
extern u8 sTritonGfx[];
extern u8 sTritonSpriteSheet[];
void TritonHomingShot_Update(struct EwramData_EntityData *entity);
void TritonWaveParticle_Update(struct EwramData_EntityData *entity);
#define SHOT_ACCEL(e) ((e)->accelX.whole)
#define SHOT_MAX_SPEED(e) ((e)->accelY.whole)
#define WAVE_ANGLE(e) ((e)->unk_4F8.whole)
#define WAVE_AGE(e) ((e)->unk_500.whole)

/* Create the homing shot (TritonHomingShot_Update) at the owner + (x, y), starting
 * towards the side the target is on. */
void Triton_SpawnHomingShot(struct EwramData_EntityData *owner, s32 x, s32 y)
{
    struct EwramData_EntityData *shot = EntityCreateInRange(0x49, 0xCF, (u32 *)TritonHomingShot_Update);
    struct HitBoxRecord box = {-2, -2, 4, 4};
    if (shot != NULL)
    {
        s32 graphics;
        shot->posX.whole = owner->posX.whole + x;
        shot->posY.whole = owner->posY.whole + y;
        shot->flipX = Entity_IsTargetRight(shot);
        shot->velX.whole = shot->flipX ? 0x8000 : -0x8000;
        shot->velY.whole = 0;
        SHOT_ACCEL(shot) = 0x800;
        SHOT_MAX_SPEED(shot) = 0xC000;
        shot->spriteLayer = owner->spriteLayer - 1;
        graphics = ObjGfx_Find(sTritonGfx);
        shot->unk_510.part8.basePalette = 7;
        Sprite_Attach(shot, sTritonSpriteSheet, graphics, 7);
        Anim_StartIndexed(shot, 7, 0, 0);
        Anim_Update(shot);
        Hit_Enable(shot, 8, TritonHomingShot_DamagePlayerOnContact, (s32)TritonHomingShot_OnHit);
        HitBox_SetAttack(shot, &box);
        HitBox_SetBody(shot, &box);
        shot->hit.flags &= ~4;
        shot->hit.flags &= ~2;
        shot->attack = owner->attack * 3 / 4;
        shot->unk_500.unk_500_entity = owner;
        shot->scrollWithBg = 1;
        shot->drawFunc = (u32 *)Sprite_DrawEntity;
    }
}
/* Homing shot: after 50 frames accelerate towards the target (at most
 * SHOT_MAX_SPEED horizontally and half of it vertically); despawn after 180
 * frames. */
void TritonHomingShot_Update(struct EwramData_EntityData *entity)
{
    s32 bounds[4];
    if (entity->animScriptIndex == 7 && entity->animDone)
        Anim_StartIndexed(entity, 8, 0, 1);
    if (entity->timer > 49)
    {
        s32 x = Target_GetX(entity) - entity->posX.whole;
        s32 y;
        {
            s32 target = Target_GetY(entity);
            s32 base = entity->posY.whole + 0x100000;
            y = target - base;
        }
        if (x < -0x10000)
        {
            entity->velX.whole -= SHOT_ACCEL(entity);
            if (entity->velX.whole <= -SHOT_MAX_SPEED(entity))
                entity->velX.whole = -SHOT_MAX_SPEED(entity);
        }
        else if (x > 0x10000)
        {
            entity->velX.whole += SHOT_ACCEL(entity);
            if (entity->velX.whole >= SHOT_MAX_SPEED(entity))
                entity->velX.whole = SHOT_MAX_SPEED(entity);
        }
        if (y < -0x10000)
        {
            entity->velY.whole -= SHOT_ACCEL(entity);
            if (entity->velY.whole <= -(SHOT_MAX_SPEED(entity) / 2))
                entity->velY.whole = -(SHOT_MAX_SPEED(entity) / 2);
        }
        else if (y > 0x10000)
        {
            entity->velY.whole += SHOT_ACCEL(entity);
            if (entity->velY.whole >= SHOT_MAX_SPEED(entity) / 2)
                entity->velY.whole = SHOT_MAX_SPEED(entity) / 2;
        }
    }
    EntityMove(entity);
    Anim_Update(entity);
    if (entity->timer++ > 179)
        EntityDelete(entity);
    else if (entity->timer & 1)
        Hit_CheckWithPlayer(bounds, entity);
}

/* Create the wave particle (TritonWaveParticle_Update) at the owner + (x, y), its
 * starting angle depending on the owner's facing. */
void Triton_SpawnWaveParticle(struct EwramData_EntityData *owner, s32 x, s32 y)
{
    struct EwramData_EntityData *effect = EntityCreateInRange(0x49, 0xCF, (u32 *)TritonWaveParticle_Update);
    if (effect != NULL)
    {
        s32 graphics;
        effect->posX.whole = owner->posX.whole + x;
        effect->posY.whole = owner->posY.whole + y;
        effect->spriteLayer = owner->spriteLayer - 1;
        graphics = ObjGfx_Find(sTritonGfx);
        Sprite_Attach(effect, sTritonSpriteSheet, graphics, 6);
        Anim_StartIndexed(effect, 6, 0, 1);
        Anim_Update(effect);
        effect->flipX = 0;
        effect->scrollWithBg = 1;
        effect->drawFunc = (u32 *)Sprite_DrawEntity;
        if (owner->flipX)
        {
            WAVE_ANGLE(effect) = 0x1F000;
            effect->timer = 24;
        }
        else
        {
            WAVE_ANGLE(effect) = 0x19000;
            effect->timer = 8;
        }
    }
}
/* Wave particle: swing the angle back and forth while moving, blink out
 * and despawn after 60 frames or off screen. */
void TritonWaveParticle_Update(struct EwramData_EntityData *entity)
{
    s32 bounds[4];
    if (entity->state == 0)
    {
        u32 time = (u8)++entity->timer;
        if (time <= 15)
            WAVE_ANGLE(entity) += 0x400;
        else if (time <= 30)
        {
            WAVE_ANGLE(entity) -= 0x400;
            if (time > 29) entity->timer = 0;
        }
        EntitySetVelocityFromAngle(entity, WAVE_ANGLE(entity), 0x10000);
        EntityMove(entity);
        entity->hidden = Entity_GetVisibilityPattern(60 - WAVE_AGE(entity));
        if (WAVE_AGE(entity)++ > 59)
            EntityDelete(entity);
        else if (EntityIsOutsideScreen(entity, 1))
            EntityDelete(entity);
    }
}
