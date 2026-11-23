#include "code_08009A0.h"
#include "code_0800B700.h"
#include "code_0800CB00.h"
#include "structs/ewram.h"

extern void Effect_SpawnPlayerInwardParticle(u8 palette);
extern void Effect_UpdateAttachedAnimation(struct EwramData_EntityData *entity);

/* Spawn an effect (update Effect_UpdateAttachedAnimation) attached to parent at offset (dx, dy). */
static inline void CreateAttachedEffect(struct EwramData_EntityData *parent, u8 animation, u8 palette, s32 dx, s32 dy)
{
    struct EwramData_EntityData *child;
    if (parent == NULL)
        return;
    child = EntityCreateInRange(0x49, 0xCF, (u32 *)Effect_UpdateAttachedAnimation);
    if (child == NULL)
        return;
    child->ext.particleAttach.anim = animation;
    child->ext.particleAttach.parent = parent;
    child->palette = palette;
    child->accelX.whole = dx;
    child->accelY.whole = dy;
    child->posX.whole = parent->posX.whole + dx;
    child->posY.whole = parent->posY.whole + dy;
    child->spriteLayer = animation;
}

/* Wait for the timer and a button press (A/B), then release the player and
 * spawn the effect above them. */
/**
 * @brief Pause and emit inward particles, then wait for A/B after the configured delay before releasing pause and spawning the attached effect.
 */
void SoulAcquisition_DrawAndWait(struct EwramData_EntityData *entity)
{
    UpdateInGameTimer();
    if (entity->timer == 0)
    {
        Game_SetMenusEnabled(0);
        gEwramData->paused = 1;
    }
    entity->roomActive = 1;
    if (entity->timer < 48)
        Effect_SpawnPlayerInwardParticle(entity->palette);
    if (entity->timer < entity->ext.soulAcquire.duration)
        entity->timer++;
    else
    {
        if (gEwramData->inputData.newInput & 3)
        {
            struct EwramData_EntityData *player;
            Game_SetMenusEnabled(1);
            gEwramData->paused = 0;
            entity->deleteRequested = 1;
            player = gEwramData->playerState.player;
            {
                /* the arguments live in registers across the null check */
                u8 animation = 12;
                s32 dy = -0x100000;
                CreateAttachedEffect(player, animation, 6, 0, dy);
            }
        }
        Hud_DrawWindowPrompt();
    }
}
