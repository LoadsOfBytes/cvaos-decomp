#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s8 ObjPalette_FindOrLoad(void *palette);
extern const s32 sUnk_0852559C[];

/**
 * @brief 4F160 | Floating background particle: state 0 loads graphics and
 * palette for kind unk_514 (0 or 2; deleteRequested on failure) and starts
 * animation unk_500 at a random frame; state 1 drifts on a sine path
 * (kind 3 uses a second angle unk_4FC), scrolls against the camera
 * velocity scaled by sUnk_0852559C[kind] and wraps around the screen
 * edges at a random position. Movement pauses while timeStopTimer is set.
 */
void BgParticle_Update(struct EwramData_EntityData *entity)
{
    s32 gfx;
    s32 pal;
    s32 angle;
    s32 vy;
    const s32 *k;
    struct BgLayerState *bg;

    switch (entity->state)
    {
    case 0:
        if (entity->unk_514.part16.spawnParam0 == 0)
        {
            gfx = ObjGfx_LoadResource((u8 *)0x081CBE5C, 0);
            if (gfx < 0)
                goto fail;
            pal = ObjPalette_FindOrLoad((void *)0x08209EE8);
            if (pal < 0)
                goto fail;
            Sprite_Attach(entity, (void *)0x0820FE44, gfx, pal);
        }
        else if (entity->unk_514.part16.spawnParam0 == 2)
        {
            gfx = ObjGfx_LoadResource((u8 *)0x081CBEB4, 0);
            if (gfx < 0 || (pal = ObjPalette_FindOrLoad((void *)0x0820A100)) < 0)
            {
            fail:
                entity->deleteRequested = 1;
                return;
            }
            Sprite_Attach(entity, (void *)0x08210DB0, gfx, pal);
        }
        Anim_StartIndexed(entity, entity->ext.bgParticle.kind, 0, 1);
        entity->animStep = RandomNumberGenerator() & 1;
        Anim_Update(entity);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->state++;
        break;
    case 1:
        if (!gEwramData->gameState.timeStopTimer)
        {
            entity->ext.bgParticle.angleX += 0x800;
            if (entity->ext.bgParticle.kind != 3)
            {
                entity->velX.whole = entity->accelX.whole;
                vy = entity->accelY.whole * Sin(entity->ext.bgParticle.angleX) / 0x10000;
            }
            else
            {
                entity->ext.bgParticle.angleY += 0xA00;
                entity->velX.whole = Sin(entity->ext.bgParticle.angleX + 0x4000) * entity->accelX.whole / 0x1000;
                vy = entity->accelY.whole * Sin(entity->ext.bgParticle.angleY) / 0x1000;
            }
            entity->velY.whole = vy;
            entity->posX.whole += entity->velX.whole;
            entity->posY.whole += vy;
            Anim_Update(entity);
        }
        {
            const s32 *tbl = sUnk_0852559C;
            k = &tbl[entity->ext.bgParticle.kind];
        }
        bg = &gEwramData->bgInfo[1];
        entity->posX.whole -= *k * bg->xVel.whole / 256;
        entity->posY.whole -= *k * bg->yVel.whole / 256;
        if ((s16)entity->posX.part16.integer > 0x110)
        {
            entity->posY.part16.integer = (RandomNumberGenerator() & 0x7F) + 1;
            entity->posX.part16.integer = -0x20;
        }
        else if ((s16)entity->posX.part16.integer < -0x20)
        {
            entity->posY.part16.integer = (RandomNumberGenerator() & 0x7F) + 1;
            entity->posX.part16.integer = 0x110;
        }
        else if ((s16)entity->posY.part16.integer > 0xC0)
        {
            entity->posY.part16.integer = -0x10;
            entity->posX.part16.integer = RandomNumberGenerator() & 0xBF;
        }
        else if ((s16)entity->posY.part16.integer < -0x20)
        {
            entity->posY.part16.integer = 0xB0;
            entity->posX.part16.integer = RandomNumberGenerator() & 0xBF;
        }
        break;
    }
}
