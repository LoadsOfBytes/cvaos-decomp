/* Effects with an entry of table 085251A8[unk_4FC] (frame, lifetime,
 * animation), graphics 081C15F4 / 0820ED60 with their own palette
 * (palette). */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"
struct Unk_085251A8 { u8 frame; u8 life; u16 pad; struct AnimScript *anim; };
extern struct Unk_085251A8 sUnk_085251A8[];
extern u8 sCommonObjectGfx[], sCommonObjectSpriteSheet[];
extern void Effect_DrawAtOffsetPair(struct EwramData_EntityData *);
extern u8 sTimedParticleFrames[];

/* Particle: random unk_500 / unk_504 in [-16, 16) px, drawn by
 * Effect_DrawAtOffsetPair, rising and accelerating until its lifetime runs out. */
/**
 * @brief Initialize a table-selected frame, random secondary draw offset and lifetime, then move with acceleration until the countdown expires.
 */
void Effect_UpdateTimedDoubleParticle(struct EwramData_EntityData *entity)
{
    struct Unk_085251A8 *info = &sUnk_085251A8[entity->unk_4FC.whole];
    switch (entity->state)
    {
    case 0:
    {
        s32 graphics = ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(entity, sCommonObjectSpriteSheet, graphics, entity->palette);
        entity->unk_500.whole = ((RandomNumberGenerator() & 0x1F) << 16) - 0x100000;
        entity->unk_504.whole = ((RandomNumberGenerator() & 0x1F) << 16) - 0x100000;
        entity->drawFunc = (u32 *)Effect_DrawAtOffsetPair;
        entity->animFrame = info->frame;
        entity->velY.whole = -0x10000;
        entity->timer = info->life;
        entity->state = 1;
        entity->scrollWithBg = 1;
        break;
    }
    case 1:
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
        entity->velX.whole += entity->accelX.whole;
        entity->velY.whole += entity->accelY.whole;
        if (--entity->timer == 0)
            entity->deleteRequested = 1;
        break;
    }
}

/* Animated effect moving with acceleration until the animation ends. */
/**
 * @brief Initialize the table-selected animation, move with acceleration in state one and request deletion when Anim_Update returns three.
 */
void Effect_UpdateTableAnimation(struct EwramData_EntityData *entity)
{
    struct Unk_085251A8 *info = &sUnk_085251A8[entity->unk_4FC.whole];
    switch (entity->state)
    {
    case 0:
    {
        s32 graphics = ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(entity, sCommonObjectSpriteSheet, graphics, entity->palette);
        Anim_Start(entity, info->anim, 0, 0);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->state = 1;
        entity->scrollWithBg = 1;
        break;
    }
    case 1:
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
        entity->velX.whole += entity->accelX.whole;
        entity->velY.whole += entity->accelY.whole;
        break;
    }
    if (Anim_Update(entity) == 3)
        entity->deleteRequested = 1;
}

/* Effect pinned to the entity at unk_4FC at offset (accelX, accelY) with
 * sound 0xBF and animation unk_4F8 until the animation ends; removed at
 * once without a parent. */
/**
 * @brief Follow the parent at stored offsets and animate until completion; request deletion immediately for a NULL parent.
 */
void Effect_UpdateAttachedAnimation(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->unk_4FC.unk_4FC_entity;
    if (parent == NULL)
    {
        entity->deleteRequested = 1;
        return;
    }
    switch (entity->state)
    {
    case 0:
    {
        s32 graphics;
        PlaySong(0xBF);
        graphics = ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(entity, sCommonObjectSpriteSheet, graphics, entity->palette);
        Anim_StartIndexed(entity, (u8)entity->unk_4F8.part8.unk_4F8, 0, 0);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->scrollWithBg = 1;
        entity->state++;
    }
    case 1:
        entity->posX.whole = parent->posX.whole + entity->accelX.whole;
        entity->posY.whole = parent->posY.whole + entity->accelY.whole;
        break;
    }
    if (Anim_Update(entity) == 3)
        entity->deleteRequested = 1;
}

/* Particle living unk_4FD frames: moves with acceleration, either rises
 * (unk_4FE == 0) or every 8 frames steps back through the frame table
 * 08525138 (subState). */
/**
 * @brief Move and accelerate, optionally rise or step backward through the frame table, draw and expire after the stored timer limit.
 */
void Effect_DrawTimedParticle(struct EwramData_EntityData *entity)
{
    u8 timer = entity->timer;
    if (timer > entity->unk_4FC.part8.unk_4FD)
    {
        entity->deleteRequested = 1;
        return;
    }
    entity->posX.whole += entity->velX.whole;
    entity->posY.whole += entity->velY.whole;
    entity->velX.whole += entity->accelX.whole;
    entity->velY.whole += entity->accelY.whole;
    entity->timer = timer + 1;
    if (entity->unk_4FC.part8.unk_4FE == 0)
        entity->velY.whole -= 0x1000;
    else if (((timer + 1) & 7) == 0 && entity->subState != 0)
    {
        entity->subState--;
        entity->animFrame = sTimedParticleFrames[entity->subState];
    }
    Sprite_Draw(entity);
}

/* Emit a particle (Effect_DrawTimedParticle, frame and palette given) six pixels above
 * the player, in a random direction at 1.5 px/frame decelerating by 1/64
 * of the speed per frame, for 20 frames. */
/**
 * @brief Spawn a short-lived particle six pixels above the player with a random direction and acceleration opposing its initial velocity.
 */
void Effect_SpawnPlayerOutwardParticle(u8 frame, u8 palette)
{
    struct EwramData **ewram = &gEwramData;
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, (u32 *)EntityUpdateNothing);
    s32 graphics, angle, vy;

    if (entity == NULL)
        return;
    entity->spriteLayer = 2;
    graphics = ObjGfx_Find(sCommonObjectGfx);
    Sprite_Attach(entity, sCommonObjectSpriteSheet, graphics, palette);
    entity->drawFunc = (u32 *)Effect_DrawTimedParticle;
    entity->animFrame = frame;
    entity->spriteLayer = 3;
    entity->unk_4FC.part8.unk_4FD = 0x14;
    {
        struct PlayerState *player = &(*ewram)->playerState;
        entity->posX.whole = player->player->posX.whole;
        entity->posY.whole = player->player->posY.whole - 0x60000;
    }
    angle = (RandomNumberGenerator() & 0xFF) << 8;
    entity->velX.whole = Sin(angle + 0x4000) * 24 / 16;
    vy = Sin(angle) * 24 / 16;
    entity->velY.whole = vy;
    entity->accelX.whole = -entity->velX.whole / 64;
    entity->accelY.whole = -vy / 64;
}

/* Emit a particle (Effect_DrawTimedParticle, given palette) on a 3 px circle around
 * the point 16 px above the player, moving inwards and stepping through the
 * frame table from entry 3, for 20 frames. */
/**
 * @brief Spawn a particle around the point sixteen pixels above the player, moving inward and stepping backward through the shared frame table.
 */
void Effect_SpawnPlayerInwardParticle(u8 palette)
{
    struct EwramData **ewram = &gEwramData;
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, (u32 *)EntityUpdateNothing);
    s32 graphics, angle, c, s, vx, vy;

    if (entity == NULL)
        return;
    entity->unk_4FC.part8.unk_4FE = 1;
    entity->subState = 3;
    entity->spriteLayer = 2;
    graphics = ObjGfx_Find(sCommonObjectGfx);
    Sprite_Attach(entity, sCommonObjectSpriteSheet, graphics, palette);
    entity->drawFunc = (u32 *)Effect_DrawTimedParticle;
    entity->animFrame = sTimedParticleFrames[entity->subState];
    entity->spriteLayer = 3;
    entity->unk_4FC.part8.unk_4FD = 0x14;
    angle = (RandomNumberGenerator() & 0xFF) << 8;
    c = Sin(angle + 0x4000);
    s = Sin(angle);
    {
        struct PlayerState *player = &(*ewram)->playerState;
        entity->posX.whole = player->player->posX.whole + c * 48;
        entity->posY.whole = player->player->posY.whole + s * 48 - 0x100000;
    }
    vx = -(c * 20) / 16;
    entity->velX.whole = vx;
    vy = -(s * 20) / 16;
    entity->velY.whole = vy;
    entity->accelX.whole = vx / 16;
    entity->accelY.whole = vy / 16;
}
