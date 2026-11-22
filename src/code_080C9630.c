/* Lightning Doll: six-state animation sequence for display mode. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern void LightningDollPlayerEffect_Update(struct EwramData_EntityData *entity);
extern s32 Player_GetOverheadY(void);
extern u8 sCommonObjectGfx[];
extern u8 sCommonObjectSpriteSheet[];

#define ANIM(e,n,loop) do { if ((e)->animScriptIndex != (n)) Anim_StartIndexed((e),(n),0,(loop)); } while (0)

static inline s32 RandomFlip(s32 mask) { return RandomNumberGenerator() & mask; }
/**
 * @brief Cycle through six animation phases with completion and timer checks. The gameplay purpose of state 2 remains unresolved.
 */
void LightningDoll_UpdateState2Animation(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
        case 0:
            ANIM(entity, 0, 0);
            if (entity->animDone || entity->timer++ > 16)
            {
                entity->subState++;
                entity->timer = 0;
            }
            break;
        case 1:
            ANIM(entity, 1, 0);
            if (entity->animDone)
                entity->subState++;
            break;
        case 2:
            ANIM(entity, 2, 1);
            if (entity->timer++ > 64)
            {
                entity->timer = 0;
                entity->subState++;
            }
            break;
        case 3:
            ANIM(entity, 3, 0);
            if (entity->animDone)
                entity->subState++;
            break;
        case 4:
            ANIM(entity, 4, 1);
            if (entity->timer++ > 128)
            {
                entity->timer = 0;
                entity->subState++;
            }
            break;
        case 5:
            ANIM(entity, 5, 0);
            if (entity->timer++ > 48)
            {
                entity->timer = 0;
                entity->subState = 0;
            }
            break;
    }
    Anim_Update(entity);
}
