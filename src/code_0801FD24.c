/* Julius_UpdateGrandCrossCast. */
#include "code_08009A0.h"
#include "player_entity.h"
#include "code/code_08040A38.h"

void Anim_StartIndexed(struct EwramData_EntityData *entity, s32 script, s32 mode, s32 flags);

/* Animation switch without hitbox, as in src/code_0801F00C.c (flags 1, 1). */
static inline void SetPlayerAnim(struct EwramData_EntityData *entity, s32 anim)
{
    if (!(gEwramData->playerState.actionLockFlags & 0x800) && entity->animScriptIndex != anim)
    {
        Anim_StartIndexed(entity, anim, 1, 1);
        entity->ext.player.requestedAnim = 0xFF;
        entity->ext.player.actionFlags &= 0xFFDFFFFF;
    }
}

/**
 * @brief Julius state 10, entered for the sub-weapon entry with unk_4 set
 * (entry 3, Grand Cross): converging particles and animation 0xA,
 * then animation 0xB for 0x10 updates, then 0xA until return to
 * state 0. Vertical motion is held at zero.
 */
void Julius_UpdateGrandCrossCast(struct EwramData_EntityData *entity)
{
    {
        u8 four = 4;
        gEwramData->playerState.player->ext.player.invulnerableTimer = four;
    }
    gEwramData->playerState.actionLockFlags |= 0x20;
    entity->velY.whole = 0;
    entity->accelY.whole = 0;
    switch (entity->subState)
    {
    case 0:
        if (!(entity->timer & 1))
        {
            s16 angle = RandomNumberGenerator();
            s32 radius = (RandomNumberGenerator() & 0x3F) + 0x40;
            s32 dx = radius * Sin(angle + 0x4000);
            s32 dy = radius * Sin(angle);
            s32 x = entity->posX.whole + dx;
            s32 y = dy - 0x100000;
            struct EwramData_EntityData *effect = Effect_CreateSharedFrame(x, entity->posY.whole + y, 0x40, 6);

            if (effect != NULL)
            {
                s32 vx, vy;

                effect->timer = 0x20;
                effect->velX.whole = vx = -dx / 64;
                effect->velY.whole = vy = -dy / 64;
                effect->accelX.whole = vx / 32;
                effect->accelY.whole = vy / 32;
            }
        }
        SetPlayerAnim(entity, 0xA);
        if (entity->timer == 0)
        {
            entity->timer = 0x60;
            entity->subState++;
        }
        else
            entity->timer--;
        break;
    case 1:
        SetPlayerAnim(entity, 0xB);
        if (entity->timer == 0)
        {
            entity->timer = 0x10;
            entity->subState++;
        }
        else
            entity->timer--;
        break;
    case 2:
        SetPlayerAnim(entity, 0xA);
        if (entity->timer != 0)
            entity->timer--;
        else
        {
            entity->state = 0;
            entity->ext.player.actionFlags &= 0xFFDFFEFF;
        }
        break;
    }
}
