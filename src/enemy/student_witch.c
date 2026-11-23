/*
 * Student Witch.
 */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "code_080D73B8.h"
#include "code/code_08040A38.h"
#include "code/code_080211F0.h"

/* Student Witch ground movement, turning and attack choice. */
extern s32 Target_IsNearEntity(struct EwramData_EntityData *, s32, s32);
extern u8 sStudentWitchWalkProfile[];

/**
 * @brief Walking state (0): vary speed by animation step, turn at obstacles
 * and select the ground or airborne attack based on proximity and randomness.
 * Preserve the existing result-dependent stage/substate assignments.
 */
void StudentWitch_UpdateWalk(struct EwramData_EntityData *entity)
{
    s32 flags;
    u8 *frame;
    s32 currentFrame;
    s32 result;
    switch (entity->subState)
    {
    case 0:
        entity->subState = 1;
        /* fall through */
    case 1:
        currentFrame = entity->animStep;
        frame = &entity->animStep;
        switch (currentFrame)
        {
        case 0: entity->ext.studentWitch.speed = 0xC000; break;
        case 1:
        case 2:
            entity->ext.studentWitch.speed -= 0x1200;
            if (entity->ext.studentWitch.speed < 0) entity->ext.studentWitch.speed = 0;
            break;
        case 3:
            if (entity->ext.studentWitch.speed <= 0xBFFF) entity->ext.studentWitch.speed += 0x4000;
            break;
        }
        flags = Entity_Walk(entity, entity->ext.studentWitch.speed, sStudentWitchWalkProfile, 8);
        if (flags & 3) entity->flipX ^= 1;
        result = flags & 8;
        if (result)
        {
            if (*(u16 *)frame != 0) break;
            result = Target_IsNearEntity(entity, 0xA0, 0x3C);
            if (result)
            {
                result = RandomNumberGenerator() & 4;
                if (result)
                {
                    entity->state = 3;
                    entity->stage = 0;
                    entity->subState = 0;
                }
                else
                    goto falling;
            }
            else goto waiting;
        }
        else goto falling;
        break;
falling:
        entity->state = 2;
        entity->stage = result;
        entity->subState = result;
        break;
waiting:
        if (++entity->stage > 4)
        {
            entity->stage = result;
            entity->flipX ^= 1;
        }
        break;
    }
}

/* Student Witch ground attack and airborne spell phases. */
extern void Entity_FaceTarget(struct EwramData_EntityData *);
extern void Afterimage_Create(struct EwramData_EntityData *, s32, s32, s32);
extern void Witch_SpawnGroundProjectile(struct EwramData_EntityData *, s32, s32, s32);

/**
 * @brief Ground attack state (3): approach for up to 120 updates, play the
 * attack animation and return to walking when it finishes.
 */
void StudentWitch_UpdateGroundAttack(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        Entity_FaceTarget(entity);
        entity->subState = 1;
        entity->ext.studentWitch.timer = 0x78;
        /* fall through */
    case 1:
        switch (entity->animStep)
        {
        case 0: entity->ext.studentWitch.speed = 0xC000; break;
        case 1:
        case 2:
            entity->ext.studentWitch.speed -= 0x1200;
            if (entity->ext.studentWitch.speed < 0) entity->ext.studentWitch.speed = 0;
            break;
        case 3:
            if (entity->ext.studentWitch.speed <= 0xBFFF) entity->ext.studentWitch.speed += 0x4000;
            break;
        }
        Entity_Walk(entity, entity->ext.studentWitch.speed, sStudentWitchWalkProfile, 0xD);
        if (--entity->ext.studentWitch.timer <= 0)
        {
            entity->state = 0;
            entity->stage = 0;
            entity->subState = 0;
        }
        else if (Target_IsNearEntity(entity, 0x64, 0x3C))
        {
            Anim_StartIndexed(entity, 0xA, 0, 0);
            entity->subState = 2;
            entity->ext.studentWitch.speed = 0;
        }
        break;
    case 2:
        Entity_Walk(entity, entity->ext.studentWitch.speed, sStudentWitchWalkProfile, 0xC);
        if (entity->animStep > 8)
        {
            if (entity->animStep == 9)
            {
                if (entity->animTimer == 0) PlaySong(0x1CA);
                entity->ext.studentWitch.speed = 0x20000;
            }
            else
            {
                entity->ext.studentWitch.speed -= 0x2000;
                if (entity->ext.studentWitch.speed < 0) entity->ext.studentWitch.speed = 0;
            }
        }
        if (!(entity->animFlags & 4)) break;
        entity->subState = 3;
        /* fall through */
    case 3:
        Anim_StartIndexed(entity, 7, 0, 1);
        entity->state = 0;
        entity->stage = 0;
        entity->subState = 0;
        break;
    }
}

/**
 * @brief Air attack state (2): jump, optionally create afterimages or cast a
 * ground projectile, and return to walking on landing; cap downward speed.
 */
void StudentWitch_UpdateAirAttack(struct EwramData_EntityData *entity)
{
    s32 flags;
    s32 timer;
    switch (entity->subState)
    {
    case 0:
        Entity_FaceTarget(entity);
        Anim_StartIndexed(entity, 8, 0, 1);
        entity->ext.studentWitch.speed = 0x8000;
        entity->velY.whole = -0xC000 - ((RandomNumberGenerator() & 7) << 11);
        entity->accelY.whole = 0x400;
        entity->ext.studentWitch.timer = 0;
        entity->subState = 1;
        /* fall through */
    case 1:
        timer = (u16)entity->ext.studentWitch.timer + 1;
        entity->ext.studentWitch.timer = timer;
        if (entity->stage == 0)
        {
            s32 signedTimer = (s16)timer;
            if (signedTimer > 0x14)
            {
                if (signedTimer <= 0x1D)
                {
                    if (!(RandomNumberGenerator() & 0x3F))
                    {
                        Afterimage_Create(entity, 0, 0x17, 0x11);
                        Afterimage_Create(entity, 0, 0x10, 0x10);
                        entity->velY.whole -= 0xC000;
                        entity->stage = 1;
                    }
                }
            }
            if (entity->stage == 0 && entity->ext.studentWitch.timer == 0x1E)
            {
                Anim_StartIndexed(entity, 9, 0, 0);
                entity->subState = 2;
            }
        }
        flags = Entity_Walk(entity, entity->ext.studentWitch.speed, sStudentWitchWalkProfile, 0);
        if (flags & 8) entity->subState = 3;
        break;
    case 2:
        entity->ext.studentWitch.timer++;
        if (entity->stage == 0 && entity->animStep == 6)
        {
            PlaySong(0x1CB);
            PlaySong(0x81);
            Witch_SpawnGroundProjectile(entity, -0xC, -0x14, 0);
            entity->stage = 1;
        }
        if (entity->animFlags & 4)
        {
            Anim_StartIndexed(entity, 8, 0, 1);
            entity->subState = 1;
        }
        flags = Entity_Walk(entity, entity->ext.studentWitch.speed, sStudentWitchWalkProfile, 0);
        if (flags & 8) entity->subState = 3;
        break;
    case 3:
        Anim_StartIndexed(entity, 7, 0, 1);
        entity->state = 0;
        entity->stage = 0;
        entity->subState = 0;
        break;
    }
    if (entity->velY.whole > 0x10000) entity->velY.whole = 0x10000;
}

/**
 * @brief Death state (4): emit rising particles for the countdown, spawn
 * a ground projectile in mode 2 and return nonzero for removal.
 */
s32 StudentWitch_UpdateDeath(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *particle;
    s32 y;
    switch (entity->subState)
    {
    case 0:
        entity->subState = 1;
        entity->drawSkip = 1;
        entity->timer = 0x20;
        /* fall through */
    case 1:
        if (!(entity->timer & 1))
        {
            u32 timer = entity->timer;
            y = (-(timer >> 1) + 4) << 16;
            particle = Effect_SpawnShared(entity->posX.whole + (Sin(timer << 13) << 3), entity->posY.whole + y, 5, 6);
            if (particle != NULL)
            {
                particle->velY.whole = -0x18000;
                particle->accelY.whole = 0x800;
            }
        }
        if (entity->timer != 0) entity->timer--;
        else entity->subState = 2;
        break;
    case 2:
        Witch_SpawnGroundProjectile(entity, 0, -0x10, 2);
        return 1;
    }
    return 0;
}

/* Witch and Student Witch initialisation and state dispatchers. */
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *);
extern void Entity_SnapToGround(struct EwramData_EntityData *);
extern void Enemy_OnContact(struct EwramData_EntityData *);
extern void StudentWitch_OnHit(struct EwramData_EntityData *, s32, u16 *);
extern void Witch_OnHit(struct EwramData_EntityData *, s32, u16 *);
extern void Entity_EnableBoxes(struct EwramData_EntityData *, void *, void *);
extern s32 Hit_EnableContact(struct EwramData_EntityData *, s32, void *, s32);
extern s32 Enemy_IsStunned(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *);
extern void Witch_UpdateWait(struct EwramData_EntityData *);
extern void Witch_UpdateCast(struct EwramData_EntityData *);
extern void Witch_UpdateCharge(struct EwramData_EntityData *);
extern s32 Witch_UpdateDeath(struct EwramData_EntityData *);
extern u8 sWitchGfx[], sWitchPalette[], sWitchSpriteSheet[];

void EnemyStudentWitchCreate(struct EwramData_EntityData *entity)
{
    if (!Entity_LoadGraphics(entity, sWitchGfx, sWitchPalette, 0, sWitchSpriteSheet))
    {
        EntityDelete(entity);
        return;
    }
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer = 0x13;
    Anim_StartIndexed(entity, 7, 0, 1);
    Anim_Update(entity);
    if (gEwramData->gameState.bestiaryPreview) entity->state = 5;
    else
    {
        Hit_EnableContact(entity, 8, Enemy_OnContact, (s32)StudentWitch_OnHit);
        Entity_UpdateBoxes(entity);
        Entity_SnapToGround(entity);
        entity->posY.part16.integer++;
        entity->scrollWithBg = 1;
        entity->state = 0;
    }
}

void EnemyStudentWitchUpdate(struct EwramData_EntityData *entity)
{
    u32 box[4];
    u32 outside = EntityIsOutsideScreen(entity, 4);
    outside = (entity->hidden = outside);
    if (!outside) Player_TrackEntity(entity);
    if (Enemy_IsStunned(entity))
    {
        if (entity->state != 4) Hit_CheckWithPlayer(box, entity);
        return;
    }
    switch (entity->state)
    {
    case 0: StudentWitch_UpdateWalk(entity); break;
    case 3: StudentWitch_UpdateGroundAttack(entity); break;
    case 2: StudentWitch_UpdateAirAttack(entity); break;
    case 4:
        if (StudentWitch_UpdateDeath(entity)) { EntityDelete(entity); return; }
        break;
    case 5:
        Anim_Update(entity);
        return;
    }
    if (entity->state != 4) Hit_CheckWithPlayer(box, entity);
    Entity_AnimateWithBoxes(entity);
}
