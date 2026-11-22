/* KillerMantle_UpdateJumpAttack (state 2 of the enemy created at 0x080BB70C). */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern void Entity_FaceTarget(struct EwramData_EntityData *entity);
extern s32 RandomSigned(s32 range);
extern void PlaySong(u16 song);
extern s32 Entity_Walk(struct EwramData_EntityData *entity, s32 speed, void *profile, s32 flags);
extern struct EwramData_EntityData *Entity_CreateObject(u32 *updateFunc);
extern void KillerMantleLandingEffect_Update(struct EwramData_EntityData *entity);
extern u8 sKillerMantleWalkProfile[];

/**
 * @brief BB518 | Jump attack (state 2): crouch (sprite 3, unk_4FE bit 0)
 * until the animation ends, then jump (sprite 4, velY about -0x1A000,
 * gravity 0x1000, forward speed unk_4F8 0x48000..0x49F00, box slot bit 1
 * off, song 0x89) slowing by 0x2000 per frame; on landing spawn
 * KillerMantleLandingEffect_Update, box slot bit 1 on, sprite 5 and slow down by 0x1000 per
 * frame until the animation ends, then state 1.
 */
void KillerMantle_UpdateJumpAttack(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child;

    switch (entity->subState)
    {
        case 0:
            Entity_FaceTarget(entity);
            Anim_StartIndexed(entity, 3, 0, 0);
            entity->unk_4FC.part16.unk_4FE[0] |= 1;
            entity->subState = 1;
        case 1:
            if (!(entity->animFlags & 4))
                break;
            entity->subState = 2;
            Anim_StartIndexed(entity, 4, 0, 0);
            entity->velY.whole = RandomSigned(0x2000) - 0x1A000;
            entity->accelY.whole = 0x1000;
            entity->unk_4F8.whole = (RandomNumberGenerator() % 32 << 8) + 0x48000;
            entity->hit.flags &= ~2;
            PlaySong(0x89);
            break;
        case 2:
            if (entity->unk_4F8.whole > 0)
                entity->unk_4F8.whole -= 0x2000;
            Entity_Walk(entity, entity->unk_4F8.whole, sKillerMantleWalkProfile, 0);
            if (!(entity->animFlags & 4))
                break;
            child = Entity_CreateObject((u32 *)KillerMantleLandingEffect_Update);
            if (child != NULL)
            {
                child->scrollWithBg = 1;
                child->unk_4F8.unk_4F8_entity = entity;
            }
            entity->hit.flags |= 2;
            entity->unk_4FC.part16.unk_4FE[0] &= ~1;
            entity->subState = 3;
            Anim_StartIndexed(entity, 5, 0, 0);
            break;
        case 3:
            if (entity->unk_4F8.whole > 0)
                entity->unk_4F8.whole -= 0x1000;
            Entity_Walk(entity, entity->unk_4F8.whole, sKillerMantleWalkProfile, 0);
            if (entity->animFlags & 4)
            {
                entity->state = 1;
                entity->stage = 0;
                entity->subState = 0;
            }
            break;
    }
}
