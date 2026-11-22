/* Is the selected target within RANGE of the entity on both axes? */
#include "structs/ewram.h"
#include "code_08009A0.h"

/* Position of the selected target (playerState.player when enemyFlags bit 2 is set,
 * otherwise linkTargetProxy or enemyTarget; without a target the entity itself).
 * The two spellings give the two code shapes of the original. */
#define TARGET_POS_A(entity, AXIS)                                   \
    ({                                                               \
        struct EwramData_EntityData *t_;                             \
        s32 v_;                                                      \
        if ((entity)->enemyFlags & 4)                                   \
            v_ = gEwramData->playerState.player->AXIS.whole;        \
        else if (!gEwramData->gameState.linkPlay)                      \
        {                                                            \
            t_ = gEwramData->playerState.enemyTarget;                    \
            if (t_ == NULL)                                          \
                v_ = (entity)->AXIS.whole;                           \
            else                                                     \
                v_ = t_->AXIS.whole;                                 \
        }                                                            \
        else                                                         \
            v_ = gEwramData->playerState.linkTargetProxy->AXIS.whole;        \
        v_;                                                          \
    })

#define TARGET_POS_B(entity, AXIS)                                   \
    ({                                                               \
        struct EwramData_EntityData *t_;                             \
        s32 v_;                                                      \
        if ((entity)->enemyFlags & 4)                                   \
            v_ = gEwramData->playerState.player->AXIS.whole;        \
        else if (!gEwramData->gameState.linkPlay)                      \
        {                                                            \
            t_ = gEwramData->playerState.enemyTarget;                    \
            v_ = (entity)->AXIS.whole;                               \
            if (t_ != NULL)                                          \
                v_ = t_->AXIS.whole;                                 \
        }                                                            \
        else                                                         \
            v_ = gEwramData->playerState.linkTargetProxy->AXIS.whole;        \
        v_;                                                          \
    })

/* |pos - target| along AXIS; the ?: inside the condition makes each arm
 * compare against RANGE on its own. */
#define TARGET_DIST(entity, AXIS)                                    \
    ((entity)->AXIS.whole - TARGET_POS_A(entity, AXIS) < 0           \
         ? TARGET_POS_B(entity, AXIS) - (entity)->AXIS.whole         \
         : (entity)->AXIS.whole - TARGET_POS_B(entity, AXIS))

/**
 * @brief Test both absolute fixed-point axis distances against the inclusive supplied range; missing default target falls back to the entity itself.
 */
u8 Entity_IsTargetWithinRange(struct EwramData_EntityData *entity, u32 range)
{
    if (TARGET_DIST(entity, posX) <= range && TARGET_DIST(entity, posY) <= range)
        return 1;
    return 0;
}
