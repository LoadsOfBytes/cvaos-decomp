/* Headhunter attacks: rolling wave (850A0-8524C), lightning bolt with its
 * impact (8538C-855C8) and the flicker effect (85694). */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

struct SingleFrameAnimation {
    u16 count;
    u16 format;
    u8 frame;
    u8 duration;
    u16 unk_6;
};
extern struct SingleFrameAnimation sHeadhunterFrameAnim;
extern u8 sHeadhunterSpriteSheet[];
extern void HeadhunterLightningBurstFlicker_Update(struct EwramData_EntityData *);
extern void HeadhunterWave_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void HeadhunterLightningImpact_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void HeadhunterLightningImpactFlicker_Update(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Effect_SpawnParticleAnim0(s32, s32, s32, s32, s32);
extern void EntityDampVelocityX(struct EwramData_EntityData *, s32);
extern void EntityAccelerateAndMove(struct EwramData_EntityData *, s32);
void HeadhunterWave_Update(struct EwramData_EntityData *entity);
void HeadhunterLightningBolt_Update(struct EwramData_EntityData *entity);
void Headhunter_SpawnLightningImpact(struct EwramData_EntityData *owner, struct EwramData_EntityData *source, s32 x, s32 y);
void HeadhunterLightningImpact_Update(struct EwramData_EntityData *entity);
void Headhunter_SpawnLightningImpactFlicker(struct EwramData_EntityData *owner, struct EwramData_EntityData *source, s32 x, s32 y);
/* bit 0 of the owner's unk_4FE ends its effects (TODO: meaning) */
#define OWNER_DONE(o) (o->ext.headhunter.flags & 1)
#define FRAME(e) (e->ext.headhunterEffect.frame)
#define ACCEL(e) ((e)->accelX.whole)

/* Create the effect HeadhunterLightningBurstFlicker_Update at the owner + (x, y). */
/**
 * @brief Create the lightning burst flicker at an owner-relative offset
 * using frame base 0x23 and the fourth palette. Preserve the factory
 * without an affine allocation despite the update cleanup branch.
 */
void Headhunter_SpawnLightningBurstFlicker(struct EwramData_EntityData *owner, s32 x, s32 y)
{
    struct EwramData_EntityData *effect = EntityCreateInRange(0x49, 0xCF, (u32 *)HeadhunterLightningBurstFlicker_Update);
    if (effect != NULL)
    {
        s32 graphics;
        effect->posX.whole = owner->posX.whole + x;
        effect->posY.whole = owner->posY.whole + y;
        effect->spriteLayer = owner->spriteLayer - 2;
        graphics = owner->ext.headhunter.graphics;
        effect->ext.headhunterEffect.graphics = graphics;
        effect->unk_510.part8.basePalette = owner->ext.headhunter.palette3;
        Sprite_Attach(effect, sHeadhunterSpriteSheet, graphics, effect->unk_510.part8.basePalette);
        FRAME(effect) = 0x23;
        effect->attack = owner->attack;
        ENTITY_OWNER(effect) = owner;
        effect->scrollWithBg = 1;
        effect->drawFunc = (u32 *)Sprite_DrawEntity;
    }
}

/* Create the rolling wave moving left (direction 0) or right (1), falling
 * towards y = 0xB0 in the room. */
/**
 * @brief Create a left/right moving wave with inward horizontal
 * acceleration and vertical velocity toward room Y 0xB0 over 128
 * updates. Install contact damage at 125 percent of owner strength.
 */
void Headhunter_SpawnWave(struct EwramData_EntityData *owner, s32 x, s32 y, s32 direction)
{
    struct EwramData_EntityData *wave = EntityCreateInRange(0x49, 0xCF, (u32 *)HeadhunterWave_Update);
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    struct HitBoxRecord box = {-4, -4, 8, 8};
    if (wave != NULL)
    {
        s32 graphics;
        wave->posX.whole = owner->posX.whole + x;
        wave->posY.whole = owner->posY.whole + y;
        switch (direction)
        {
        case 0:
            wave->flipX = 1;
            wave->velX.whole = -0x30000;
            ACCEL(wave) = 0x2000;
            break;
        case 1:
            wave->flipX = 0;
            wave->velX.whole = 0x30000;
            ACCEL(wave) = -0x2000;
            break;
        }
        wave->velY.whole = (0xB00000 - (wave->posY.whole + camera->yPos.whole)) / 128;
        wave->spriteLayer = owner->spriteLayer - 1;
        graphics = owner->ext.headhunter.graphics;
        wave->ext.headhunterEffect.graphics = graphics;
        wave->unk_510.part8.basePalette = owner->ext.headhunter.palette3;
        Sprite_Attach(wave, sHeadhunterSpriteSheet, graphics, wave->unk_510.part8.basePalette);
        FRAME(wave) = 0x26;
        Hit_Enable(wave, 8, HeadhunterWave_DamagePlayerOnContact, 0);
        HitBox_SetAttack(wave, &box);
        wave->hit.flags |= 4;
        wave->hit.flags &= ~2;
        wave->attack = owner->attack + (owner->attack >> 2);
        ENTITY_OWNER(wave) = owner;
        wave->scrollWithBg = 1;
        wave->drawFunc = (u32 *)Sprite_DrawEntity;
    }
}

/* Rolling wave: shed particles every 4 frames, reverse acceleration at 65,
 * grow, then fade out; despawn after 172 frames. */
/**
 * @brief Move the wave with frame growth and particles, reverse horizontal
 * acceleration at 65 and disable contact after 147. Preserve the
 * double horizontal damping; delete after 171 or owner cancel.
 */
void HeadhunterWave_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = ENTITY_OWNER(entity);
    s32 bounds[4];
    struct SingleFrameAnimation animation = sHeadhunterFrameAnim;
    if ((entity->timer & 3) == 0)
    {
        s32 x = entity->posX.whole + (RandomNumberGenerator() & 0x7FFFF) - 0x40000;
        s32 y = entity->posY.whole + (RandomNumberGenerator() & 0x7FFFF) - 0x40000;
        Effect_SpawnParticleAnim0(x, y, entity->velX.whole / 2, 0, entity->spriteLayer + 1);
    }
    if (++entity->timer == 65) ACCEL(entity) = -ACCEL(entity);
    if (entity->timer == 100) FRAME(entity)++;
    if (entity->timer == 106) FRAME(entity)++;
    if (entity->timer == 148) FRAME(entity) = 0x26;
    if (entity->timer == 154) FRAME(entity) = 0x28;
    if (entity->timer == 160) FRAME(entity) = 0x2A;
    if (entity->timer == 166) FRAME(entity) = 0x32;
    if (entity->timer > 147)
    {
        /* the original damps the horizontal velocity twice */
        EntityDampVelocityX(entity, 0x10);
        EntityDampVelocityX(entity, 0x10);
    }
    if (entity->timer <= 36 && (entity->timer & 3) == 0 && (s8)FRAME(entity) <= 47)
        FRAME(entity)++;
    EntityAccelerateAndMove(entity, 0x40000);
    Anim_Start(entity, (struct AnimScript *)&animation, 2, 1);
    animation.frame = FRAME(entity);
    Anim_Update(entity);
    if (entity->timer > 171)
        EntityDelete(entity);
    else
    {
        if (entity->timer <= 147) Hit_CheckWithPlayer(bounds, entity);
        if (OWNER_DONE(owner)) EntityDelete(entity);
    }
}

/* Create a lightning bolt at (x, y), kept inside the room x range
 * 0x68..0x197. */
/**
 * @brief Create a bolt at the supplied position, clamping room X to
 * 0x68..0x197. Copy owner graphics, damage and palette variant 3.
 */
void Headhunter_SpawnLightningBolt(struct EwramData_EntityData *owner, s32 x, s32 y)
{
    struct EwramData_EntityData *bolt = EntityCreateInRange(0x49, 0xCF, (u32 *)HeadhunterLightningBolt_Update);
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    if (bolt != NULL)
    {
        s32 graphics;
        bolt->posX.whole = x;
        bolt->posY.whole = y;
        if (x + camera->xPos.whole < 0x680000)
            bolt->posX.whole = 0x680000 - camera->xPos.whole;
        if (bolt->posX.whole + camera->xPos.whole > 0x1970000)
            bolt->posX.whole = 0x1970000 - camera->xPos.whole;
        bolt->flipX = RandomNumberGenerator() % 2;
        bolt->spriteLayer = owner->spriteLayer - 3;
        graphics = owner->ext.headhunter.graphics;
        bolt->ext.headhunterEffect.graphics = graphics;
        bolt->unk_510.part8.basePalette = owner->ext.headhunter.palette3;
        Sprite_Attach(bolt, sHeadhunterSpriteSheet, graphics, bolt->unk_510.part8.basePalette);
        bolt->ext.headhunterEffect.frame = 0x1A;
        bolt->objMode = 1;
        bolt->attack = owner->attack;
        ENTITY_OWNER(bolt) = owner;
        bolt->scrollWithBg = 1;
        bolt->drawFunc = (u32 *)Sprite_DrawEntity;
    }
}

/* Lightning bolt: at 20 frames create the impact on the ground (y 0xBF). */
/**
 * @brief Alternate bolt frames and create a ground impact at timer 20.
 * Delete on signed timer expiry or owner effect-cancel bit 0.
 */
void HeadhunterLightningBolt_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = ENTITY_OWNER(entity);
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    struct SingleFrameAnimation animation = sHeadhunterFrameAnim;
    if (entity->timer > 8) entity->objMode = 0;
    if (entity->timer == 20)
    {
        Headhunter_SpawnLightningImpact(entity, owner, entity->posX.whole, 0xBF0000 - camera->yPos.whole);
        PlaySong(0x75);
    }
    Anim_Start(entity, (struct AnimScript *)&animation, 2, 1);
    animation.frame = RandomNumberGenerator() % 2 + entity->ext.headhunterEffect.frame;
    Anim_Update(entity);
    if ((s8)entity->timer++ > 34)
        EntityDelete(entity);
    else if (OWNER_DONE(owner))
        EntityDelete(entity);
}

/* Create the bolt impact (hit box -2, -64, 4x64). */
/**
 * @brief Create the lightning ground impact at the supplied position,
 * with a 4-by-64 attack box, contact callback and 125 percent of
 * the bolt strength; retain both bolt and boss links.
 */
void Headhunter_SpawnLightningImpact(struct EwramData_EntityData *owner, struct EwramData_EntityData *source, s32 x, s32 y)
{
    struct EwramData_EntityData *impact = EntityCreateInRange(0x49, 0xCF, (u32 *)HeadhunterLightningImpact_Update);
    struct HitBoxRecord box = {-2, -64, 4, 64};
    if (impact != NULL)
    {
        s32 graphics;
        impact->posX.whole = x;
        impact->posY.whole = y;
        impact->spriteLayer = owner->spriteLayer + 1;
        graphics = owner->ext.headhunter.graphics;
        impact->ext.headhunterEffect.graphics = graphics;
        impact->unk_510.part8.basePalette = owner->unk_510.part8.basePalette;
        Sprite_Attach(impact, sHeadhunterSpriteSheet, graphics, impact->unk_510.part8.basePalette);
        impact->ext.headhunterEffect.frame = 0x1C;
        Hit_Enable(impact, 8, HeadhunterLightningImpact_DamagePlayerOnContact, 0);
        HitBox_SetAttack(impact, &box);
        impact->hit.flags |= 2;
        impact->hit.flags |= 4;
        impact->attack = owner->attack + (owner->attack >> 2);
        ENTITY_OWNER(impact) = owner;
        impact->ext.headhunterEffect.source = source;
        impact->scrollWithBg = 1;
        impact->drawFunc = (u32 *)Sprite_DrawEntity;
    }
}

/* Bolt impact: animate, spawn the flicker at 9 frames, vanish after 16. */
/**
 * @brief Change impact frames, adjust hit flags at timer 8 and create
 * the short flicker at 9. Check contact until removal after 15
 * or the boss effect-cancel bit; preserve the redundant limit check.
 */
void HeadhunterLightningImpact_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = entity->ext.headhunterEffect.source;
    s32 bounds[4];
    struct SingleFrameAnimation animation = sHeadhunterFrameAnim;
    if (++entity->timer == 5)
    {
        FRAME(entity)++;
        PlaySong(0x131);
    }
    if (entity->timer == 8)
    {
        FRAME(entity)++;
        entity->hit.flags &= ~2;
    }
    if (entity->timer == 9)
        Headhunter_SpawnLightningImpactFlicker(entity, owner, entity->posX.whole, entity->posY.whole);
    if (entity->timer > 7 && RandomNumberGenerator() % 2 == 0)
    {
        if ((s8)FRAME(entity) == 30) FRAME(entity) = 31;
        else FRAME(entity) = 30;
    }
    Anim_Start(entity, (struct AnimScript *)&animation, 2, 1);
    animation.frame = FRAME(entity);
    Anim_Update(entity);
    if (entity->timer > 15)
        EntityDelete(entity);
    else
    {
        if (entity->timer <= 32) Hit_CheckWithPlayer(bounds, entity);
        if (OWNER_DONE(owner)) EntityDelete(entity);
    }
}

/* Create the flicker effect HeadhunterLightningImpactFlicker_Update at an absolute position. */
/**
 * @brief Create the short impact flicker at the supplied position,
 * copying impact graphics and retaining its boss link for cancel.
 */
void Headhunter_SpawnLightningImpactFlicker(struct EwramData_EntityData *owner, struct EwramData_EntityData *source, s32 x, s32 y)
{
    struct EwramData_EntityData *effect = EntityCreateInRange(0x49, 0xCF, (u32 *)HeadhunterLightningImpactFlicker_Update);
    if (effect != NULL)
    {
        s32 graphics;
        effect->posX.whole = x;
        effect->posY.whole = y;
        effect->flipX = RandomNumberGenerator() % 2;
        effect->spriteLayer = owner->spriteLayer - 1;
        graphics = owner->ext.headhunter.graphics;
        effect->ext.headhunterEffect.graphics = graphics;
        effect->unk_510.part8.basePalette = owner->unk_510.part8.basePalette;
        Sprite_Attach(effect, sHeadhunterSpriteSheet, graphics, effect->unk_510.part8.basePalette);
        effect->ext.headhunterEffect.frame = 0x20;
        effect->attack = owner->attack;
        ENTITY_OWNER(effect) = owner;
        effect->ext.headhunterEffect.source = source;
        effect->scrollWithBg = 1;
        effect->drawFunc = (u32 *)Sprite_DrawEntity;
    }
}
