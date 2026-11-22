/* Student Witch ground attack and airborne spell phases. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"

/*
 * Student Witch: 16.16 walk speed passed to Entity_Walk (eased per step) and
 * the frame counter of the current attack step. Token-identical expansion.
 */
#define STUDENT_WITCH_SPEED(e) e->unk_4F8.whole
#define STUDENT_WITCH_TIMER(e) e->unk_4FC.part16.unk_4FC
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
        STUDENT_WITCH_TIMER(entity) = 0x78;
        /* fall through */
    case 1:
        switch (entity->animStep)
        {
        case 0: STUDENT_WITCH_SPEED(entity) = 0xC000; break;
        case 1:
        case 2:
            STUDENT_WITCH_SPEED(entity) -= 0x1200;
            if (STUDENT_WITCH_SPEED(entity) < 0) STUDENT_WITCH_SPEED(entity) = 0;
            break;
        case 3:
            if (STUDENT_WITCH_SPEED(entity) <= 0xBFFF) STUDENT_WITCH_SPEED(entity) += 0x4000;
            break;
        }
        Entity_Walk(entity, STUDENT_WITCH_SPEED(entity), sStudentWitchWalkProfile, 0xD);
        if (--STUDENT_WITCH_TIMER(entity) <= 0)
        {
            entity->state = 0;
            entity->stage = 0;
            entity->subState = 0;
        }
        else if (Target_IsNearEntity(entity, 0x64, 0x3C))
        {
            Anim_StartIndexed(entity, 0xA, 0, 0);
            entity->subState = 2;
            STUDENT_WITCH_SPEED(entity) = 0;
        }
        break;
    case 2:
        Entity_Walk(entity, STUDENT_WITCH_SPEED(entity), sStudentWitchWalkProfile, 0xC);
        if (entity->animStep > 8)
        {
            if (entity->animStep == 9)
            {
                if (entity->animTimer == 0) PlaySong(0x1CA);
                STUDENT_WITCH_SPEED(entity) = 0x20000;
            }
            else
            {
                STUDENT_WITCH_SPEED(entity) -= 0x2000;
                if (STUDENT_WITCH_SPEED(entity) < 0) STUDENT_WITCH_SPEED(entity) = 0;
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
        STUDENT_WITCH_SPEED(entity) = 0x8000;
        entity->velY.whole = -0xC000 - ((RandomNumberGenerator() & 7) << 11);
        entity->accelY.whole = 0x400;
        STUDENT_WITCH_TIMER(entity) = 0;
        entity->subState = 1;
        /* fall through */
    case 1:
        timer = (u16)STUDENT_WITCH_TIMER(entity) + 1;
        STUDENT_WITCH_TIMER(entity) = timer;
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
            if (entity->stage == 0 && STUDENT_WITCH_TIMER(entity) == 0x1E)
            {
                Anim_StartIndexed(entity, 9, 0, 0);
                entity->subState = 2;
            }
        }
        flags = Entity_Walk(entity, STUDENT_WITCH_SPEED(entity), sStudentWitchWalkProfile, 0);
        if (flags & 8) entity->subState = 3;
        break;
    case 2:
        STUDENT_WITCH_TIMER(entity)++;
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
        flags = Entity_Walk(entity, STUDENT_WITCH_SPEED(entity), sStudentWitchWalkProfile, 0);
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
