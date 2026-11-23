/* Student Witch ground attack and airborne spell phases. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"

extern void Entity_FaceTarget(struct EwramData_EntityData *);
extern s32 Target_IsNearEntity(struct EwramData_EntityData *, s32, s32);
extern void Afterimage_Create(struct EwramData_EntityData *, s32, s32, s32);
extern void Witch_SpawnGroundProjectile(struct EwramData_EntityData *, s32, s32, s32);
extern u8 sStudentWitchWalkProfile[];

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
