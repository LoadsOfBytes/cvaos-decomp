/* Chaos effect updates and creation. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "code_080D73B8.h"
extern void Chaos2_SpawnEffect(struct EwramData_EntityData *, s32);
extern void EntityMove(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern s32 EntityIsOutsideScreen(struct EwramData_EntityData *, s32);
extern u8 sChaos2SpriteSheet[];
extern void Chaos2AffineEffect_Update(struct EwramData_EntityData *);
struct ChaosEffectGfx { s32 tiles; s32 palette; };

/* Move, bounce within the screen during the first 100 frames, and expire. */
void Chaos2SnakeShot_Update(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    struct EwramData_EntityData *owner = entity->ext.chaosChild.link->ext.chaosChild.link;
    EntityMove(entity);
    Hit_CheckWithPlayer(collision, entity);
    if (entity->timer < 100)
    {
        if ((u32)(entity->posX.whole - 0x80000) > 0xE00000)
            entity->velX.whole = -entity->velX.whole;
        if ((u32)(entity->posY.whole - 0x80000) > 0x900000)
            entity->velY.whole = -entity->velY.whole;
    }
    if (EntityIsOutsideScreen(entity, 1) || entity->timer++ > 0xEF)
    {
        EntityDelete(entity);
    }
    else if (owner != NULL && owner->updateFunc != NULL && owner->enemyId == entity->enemyId && owner->state != 3)
    {
        Anim_Update(entity);
    }
    else
    {
        EntityDelete(entity);
    }
}

/* Create an affine effect with initial transform parameters and a 60-frame timer. */
void Chaos2_CreateAffineEffect(struct EwramData_EntityData *parent)
{
    struct EwramData_EntityData *effect = EntityCreateInRange(0x49, 0xCF, (u32 *)Chaos2AffineEffect_Update);
    struct ChaosEffectGfx *gfx = (struct ChaosEffectGfx *)parent->ext.chaos.data.ptr;
    if (effect != NULL)
    {
        s32 tiles;
        u16 matrix;
        effect->posX.part16.integer = parent->posX.part16.integer;
        effect->posY.part16.integer = parent->posY.part16.integer;
        effect->spriteLayer = parent->spriteLayer + 1;
        tiles = gfx->tiles;
        effect->basePalette = gfx->palette;
        Sprite_Attach(effect, sChaos2SpriteSheet, tiles, effect->basePalette);
        effect->animFrame = 0x24;
        matrix = ObjAffine_Alloc(1);
        if ((s16)matrix < 0)
        {
            EntityDelete(effect);
        }
        else
        {
            effect->affineIndex = matrix;
            effect->affine = 1;
            ObjAffine_Set(0, 0x10000, 0x10000, effect->flipX, effect->flipY, effect->affineIndex);
            effect->enemyId = parent->enemyId;
            effect->ext.chaosChild.link = parent;
            effect->scrollWithBg = 0;
            effect->velY.whole = 0xB000;
            effect->ext.chaos2Effect.timer = 0x20;
            effect->timer = 0x3C;
            effect->drawFunc = (u32 *)Sprite_DrawEntity;
        }
    }
}



/* The velocity slots hold affine angle, scale, and angular step here.
 * The parent's lifetime and identity guard the allocated affine matrix. */
void Chaos2AffineEffect_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.chaosChild.link;
    if (parent != NULL && parent->updateFunc != NULL && parent->enemyId == entity->enemyId)
    {
        if (entity->state == 0)
        {
            if (entity->stage == 0)
            {
                if (--entity->timer < 2)
                    entity->stage++;
            }
            else if (entity->stage == 1)
            {
                entity->velY.whole += 0x100;
                if (entity->velY.whole > 0xFFFF)
                {
                    entity->velY.whole = 0x10000;
                    entity->stage++;
                }
            }
            else
            {
                entity->velY.whole -= 0x100;
                if (entity->velY.whole <= 0xB000)
                {
                    entity->velY.whole = 0xB000;
                    entity->stage = 0;
                    entity->timer = (RandomNumberGenerator() % 0x80) + 100;
                }
            }
            entity->velX.whole += 0x80;
            if (entity->ext.chaos2Effect.timer != 0)
                entity->ext.chaos2Effect.timer--;
            else
            {
                entity->ext.chaos2Effect.timer = (RandomNumberGenerator() % 0x40) + 0x10;
                Chaos2_SpawnEffect(entity, (RandomNumberGenerator() & 7) + 9);
            }
            if (parent->state == 3)
            {
                entity->state = 1;
                entity->accelX.whole = 0x180;
                entity->subState = 0;
                entity->ext.chaos2Effect.timer = 1;
                entity->timer = 16;
            }
        }
        else if (entity->state == 1)
        {
            if (entity->stage == 0)
            {
                if (--entity->timer < 2)
                {
                    entity->stage++;
                    PlaySong(0x14A);
                }
            }
            else if (entity->stage == 1)
            {
                entity->velY.whole += 0x400;
                if (entity->velY.whole > 0xFFFF)
                {
                    entity->velY.whole = 0x10000;
                    entity->stage++;
                }
            }
            else
            {
                entity->velY.whole -= 0x400;
                if (entity->velY.whole <= 0xB000)
                {
                    entity->velY.whole = 0xB000;
                    entity->stage = 0;
                    entity->timer = 16;
                }
            }
            entity->velX.whole += entity->accelX.whole;
            if (entity->accelX.whole < 0x300)
                entity->accelX.whole += 2;
            else
                entity->state = 2;
            if (entity->ext.chaos2Effect.timer != 0)
                entity->ext.chaos2Effect.timer--;
            else
            {
                entity->ext.chaos2Effect.timer = RandomNumberGenerator() % 0x10;
                Chaos2_SpawnEffect(entity, (RandomNumberGenerator() & 7) + 9);
            }
        }
        else if (entity->state == 2)
        {
            if (entity->stage == 0)
            {
                if (--entity->timer < 2)
                {
                    entity->stage++;
                    PlaySong(0x14A);
                }
            }
            else if (entity->stage == 1)
            {
                entity->velY.whole += 0x200;
                if (entity->velY.whole > 0xFFFF)
                {
                    entity->velY.whole = 0x10000;
                    entity->stage++;
                }
            }
            else
            {
                entity->velY.whole -= 0x200;
                if (entity->velY.whole <= 0xB000)
                {
                    entity->velY.whole = 0xB000;
                    entity->stage = 0;
                    entity->timer = 16;
                    if (entity->accelX.whole <= 0x40)
                        entity->state = 3;
                }
            }
            entity->velX.whole += entity->accelX.whole;
            if (entity->accelX.whole > 0x40)
                entity->accelX.whole -= 4;
            if (entity->ext.chaos2Effect.timer != 0)
                entity->ext.chaos2Effect.timer--;
            else
            {
                entity->ext.chaos2Effect.timer = (RandomNumberGenerator() % 0x20) + 0x20;
                Chaos2_SpawnEffect(entity, (RandomNumberGenerator() & 7) + 9);
            }
        }
        else if (entity->state == 3)
        {
            entity->velY.whole -= 0x200;
            if (entity->velY.whole <= 0x8000)
            {
                entity->velY.whole = 0x8000;
                parent->ext.chaos2.defeatReady = 1;
                entity->hidden = 1;
            }
            entity->velX.whole += entity->accelX.whole;
        }
        ObjAffine_Set(entity->velX.whole, entity->velY.whole, entity->velY.whole, entity->flipX, entity->flipY, entity->affineIndex);
    }
    else
    {
        ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
    }
}
