/* Beam Skeleton firing sequence and vertical affine scale. */
#include "code/code_08039340.h"
#include "beam_skeleton_entity.h"
#include "structs/ewram.h"
extern void BeamSkeleton_CreateBeam(struct EwramData_EntityData *entity);
#define ANIM(e,n,loop) do { if ((e)->animScriptIndex != (n)) Anim_StartIndexed((e),(n),0,(loop)); } while (0)
#define SCALE(e) (*(u16 *)&(e)->unk_504.part16.unk_504)

static inline u16 FullScale(void) { return 0xFFFF; }
/**
 * @brief Firing state (3): sequence animations and shared vertical affine
 * scale, create beam entities at frame 19 and maintain the beam lifetime
 * flag until recovery. Preserve countdown-underflow and fallthrough behavior.
 */
void BeamSkeleton_UpdateFire(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
        case 0:
            ANIM(entity,2,0);
            if (entity->animDone)
            {
                entity->subState = 1;
                entity->stage = 0;
            }
            break;
        case 1:
            switch (entity->stage)
            {
                case 0:
                    ANIM(entity,3,1);
                    entity->timer = 32;
                    entity->stage++;
                    SCALE(entity) = 0;
                    ObjAffine_Set(0,0x10000,0,entity->flipX,0,entity->affineIndex);
                case 1:
                    if (--entity->timer == 0xFF)
                    {
                        BEAM_SKELETON_FIRING(entity) |= 1;
                        entity->subState = 2;
                        entity->stage = 0;
                    }
                    break;
            }
            break;
        case 2:
            switch (entity->stage)
            {
                case 0:
                    SCALE(entity) = FullScale();
                    ANIM(entity,4,0);
                case 1:
                    if (entity->animFrame == 19)
                    {
                        BeamSkeleton_CreateBeam(entity);
                        entity->subState = 3;
                        entity->stage = 0;
                    }
                    break;
            }
            break;
        case 3:
            switch (entity->stage)
            {
                case 0:
                    ANIM(entity,5,1);
                    entity->timer = 42;
                    entity->stage++;
                case 1:
                {
                    s32 scale = SCALE(entity) + 0x2000;
                    s32 zero;
                    if (scale > 0xFFFF)
                        scale = 0xFFFF;
                    zero = 0;
                    SCALE(entity) = scale;
                    ObjAffine_Set(0,0x10000,SCALE(entity),entity->flipX,zero,entity->affineIndex);
                    if (--entity->timer == 0xFF)
                        entity->stage++;
                    break;
                }
                case 2:
                {
                    s32 scale = SCALE(entity) - 0x400;
                    s32 zero;
                    if (scale < 0)
                        scale = 0;
                    zero = 0;
                    SCALE(entity) = scale;
                    ObjAffine_Set(0,0x10000,SCALE(entity),entity->flipX,zero,entity->affineIndex);
                    if (SCALE(entity) == 0)
                    {
                        entity->timer = 8;
                        entity->subState = 4;
                        entity->stage = 0;
                    }
                    break;
                }
            }
            break;
        case 4:
            BEAM_SKELETON_FIRING(entity) &= 0xFE;
            if (--entity->timer == 0xFF)
            {
                entity->state = 1;
                entity->stage = 0;
                entity->subState = 0;
            }
            break;
    }
}
