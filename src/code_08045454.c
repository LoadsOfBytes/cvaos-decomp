/* Effect_DrawPlayerFlash: growing flash effect that follows the player and toggles the blend. */
#include "code_08009A0.h"
#include "code_0800B700.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void Effect_SpawnPlayerOutwardParticle(u8 frame, u8 palette);

static inline s32 GrowX(s32 t)
{
    s32 r;

    if (t <= 15)
        r = 0x10000 - Sin((t << 10) + 0x4000);
    else
        r = 0x10000;
    return r;
}

static inline s32 GrowY(s32 t)
{
    s32 r;

    if (t <= 7)
        r = 0x2000;
    else if (t <= 15)
        r = 0x10000 - Sin(t << 11);
    else
        r = 0x10000;
    return r;
}

struct AffineSlot16 { u16 slot:5; u16 rest:3; };
/**
 * @brief Grow and draw the player-following flash, emit particles, pause at the existing timer point and release pause/affine state at expiration.
 */
void Effect_DrawPlayerFlash(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *player = gEwramData->playerState.player;
    s32 scaleX, scaleY;

    scaleX = GrowX(entity->timer);
    scaleY = GrowY(entity->timer);
    {
        u8 *flags = (u8 *)&entity->accelY + 5;
        *flags |= 4;
    }
    if (entity->timer <= entity->ext.playerFlash.duration)
        entity->timer++;
    if (entity->timer <= 2)
    {
        if (entity->timer == 2)
        {
            Game_SetMenusEnabled(0);
            gEwramData->paused = 1;
        }
        return;
    }
    if (entity->timer <= 0x27 && (entity->timer & 1))
        Effect_SpawnPlayerOutwardParticle(0x49, 6);
    if (entity->timer > entity->ext.playerFlash.duration)
    {
        ObjAffine_Free(entity->affineIndex);
        if (!(gEwramData->gameState.stateFlags & 0x100))
            Game_SetMenusEnabled(1);
        gEwramData->paused = 0;
        entity->deleteRequested = 1;
        return;
    }
    if (entity->timer <= 0x34 || !(entity->timer & 1))
    {
        entity->posY.part16.integer = player->posY.part16.integer - 0x20;
        entity->posX.part16.integer = player->posX.part16.integer;
        ObjAffine_Set(0, scaleX, scaleY, 0, 0, entity->affineIndex);
        Sprite_Draw(entity);
    }
}
