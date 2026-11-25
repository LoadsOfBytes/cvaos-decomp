/*
 * Chaos (final boss): first form, orbs, rings, shots and marks, second form, its parts, snake and effects.
 */
#include "core.h"
#include "music.h"
#include "player_level.h"
#include "graphics.h"
#include "structs/ewram.h"
#include "player_core.h"
#include "sound.h"
#include "syscalls.h"
#include "structs/main.h"
#include "gameplay.h"
#include "entity_util.h"
#include "map_screen.h"
#include "map_flags.h"
#include "boss_defeat.h"
#include "damage_numbers.h"
#include "effects.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "entity_motion.h"
#include "hitbox.h"
#include "math_anim.h"
#include "oam.h"
#include "obj_gfx.h"
#include "obj_palette.h"
#include "palette.h"
#include "data/data_118B48.h"
#include "data/data_4F1444.h"
#include "player_soma.h"
#include "soul_pickup.h"
#include "entity_hit.h"

/* Chaos core creation. */
/* allocation of 10 words in the Chaos core's unk_504 */
struct ChaosData {
    s32 graphics;
    s32 palette;
    s32 palette2;
    s32 bgRotation;
    s32 bgAngle;
    s32 bgScale;
    s32 bgRadius;
    struct EwramData_EntityData *orbs[3];
};

extern void *memcpy(void *dst, const void *src, unsigned long size);

s32 Chaos_StartSecondForm(struct EwramData_EntityData *);
u8 Chaos_LoadGraphics(struct EwramData_EntityData *);
struct EwramData_EntityData *Chaos_CreateBackground(struct EwramData_EntityData *);
struct EwramData_EntityData *Chaos_CreateOuterRing(struct EwramData_EntityData *, u16, u16, u16);
struct EwramData_EntityData *Chaos_CreateInnerRing(struct EwramData_EntityData *, u16, u16, u16);
struct EwramData_EntityData *Chaos_CreateOrb(struct EwramData_EntityData *, u16, s32, s32);
void Chaos_CreateMark(struct EwramData_EntityData *, s32);
void ChaosOuterRing_Update(struct EwramData_EntityData *entity);
void ChaosInnerRing_Update(struct EwramData_EntityData *entity);
void ChaosOrb_Update(struct EwramData_EntityData *entity);
void ChaosOrb_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit);
void ChaosOrb_CreateHalo(struct EwramData_EntityData *, s32);
struct EwramData_EntityData *ChaosOrb_CreateAura(struct EwramData_EntityData *orb, s32 pal, s32 kind, s32 timer);
void ChaosOrb_CreateShellPiece(struct EwramData_EntityData *orb, s32 frame, s32 timer);
void ChaosOrb_SpawnSeedShot(struct EwramData_EntityData *orb, s32 x, s32 y, s32 delay);
void ChaosOrb_SpawnSpark(struct EwramData_EntityData *orb, s32 x, s32 y);
void ChaosOrb_SpawnSoulShot(struct EwramData_EntityData *orb, s32 x, s32 y);
void ChaosShellPiece_Update(struct EwramData_EntityData *entity);
void ChaosSeedShot_Update(struct EwramData_EntityData *);
void ChaosSeedShot_OnHit(struct EwramData_EntityData *, struct EwramData_EntityData *, u16 *);
void ChaosSeedShot_DamagePlayerOnContact(struct EwramData_EntityData *);
void ChaosSeedShot_Update(struct EwramData_EntityData *entity);
void ChaosSeedShot_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit);
void ChaosSpark_Update(struct EwramData_EntityData *);
void ChaosSpark_Draw(struct EwramData_EntityData *);
void ChaosSpark_OnContact(struct EwramData_EntityData *);
void ChaosShellPiece_Update(struct EwramData_EntityData *entity);
void ChaosSeedShot_Update(struct EwramData_EntityData *entity);
void ChaosSeedShot_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit);
void ChaosSoulShot_DamagePlayerOnContact(struct EwramData_EntityData *);
void ChaosSoulShot_Update(struct EwramData_EntityData *);
void ChaosMark_Update(struct EwramData_EntityData *);
void ChaosHalo_Update(struct EwramData_EntityData *entity);
void ChaosAura_Update(struct EwramData_EntityData *);
void ChaosMark_Update(struct EwramData_EntityData *entity);
void ChaosHalo_Update(struct EwramData_EntityData *entity);
void ChaosAura_Update(struct EwramData_EntityData *entity);
void Chaos_UpdateIdle(struct EwramData_EntityData *);
u8 EnemyChaos2Create(struct EwramData_EntityData *);
void EnemyChaos2Update(struct EwramData_EntityData *);
void Chaos2_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit);
void Chaos2_CreateAffineEffect(struct EwramData_EntityData *);
void Chaos2_StepPaletteSequence(struct EwramData_EntityData *);
void Chaos2_SpawnShot(struct EwramData_EntityData *, s32, s32, s32);
void Chaos2_SummonPart(struct EwramData_EntityData *, s32, s32, s32, s32);
void Chaos2_SpawnEffectAbove(struct EwramData_EntityData *, s32);
void Chaos_StartPaletteSequence(struct EwramData_EntityData *entity, s32 sequence);
void Chaos2_CreatePart(struct EwramData_EntityData *core, s32 x, s32 y, s32 mirror);
void Chaos2_CreateSnake(struct EwramData_EntityData *core, s32 x, s32 y, s32 unused);
void Chaos2_SpawnSpinningDebris(struct EwramData_EntityData *, s32, s32);
void Chaos2_SpawnShard(struct EwramData_EntityData *, s32, s32);
void Chaos2Part_Update(struct EwramData_EntityData *);
void Chaos2Part_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit);
void Chaos2Part_DamagePlayerOnContact(struct EwramData_EntityData *);
void Chaos2Shot_Update(struct EwramData_EntityData *);
void Chaos2Shot_DamagePlayerOnContact(struct EwramData_EntityData *);
void Chaos2SummonedPart_Update(struct EwramData_EntityData *);
void Chaos2SummonedPart_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit);
void Chaos2SummonedPart_OnContact(struct EwramData_EntityData *);
void Chaos2_SpawnSpark(struct EwramData_EntityData *);
void Chaos2Spark_Update(struct EwramData_EntityData *);
void Chaos2SnakeHead_Update(struct EwramData_EntityData *);
void Chaos2SnakeSegment_Update(struct EwramData_EntityData *);
void Chaos2SnakeSegment_EnableHitBoxes(struct EwramData_EntityData *);
void Chaos2SnakeHead_DamagePlayerOnContact(struct EwramData_EntityData *);
void Chaos2SnakeHead_FireShot(struct EwramData_EntityData *, s32);
void Chaos2SnakeSegment_DamagePlayerOnContact(struct EwramData_EntityData *entity);
void Chaos2SnakeShot_Update(struct EwramData_EntityData *);
void Chaos2SnakeShot_DamagePlayerOnContact(struct EwramData_EntityData *);
void Chaos2_SpawnEffect(struct EwramData_EntityData *, s32);
void Chaos2AffineEffect_Update(struct EwramData_EntityData *);
void Chaos2Effect_Update(struct EwramData_EntityData *entity);
void Chaos2SpinningDebris_Update(struct EwramData_EntityData *entity);
void Chaos2_UpdateIdle(struct EwramData_EntityData *entity);
void Chaos2Shard_Update(struct EwramData_EntityData *entity);

/* Create the Chaos core with its background, two rings and three orbs
 * (spawnParam1 set: the other form Chaos_StartSecondForm); after the fight only the
 * frozen state 4. */
void EnemyChaosCreate(struct EwramData_EntityData *core)
{
    struct ChaosData *data;

    if (core->spawnParam1 != 0)
    {
        if (Chaos_StartSecondForm(core) == 0)
            return;
        EntityDelete(core);
        return;
    }
    if (core->spawnParam0 != 0 && gEwramData->gameState.bossRushMode == 0
        && (gEwramData->gameState.bossEventFlags & 0x80))
    {
        EntityDelete(core);
        return;
    }
    data = Heap_Alloc(10);
    core->ext.chaos.data.value = (s32)data;
    if (data == NULL)
    {
        EntityDelete(core);
        return;
    }
    if (!Chaos_LoadGraphics(core))
    {
        EntityDelete(core);
        return;
    }
    {
        if (gEwramData->gameState.bestiaryPreview)
        {
            core->state = 4;
            return;
        }
        if (Chaos_CreateBackground(core) == NULL)
        {
            EntityDelete(core);
            return;
        }
        if (Chaos_CreateInnerRing(core, 0x4000, 0x8000, 2) == NULL)
        {
            EntityDelete(core);
            return;
        }
        if (Chaos_CreateOuterRing(core, 0, 0x4000, 3) == NULL)
        {
            EntityDelete(core);
            return;
        }
        if ((data->orbs[0] = Chaos_CreateOrb(core, 0x4000, 0, 0)) == NULL)
        {
            EntityDelete(core);
            return;
        }
        if ((data->orbs[1] = Chaos_CreateOrb(core, 0x8500, 0, 1)) == NULL)
        {
            EntityDelete(core);
            return;
        }
        if ((data->orbs[2] = Chaos_CreateOrb(core, 0xFB00, 0, 2)) == NULL)
        {
            EntityDelete(core);
            return;
        }
        Anim_StartIndexed(core, 0, 0, 1);
        Anim_Update(core);
        core->hidden = 1;
        Enemy_ResetFromTemplate(core);
        core->hit.flags |= 4;
        core->hit.flags |= 2;
        core->state = 0;
        Player_SetSelectedTarget(core);
        gEwramData->gameState.stateFlags |= 2;
        gEwramData->gameState.stateFlags |= 1;
        gEwramData->gameState.stateFlags |= 0x200000;
        gEwramData->gameState.stateFlags |= 0x800000;
        gEwramData->startPauseEnabled = 0;
        core->timer = 0;
        core->ext.chaos.unusedRandom = RandomNumberGenerator() % 64;
        core->ext.chaos.attackTimer0 = (RandomNumberGenerator() & 0x7F) + 60;
        core->ext.chaos.attackTimer1 = (RandomNumberGenerator() & 0x7F) + 120;
        core->ext.chaos.attackTimer2 = (RandomNumberGenerator() & 0x7F) + 60;
        Music_SetCurrent(0x1F);
        data->bgScale = 0xC0;
        data->bgRadius = 0x40000;
    }
}

/* Chaos core: fight loop (1FEC). */

#define ACTIVE(e) (e->ext.chaos.activeOrbs)  /* orbs currently attacking */
#define BROKEN(e) (e->ext.chaos.brokenOrbs)  /* orbs destroyed */

/* Fight: turn the background, let each living orb attack after a random
 * pause; when all three orbs are broken switch to state 3. */
void Chaos_UpdateFight(struct EwramData_EntityData *core)
{
    struct ChaosData *data = (struct ChaosData *)core->ext.chaos.data.value;

    if (core->subState != 0)
        return;
    data->bgRotation += 0x80;
    data->bgAngle += 0x200;
    if (!(BROKEN(core) & 1) && !(ACTIVE(core) & 1))
    {
        if (core->ext.chaos.attackTimer0 != 0)
            core->ext.chaos.attackTimer0--;
        else
        {
            core->ext.chaos.attackTimer0 = RandomNumberGenerator() % 128 + 60;
            ACTIVE(core) |= 1;
            core->ext.chaos.busy = 0;
        }
    }
    if (!(BROKEN(core) & 2) && !(ACTIVE(core) & 2))
    {
        if (core->ext.chaos.attackTimer1 != 0)
            core->ext.chaos.attackTimer1--;
        else
        {
            core->ext.chaos.attackTimer1 = RandomNumberGenerator() % 128 + 120;
            ACTIVE(core) |= 2;
            core->ext.chaos.busy = 0;
        }
    }
    if (!(BROKEN(core) & 4) && !(ACTIVE(core) & 4))
    {
        if (core->ext.chaos.attackTimer2 != 0)
            core->ext.chaos.attackTimer2--;
        else
        {
            core->ext.chaos.attackTimer2 = RandomNumberGenerator() % 128 + 60;
            ACTIVE(core) |= 4;
            core->ext.chaos.busy = 0;
        }
    }
    if ((BROKEN(core) & 7) == 7)
    {
        core->state = 3;
        core->subState = 0;
        core->stage = 0;
    }
}

/* Chaos core: intro (20D0). (signed modulo through a separate variable). */
/* Intro: hold the player, fade from white, shake the screen, show the
 * marks and let the three orbs fire at the player, then start the fight. */
void Chaos_UpdateIntro(struct EwramData_EntityData *core)
{
    struct ChaosData *data = (struct ChaosData *)core->ext.chaos.data.value;

    SetPlayerInput(2, 0);
    switch (core->subState)
    {
    case 0:
        core->subState = 1;
        BgPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0, 0x10000, 0x78);
        ObjPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0, 0x10000, 0x78);
        core->timer = 0x78;
        core->stage = 0;
        Player_StartScriptedAnimation(0x32, 1, 0, 0);
        gEwramData->startPauseEnabled = 0;
        break;
    case 1:
        Player_StartScriptedAnimation(0x32, 1, 0, 0);
        if (--core->timer == 0)
        {
            ObjPalette_StopColorFade();
            BgPalette_StopColorFade();
            core->subState++;
            core->timer = 0;
        }
        break;
    case 2:
        if (!(core->timer & 4))
            Player_StartScriptedAnimation(0x52, 0, 0, 0x8000);
        else
            Player_StartScriptedAnimation(0x52, 0, 0, -0x8000);
        if (core->timer == 0x51)
        {
            Chaos_CreateMark(core, 0);
            Chaos_CreateMark(core, 1);
            Chaos_CreateMark(core, 2);
        }
        if (core->timer >= 0x29 && core->timer <= 0x95 && (core->timer & 7) == 0)
        {
            s32 v = core->timer >> 3;
            s32 n = v % 3;

            switch (n)
            {
            case 0:
                Effect_CreateHomingOrb(data->orbs[0], (s16)gEwramData->playerState.player->posX.part16.integer,
                             (s16)gEwramData->playerState.player->posY.part16.integer - 0x10, 0, n);
                break;
            case 1:
                Effect_CreateHomingOrb(data->orbs[1], (s16)gEwramData->playerState.player->posX.part16.integer,
                             (s16)gEwramData->playerState.player->posY.part16.integer - 0x10, 0, n);
                break;
            case 2:
                Effect_CreateHomingOrb(data->orbs[2], (s16)gEwramData->playerState.player->posX.part16.integer,
                             (s16)gEwramData->playerState.player->posY.part16.integer - 0x10, 0, n);
                break;
            }
        }
        if (core->timer++ > 0xF9)
        {
            gEwramData->gameState.stateFlags &= ~1;
            gEwramData->startPauseEnabled = 1;
            core->state = 1;
            core->subState = 0;
            core->stage = 0;
            Player_LockAndCancelAllSouls();
        }
        break;
    }
}

/* Chaos core defeat (22A0). */

#define ROT_SPEED(e) ((e)->velX.whole)
#define ANGLE_SPEED(e) ((e)->velY.whole)
#define SCALE_SPEED(e) ((e)->accelX.whole)

/* Defeat: the background spins faster, closes in and grows, the screen
 * fades to black and the ending sequence sChaosDestinationRoom starts. */
void Chaos_UpdateDefeat(struct EwramData_EntityData *core)
{
    struct ChaosData *data = (struct ChaosData *)core->ext.chaos.data.value;

    SetPlayerInput(2, 0);
    switch (core->subState)
    {
    case 0:
        Chaos_SpawnDefeatEffect(core->posX.whole, core->posY.whole);
        core->subState++;
        core->timer = 120;
        ROT_SPEED(core) = 0x80;
        ANGLE_SPEED(core) = 0x200;
        SCALE_SPEED(core) = 0;
        gEwramData->gameState.stateFlags |= 1;
        gEwramData->startPauseEnabled = 0;
        break;
    case 1:
        if ((s8)--core->timer <= 0)
        {
            core->subState++;
            core->stage = 0;
            SCALE_SPEED(core) = 1;
            core->timer = 0;
            PlaySong(0x149);
        }
        break;
    case 2:
        if (ROT_SPEED(core) <= 0x7FF)
            ROT_SPEED(core) += 4;
        if (ANGLE_SPEED(core) <= 0x3FF)
            ANGLE_SPEED(core) = 0x100;
        ANGLE_SPEED(core) += 0x20;
        if (data->bgRadius > 0)
        {
            data->bgRadius -= 0x800;
            if (data->bgRadius < 0)
                data->bgRadius = 0;
        }
        if (core->timer <= 59)
            core->timer++;
        else if (core->timer <= 89)
        {
            core->timer++;
            SCALE_SPEED(core) = 2;
        }
        else if (core->timer <= 119)
        {
            core->timer++;
            SCALE_SPEED(core) = 3;
        }
        if (data->bgScale > 0x200 && core->stage == 0)
        {
            BgPalette_StartColorFade(0, 0, 0, 0x10000, 0, 0xC0);
            ObjPalette_StartColorFade(0, 0, 0, 0x10000, 0, 0xD0);
            PlaySong(0x1010);
            core->stage++;
        }
        if (core->stage == 1 && !ObjPalette_GetPendingFades())
        {
            BgPalette_StopColorFade();
            ObjPalette_StopColorFade();
            RoomJump_SetDestination(sChaosDestinationRoom, 0, 0, 0x50, 0xA0);
            RoomJump_Begin(&gEwramData->gameState);
            gEwramData->gameState.stateFlags &= ~1;
        }
        break;
    }
    data->bgRotation += ROT_SPEED(core);
    data->bgAngle += ANGLE_SPEED(core);
    data->bgScale += SCALE_SPEED(core);
}

/* Chaos core damage (2454) and its rotating background (24D4). */

#define TO_PIXELS(d) ((d) < 0 ? -(-(d) >> 16) : (d) >> 16)

/* Core hit: lose HP; at 0 start dying (state 3, BossDefeatEffect_Spawn at the core). */
void Chaos_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
{
    s32 damage = Hit_ResolveDamage(entity, attacker, hit, Player_CalculateAttackPower(hit[2]));

    entity->hitFlashTimer = 8;
    {
        u16 hp = entity->hp;

        damage <<= 16;
        damage >>= 16;
        entity->hp = hp - damage;
    }
    Entity_ShowDamage(entity, damage, 0);
    if ((s16)entity->hp <= 0)
    {
        Enemy_DropRewards(entity);
        entity->palette = entity->basePalette;
        entity->timer = 60;
        entity->state = 3;
        entity->subState = 0;
        BossDefeatEffect_Spawn(entity->posX.whole, entity->posY.whole);
    }
}

/* Background entity of the core: set the BG2 affine transform (centre
 * (0x78, 0x50) moved on a circle, scale and rotation from ChaosData). */
void ChaosBackground_Update(struct EwramData_EntityData *entity)
{
    struct BgAffineSrcData src;
    struct EwramData_EntityData *core = entity->ext.chaosChild.link;
    struct ChaosData *data = (struct ChaosData *)core->ext.chaos.data.value;

    if (core == NULL || core->updateFunc == NULL
        || core->enemyId != entity->enemyId)
        return;
    src.texX = 0x8000;
    src.texY = 0x8000;
    src.scrX = TO_PIXELS(FixedMultiply(Sin(data->bgAngle + 0x4000), data->bgRadius)) + 0x78;
    src.scrY = TO_PIXELS(FixedMultiply(Sin(data->bgAngle), data->bgRadius)) + 0x50;
    src.sx = data->bgScale;
    src.sy = src.sx;
    src.alpha = data->bgRotation;
    BgAffineSet(&src, (struct BgAffineDstData *)&gDisplayRegisters.bg2PA, 1);
}

/* Chaos: the two rotating rings (create 25A4/27F8, update 26BC/2910). */

#define SPIN(e) (*(u16 *)&e->ext.chaosRing.spin)
#define ORBIT(e) (*(u16 *)&e->ext.chaosRing.orbit)
#define SPIN_BASE(e) (*(u16 *)&e->ext.chaosRing.spinOffset)
#define ORBIT_BASE(e) (*(u16 *)&e->ext.chaosRing.orbitOffset)
#define CENTER_X(e) (e->ext.chaosRing.centerX)
#define CENTER_Y(e) (e->ext.chaosRing.centerY)
#define RADIUS(e) (e->ext.chaosRing.radius)

/* Create the outer ring (frame 0) around (0x78, 0x50). */
struct EwramData_EntityData *Chaos_CreateOuterRing(struct EwramData_EntityData *owner, u16 spin, u16 orbit, u16 radius)
{
    struct EwramData_EntityData *ring = EntityCreateInRange(0x49, 0xCF, ChaosOuterRing_Update);
    struct ChaosData *data = (struct ChaosData *)owner->ext.chaos.data.value;
    s32 graphics;
    u16 slot;

    if (ring == NULL)
        return NULL;
    ring->posX.part16.integer = 0xFF80;
    ring->posY.part16.integer = 0xFF80;
    CENTER_X(ring) = 0x78;
    CENTER_Y(ring) = 0x50;
    ring->spriteLayer = 0x1A;
    graphics = data->graphics;
    ring->basePalette = data->palette;
    Sprite_Attach(ring, sChaosSpriteSheet, graphics, ring->basePalette);
    ring->animFrame = 0;
    if ((s32)((slot = ObjAffine_Alloc(1)) << 16) < 0)
    {
        EntityDelete(ring);
        return NULL;
    }
    ring->affineIndex = slot;
    ring->affine = 1;
    ObjAffine_Set(0, 0x10000, 0x10000, ring->flipX, ring->flipY, ring->affineIndex);
    RADIUS(ring) = radius;
    SPIN_BASE(ring) = spin;
    ORBIT_BASE(ring) = orbit;
    ring->enemyId = owner->enemyId;
    ring->ext.chaosChild.link = owner;
    ring->scrollWithBg = 0;
    ring->hit.flags |= 4;
    ring->hit.flags |= 2;
    ring->drawFunc = Sprite_DrawEntity;
    return ring;
}

/* Outer ring: spin and orbit (unless the game is paused by stateFlags bit 0),
 * blink out for 60 frames once the owner enters state 3. */
void ChaosOuterRing_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = entity->ext.chaosChild.link;
    struct EwramData *ewram = gEwramData;
    s32 x, y;

    if (owner != NULL && owner->updateFunc != NULL && owner->enemyId == entity->enemyId)
    {
        if (entity->state == 0)
        {
            if (!(ewram->gameState.stateFlags & 1))
            {
                SPIN(entity) += 0x70;
                ORBIT(entity) += 0x1E0;
            }
            if (owner->state == 3)
            {
                entity->state++;
                entity->timer = 60;
            }
        }
        else if (entity->state == 1)
        {
            SPIN(entity) += 0x70;
            ORBIT(entity) += 0x1E0;
            entity->hidden = Entity_GetVisibilityPattern(entity->timer);
            if ((s8)--entity->timer <= 0)
            {
                ObjAffine_Free(entity->affineIndex);
                EntityDelete(entity);
                return;
            }
        }
        x = Sin(ORBIT(entity) + ORBIT_BASE(entity) + 0x4000) * RADIUS(entity);
        y = Sin(ORBIT(entity) + ORBIT_BASE(entity)) * RADIUS(entity);
        entity->posX.whole = (CENTER_X(entity) << 16) + x;
        entity->posY.whole = (CENTER_Y(entity) << 16) + y;
        ObjAffine_Set(SPIN(entity) + SPIN_BASE(entity), 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
        return;
    }
    ObjAffine_Free(entity->affineIndex);
    EntityDelete(entity);
}

/* Create the inner ring (frame 1). */
struct EwramData_EntityData *Chaos_CreateInnerRing(struct EwramData_EntityData *owner, u16 spin, u16 orbit, u16 radius)
{
    struct EwramData_EntityData *ring = EntityCreateInRange(0x49, 0xCF, ChaosInnerRing_Update);
    struct ChaosData *data = (struct ChaosData *)owner->ext.chaos.data.value;
    s32 graphics;
    u16 slot;

    if (ring == NULL)
        return NULL;
    ring->posX.part16.integer = 0xFF80;
    ring->posY.part16.integer = 0xFF80;
    CENTER_X(ring) = 0x78;
    CENTER_Y(ring) = 0x50;
    ring->spriteLayer = 0x1A;
    graphics = data->graphics;
    ring->basePalette = data->palette;
    Sprite_Attach(ring, sChaosSpriteSheet, graphics, ring->basePalette);
    ring->animFrame = 1;
    if ((s32)((slot = ObjAffine_Alloc(1)) << 16) < 0)
    {
        EntityDelete(ring);
        return NULL;
    }
    ring->affineIndex = slot;
    ring->affine = 1;
    ObjAffine_Set(0, 0x10000, 0x10000, ring->flipX, ring->flipY, ring->affineIndex);
    RADIUS(ring) = radius;
    SPIN_BASE(ring) = spin;
    ORBIT_BASE(ring) = orbit;
    ring->enemyId = owner->enemyId;
    ring->ext.chaosChild.link = owner;
    ring->scrollWithBg = 0;
    ring->hit.flags |= 4;
    ring->hit.flags |= 2;
    ring->drawFunc = Sprite_DrawEntity;
    return ring;
}

/* Inner ring: like ChaosOuterRing_Update, slower and spinning the other way. */
void ChaosInnerRing_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = entity->ext.chaosChild.link;
    struct EwramData *ewram = gEwramData;
    s32 x, y;

    if (owner != NULL && owner->updateFunc != NULL && owner->enemyId == entity->enemyId)
    {
        if (entity->state == 0)
        {
            if (!(ewram->gameState.stateFlags & 1))
            {
                SPIN(entity) += 0x60;
                ORBIT(entity) += 0x1C0;
            }
            if (owner->state == 3)
            {
                entity->state++;
                entity->timer = 60;
            }
        }
        else if (entity->state == 1)
        {
            SPIN(entity) += 0x60;
            ORBIT(entity) += 0x1C0;
            entity->hidden = Entity_GetVisibilityPattern(entity->timer);
            if ((s8)--entity->timer <= 0)
            {
                ObjAffine_Free(entity->affineIndex);
                EntityDelete(entity);
                return;
            }
        }
        x = Sin(ORBIT(entity) + ORBIT_BASE(entity) + 0x4000) * RADIUS(entity);
        y = Sin(ORBIT(entity) + ORBIT_BASE(entity)) * RADIUS(entity);
        entity->posX.whole = (CENTER_X(entity) << 16) + x;
        entity->posY.whole = (CENTER_Y(entity) << 16) + y;
        ObjAffine_Set(-(SPIN(entity) + SPIN_BASE(entity)), 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
        return;
    }
    ObjAffine_Free(entity->affineIndex);
    EntityDelete(entity);
}

/* Create one of the three orbs (kind 0-2, 3000 HP, hit by ChaosOrb_OnHit,
 * box 0x350ADFFB = x -5, y -33, w 10, h 53) on the core's circle. */
struct EwramData_EntityData *Chaos_CreateOrb(struct EwramData_EntityData *owner, u16 spin, s32 anim, s32 kind)
{
    u32 box;
    struct EwramData_EntityData *orb = EntityCreateInRange(0x49, 0xCF, ChaosOrb_Update);
    struct ChaosData *data;
    s32 graphics;

    box = 0x350ADFFB;
    data = (struct ChaosData *)owner->ext.chaos.data.value;
    if (orb != NULL)
    {
        orb->posX.part16.integer = 0xFF80;
        orb->posY.part16.integer = 0xFF80;
        CENTER_X(orb) = 0x78;
        CENTER_Y(orb) = 0x50;
        orb->spriteLayer = 0x12;
        graphics = data->graphics;
        orb->basePalette = data->palette;
        Sprite_Attach(orb, sChaosSpriteSheet, graphics, orb->basePalette);
        Anim_StartIndexed(orb, anim, 2, 0);
        Anim_Update(orb);
        SPIN_BASE(orb) = spin;
        orb->enemyId = owner->enemyId;
        orb->ext.chaosChild.link = owner;
        orb->scrollWithBg = 0;
        orb->ext.chaosOrb.kind = kind;
        orb->attack = 0x50;
        orb->hp = 3000;
        Hit_Enable(orb, 8, NULL, ChaosOrb_OnHit);
        HitBox_SetBody(orb, &box);
        HitBox_SetAttack(orb, &box);
        orb->hit.flags &= ~4;
        orb->hit.flags |= 2;
        orb->drawFunc = Sprite_DrawEntity;
        switch (kind)
        {
        case 0:
            ChaosOrb_CreateAura(orb, 1, 0, 0xDC);
            break;
        case 1:
            ChaosOrb_CreateAura(orb, 0, 1, 0xDC);
            break;
        case 2:
            ChaosOrb_CreateAura(orb, 2, 2, 0xDC);
            break;
        }
        ChaosOrb_CreateHalo(orb, 0xDC);
        return orb;
    }
    return NULL;
}

/* Chaos orb: circles the core, opens when the core asks for an attack, breaks.
 * (failure block at the end via goto, nested
 * position tests so cross-jump shares the compare). */

#define OFS_X(e) (e->ext.chaosOrb.offsetX)
#define OFS_Y(e) (e->ext.chaosOrb.offsetY)
#define KIND(e) ((u8)e->ext.chaosOrb.kind)
#define REQUEST(c) (c->ext.chaos.activeOrbs)
#define BUSY(c) (c->ext.chaos.busy)

/**
 * @brief Orb update: ride on the core's circle with its halo and aura, attack
 * when the core allows it (seed shots, sparks or soul shots by kind) and
 * break apart when its HP is gone (state 10, shell pieces); delete with
 * the core.
 */
void ChaosOrb_Update(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    struct EwramData_EntityData *core = entity->ext.chaosChild.link;
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    struct ChaosData *data = (struct ChaosData *)core->ext.chaos.data.value;
    s32 x, y, limit;

    if (core != NULL && core->updateFunc != NULL && core->enemyId == entity->enemyId)
    {
        if (entity->state == 0)
        {
            x = (Sin(data->bgAngle + 0x4000) >> 14) + OFS_X(entity) + 2;
            y = (Sin(data->bgAngle) >> 14) + OFS_Y(entity) - 12;
            entity->posX.whole = Sin(0x4000 - (data->bgRotation + (u16)entity->ext.chaosOrb.angle)) * 60;
            entity->posY.whole = Sin(-(data->bgRotation + (u16)entity->ext.chaosOrb.angle)) * 60;
            entity->posX.part16.integer += x;
            entity->posY.part16.integer += y;
            x = entity->posX.whole + bg->xPos.whole;
            y = entity->posY.whole + bg->yPos.whole;
            switch (entity->subState)
            {
            case 0:
                if (KIND(entity) == 0)
                {
                    if (!(REQUEST(core) & 1))
                        break;
                    if (x > 0x7FFFFF)
                    {
                        if (y <= 0xA00000)
                            goto busy;
                    }
                    else if (y <= 0x700000)
                        goto busy;
                    entity->subState = 10;
                    entity->stage = 0;
                }
                else if (KIND(entity) == 1)
                {
                    if (!(REQUEST(core) & 2))
                        break;
                    if (x > 0x7FFFFF)
                    {
                        if ((u32)(y - 0x800001) > 0xFFFFE)
                            goto busy;
                    }
                    else if (y > 0x8FFFFF)
                        goto busy;
                    entity->subState = 30;
                    entity->stage = 0;
                }
                else if (KIND(entity) == 2)
                {
                    if (!(REQUEST(core) & 4))
                        break;
                    entity->subState = 20;
                    entity->stage = 0;
                }
                break;
            case 10:
                switch (entity->stage)
                {
                case 0:
                    entity->stage = 1;
                    ChaosOrb_SpawnSeedShot(entity, 0, -0x300000, 20);
                    ChaosOrb_SpawnSeedShot(entity, -0x200000, -0x140000, 50);
                    ChaosOrb_SpawnSeedShot(entity, 0x200000, -0x140000, 80);
                    entity->timer = 100;
                    break;
                case 1:
                    if ((s8)--entity->timer <= 0)
                    {
                        entity->subState = 0;
                        BUSY(core) = 1;
                        REQUEST(core) &= ~1;
                    }
                    break;
                }
                break;
            case 20:
                switch (entity->stage)
                {
                case 0:
                    entity->stage = 1;
                    ChaosOrb_SpawnSpark(entity, 0, 0);
                    entity->timer = 250;
                    break;
                case 1:
                    if (--entity->timer <= 1)
                    {
                        entity->subState = 0;
                        BUSY(core) = 1;
                        REQUEST(core) &= ~4;
                    }
                    break;
                }
                break;
            case 30:
                switch (entity->stage)
                {
                case 0:
                    entity->stage = 1;
                    ChaosOrb_SpawnSoulShot(entity, 0, 0);
                    entity->timer = 250;
                    entity->hidden = 1;
                    entity->hit.flags |= 4;
                    entity->ext.chaosOrb.shotDone = 0;
                    break;
                case 1:
                    if (entity->ext.chaosOrb.shotDone)
                    {
                        entity->subState = 0;
                        BUSY(core) = 1;
                        entity->hidden = 0;
                        entity->hit.flags &= ~4;
                        REQUEST(core) &= ~2;
                    }
                    break;
                }
                break;
            case 40:
                if (Entity_GetVisibilityPattern(entity->timer--) == 1)
                    entity->palette = 4;
                else
                    entity->palette = entity->basePalette;
                if (entity->timer == 0)
                {
                    entity->subState = 0;
                    entity->palette = entity->basePalette;
                busy:
                    BUSY(core) = 1;
                }
                break;
            }
            Anim_Update(entity);
            Hit_CheckWithPlayer(&result, entity);
            Enemy_UpdateHitFlash(entity);
        }
        else if (entity->state == 10)
        {
            x = (Sin(data->bgAngle + 0x4000) >> 14) + OFS_X(entity) + 2;
            y = (Sin(data->bgAngle) >> 14) + OFS_Y(entity) - 12;
            entity->posX.whole = Sin(0x4000 - (data->bgRotation + (u16)entity->ext.chaosOrb.angle)) * 60;
            entity->posY.whole = Sin(-(data->bgRotation + (u16)entity->ext.chaosOrb.angle)) * 60;
            entity->posX.part16.integer += x;
            entity->posY.part16.integer += y;
            if (entity->subState == 0)
            {
                entity->subState++;
                entity->hidden = 1;
                entity->timer = 120;
                ChaosOrb_CreateShellPiece(entity, 0, 20);
                ChaosOrb_CreateShellPiece(entity, 1, 32);
                ChaosOrb_CreateShellPiece(entity, 2, 44);
                PlaySong(0x71);
                switch (KIND(entity))
                {
                case 0:
                    core->ext.chaos.brokenOrbs |= 1;
                    break;
                case 1:
                    core->ext.chaos.brokenOrbs |= 2;
                    break;
                case 2:
                    core->ext.chaos.brokenOrbs |= 4;
                    break;
                }
            }
            else if (entity->subState == 1)
            {
                if (entity->timer > 0x50 && !(entity->timer & 7))
                    Effect_CreateHomingOrb(gEwramData->playerState.player, (s16)entity->posX.part16.integer,
                                 (s16)entity->posY.part16.integer, -0x100000, KIND(entity));
                if ((s8)--entity->timer <= 0)
                    entity->subState++;
            }
            else if (entity->subState == 2)
            {
                switch (KIND(entity))
                {
                case 0:
                    entity->deleteRequested = 1;
                    gEwramData->playerState.actionLockFlags &= ~0x100;
                    break;
                case 1:
                    entity->deleteRequested = 1;
                    gEwramData->playerState.actionLockFlags &= ~0x200;
                    break;
                case 2:
                    entity->deleteRequested = 1;
                    gEwramData->playerState.actionLockFlags &= ~0x400;
                    break;
                }
            }
        }
        return;
    }
    entity->state = 10;
    entity->subState = 2;
}

/* Chaos orb damage (3104) and the shell pieces (3178/3210). */

struct HitBox {
    s8 x;
    s8 y;
    u8 w;
    u8 h;
};
#define OFFSET_X(e) (e->ext.chaosShot.offsetX)
#define OFFSET_Y(e) (e->ext.chaosShot.offsetY)

#define REFLECTED(e) (e->ext.chaosShot.reflected)

/* Orb hit: lose HP; at 0 switch to the breaking state 10. */
void ChaosOrb_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
{
    s32 damage = Hit_ResolveDamage(entity, attacker, hit, Player_CalculateAttackPower(hit[2]));

    entity->hitFlashTimer = 8;
    damage <<= 16;
    damage >>= 16;
    Entity_ShowDamage(entity, damage, 0);
    if ((s16)(entity->hp -= damage) <= 0)
    {
        entity->palette = entity->basePalette;
        entity->timer = 40;
        entity->hitFlashTimer = 0;
        entity->state = 10;
        entity->subState = 0;
        entity->stage = 0;
    }
}

/* Create a shell piece (frame 3 + frame) over the orb for `timer` frames. */
void ChaosOrb_CreateShellPiece(struct EwramData_EntityData *orb, s32 frame, s32 timer)
{
    struct EwramData_EntityData *owner = orb->ext.chaosChild.link;
    struct EwramData_EntityData *piece = EntityCreateInRange(0x49, 0xCF, ChaosShellPiece_Update);
    struct ChaosData *data = (struct ChaosData *)owner->ext.chaos.data.value;

    if (piece != NULL)
    {
        s32 graphics;

        piece->posX.part16.integer = 0xFF80;
        piece->posY.part16.integer = 0xFF80;
        piece->spriteLayer = orb->spriteLayer;
        graphics = data->graphics;
        piece->basePalette = data->palette;
        Sprite_Attach(piece, sChaosSpriteSheet, graphics, piece->basePalette);
        piece->animMode = 2;
        piece->animFrame = frame + 3;
        piece->enemyId = orb->enemyId;
        piece->ext.chaosChild.link = orb;
        piece->scrollWithBg = 0;
        piece->timer = timer;
        piece->drawFunc = Sprite_DrawEntity;
    }
}

/* Shell piece: stick to the orb, then fall blinking for 60 frames. */
void ChaosShellPiece_Update(struct EwramData_EntityData *entity)
{
    u8 unused[16];
    struct EwramData_EntityData *orb = entity->ext.chaosChild.link;

    if (entity->state == 0)
    {
        if (orb == NULL || orb->updateFunc == NULL
            || orb->enemyId != entity->enemyId)
            return;
        entity->posX.whole = orb->posX.whole;
        entity->posY.whole = orb->posY.whole;
        if ((s8)--entity->timer <= 0)
        {
            entity->state++;
            entity->accelY.whole = 0x2000;
            entity->timer = 60;
        }
    }
    else if (entity->state == 1)
    {
        entity->scrollWithBg = 1;
        EntityAccelerateAndMove(entity, 0x40000);
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
        if ((s8)--entity->timer <= 0)
            EntityDelete(entity);
    }
}

/* Create a seed shot at the orb + (x, y) that waits `delay` frames (box
 * 0x0A06FDFD = x -3, y -3, w 6, h 10, harmed by ChaosSeedShot_DamagePlayerOnContact, bounced by
 * ChaosSeedShot_OnHit). */
void ChaosOrb_SpawnSeedShot(struct EwramData_EntityData *orb, s32 x, s32 y, s32 delay)
{
    struct EwramData_EntityData *owner = orb->ext.chaosChild.link;
    struct EwramData_EntityData *shot = EntityCreateInRange(0x49, 0xCF, ChaosSeedShot_Update);
    struct HitBox box = { -3, -3, 6, 10 };
    struct ChaosData *data = (struct ChaosData *)owner->ext.chaos.data.value;
    if (shot != NULL)
    {
        s32 graphics;

        shot->posX.part16.integer = 0xFF80;
        shot->posY.part16.integer = 0xFF80;
        OFFSET_X(shot) = x;
        OFFSET_Y(shot) = y;
        shot->spriteLayer = orb->spriteLayer - 1;
        graphics = data->graphics;
        shot->basePalette = data->palette2;
        Sprite_Attach(shot, sChaosSpriteSheet, graphics, shot->basePalette);
        Anim_StartIndexed(shot, 4, 2, 0);
        Anim_Update(shot);
        Hit_Enable(shot, 8, ChaosSeedShot_DamagePlayerOnContact, ChaosSeedShot_OnHit);
        HitBox_SetBody(shot, &box);
        HitBox_SetAttack(shot, &box);
        shot->hit.flags &= ~4;
        shot->hit.flags &= ~2;
        shot->enemyId = orb->enemyId;
        shot->ext.chaosChild.link = orb;
        shot->scrollWithBg = 0;
        shot->timer = delay;
        shot->hidden = 1;
        shot->attack = 100;
        shot->enemyFlags |= 1;
        shot->drawFunc = Sprite_DrawEntity;
    }
}

/* Chaos seed shots (33B8/3504) and sparks (35A0). */

/* Seed shot: follow the orb, appear after the delay, then fly at the
 * target (enemyTarget) when its animation ends; it dies with the orb. */
void ChaosSeedShot_Update(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    struct EwramData_EntityData *orb = entity->ext.chaosChild.link;
    struct EwramData *ewram = gEwramData;
    struct BgLayerState *camera = &ewram->bgInfo[1];

    if (entity->state == 0)
    {
        if (orb != NULL && orb->updateFunc != NULL && orb->enemyId == entity->enemyId)
        {
            if (entity->stage == 0)
            {
                if ((s8)--entity->timer <= 0)
                {
                    entity->hidden = 0;
                    entity->stage++;
                    Anim_StartIndexed(entity, 4, 2, 0);
                    PlaySong(0x144);
                }
            }
            else if (entity->stage == 1 && entity->animDone)
            {
                struct EwramData_EntityData *target;
                entity->state++;
                target = ewram->playerState.enemyTarget;
                Entity_AimVelocityAtPoint(entity, target->posX.whole + camera->xPos.whole,
                             target->posY.whole + camera->yPos.whole - 0x100000, 0x20000);
                Anim_StartIndexed(entity, 5, 2, 1);
                REFLECTED(entity) = 0;
                PlaySong(0x143);
            }
            entity->posX.whole = orb->posX.whole + OFFSET_X(entity);
            entity->posY.whole = orb->posY.whole + OFFSET_Y(entity);
            Anim_Update(entity);
        }
    }
    else if (entity->state == 1)
    {
        entity->scrollWithBg = 1;
        EntityAccelerateAndMove(entity, 0x40000);
        Anim_Update(entity);
        if (EntityIsOutsideScreen(entity, 4))
        {
            EntityDelete(entity);
            return;
        }
        if (REFLECTED(entity) == 0)
            Hit_CheckWithPlayer(&result, entity);
        Enemy_UpdateHitFlash(entity);
    }
    if (orb->state == 10)
        EntityDelete(entity);
}

/* Seed shot hit by the player: bounce away from the attacker. */
void ChaosSeedShot_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
{
    s32 dx, dy;
    u16 angle;

    Hit_ResolveDamage(entity, attacker, hit, Player_CalculateAttackPower(hit[2]));
    entity->hitFlashTimer = 8;
    REFLECTED(entity) = 1;
    dx = entity->posX.whole - attacker->posX.whole;
    dy = entity->posY.whole - attacker->posY.whole;
    angle = ArcTan2(TO_PIXELS(dx), TO_PIXELS(dy));
    dx = Sin(angle + 0x4000);
    dy = Sin(angle);
    entity->velX.whole = FixedMultiply(dx, 0x30000);
    entity->velY.whole = FixedMultiply(dy, 0x30000);
}

/* Create a spark at the orb + (x, y) (box 0x0404FEFE = x -2, y -2, w 4,
 * h 4, harmless to the player, drawn by ChaosSpark_Draw). */
void ChaosOrb_SpawnSpark(struct EwramData_EntityData *orb, s32 x, s32 y)
{
    u32 box;
    struct EwramData_EntityData *owner = orb->ext.chaosChild.link;
    struct EwramData_EntityData *spark = EntityCreateInRange(0x49, 0xCF, ChaosSpark_Update);
    struct ChaosData *data;

    box = 0x0404FEFE;
    data = (struct ChaosData *)owner->ext.chaos.data.value;
    if (spark != NULL)
    {
        s32 graphics;

        spark->posX.part16.integer = 0xFF80;
        spark->posY.part16.integer = 0xFF80;
        OFFSET_X(spark) = x;
        OFFSET_Y(spark) = y;
        spark->spriteLayer = orb->spriteLayer - 1;
        graphics = data->graphics;
        spark->basePalette = data->palette2;
        Sprite_Attach(spark, sChaosSpriteSheet, graphics, spark->basePalette);
        Anim_StartIndexed(spark, 6, 2, 0);
        Anim_Update(spark);
        Hit_Enable(spark, 8, ChaosSpark_OnContact, 0);
        HitBox_SetAttack(spark, &box);
        spark->hit.flags |= 4;
        spark->hit.flags &= ~2;
        spark->enemyId = orb->enemyId;
        spark->ext.chaosChild.link = orb;
        spark->scrollWithBg = 0;
        spark->ext.chaosSoulShot.spread = 0x1000000;
        spark->drawFunc = ChaosSpark_Draw;
    }
}

#define SPREAD(e) (e->ext.chaosSoulShot.spread)
#define MASH(e) ((e)->ext.chaosSoulShot.mash)

/* Chaos soul grab: holds the player, who can mash buttons to break free. */
void ChaosSpark_Update(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    struct EwramData_EntityData *orb = entity->ext.chaosChild.link;
    struct EwramData *ewram = gEwramData;
    struct BgLayerState *camera = &ewram->bgInfo[1];
    struct EwramData_EntityData *target;
    s32 x, y;
    s32 state = entity->state;

    if (state == 0)
    {
        if (orb != NULL && orb->updateFunc != NULL && orb->enemyId == entity->enemyId)
        {
            if (entity->stage == 0)
            {
                if (SPREAD(entity) != 0)
                {
                    SPREAD(entity) -= 0x40000;
                    if (SPREAD(entity) <= 0)
                    {
                        SPREAD(entity) = state;
                        entity->stage++;
                        entity->timer = 30;
                        PlaySong(0x141);
                    }
                }
            }
            else if (entity->stage == 1)
            {
                if ((s8)entity->timer-- <= 0)
                {
                    entity->stage = state;
                    entity->state = 1;
                    entity->timer = 0x78;
                    target = ewram->playerState.enemyTarget;
                    x = target->posX.whole + camera->xPos.whole;
                    y = target->posY.whole + camera->yPos.whole - 0x100000;
                    Entity_AimVelocityAtPoint(entity, x, y, 0x4000);
                    entity->ext.chaosShot.grabbed = state;
                    return;
                }
            }
            entity->posX.whole = orb->posX.whole + OFFSET_X(entity);
            entity->posY.whole = orb->posY.whole + OFFSET_Y(entity);
            Anim_Update(entity);
        }
    }
    else if (state == 1)
    {
        entity->scrollWithBg = 1;
        if (entity->timer != 0)
        {
            entity->timer--;
            target = ewram->playerState.enemyTarget;
            x = target->posX.whole + camera->xPos.whole;
            y = target->posY.whole + camera->yPos.whole - 0x100000;
            Entity_AimAccelerationAtPoint(entity, x, y, 0x2000);
        }
        EntityAccelerateAndMove(entity, 0x30000);
        Anim_Update(entity);
        if (EntityIsOutsideScreen(entity, 4))
        {
            EntityDelete(entity);
            return;
        }
        if (entity->ext.chaosShot.grabbed == 1)
        {
            entity->state = 2;
            entity->subState = 0;
            entity->timer = 0x90;
            MASH(entity) = 0;
            OFFSET_X(entity) = gEwramData->playerState.enemyTarget->posX.whole - entity->posX.whole;
            OFFSET_Y(entity) = gEwramData->playerState.enemyTarget->posY.whole - entity->posY.whole;
            entity->spriteLayer = 9;
        }
        Hit_CheckWithPlayer(&result, entity);
    }
    else if (state == 2)
    {
        if ((entity->timer & 7) == 0)
        {
            struct EwramData_EntityData *child;
            s32 x2 = entity->posX.whole + RandomNumberGenerator() % 0x60000 - 0x30000;
            child = Effect_CreateFromTable(x2, entity->posY.whole + RandomNumberGenerator() % 0x60000 - 0x30000,
                                 0, -0x2000, 7, 6);
            if (child != NULL)
            {
                x = orb->posX.whole + camera->xPos.whole;
                y = orb->posY.whole + camera->yPos.whole;
                Entity_AimAccelerationAtPoint(child, x, y, 0x1000);
            }
        }
        Player_ConsumeMP(1);
        if ((u8)(entity->timer % 0x24) == 0)
            PlaySong(0x142);
        ewram = gEwramData;
        entity->posX.whole = ewram->playerState.enemyTarget->posX.whole - OFFSET_X(entity);
        entity->posY.whole = ewram->playerState.enemyTarget->posY.whole - OFFSET_Y(entity);
        if ((ewram->inputData.playerNewInput & 0x3FF) && MASH(entity) <= 99)
            MASH(entity)++;
        if (SPREAD(entity) <= 0x40000)
            SPREAD(entity) += 0x10000;
        /* Released when time runs out or after 20 mashed inputs. */
        if ((u8)--entity->timer <= 1 || MASH(entity) > 19)
        {
            entity->state = 3;
            return;
        }
    }
    else if (state == 3)
    {
        if (SPREAD(entity) > 0x1000000)
        {
            EntityDelete(entity);
            return;
        }
        SPREAD(entity) += 0x60000;
    }
    if (orb->state == 10)
        entity->state = 3;
}

/* Chaos spark drawing (392C) and the soul shot creation (39DC). */

/* Draw the spark four times, mirrored around its position by SPREAD. */
void ChaosSpark_Draw(struct EwramData_EntityData *entity)
{
    s32 x = entity->posX.whole;
    s32 y = entity->posY.whole;

    entity->posX.whole = x - SPREAD(entity);
    entity->posY.whole = y - SPREAD(entity);
    entity->flipX = 0;
    entity->flipY = 0;
    Sprite_DrawEntity(entity);
    entity->posX.whole = x + SPREAD(entity);
    entity->posY.whole = y - SPREAD(entity);
    entity->flipX = 1;
    entity->flipY = 0;
    Sprite_DrawEntity(entity);
    entity->posX.whole = x - SPREAD(entity);
    entity->posY.whole = y + SPREAD(entity);
    entity->flipX = 0;
    entity->flipY = 1;
    Sprite_DrawEntity(entity);
    entity->posX.whole = x + SPREAD(entity);
    entity->posY.whole = y + SPREAD(entity);
    entity->flipX = 1;
    entity->flipY = 1;
    Sprite_DrawEntity(entity);
    entity->posX.whole = x;
    entity->posY.whole = y;
}

/* Create a soul shot at the orb + (x, y) facing the target (10 HP, box
 * 0x1A30F0E8 = x -24, y -16, w 48, h 26, harmed by ChaosSoulShot_DamagePlayerOnContact). */
void ChaosOrb_SpawnSoulShot(struct EwramData_EntityData *orb, s32 x, s32 y)
{
    u32 box;
    struct EwramData_EntityData *owner = orb->ext.chaosChild.link;
    struct EwramData **ewram = &gEwramData;
    struct EwramData_EntityData *shot = EntityCreateInRange(0x49, 0xCF, ChaosSoulShot_Update);
    struct ChaosData *data;

    box = 0x1A30F0E8;
    data = (struct ChaosData *)owner->ext.chaos.data.value;
    if (shot != NULL)
    {
        s32 graphics;

        shot->posX.part16.integer = 0xFF80;
        shot->posY.part16.integer = 0xFF80;
        OFFSET_X(shot) = x;
        OFFSET_Y(shot) = y;
        shot->spriteLayer = orb->spriteLayer - 1;
        graphics = data->graphics;
        shot->basePalette = data->palette;
        Sprite_Attach(shot, sChaosSpriteSheet, graphics, shot->basePalette);
        shot->animMode = 2;
        Anim_StartIndexed(shot, 1, 2, 0);
        Anim_Update(shot);
        Hit_Enable(shot, 8, ChaosSoulShot_DamagePlayerOnContact, 0);
        HitBox_SetAttack(shot, &box);
        shot->hit.flags |= 4;
        shot->hit.flags &= ~2;
        shot->enemyId = orb->enemyId;
        shot->ext.chaosChild.link = orb;
        shot->scrollWithBg = 0;
        shot->attack = 0x78;
        shot->hp = 10;
        {
            s32 targetX = (*ewram)->playerState.enemyTarget->posX.whole;
            if (orb->posX.whole >= targetX)
                shot->flipX = 0;
            else
                shot->flipX = 1;
        }
        shot->drawFunc = Sprite_DrawEntity;
        PlaySong(0x78);
    }
}

/* ChaosSoulShot_Update: Chaos soul shot update (created by ChaosOrb_SpawnSoulShot). */

/**
 * @brief Soul shot update: fly toward the target (sounds 0x148 / 0x78), animate and
 * delete off screen or when its sequence ends.
 */
void ChaosSoulShot_Update(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    struct EwramData_EntityData *orb = entity->ext.chaosChild.link;
    struct EwramData *ewram = gEwramData;
    struct BgLayerState *camera = &ewram->bgInfo[1];
    s32 x, y;
    s32 state = entity->state;

    if (state == 0)
    {
        if (orb == NULL || orb->updateFunc == NULL
            || orb->enemyId != entity->enemyId)
            return;
        if (entity->stage == 0)
        {
            if (entity->animDone)
            {
                entity->stage++;
                entity->timer = 30;
                Anim_StartIndexed(entity, 3, 2, 1);
            }
        }
        else if (entity->stage == 1)
        {
            if ((s8)entity->timer-- <= 0)
            {
                struct EwramData_EntityData *target;
                s32 speed;

                entity->stage = state;
                entity->state = 1;
                target = ewram->playerState.enemyTarget;
                x = target->posX.whole + camera->xPos.whole;
                y = target->posY.whole + camera->yPos.whole - 0x100000;
                speed = 0x30000;
                Entity_AimVelocityAtPoint(entity, x, y, speed);
                if (entity->flipX)
                {
                    if (entity->velX.whole >= 0) goto sound;
                }
                else if (entity->velX.whole <= 0)
                    goto sound;
                entity->velX.whole = state;
                if (entity->velY.whole <= 0x2FFFF)
                    entity->velY.whole = speed;
            sound:
                PlaySong(0x148);
                return;
            }
        }
        entity->posX.whole = orb->posX.whole + OFFSET_X(entity);
        entity->posY.whole = orb->posY.whole + OFFSET_Y(entity);
        Anim_Update(entity);
    }
    else if (state == 1)
    {
        if (entity->stage == 0)
        {
            entity->stage++;
            entity->scrollWithBg = 1;
            Afterimage_Create(entity, 0, 12, 0x24);
            Afterimage_Create(entity, 0, 6, 0x24);
        }
        EntityAccelerateAndMove(entity, 0x60000);
        Anim_Update(entity);
        if (EntityIsOutsideScreen(entity, 4))
        {
            entity->state++;
            entity->subState = 0;
            entity->posX.whole = 0x800000 - camera->xPos.whole;
            entity->posY.whole = -0x300000;
            entity->timer = 30;
            entity->hp = 0;
        }
        else
            Hit_CheckWithPlayer(&result, entity);
    }
    else if (entity->state == 2)
    {
        if (entity->subState == 0)
        {
            s32 posX;
            s32 posY;
            s32 vx, vy;

            x = orb->posX.whole - entity->posX.whole;
            posX = entity->posX.whole;
            y = orb->posY.whole - entity->posY.whole;
            posY = entity->posY.whole;
            vx = x / entity->timer;
            entity->velX.whole = vx;
            vy = y / entity->timer;
            entity->velY.whole = vy;
            entity->posX.whole = posX + vx;
            entity->posY.whole = posY + vy;
            if (vx >= 0) entity->flipX = 1;
            else entity->flipX = 0;
            if (--entity->timer == 0)
            {
                entity->subState++;
                Anim_StartIndexed(entity, 2, 2, 0);
                PlaySong(0x78);
            }
        }
        else if (entity->subState == 1)
        {
            entity->posX.whole = orb->posX.whole;
            entity->posY.whole = orb->posY.whole;
            if (entity->animDone)
            {
                orb->ext.chaosOrb.shotDone = 1;
                EntityDelete(entity);
                return;
            }
        }
        Anim_Update(entity);
    }
}

/* Chaos: mark on the player (3D40). */

#undef KIND
#define KIND(e) (*(u8 *)&e->ext.chaosOrb.kind)

/* Create the mark of orb kind `kind` next to the player. */
void Chaos_CreateMark(struct EwramData_EntityData *core, s32 kind)
{
    struct EwramData **ewram = &gEwramData;
    struct EwramData_EntityData *mark = EntityCreateInRange(0x49, 0xCF, ChaosMark_Update);
    struct MarkFrames frames;
    struct ChaosData *data;

    frames = sChaosMarkFrames;
    data = (struct ChaosData *)core->ext.chaos.data.value;
    if (mark != NULL)
    {
        s32 graphics;

        mark->posX.part16.integer = 0xFF80;
        mark->posY.part16.integer = 0xFF80;
        mark->spriteLayer = (*ewram)->playerState.player->spriteLayer - 1;
        graphics = data->graphics;
        mark->basePalette = data->palette2;
        Sprite_Attach(mark, sChaosSpriteSheet, graphics, mark->basePalette);
        mark->animMode = 2;
        mark->animFrame = frames.v[kind] + 0x14;
        mark->enemyId = core->enemyId;
        mark->ext.chaosChild.link = core;
        mark->scrollWithBg = 0;
        KIND(mark) = kind;
        mark->timer = 40;
        mark->drawFunc = Sprite_DrawEntity;
    }
}

/* ChaosMark_Update: soul halo following the player; matched. */
#define IDX(e) (*(u8 *)&e->ext.chaosOrb.kind)

/**
 * @brief Mark update: stay next to the player at the offset of its orb kind,
 * step through its show sequence (states 0 / 1) and blink out (state 2).
 */
void ChaosMark_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.chaosChild.link;
    struct EwramData *ew = gEwramData;
    struct HaloOffsets offs = sChaosMarkOffsets;

    if (entity->state == 0)
    {
        entity->posX.whole = ew->playerState.player->posX.whole + offs.o[IDX(entity)].x;
        entity->posY.whole = ew->playerState.player->posY.whole + offs.o[IDX(entity)].y;
        if (Entity_GetVisibilityPattern(entity->timer))
            entity->hidden = 0;
        else
            entity->hidden = 1;
        if ((s8)--entity->timer <= 0)
            entity->state++;
    }
    else if (entity->state == 1)
    {
        entity->posX.whole = ew->playerState.player->posX.whole + offs.o[IDX(entity)].x;
        entity->posY.whole = ew->playerState.player->posY.whole + offs.o[IDX(entity)].y;
        if (IDX(entity) == 0)
        {
            if (parent->ext.chaos.brokenOrbs & 1)
            {
                entity->state++;
                entity->timer = 0x28;
            }
        }
        else if (IDX(entity) == 1)
        {
            if (parent->ext.chaos.brokenOrbs & 2)
            {
                entity->state++;
                entity->timer = 0x28;
            }
        }
        else if (IDX(entity) == 2)
        {
            if (parent->ext.chaos.brokenOrbs & 4)
            {
                entity->state++;
                entity->timer = 0x28;
            }
        }
    }
    else if (entity->state == 2)
    {
        entity->posX.whole = ew->playerState.player->posX.whole + offs.o[IDX(entity)].x;
        entity->posY.whole = ew->playerState.player->posY.whole + offs.o[IDX(entity)].y;
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
        if ((s8)--entity->timer <= 0)
            entity->deleteRequested = 1;
    }
}

/* Chaos: orb halo (3F84/4010). */

/* Create the orb's halo (frame 0x13) for `timer` frames. */
void ChaosOrb_CreateHalo(struct EwramData_EntityData *orb, s32 timer)
{
    struct EwramData_EntityData *halo = EntityCreateInRange(0x49, 0xCF, ChaosHalo_Update);
    struct ChaosData *data = (struct ChaosData *)orb->ext.chaosChild.link->ext.chaos.data.value;

    if (halo != NULL)
    {
        s32 graphics;

        halo->posX.part16.integer = 0xFF80;
        halo->posY.part16.integer = 0xFF80;
        halo->spriteLayer = orb->spriteLayer - 1;
        graphics = data->graphics;
        halo->basePalette = data->palette2;
        Sprite_Attach(halo, sChaosSpriteSheet, graphics, halo->basePalette);
        halo->animMode = 2;
        halo->animFrame = 0x13;
        halo->enemyId = orb->enemyId;
        halo->ext.chaosChild.link = orb;
        halo->scrollWithBg = 0;
        halo->timer = timer;
        halo->drawFunc = Sprite_DrawEntity;
    }
}

/* Halo: blink in under the orb, then show with it until it breaks. */
void ChaosHalo_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *orb = entity->ext.chaosChild.link;

    if (orb == NULL || orb->updateFunc == NULL
        || orb->enemyId != entity->enemyId)
    {
        entity->deleteRequested = 1;
        return;
    }
    entity->posX.whole = orb->posX.whole;
    entity->posY.whole = orb->posY.whole + 0x40000;
    if (entity->state == 0)
    {
        if (Entity_GetVisibilityPattern(entity->timer))
            entity->hidden = 0;
        else
            entity->hidden = 1;
        if (--entity->timer <= 1)
            entity->state++;
        return;
    }
    entity->hidden = orb->hidden;
    if (orb->state != 10)
        return;
    entity->deleteRequested = 1;
}

/* Chaos: orb aura (40B4/4184) and the core's companion entity (4238). */

/* Create the aura of an orb (palette 6 + pal, random start frame). */
struct EwramData_EntityData *ChaosOrb_CreateAura(struct EwramData_EntityData *orb, s32 pal, s32 kind, s32 timer)
{
    struct EwramData_EntityData *aura = EntityCreateInRange(0x49, 0xCF, ChaosAura_Update);

    if (aura != NULL)
    {
        s32 graphics;

        aura->posX.part16.integer = 0xFF80;
        aura->posY.part16.integer = 0xFF80;
        aura->spriteLayer = orb->spriteLayer - 2;
        graphics = ObjGfx_Find(sCommonObjectGfx);
        aura->basePalette = pal + 6;
        Sprite_Attach(aura, sCommonObjectSpriteSheet, graphics, aura->basePalette);
        Anim_StartIndexed(aura, 6, 0, 1);
        Anim_Update(aura);
        aura->animTimer = RandomNumberGenerator() % 8;
        aura->enemyId = orb->enemyId;
        aura->ext.chaosChild.link = orb;
        aura->scrollWithBg = 0;
        KIND(aura) = kind;
        aura->timer = timer;
        aura->hit.flags |= 4;
        aura->hit.flags |= 2;
        aura->drawFunc = Sprite_DrawEntity;
        return aura;
    }
    return NULL;
}

/* Aura: blink in at the orb, then flicker with it until it breaks. */
void ChaosAura_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *orb = entity->ext.chaosChild.link;

    if (orb == NULL || orb->updateFunc == NULL
        || orb->enemyId != entity->enemyId)
    {
        entity->deleteRequested = 1;
        return;
    }
    entity->posX.whole = orb->posX.whole;
    entity->posY.whole = orb->posY.whole + 0x40000;
    if (entity->state == 0)
    {
        if (Entity_GetVisibilityPattern(entity->timer))
            entity->hidden = 0;
        else
            entity->hidden = 1;
        if (--entity->timer <= 1)
            entity->state++;
        return;
    }
    entity->hidden = orb->hidden;
    if (RandomNumberGenerator() & 0x1F)
        Anim_Update(entity);
    if (orb->state != 10)
        return;
    entity->deleteRequested = 1;
}

/* Create the helper entity ChaosBackground_Update at the core. */
struct EwramData_EntityData *Chaos_CreateBackground(struct EwramData_EntityData *core)
{
    struct EwramData_EntityData *helper = EntityCreateInRange(0x49, 0xCF, ChaosBackground_Update);

    if (helper != NULL)
    {
        helper->posX.whole = core->posX.whole;
        helper->posY.whole = core->posY.whole;
        helper->enemyId = core->enemyId;
        helper->ext.chaosChild.link = core;
        return helper;
    }
    return NULL;
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   ChaosSeedShot_DamagePlayerOnContact = Enemy_DamagePlayerOnContact */

/**
 * @brief A4274 | Same as Enemy_DamagePlayerOnContact.
 */
void ChaosSeedShot_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}

/* A42A0 | Contact callback of the sparks: mark the grab (ext.chaosShot.grabbed);
 * ChaosSpark_Update then holds the player until they break free. */
void ChaosSpark_OnContact(struct EwramData_EntityData *entity)
{
    entity->ext.chaosShot.grabbed = 1;
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   ChaosSoulShot_DamagePlayerOnContact = Enemy_DamagePlayerOnContact */

/**
 * @brief A42A8 | Same as Enemy_DamagePlayerOnContact.
 */
void ChaosSoulShot_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}

/* A42D4 | Load the Chaos graphics and its two palettes into the core and
 * its ChaosData (unk_504). */
u8 Chaos_LoadGraphics(struct EwramData_EntityData *core)
{
    s32 graphics;
    struct ChaosData *data = (struct ChaosData *)core->ext.chaos.data.value;

    graphics = ObjGfx_LoadResource(sChaosGfx, 0);

    if (graphics < 0)
        return 0;
    core->basePalette = ObjPalette_FindOrLoadVariant(sChaosPalette, 0);
    data->graphics = graphics;
    data->palette = core->basePalette;
    data->palette2 = ObjPalette_FindOrLoadVariant(sChaosPalette, 1);
    Sprite_Attach(core, sChaosSpriteSheet, graphics, core->basePalette);
    core->drawFunc = Sprite_DrawEntity;
    return 1;
}

/* Chaos core: dispatch the states (0 Chaos_UpdateIntro, 1 Chaos_UpdateFight, 2
 * Chaos_UpdateIdle, 3 Chaos_UpdateDefeat dying, 4 frozen). */
void EnemyChaosUpdate(struct EwramData_EntityData *entity)
{
    struct HitResult results[3]; /* only results[0] is written; the original reserved 48 bytes */

    if (entity->state != 3 && Entity_IsInRect(entity, -16, 0, 272, 176))
        Player_TrackEntity(entity);
    if (Enemy_IsStunned(entity))
    {
        if (entity->state != 3)
            Hit_CheckWithPlayer(results, entity);
        return;
    }
    switch (entity->state)
    {
    case 0:
        Chaos_UpdateIntro(entity);
        break;
    case 1:
        Chaos_UpdateFight(entity);
        break;
    case 2:
        Chaos_UpdateIdle(entity);
        break;
    case 3:
        Chaos_UpdateDefeat(entity);
        break;
    case 4:
        Anim_Update(entity);
        return;
    }
    Enemy_CheckHitPlayer(entity);
}

/* Chaos_UpdateIdle. */
/**
 * @brief A43E4 | Sub-state 0: start the animation (Anim_StartIndexed with
 * 0, 0, 0) and advance the sub-state.
 */
void Chaos_UpdateIdle(struct EwramData_EntityData *entity)
{
    if (entity->subState == 0)
    {
        Anim_StartIndexed(entity, 0, 0, 0);
        entity->subState++;
    }
}

/* allocation of 10 words in the core's unk_504 (second form) */
struct Chaos2Data {
    s32 graphics;
    s32 palette;
    s32 palette2;
    s32 timer;    /* 0x0C */
    s32 count;    /* 0x10 */
    s32 summoned; /* 0x14 */
    s32 paletteStep;
    s32 paletteFrameTime;
    s32 paletteSequenceDone;
    u8 *paletteSequence;  /* 0x24: count, then (colour, frames) pairs */
};

/**
 * @brief A4408 | Turn the core into the second form of Chaos (3000 HP,
 * box 0x1010F8F8 = x -8, y -8, w 16, h 16, hit by Chaos2_OnHit). Returns
 * 1 when the core must be deleted.
 */
s32 Chaos_StartSecondForm(struct EwramData_EntityData *core)
{
    u32 box = 0x1010F8F8;
    struct Chaos2Data *data;

    core->updateFunc = EnemyChaos2Update;
    data = Heap_Alloc(10);
    core->ext.chaos.data.value = (s32)data;
    if (data != NULL)
    {
        if (EnemyChaos2Create(core))
        {
            Anim_StartIndexed(core, 8, 2, 0);
            Anim_Update(core);
            Enemy_ResetFromTemplate(core);
            Hit_Enable(core, 8, NULL, Chaos2_OnHit);
            HitBox_SetBody(core, &box);
            core->hit.flags &= ~4;
            core->hit.flags |= 2;
            core->spriteLayer = 0x1A;
            core->hp = 3000;
            core->state = 0;
            Player_SetSelectedTarget(core);
            gEwramData->gameState.stateFlags |= 2;
            gEwramData->gameState.stateFlags |= 1;
            gEwramData->gameState.stateFlags |= 0x200000;
            gEwramData->gameState.stateFlags |= 0x800000;
            gEwramData->startPauseEnabled = 0;
            core->timer = 0;
            data->timer = RandomNumberGenerator() % 128 + 60;
            core->ext.chaos2.partCount = 4;
            core->ext.chaos2.baseDefense = core->defense;
            Chaos2_CreateAffineEffect(core);
            Music_SetCurrent(0x12);
            return 0;
        }
        EntityDelete(core);
    }
    return 1;
}

#define ANGLE(e) ((e)->velX.whole)
#define SPIN_SPEED(e) ((e)->accelX.whole)

/**
 * @brief A4508 | Second form AI: after a random pause either fire a ring of
 * 12 shots (Chaos2_SpawnShot) while turning, or once summon the four
 * Chaos2_SummonPart parts after the sequence sChaos2PaletteSequence3.
 */
void Chaos2_UpdateAttack(struct EwramData_EntityData *core)
{
    struct Chaos2Data *data = (struct Chaos2Data *)core->ext.chaos.data.value;

    switch (core->subState)
    {
    case 0:
        if (data->timer != 0)
        {
            data->timer--;
            break;
        }
        data->timer = (RandomNumberGenerator() & 0x7F) + 180;
        if (RandomNumberGenerator() & 1)
        {
            core->subState = 1;
            data->count = 0;
            Chaos_StartPaletteSequence(core, (s32)sChaos2PaletteSequence2);
            core->timer = 0x38;
            if (!(RandomNumberGenerator() & 1))
                SPIN_SPEED(core) = 0x1556;
            else
                SPIN_SPEED(core) = -0x1555;
            ANGLE(core) = 0x4000;
        }
        else if (data->summoned == 0)
        {
            core->subState = 10;
            Chaos_StartPaletteSequence(core, (s32)sChaos2PaletteSequence3);
            core->timer = 0;
        }
        else
            data->timer = (RandomNumberGenerator() & 0x7F) + 180;
        break;
    case 1:
        Chaos2_StepPaletteSequence(core);
        if ((s8)--core->timer <= 0)
        {
            core->subState++;
            core->timer = 10;
        }
        break;
    case 2:
        if (core->timer++ > 5)
        {
            s32 x, y;

            core->timer = 0;
            x = Sin(ANGLE(core) + 0x4000);
            x = FixedMultiply(x, 0x180000);
            y = Sin(ANGLE(core));
            y = FixedMultiply(y, 0x180000);
            Chaos2_SpawnShot(core, x, y, ANGLE(core));
            PlaySong(0x145);
            ANGLE(core) += SPIN_SPEED(core);
            if (++data->count > 11)
            {
                core->subState = 0;
                data->timer = RandomNumberGenerator() % 128 + 180;
            }
        }
        break;
    case 10:
        Chaos2_StepPaletteSequence(core);
        if (core->timer <= 59)
        {
            core->timer++;
            if ((core->timer & 3) == 0)
                Chaos2_SpawnEffectAbove(core, ((core->timer >> 2) & 7) + 9);
        }
        if (data->paletteSequenceDone != 0)
            core->subState++;
        break;
    case 11:
        Chaos2_SummonPart(core, 0, 0, -0x60000, -0x30000);
        Chaos2_SummonPart(core, 0, 0, 0x60000, -0x30000);
        Chaos2_SummonPart(core, 0, 0, -0x60000, 0x30000);
        Chaos2_SummonPart(core, 0, 0, 0x60000, 0x30000);
        data->summoned = 1;
        core->subState = 0;
        core->timer = 0;
        data->timer = RandomNumberGenerator() % 128 + 180;
        PlaySong(0x85);
        break;
    }
}

/**
 * @brief A4728 | Chaos intro sequence (subState, player input frozen):
 * fades the screen (BgPalette_StartColorFade / ObjPalette_StartColorFade), plays the sequence
 * sChaos2PaletteSequence1 while Chaos2_StepPaletteSequence animates the core for 60 + 120 frames,
 * then fades back in, places the four parts (Chaos2_CreatePart) and after 40
 * frames ends the fades, releases the input (stateFlags bit 0), sets
 * startPauseEnabled and starts the fight (state 1, Chaos2_CreateSnake). Step 10 does
 * the same once the fade ObjPalette_GetPendingFades has finished.
 */
void Chaos2_UpdateIntro(struct EwramData_EntityData *core)
{
    SetPlayerInput(2, 0);
    switch (core->subState)
    {
    case 0:
        core->subState++;
        BgPalette_StartColorFade(0, 0, 0, 0x10000, 0, 1);
        ObjPalette_StartColorFade(0, 0, 0, 0, 0x10000, 0x3C);
        core->stage = 0;
        gEwramData->startPauseEnabled = 0;
        core->timer = 0;
        Chaos_StartPaletteSequence(core, (s32)sChaos2PaletteSequence1);
        gEwramData->gameState.stateFlags |= 0x20;
        break;
    case 1:
        gEwramData->startPauseEnabled = 0;
        BgPalette_StartColorFade(0, 0, 0, 0x10000, 0, 1);
        Chaos2_StepPaletteSequence(core);
        if (core->timer++ > 0x3B)
        {
            core->subState++;
            core->timer = 0;
        }
        break;
    case 2:
        BgPalette_StartColorFade(0, 0, 0, 0x10000, 0, 1);
        Chaos2_StepPaletteSequence(core);
        if (core->timer++ > 0x77)
        {
            core->subState++;
            core->timer = 0;
            ObjPalette_StartColorFade(0, 0, 0, 0x10000, 0, 0x28);
        }
        break;
    case 3:
        BgPalette_StartColorFade(0, 0, 0, 0x10000, 0, 1);
        if (core->timer++ > 0x27)
        {
            core->subState++;
            core->timer = 0;
            BgPalette_StartColorFade(0, 0, 0, 0, 0x10000, 0x28);
            ObjPalette_StartColorFade(0, 0, 0, 0, 0x10000, 0x28);
            Chaos2_CreatePart(core, 0x180000, 0x280000, 0);
            Chaos2_CreatePart(core, 0xD80000, 0x280000, 1);
            Chaos2_CreatePart(core, 0x180000, 0x780000, 2);
            Chaos2_CreatePart(core, 0xD80000, 0x780000, 3);
            gEwramData->gameState.stateFlags &= ~0x20;
        }
        break;
    case 4:
        if (core->timer++ > 0x27)
        {
            BgPalette_StopColorFade();
            ObjPalette_StopColorFade();
            core->timer = 0;
            core->state = 1;
            core->subState = 0;
            core->stage = 0;
            gEwramData->gameState.stateFlags &= ~1;
            gEwramData->startPauseEnabled = 1;
            Chaos2_CreateSnake(core, 0, 0, 0);
        }
        break;
    case 10:
        if (ObjPalette_GetPendingFades() == 0)
        {
            BgPalette_StopColorFade();
            ObjPalette_StopColorFade();
            core->state = 1;
            core->subState = 0;
            core->stage = 0;
            gEwramData->gameState.stateFlags &= ~1;
            gEwramData->startPauseEnabled = 1;
            Chaos2_CreateSnake(core, 0, 0, 0);
        }
        break;
    }
}

/* 0x10000..0x2FFFF; the masked random value lives in r1 as in the original. */
#define RANDOM_SPEED() (({ register u32 r asm("r1") = RandomNumberGenerator() & 0x1FFFF; r; }) + 0x10000) /* FAKEMATCH: pinned register */

/**
 * @brief Defeat state (state 3): hold the player, run the palette sequence and
 * scatter spinning debris and shards (Chaos2_SpawnSpinningDebris /
 * Chaos2_SpawnShard) before the ending.
 */
void Chaos2_UpdateDefeat(struct EwramData_EntityData *entity)
{
    s32 i;
    u8 anim;

    SetPlayerInput(2, 0);
    switch (entity->subState)
    {
    case 0:
        Chaos2_StepPaletteSequence(entity);
        if (!entity->ext.chaos2.defeatReady)
            break;
        entity->subState++;
        Anim_StartIndexed(entity, 0x11, 2, 0);
        break;
    case 1:
        if (!entity->animTimer)
        {
            anim = entity->animStep;
            if (anim == 1 || anim == 3 || anim == 4 || anim == 5)
                PlaySong(0x167);
        }
        if (!entity->animDone)
            break;
        entity->subState++;
        entity->timer = 0x3C;
        PlaySong(0x168);
        for (i = 0; i < 4; i++)
            Chaos2_SpawnSpinningDebris(entity, RandomNumberGenerator() & 0xFFFF, RANDOM_SPEED());
        for (i = 0; i < 6; i++)
            Chaos2_SpawnShard(entity, RandomNumberGenerator() & 0xFFFF, RANDOM_SPEED());
        entity->hidden = 1;
        break;
    case 2:
        if (entity->timer > 0x38)
        {
            Chaos2_SpawnSpinningDebris(entity, RandomNumberGenerator() & 0xFFFF, RANDOM_SPEED());
            for (i = 0; i < 4; i++)
                Chaos2_SpawnShard(entity, RandomNumberGenerator() & 0xFFFF, RANDOM_SPEED());
        }
        if ((s8)--entity->timer > 0)
            break;
        BgPalette_StartColorFade(0, 0, 0, 0x10000, 0, 0x3C);
        ObjPalette_StartColorFade(0, 0, 0, 0x10000, 0, 0x3C);
        PlaySong(0x1010);
        entity->subState++;
        entity->timer = 0x3A;
        break;
    case 3:
        if (--entity->timer > 1)
            break;
        gEwramData->gameState.bossEventFlags |= 0x80;
        Player_SetSelectedTarget(NULL);
        gEwramData->gameState.stateFlags = (gEwramData->gameState.stateFlags & ~2) | 0x4000;
        RoomJump_SetDestination((void *)sDebugEventRoom14, 0, 0, 0x80, 0x70);
        RoomJump_Begin(&gEwramData->gameState);
        gEwramData->gameState.stateFlags = (gEwramData->gameState.stateFlags & ~1) | 0x800000;
        gEwramData->startPauseEnabled = 0;
        break;
    }
}

/* Hit callback: compute damage and perform the boss's death transition. */
static inline s32 SubtractDamage(u16 health, s16 damage) { return health - damage; }
void Chaos2_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
{
    u16 *savedHit = (u16 *)hit;
    s32 timer = (s8)entity->ext.chaos2.partCount * 20 + (u8)entity->ext.chaos2.baseDefense;
    s32 damage;
    if (timer <= 255) entity->defense = timer;
    else entity->defense = 255;
    damage = Hit_ResolveDamage(entity, attacker, savedHit, Player_CalculateAttackPower(hit[2]));
    entity->hitFlashTimer = 8;
    entity->hp = SubtractDamage(entity->hp, damage);
    Entity_ShowDamage(entity, (s16)damage, 0);
    if ((s16)entity->hp <= 0)
    {
        Enemy_DropRewards(entity);
        entity->palette = entity->basePalette;
        entity->timer = 120;
        entity->ext.chaos2.defeatReady = 0;
        entity->state = 3;
        entity->subState = 0;
        BossDefeatEffect_Spawn(entity->posX.whole, entity->posY.whole);
        gEwramData->gameState.stateFlags &= ~1;
        PlaySong(0x1010);
        gEwramData->startPauseEnabled = 0;
    }
}

/* Chaos second form: palette sequence (4CE4) and part creation (4D64). */

/* Step the palette sequence: hold each colour for its frame count and
 * load it into the core's palette slot when it changes. */
void Chaos2_StepPaletteSequence(struct EwramData_EntityData *core)
{
    struct Chaos2Data *data = (struct Chaos2Data *)core->ext.chaos.data.value;
    u8 *sequence = data->paletteSequence;
    u8 palette, colour, slot;

    if (data->paletteSequenceDone != 0)
        return;
    if (++data->paletteFrameTime >= sequence[data->paletteStep * 2 + 2])
    {
        data->paletteFrameTime = 0;
        if (++data->paletteStep >= sequence[0])
        {
            data->paletteSequenceDone = 1;
            return;
        }
    }
    palette = core->basePalette;
    colour = sequence[data->paletteStep * 2 + 1];
    slot = palette - 10;
    if (gEwramData->gameState.sharedObjPaletteVariants[slot] != colour)
    {
        gEwramData->gameState.sharedObjPaletteVariants[slot] = colour;
        ObjPalette_LoadRange((u8 *)gEwramData->gameState.sharedObjPaletteSources[slot], colour, 1, palette);
    }
}

/* Create a 3000 HP part (graphics sChaos2SpriteSheet, random start frame,
 * box 0x1010F8F8, hit by Chaos2Part_DamagePlayerOnContact / Chaos2Part_OnHit) at (x, y), mirrored
 * by bits 0/1 of mirror. */
void Chaos2_CreatePart(struct EwramData_EntityData *core, s32 x, s32 y, s32 mirror)
{
    struct EwramData_EntityData *part = EntityCreateInRange(0x49, 0xCF, Chaos2Part_Update);
    struct HitBox box = { -8, -8, 16, 16 };
    struct Chaos2Data *data = (struct Chaos2Data *)core->ext.chaos.data.value;
    if (part != NULL)
    {
        s32 graphics;

        part->posX.whole = x;
        part->posY.whole = y;
        part->spriteLayer = 0x14;
        graphics = data->graphics;
        part->basePalette = data->palette;
        Sprite_Attach(part, sChaos2SpriteSheet, graphics, part->basePalette);
        Anim_StartIndexed(part, 0, 2, 1);
        part->animMode = 2;
        part->animTimer = RandomNumberGenerator() % 6;
        part->enemyId = core->enemyId;
        part->ext.chaosChild.link = core;
        part->scrollWithBg = 0;
        part->attack = 0x55;
        part->hp = 3000;
        part->enemyFlags |= 1;
        Hit_Enable(part, 8, Chaos2Part_DamagePlayerOnContact, Chaos2Part_OnHit);
        HitBox_SetBody(part, &box);
        HitBox_SetAttack(part, &box);
        part->hit.flags &= ~4;
        part->hit.flags &= ~2;
        if (mirror & 1)
            part->flipX = 1;
        if (mirror & 2)
            part->flipY = 1;
        part->drawFunc = Sprite_DrawEntity;
    }
}

/* Chaos second form: part update (4E80). */

#define FLASH(e) (e->ext.chaos2Part.flash)

/* Part: hurt the player, restore the frame after a hit flash, break when
 * the core is gone or dying; broken parts smoke and blink out. */
void Chaos2Part_Update(struct EwramData_EntityData *part)
{
    struct HitResult result;
    struct EwramData_EntityData *core = part->ext.chaosChild.link;

    if (part->state == 0)
    {
        if (part->hitFlashTimer != 0)
            Enemy_UpdateHitFlash(part);
        if (part->animScriptIndex == 1 && (s16)FLASH(part)-- <= 0)
            Anim_StartIndexed(part, 0, 2, 1);
        Hit_CheckWithPlayer(&result, part);
        if (core != NULL && core->updateFunc != NULL
            && core->enemyId == part->enemyId
            && core->state != 3)
        {
            if (RandomNumberGenerator() % 16)
                Anim_Update(part);
            return;
        }
        part->state = 10;
        part->subState = 0;
        part->hp = 0;
        part->palette = part->basePalette;
        part->timer = 40;
        part->hitFlashTimer = 0;
    }
    else if (part->state == 10)
    {
        if ((part->timer & 3) == 0)
            Effect_SpawnRandom(part->posX.whole, part->posY.whole - 0x60000, 0x100000, 0xC0000, part->spriteLayer - 1);
        part->hidden = Entity_GetVisibilityPattern(part->timer);
        if ((s8)--part->timer <= 0)
        {
            EntityDelete(part);
            return;
        }
    }
    Anim_Update(part);
}

/* Chaos second form: part damage (4FA0). */
/* Part hit: flash (ext.chaos2Part.flash = 30), lose HP; at 0 break (state 10)
 * and lower the core's ext.chaos2.partCount. */
void Chaos2Part_OnHit(struct EwramData_EntityData *part, struct EwramData_EntityData *attacker, u16 *hit)
{
    struct EwramData_EntityData *core = part->ext.chaosChild.link;
    s32 damage = Hit_ResolveDamage(part, attacker, hit, Player_CalculateAttackPower(hit[2]));
    s32 zero;

    part->hitFlashTimer = 8;
    zero = 0;
    part->ext.chaos2Part.flash = 30;
    damage <<= 16;
    damage >>= 16;
    Entity_ShowDamage(part, damage, 0);
    part->hp -= damage;
    Anim_StartIndexed(part, 1, 2, 0);
    if ((s16)part->hp <= 0)
    {
        PlaySong(0x6E);
        part->palette = part->basePalette;
        part->timer = 40;
        part->hitFlashTimer = zero;
        part->state = 10;
        part->subState = zero;
        if (*(s8 *)&core->ext.chaos2.partCount != 0)
            core->ext.chaos2.partCount--;
    }
}

struct ChaosShotGfx { s32 tiles; s32 unused; s32 palette; };
/* Create a shot, selecting its frame and flips from 13 angle sectors. */
void Chaos2_SpawnShot(struct EwramData_EntityData *parent, s32 dx, s32 dy, s32 angle)
{
    u32 collision = 0x0808FCFC;
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, Chaos2Shot_Update);
    struct ChaosAngleFrames frames = sChaos2ShotAngleFrames;
    struct ChaosShotGfx *gfx = (struct ChaosShotGfx *)parent->ext.chaos.data.ptr;
    if (child != NULL)
    {
        s32 vx, vy, tiles, index, zero;
        child->posX.whole = parent->posX.whole + dx;
        child->posY.whole = parent->posY.whole + dy;
        vx = FixedMultiply(Sin(angle + 0x4000), 0x30000);
        vy = FixedMultiply(Sin(angle), 0x30000);
        child->velX.whole = vx;
        child->velY.whole = vy;
        child->spriteLayer = 0x11;
        tiles = gfx->tiles;
        child->basePalette = gfx->palette;
        child->palette = (u8)gfx->palette;
        Sprite_Attach(child, sChaos2SpriteSheet, tiles, child->basePalette);
        index = (angle & 0xFFFF) / 0x1555;
        child->animFrame = frames.entries[index][0];
        child->animMode = 2;
        if (frames.entries[index][1] & 1)
            child->flipX = 1;
        if (frames.entries[index][1] & 2)
            child->flipY = 1;
        child->enemyId = parent->enemyId;
        child->ext.chaosChild.link = parent;
        zero = 0;
        child->scrollWithBg = zero;
        child->attack = 0x64;
        Hit_Enable(child, 8, Chaos2Shot_DamagePlayerOnContact, 0);
        HitBox_SetAttack(child, &collision);
        child->hit.flags |= 4;
        child->hit.flags |= zero;
        child->hit.flags &= ~2;
        child->drawFunc = Sprite_DrawEntity;
        child->timer = 78;
    }
}

/* Chaos second form: ring shot update (51C0). */
/* frame and mirror bits of a shot per twelfth of a turn */
struct ShotFrame {
    u16 frame;
    u16 mirror;
};

/* Shot: wait with the core, then fly; vanish off screen, after 240
 * frames or when the core is gone or dying. */
void Chaos2Shot_Update(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    struct EwramData_EntityData *core = entity->ext.chaosChild.link;

    if (entity->state == 0)
    {
        if (--entity->timer <= 1)
        {
            entity->state++;
            PlaySong(0x146);
        }
        if (core != NULL && core->updateFunc != NULL
            && core->enemyId == entity->enemyId
            && core->state != 3)
            return;
        EntityDelete(entity);
        return;
    }
    else if (entity->state == 1)
    {
        EntityMove(entity);
        Hit_CheckWithPlayer(&result, entity);
        if (EntityIsOutsideScreen(entity, 1) || entity->timer++ > 0xEF)
        {
            EntityDelete(entity);
            return;
        }
        if (core == NULL || core->updateFunc == NULL
            || core->enemyId != entity->enemyId
            || core->state == 3)
            EntityDelete(entity);
    }
}

/* Chaos second form: summoned part (create 5270). */

/* Create a summoned part at the core + (x, y) moving with (vx, vy)
 * (600 HP, box x -3, y -3, w 6, h 6, hit by Chaos2SummonedPart_OnContact / Chaos2SummonedPart_OnHit). */
void Chaos2_SummonPart(struct EwramData_EntityData *core, s32 x, s32 y, s32 vx, s32 vy)
{
    struct EwramData_EntityData *part = EntityCreateInRange(0x49, 0xCF, Chaos2SummonedPart_Update);
    struct HitBox box = { -3, -3, 6, 6 };
    struct Chaos2Data *data = (struct Chaos2Data *)core->ext.chaos.data.value;

    if (part != NULL)
    {
        s32 graphics;

        part->posX.whole = core->posX.whole + x;
        part->posY.whole = core->posY.whole + y;
        part->velX.whole = vx;
        part->velY.whole = vy;
        part->spriteLayer = 0x11;
        graphics = data->graphics;
        part->basePalette = data->palette2;
        Sprite_Attach(part, sChaos2SpriteSheet, graphics, part->basePalette);
        Anim_StartIndexed(part, 6, 2, 1);
        part->animMode = 2;
        part->animTimer = RandomNumberGenerator() % 6;
        part->enemyId = core->enemyId;
        part->ext.chaosChild.link = core;
        part->scrollWithBg = 0;
        part->attack = 0x6E;
        part->hp = 600;
        part->enemyFlags |= 1;
        Hit_Enable(part, 9, Chaos2SummonedPart_OnContact, Chaos2SummonedPart_OnHit);
        Hit_SetTimerOverride(3, 2);
        HitBox_SetBody(part, &box);
        HitBox_SetAttack(part, &box);
        part->hit.flags |= 4;
        part->hit.flags |= 2;
        part->drawFunc = Sprite_DrawEntity;
    }
}

/* Chaos second form: summoned part update (537C). */

#define LIFETIME(e) ((e)->ext.chaosChild.lifetime)

/* Summoned part: home in on the target (enemyTarget), become harmless once
 * slow, break after 1500 frames or when the core is gone; state 9 bursts
 * into sparks, state 10 blinks out. */
void Chaos2SummonedPart_Update(struct EwramData_EntityData *part)
{
    struct HitResult result;
    struct PlayerContactDamage fx;
    struct EwramData_EntityData *core = part->ext.chaosChild.link;
    struct EwramData **ewram = &gEwramData;
    struct Chaos2Data *data = (struct Chaos2Data *)core->ext.chaos.data.value;

    data->summoned = 1;
    if (part->state == 0)
    {
        Hit_CheckWithPlayer(&result, part);
        if (core != NULL && core->updateFunc != NULL
            && core->enemyId == part->enemyId
            && core->state != 3)
        {
            struct EwramData_EntityData *target = (*ewram)->playerState.enemyTarget;
            s32 dx = target->posX.whole - part->posX.whole;
            s32 ty = target->posY.whole;
            s32 py = part->posY.whole + 0x100000;
            s32 dy = ty - py;
            u16 angle = ArcTan2(TO_PIXELS(dx), TO_PIXELS(dy));

            dx = Sin(angle + 0x4000);
            dy = Sin(angle);
            dx = FixedMultiply(dx, 0x3000);
            dy = FixedMultiply(dy, 0x3000);
            part->posX.whole += dx;
            part->posY.whole += dy;
            EntityDampVelocityX(part, 0x18);
            EntityDampVelocityY(part, 0x18);
            EntityMove(part);
            if (part->stage == 0
                && (part->velX.whole < 0 ? -part->velX.whole : part->velX.whole) <= 0x1FF)
            {
                part->stage++;
                part->hit.flags &= ~4;
                part->hit.flags &= ~2;
            }
            if (RandomNumberGenerator() % 16)
                Anim_Update(part);
            if (LIFETIME(part)++ > 0x5DB)
            {
                part->state = 10;
                part->subState = 0;
                part->hp = 0;
                part->palette = part->basePalette;
                part->timer = 40;
                part->hitFlashTimer = 0;
            }
            return;
        }
        part->state = 10;
        part->subState = 0;
        part->hp = 0;
        part->palette = part->basePalette;
        part->timer = 40;
        part->hitFlashTimer = 0;
    }
    else if (part->state == 9)
    {
        if ((part->timer & 3) == 0)
            Chaos2_SpawnSpark(part);
        if ((s8)--part->timer <= 0)
        {
            part->deleteRequested = 1;
            fx.attack = part->attack;
            fx.attributes = 0;
            fx.unused = 0;
            fx.reaction = 0;
            fx.sourceX = part->posX.whole;
            fx.sourceY = part->posY.whole;
            Player_ApplyContactDamage(&fx);
            if (core != NULL && core->updateFunc != NULL
                && core->enemyId == part->enemyId)
                data->summoned = 0;
        }
    }
    else if (part->state == 10)
    {
        part->hidden = Entity_GetVisibilityPattern(part->timer);
        if ((s8)--part->timer <= 0)
        {
            part->deleteRequested = 1;
            if (core != NULL && core->updateFunc != NULL
                && core->enemyId == part->enemyId)
                data->summoned = 0;
        }
        return;
    }
    Anim_Update(part);
}

/* Chaos second form: summoned part damage (55F0). */
/* Summoned part hit: bounce away from the attacker (speed 2 px), lose HP;
 * at 0 break (state 10). */
void Chaos2SummonedPart_OnHit(struct EwramData_EntityData *part, struct EwramData_EntityData *attacker, u16 *hit)
{
    u16 damage = Hit_ResolveDamage(part, attacker, hit, Player_CalculateAttackPower(hit[2]));
    s32 dx, dy;
    u16 angle;
    s32 zero;

    dx = part->posX.whole - attacker->posX.whole;
    dy = part->posY.whole - attacker->posY.whole;
    angle = ArcTan2(TO_PIXELS(dx), TO_PIXELS(dy));
    dx = Sin(angle + 0x4000);
    dy = Sin(angle);
    part->velX.whole = FixedMultiply(dx, 0x20000);
    part->velY.whole = FixedMultiply(dy, 0x20000);
    {
        s32 hp = part->hp;
        s32 d = (s16)damage;

        hp -= d;
        zero = 0;
        part->hp = hp;
        if ((s16)hp > 0)
            return;
    }
    {
        part->palette = part->basePalette;
        part->timer = 40;
        part->hitFlashTimer = zero;
        part->state = 10;
        part->subState = zero;
    }
}

/* Chaos second form: spark burst (56CC). */

/* Create a spark (animation sChaos2SparkAnimScript) at a random angle on a circle
 * of radius 48 around the core, flying inwards and slowing down. */
void Chaos2_SpawnSpark(struct EwramData_EntityData *core)
{
    struct EwramData_EntityData *spark = EntityCreateInRange(0x49, 0xCF, Chaos2Spark_Update);

    if (spark != NULL)
    {
        s32 angle, c, s, vx, vy, graphics;

        spark->posX.whole = -0x800000;
        spark->posY.whole = -0x800000;
        spark->spriteLayer = core->spriteLayer - 1;
        graphics = ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(spark, sCommonObjectSpriteSheet, graphics, 7);
        Anim_Start(spark, &sChaos2SparkAnimScript, 0, 0);
        spark->hit.flags |= 4;
        spark->hit.flags |= 2;
        spark->drawFunc = Sprite_DrawEntity;
        spark->timer = 20;
        angle = (RandomNumberGenerator() % 256) << 8;
        c = Sin(angle + 0x4000);
        s = Sin(angle);
        OFFSET_X(spark) = c * 48;
        OFFSET_Y(spark) = s * 48 - 0x100000;
        vx = -(c * 20) / 16;
        spark->velX.whole = vx;
        vy = -(s * 20) / 16;
        spark->velY.whole = vy;
        spark->accelX.whole = vx / 16;
        spark->accelY.whole = vy / 16;
    }
}

/* Chaos second form: body creation (57BC). */

#undef ANGLE
#define ANGLE(e) (e->ext.chaosSnake.angle)
#define INDEX(e) (e->ext.chaosSnake.index)

/* Create the snake-like body at the core + (x, y): the head (Chaos2SnakeHead_Update)
 * with a 120-entry position history and 8 segments (Chaos2SnakeSegment_Update). */
void Chaos2_CreateSnake(struct EwramData_EntityData *core, s32 x, s32 y, s32 unused)
{
    u8 *trail;
    struct EwramData_EntityData *seg;
    s32 i;
    u16 slot;
    s32 graphics;
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    struct EwramData_EntityData *head = EntityCreateInRange(0x49, 0xCF, Chaos2SnakeHead_Update);
    struct HitBox box = { -8, -8, 16, 16 };
    struct Chaos2Data *data = (struct Chaos2Data *)core->ext.chaos.data.value;

    if (head != NULL)
    {
        if ((head->ext.chaosSnake.trail = (s32)Heap_Alloc(0xB4)) == 0)
        {
            EntityDelete(head);
            return;
        }
        head->posX.whole = core->posX.whole + x;
        head->posY.whole = core->posY.whole + y;
        head->spriteLayer = 0x1E;
        graphics = data->graphics;
        head->basePalette = data->palette2;
        Sprite_Attach(head, sChaos2SpriteSheet, graphics, head->basePalette);
        head->animFrame = 0xB;
        head->animMode = 2;
        head->enemyId = core->enemyId;
        head->ext.chaosChild.link = core;
        head->scrollWithBg = 0;
        head->attack = 0x55;
        head->ext.chaosSnake.cameraX = bg->xPos.part16.integer;
        head->ext.chaosSnake.cameraY = bg->yPos.part16.integer;
        head->ext.chaosSnake.mode = 0;
        head->ext.chaosSnake.point = 4;
        Hit_Enable(head, 8, Chaos2SnakeHead_DamagePlayerOnContact, 0);
        HitBox_SetAttack(head, &box);
        head->hit.flags |= 4;
        head->hit.flags &= ~2;
        head->drawFunc = Sprite_DrawEntity;
        head->flipX = 0;
        ANGLE(head) = 0x4000;
        slot = ObjAffine_Alloc(1);
        if ((s16)slot < 0)
        {
            EntityDelete(head);
            return;
        }
        head->affineIndex = slot;
        head->affine = 1;
        ObjAffine_Set(0, 0x10000, 0x10000, head->flipX, head->flipY, head->affineIndex);
        trail = (u8 *)head->ext.chaosSnake.trail;
        for (i = 119; i >= 0; i--)
        {
            trail[i * 6 + 0] = 0;
            trail[i * 6 + 1] = 0;
            trail[i * 6 + 2] = head->animFrame;
            trail[i * 6 + 3] = head->spriteLayer;
            trail[i * 6 + 4] = (u16)ANGLE(head) >> 8;
            trail[i * 6 + 5] = ANGLE(head);
        }
        i = 7;
        do
        {
            seg = Entity_CreateObject(Chaos2SnakeSegment_Update);
            if (seg != NULL)
            {
                seg->spriteLayer = head->spriteLayer + 1;
                Sprite_Attach(seg, sChaos2SpriteSheet, data->graphics, head->basePalette);
                Chaos2SnakeSegment_EnableHitBoxes(seg);
                seg->enemyId = head->enemyId;
                seg->basePalette = head->basePalette;
                seg->scrollWithBg = 0;
                seg->spriteLayer = head->spriteLayer + 1;
                seg->drawFunc = Sprite_DrawEntity;
                seg->ext.chaosChild.link = head;
                INDEX(seg) = i;
                seg->timer = i;
                seg->attack = head->attack;
                seg->ext.chaosSnake.cameraX = head->ext.chaosSnake.cameraX;
                seg->ext.chaosSnake.cameraY = head->ext.chaosSnake.cameraY;
                seg->posX.whole = head->posX.whole;
                seg->posY.whole = head->posY.whole;
                seg->animFrame = 0xE;
                seg->animMode = 2;
                if (i == 7)
                {
                    seg->animFrame = 5;
                    slot = ObjAffine_Alloc(1);
                    if ((s16)slot < 0)
                    {
                        /* loop.c moves this block before the loop */
                        EntityDelete(seg);
                        return;
                    }
                    seg->affineIndex = slot;
                    seg->affine = 1;
                    ObjAffine_Set((u16)ANGLE(seg) - 0x8000, 0xC000, 0xC000, seg->flipX, seg->flipY, seg->affineIndex);
                }
                continue;
            }
            /* the two earlier failures are separate copies; jump2
               cross-jumps them into this block */
            EntityDelete(head);
            return;
        } while (--i >= 0);
    }
}

/* Chaos second form: head update (5A7C). */

#define TARGET_X(e) (e->ext.chaosSnake.targetX)
#define TARGET_Y(e) (e->ext.chaosSnake.targetY)
#undef ANGLE
#define ANGLE(e) (*(u16 *)&e->ext.chaosSnake.angle)
#define TIMER(e) (*(u16 *)&e->ext.chaosSnake.timer)
#define TURN(e) (e->ext.chaosSnake.turn)
#define MODE(e) (e->ext.chaosSnake.mode)
#define POINT(e) (e->ext.chaosSnake.point)

/* Shift the 120-entry history (6 bytes each) by one entry. */
#define SHIFT_TRAIL(trail)                                  \
    {                                                       \
        s32 i_;                                             \
        for (i_ = 119; i_ > 0; i_--)                        \
        {                                                   \
            (trail)[i_ * 6 + 0] = (trail)[i_ * 6 - 6];      \
            (trail)[i_ * 6 + 1] = (trail)[i_ * 6 - 5];      \
            (trail)[i_ * 6 + 2] = (trail)[i_ * 6 - 4];      \
            (trail)[i_ * 6 + 3] = (trail)[i_ * 6 - 3];      \
            (trail)[i_ * 6 + 4] = (trail)[i_ * 6 - 2];      \
            (trail)[i_ * 6 + 5] = (trail)[i_ * 6 - 1];      \
        }                                                   \
    }

/* Pick a new waypoint 0-7 different from the current one: with probability
 * 1/3 per try one of the outer points 4-7, otherwise one of 0-3. */
#define PICK(e, r, L)                                               \
    do                                                              \
    {                                                               \
        while (RandomNumberGenerator() % 3 == 0)                    \
        {                                                           \
            r = (RandomNumberGenerator() & 3) + 4;                  \
            if (POINT(e) != r) { POINT(e) = r; goto L; }            \
        }                                                           \
        r = RandomNumberGenerator() & 3;                            \
    } while (POINT(e) == r);                                        \
    POINT(e) = r;                                                   \
    L:;

/**
 * @brief Snake head update: move along its path, record the 120-entry position
 * history for the segments and fire shots (Chaos2SnakeHead_FireShot); break
 * apart with the core.
 */
void Chaos2SnakeHead_Update(struct EwramData_EntityData *e)
{
    struct EwramData_EntityData *core = e->ext.chaosChild.link;
    u8 unused[0x20];
    struct HitResult result;
    u8 t8[8];
    u16 t0[5][2];
    u16 t1[8][2];
    u16 t1b[16];
    u16 t2[8][3];
    u16 t3[8][3];
    u8 *trail;

    memcpy(t8, sUnk_08119264, 8);
    memcpy(t0, sUnk_0811926C, 0x14);
    memcpy(t1, sUnk_08119280, 0x20);
    memcpy(t1b, sUnk_081192A0, 0x20);
    memcpy(t2, sUnk_081192C0, 0x30);
    memcpy(t3, sUnk_081192C0, 0x30);
    trail = (u8 *)e->ext.chaosSnake.trail;

    if (e->state == 0)
    {
        if (core != NULL && core->updateFunc != NULL
            && core->enemyId == e->enemyId)
        {
            switch (e->subState)
            {
            case 0:
                e->subState = 1;
                e->accelY.whole = 0x800;
            case 1:
                if (MODE(e) == 0)
                {
                    TARGET_X(e) = t0[POINT(e)][0];
                    TARGET_Y(e) = t0[POINT(e)][1];
                }
                else if (MODE(e) == 1)
                {
                    TARGET_X(e) = t1[POINT(e)][0];
                    TARGET_Y(e) = t1[POINT(e)][1];
                }
                else if (MODE(e) == 2)
                {
                    TARGET_X(e) = t2[POINT(e)][0];
                    TARGET_Y(e) = t2[POINT(e)][1];
                }
                else if (MODE(e) == 3)
                {
                    TARGET_X(e) = t3[POINT(e)][0];
                    TARGET_Y(e) = t3[POINT(e)][1];
                }
                e->subState++;
                e->timer = 0;
                TIMER(e) = 0;
                SHIFT_TRAIL(trail);
                trail[0] = 0;
                trail[1] = 0;
                trail[2] = e->animFrame;
                trail[3] = e->spriteLayer;
                trail[4] = (u16)ANGLE(e) >> 8;
                trail[5] = ANGLE(e);
                break;
            case 2:
            {
                s8 ox, oy;
                s32 dx, dy, dist;
                u16 diff;
                s32 r;

                dx = TARGET_X(e) - (s16)e->posX.part16.integer;
                dy = TARGET_Y(e) - e->posY.part16.integer;
                dx = dx * dx;
                dy = dy * dy;
                dist = Sqrt(dx + dy);
                if (dist <= 0)
                {
                    e->posX.part16.integer = TARGET_X(e);
                    e->posX.part16.fraction = 0;
                    e->posY.part16.integer = TARGET_Y(e);
                    e->posY.part16.fraction = 0;
                    e->accelX.whole = 0;
                }
                ox = e->posX.part16.integer;
                oy = e->posY.part16.integer;
                dx = TARGET_X(e) - (s16)e->posX.part16.integer;
                dy = TARGET_Y(e) - e->posY.part16.integer;
                { s32 a_ = ArcTan2(dx, dy); diff = (u16)(a_ - ANGLE(e)); }
                if (dist <= 5)
                {
                    /* FAKEMATCH: the target keeps two register copies of diff
                     * (cmp in r0, add in r1). */
                    s32 c_ = diff;
                    asm("" : "+r"(c_)); /* FAKEMATCH */
                    if (c_ > 0x20)
                        TURN(e) += 0x20;
                    else
                    {
                        s32 d_ = diff;
                        asm("" : "+r"(d_)); /* FAKEMATCH */
                        TURN(e) += d_;
                    }
                }
                else
                {
                    if (TURN(e) <= 0x27F)
                        TURN(e) += 8;
                    else
                        TURN(e) = 0x280;
                }
                if ((s16)diff >= 0)
                {
                    if (diff > TURN(e))
                        ANGLE(e) += TURN(e);
                    else if (diff > *(vu16 *)&TURN(e) >> 1)
                        ANGLE(e) = diff + ANGLE(e);
                }
                else
                {
                    if ((s32)diff < 0x10000 - TURN(e))
                        ANGLE(e) -= TURN(e);
                    else if ((s32)diff < 0x10000 - (TURN(e) >> 1))
                    {

                        ANGLE(e) = diff + ANGLE(e);
                    }
                }
                if (MODE(e) == 0)
                {
                    if (e->spriteLayer > 0x11)
                        e->spriteLayer--;
                    if (e->timer > 5)
                    {
                        if (e->animFrame == 10)
                            e->animFrame = 11;
                    }
                    else if (e->animFrame == 5)
                        e->animFrame = 10;
                }
                else if (MODE(e) == 1)
                {
                    if (e->timer > 9)
                    {
                        if (e->animFrame == 10)
                            e->animFrame = 5;
                    }
                    else if (e->animFrame == 5)
                        e->animFrame = 10;
                }
                else if (MODE(e) == 2)
                {
                    if (e->timer > 9)
                    {
                        if (e->animFrame == 10)
                            e->animFrame = 5;
                    }
                    else if (e->animFrame == 5)
                        e->animFrame = 10;
                }
                else if (MODE(e) == 3)
                {
                    if (e->timer > 9 && e->animFrame == 6)
                        e->animFrame = 5;
                }
                dx = Sin(ANGLE(e) + 0x4000);
                dy = Sin(ANGLE(e));
                if (e->animFrame != 11)
                    ObjAffine_Set(ANGLE(e) - 0x8000, 0x10000, 0x10000, e->flipX, e->flipY, e->affineIndex);
                else
                    ObjAffine_Set(0, 0x10000, 0x10000, e->flipX, e->flipY, e->affineIndex);
                if (dist > 0x28)
                {
                    if (e->accelX.whole <= 0x27FFF)
                        e->accelX.whole += e->accelY.whole * 2;
                }
                else if (dist > 0x20)
                {
                    if (e->accelX.whole > 0x20000)
                        e->accelX.whole -= e->accelY.whole * 3;
                }
                else if (dist > 0x18)
                {
                    if (e->accelX.whole > 0x18000)
                        e->accelX.whole -= e->accelY.whole * 2;
                }
                else if (dist > 0xC)
                {
                    if (e->accelX.whole > 0x10000)
                        e->accelX.whole -= e->accelY.whole * 2;
                }
                else if (dist > 4)
                {
                    if (e->accelX.whole > 0x8000)
                        e->accelX.whole -= e->accelY.whole * 2;
                }
                else if (e->accelX.whole > 0x4000)
                    e->accelX.whole -= e->accelY.whole * 2;
                else if (e->accelX.whole > 0x1000)
                    e->accelX.whole -= e->accelY.whole;
                else if (e->accelX.whole <= 0xFFF)
                    e->accelX.whole = 0x1000;
                dx = FixedMultiply(dx, e->accelX.whole);
                dy = FixedMultiply(dy, e->accelX.whole);
                e->velX.whole = dx;
                e->velY.whole = dy;
                EntityMove(e);
                e->timer++;
                TIMER(e)++;
                ox = (u8)e->posX.part16.integer - ({ s32 t_ = (s8)ox; t_; });
                oy = (u8)e->posY.part16.integer - ({ s32 t_ = (s8)oy; t_; });
                SHIFT_TRAIL(trail);
                trail[0] = ox;
                trail[1] = oy;
                trail[2] = e->animFrame;
                trail[3] = e->spriteLayer;
                trail[4] = (u16)ANGLE(e) >> 8;
                trail[5] = ANGLE(e);
                if (dist <= 2)
                {

                    if (MODE(e) == 0)
                    {
                        e->subState = 1;
                        MODE(e) = 2;
                        do
                            r = RandomNumberGenerator() & 7;
                        while (POINT(e) == r);
                        POINT(e) = r;
                        e->animFrame = 5;
                    }
                    else if (MODE(e) == 1)
                    {
                        e->subState = 3;
                        e->timer = 0;
                        e->stage = 0;
                    }
                    else if (MODE(e) == 2)
                    {
                        e->subState = 1;
                        e->spriteLayer = 0x19;
                        e->hit.flags |= 2;
                        e->hit.flags |= 4;
                        MODE(e) = 3;
                        PICK(e, r, pk1);
                    }
                    else if (MODE(e) == 3)
                    {
                        e->subState = 1;
                        if (POINT(e) <= 3 && (RandomNumberGenerator() & 0x7F) <= 0x20)
                        {
                            e->spriteLayer = 0x11;
                            e->hit.flags &= ~2;
                            e->hit.flags &= ~4;
                            MODE(e) = 0;
                        }
                        else if ((RandomNumberGenerator() & 0x7F) <= 0x40)
                        {
                            e->spriteLayer = 0x11;
                            e->hit.flags &= ~2;
                            e->hit.flags &= ~4;
                            MODE(e) = 1;
                            PICK(e, r, pk2);
                        }
                        else
                        {
                            e->spriteLayer = 0x11;
                            e->hit.flags &= ~2;
                            e->hit.flags &= ~4;
                            MODE(e) = 2;
                            /* Same as PICK, written with gotos: the outer
                             * retry must jump straight to the inner test so
                             * that cross-jumping keeps the outer compare. */
                            {
                                s32 m_ = 3;

                                goto t3;
                            b3:
                                r = (RandomNumberGenerator() & m_) + 4;
                                if (POINT(e) != r)
                                    goto pk3;
                            t3:
                                if (RandomNumberGenerator() % 3 == 0)
                                    goto b3;
                                r = RandomNumberGenerator() & m_;
                                if (POINT(e) == r)
                                    goto t3;
                            }
                        pk3:
                            POINT(e) = r;
                        }
                    }
                    TURN(e) = 0;
                }
                else if (TIMER(e) > 0x12B)
                {

                    e->subState = 1;
                    do
                        r = RandomNumberGenerator() & 3;
                    while (POINT(e) == r);
                    POINT(e) = r;
                }
                if (core->state == 3)
                {
                    e->state = 10;
                    e->subState = 0;
                    e->velX.whole = 0;
                    e->velY.whole = 0;
                    e->accelX.whole = 0;
                    e->accelY.whole = 0x2000;
                    e->timer = 0x28;
                }
                break;
            }
            case 3:
                switch (e->stage)
                {
                case 0:
                    Anim_Start(e, (struct AnimScript *)((u8 *)&sChaos2SparkAnimScript + 0x14), 2, 0);
                    e->stage++;
                    PlaySong(0x77);
                case 1:
                    if (*(u16 *)&e->animStep == 9)
                    {
                        Chaos2SnakeHead_FireShot(e, ANGLE(e));
                        PlaySong(0x147);
                    }
                    if (e->animDone)
                    {
                        MODE(e) = 2;
                        e->subState = 1;
                        e->stage = 0;
                        POINT(e) = (RandomNumberGenerator() % 4) + 4;
                    }
                    break;
                }
                Anim_Update(e);
                SHIFT_TRAIL(trail);
                trail[0] = 0;
                trail[1] = 0;
                trail[2] = 5;
                trail[3] = e->spriteLayer;
                trail[4] = (u16)ANGLE(e) >> 8;
                trail[5] = ANGLE(e);
                break;
            }
            Hit_CheckWithPlayer(&result, e);
        }
        else
        {
            e->state = 10;
            e->subState = 0;
            e->hp = 0;
            e->palette = e->basePalette;
            e->timer = 0x28;
            e->hitFlashTimer = 0;
            e->accelX.whole = 0;
            e->accelY.whole = 0x2000;
        }
    }
    else if (e->state == 10)
    {
        if ((e->timer & 3) == 0)
            Effect_SpawnRandom(e->posX.whole, e->posY.whole - 0x60000, 0x100000, 0xC0000, e->spriteLayer - 1);
        EntityAccelerateAndMove(e, 0x70000);
        e->hidden = Entity_GetVisibilityPattern(e->timer);
        if ((s8)--e->timer <= 0)
        {
            if (e->ext.chaosSnake.trail != 0)
                Heap_Free((void *)e->ext.chaosSnake.trail);
            ObjAffine_Free(e->affineIndex);
            EntityDelete(e);
        }
    }
}

/* Chaos second form: body segment update (62B8). */
/* The head's position history (unk_508): 120 entries of 6 bytes
 * (dx, dy, animation, priority, angle high, angle low). */
#define TRAIL_DX 0
#define TRAIL_DY 1
#define TRAIL_ANIM 2
#define TRAIL_PRIORITY 3
#define TRAIL_ANGLE_HI 4
#define TRAIL_ANGLE_LO 5

#undef INDEX
#define INDEX(e) ((s8)e->ext.chaosSnake.index)
#define TRAIL(t, e, f) ((t)[INDEX(e) * 48 + 48 + (f)])

/* Segment unk_4FE of the body: follow the head's trail 8 entries per
 * segment, compensating camera movement; the last segment (7) also takes
 * the rotation. When the head breaks (state 10) fall and blink out. */
void Chaos2SnakeSegment_Update(struct EwramData_EntityData *seg)
{
    struct HitResult result;
    struct EwramData_EntityData *head = seg->ext.chaosChild.link;
    struct BgLayerState *bg = &gEwramData->bgInfo[1];

    if (seg->state == 0)
    {
        u8 *trail;

        if (head != NULL && head->updateFunc != NULL
            && head->enemyId == seg->enemyId)
        {
        trail = (u8 *)head->ext.chaosSnake.trail;
        seg->hidden = head->hidden;
        seg->palette = head->palette;
        seg->timer++;
        seg->velX.part16.integer = (s8)TRAIL(trail, seg, TRAIL_DX)
            + ((u8)seg->ext.chaosSnake.cameraX - bg->xPos.part16.integer);
        seg->velY.part16.integer = (s8)TRAIL(trail, seg, TRAIL_DY)
            + (seg->ext.chaosSnake.cameraY - bg->yPos.part16.integer);
        seg->posX.part16.integer += seg->velX.part16.integer;
        seg->posY.part16.integer += seg->velY.part16.integer;
        if ((seg->spriteLayer = TRAIL(trail, seg, TRAIL_PRIORITY) + 1) != 0x12)
        {
            seg->hit.flags |= 2;
            seg->hit.flags |= 4;
        }
        else
        {
            seg->hit.flags &= ~2;
            seg->hit.flags &= ~4;
        }
        seg->ext.chaosSnake.cameraX = bg->xPos.part16.integer;
        seg->ext.chaosSnake.cameraY = bg->yPos.part16.integer;
        if (INDEX(seg) != 7)
        {
            s8 dx = (s8)TRAIL(trail, seg, TRAIL_DX);

            if (dx > 0)
                seg->flipX = 1;
            else if (dx < 0)
                seg->flipX = 0;
        }
        if (INDEX(seg) == 7)
        {
            ObjAffine_Set((((s8)TRAIL(trail, seg, TRAIL_ANGLE_HI) << 8) & 0xFF00) | TRAIL(trail, seg, TRAIL_ANGLE_LO),
                         0xC000, 0xC000, seg->flipX, seg->flipY, seg->affineIndex);
            {
                s8 anim = TRAIL(trail, seg, TRAIL_ANIM);

                if (anim == 5)
                    seg->animFrame = 5;
                else if (anim == 6)
                    seg->animFrame = 10;
                else if (anim == 10)
                    seg->animFrame = 6;
                else if (anim == 11)
                    seg->animFrame = 6;
            }
        }
        else
        {
            s8 anim = TRAIL(trail, seg, TRAIL_ANIM);

            if (anim == 5)
                seg->animFrame = 14;
            else if (anim == 11)
                seg->animFrame = 15;
        }
        if ((seg->timer & 7) == 0)
            Hit_CheckWithPlayer(&result, seg);
        if (head->state == 10)
        {
            seg->state = 1;
            seg->subState = 0;
            seg->accelX.whole = 0;
            seg->accelY.whole = 0x2000;
            seg->timer = RandomNumberGenerator() % 4 + 60;
        }
        }
        else
            EntityDelete(seg);
        return;
    }
    if (seg->subState == 0)
    {
        seg->subState++;
        seg->scrollWithBg = 1;
    }
    EntityAccelerateAndMove(seg, 0x70000);
    seg->hidden = Entity_GetVisibilityPattern(seg->timer);
    if ((s8)--seg->timer <= 0)
    {
        seg->deleteRequested = 1;
        if (INDEX(seg) == 7)
            ObjAffine_Free(seg->affineIndex);
    }
}

/* Chaos2SnakeSegment_EnableHitBoxes (hit boxes). */

/**
 * @brief A657C | Activate box slot 8 with Chaos2SnakeSegment_DamagePlayerOnContact and set both boxes to
 * the bytes of 0x0808FCFC (x -4, y -4, w 8, h 8 - not a ROM address);
 * clear flags bit 1, set bit 2.
 */
void Chaos2SnakeSegment_EnableHitBoxes(struct EwramData_EntityData *entity)
{
    u32 box = 0x0808FCFC;

    Hit_Enable(entity, 8, Chaos2SnakeSegment_DamagePlayerOnContact, 0);
    HitBox_SetBody(entity, &box);
    HitBox_SetAttack(entity, &box);
    entity->hit.flags &= ~2;
    entity->hit.flags |= 4;
}

/* Chaos second form: create a moving child from angle and shared graphics. */
struct ChaosChildGfx { s32 tiles; s32 unused; s32 palette; };

/**
 * @brief Fire a shot from the snake head at angle (minus 0x1300) that bounces
 * inside the screen (Chaos2SnakeShot_Update).
 */
void Chaos2SnakeHead_FireShot(struct EwramData_EntityData *parent, s32 angle)
{
    /* Packed box: x=-4, y=-4, width=8, height=8. */
    u32 collision = 0x0808FCFC;
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, Chaos2SnakeShot_Update);
    struct ChaosChildGfx *gfx = (struct ChaosChildGfx *)parent->ext.chaosChild.link->ext.chaos.data.ptr;
    angle -= 0x1300;
    if (child != NULL)
    {
        s32 x, y, tiles;
        x = Sin(angle + 0x4000);
        x = FixedMultiply(x, 0x1C0000);
        y = Sin(angle);
        y = FixedMultiply(y, 0x1C0000);
        child->posX.whole = parent->posX.whole + x;
        child->posY.whole = parent->posY.whole + y;
        x = Sin(angle + 0x4000);
        x = FixedMultiply(x, 0x30000);
        y = Sin(angle);
        y = FixedMultiply(y, 0x30000);
        child->velX.whole = x;
        child->velY.whole = y;
        child->spriteLayer = parent->spriteLayer + 1;
        tiles = gfx->tiles;
        child->basePalette = gfx->palette;
        child->palette = (u8)gfx->palette;
        Sprite_Attach(child, sChaos2SpriteSheet, tiles, child->basePalette);
        Anim_StartIndexed(child, 7, 2, 1);
        Anim_Update(child);
        child->animFrame = 0x1E;
        child->animMode = 2;
        child->enemyId = parent->enemyId;
        child->ext.chaosChild.link = parent;
        child->scrollWithBg = 1;
        child->attack = 0x82;
        Hit_Enable(child, 8, Chaos2SnakeShot_DamagePlayerOnContact, 0);
        HitBox_SetAttack(child, &collision);
        child->hit.flags |= 4;
        child->hit.flags &= ~2;
        child->drawFunc = Sprite_DrawEntity;
    }
}

/* Chaos effect updates and creation. */
struct ChaosEffectGfx { s32 tiles; s32 palette; };

/* Move, bounce within the screen during the first 100 frames, and expire. */
void Chaos2SnakeShot_Update(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    struct EwramData_EntityData *owner = entity->ext.chaosChild.link->ext.chaosChild.link;
    EntityMove(entity);
    Hit_CheckWithPlayer(&result, entity);
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
    struct EwramData_EntityData *effect = EntityCreateInRange(0x49, 0xCF, Chaos2AffineEffect_Update);
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
            effect->drawFunc = Sprite_DrawEntity;
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

/* Chaos second form: effect and spinning projectile creation. */

/* Copy graphics from the parent's owner and attach an animated effect. */
void Chaos2_SpawnEffect(struct EwramData_EntityData *parent, s32 frame)
{
    struct EwramData_EntityData *effect = EntityCreateInRange(0x49, 0xCF, Chaos2Effect_Update);
    struct ChaosEffectGfx *gfx = (struct ChaosEffectGfx *)parent->ext.chaosChild.link->ext.chaos.data.ptr;
    if (effect != NULL)
    {
        s32 tiles;
        effect->posX.part16.integer = parent->posX.part16.integer;
        effect->posY.part16.integer = parent->posY.part16.integer;
        effect->spriteLayer = parent->spriteLayer;
        tiles = gfx->tiles;
        effect->basePalette = gfx->palette;
        Sprite_Attach(effect, sChaos2SpriteSheet, tiles, effect->basePalette);
        Anim_StartIndexed(effect, (u16)frame, 2, 0);
        Anim_Update(effect);
        effect->enemyId = parent->enemyId;
        effect->ext.chaosChild.link = parent;
        effect->scrollWithBg = 0;
        effect->drawFunc = Sprite_DrawEntity;
    }
}

/* Attach an animated effect one layer above the parent. */
void Chaos2_SpawnEffectAbove(struct EwramData_EntityData *parent, s32 frame)
{
    struct EwramData_EntityData *effect = EntityCreateInRange(0x49, 0xCF, Chaos2Effect_Update);
    struct ChaosEffectGfx *gfx = (struct ChaosEffectGfx *)parent->ext.chaos.data.ptr;
    if (effect != NULL)
    {
        s32 tiles;
        effect->posX.part16.integer = parent->posX.part16.integer;
        effect->posY.part16.integer = parent->posY.part16.integer;
        effect->spriteLayer = parent->spriteLayer + 1;
        tiles = gfx->tiles;
        effect->basePalette = gfx->palette;
        Sprite_Attach(effect, sChaos2SpriteSheet, tiles, effect->basePalette);
        Anim_StartIndexed(effect, (u16)frame, 2, 0);
        Anim_Update(effect);
        effect->enemyId = parent->enemyId;
        effect->ext.chaosChild.link = parent;
        effect->scrollWithBg = 0;
        effect->drawFunc = Sprite_DrawEntity;
    }
}

/* Create a spinning projectile; delete it if no affine matrix is free. */
void Chaos2_SpawnSpinningDebris(struct EwramData_EntityData *parent, s32 angle, s32 speed)
{
    struct EwramData_EntityData *effect = EntityCreateInRange(0x49, 0xCF, Chaos2SpinningDebris_Update);
    struct ChaosEffectGfx *gfx = (struct ChaosEffectGfx *)parent->ext.chaos.data.ptr;
    if (effect != NULL)
    {
        s32 tiles;
        u16 matrix;
        effect->posX.part16.integer = parent->posX.part16.integer;
        effect->posY.part16.integer = parent->posY.part16.integer;
        effect->spriteLayer = parent->spriteLayer;
        tiles = gfx->tiles;
        effect->basePalette = gfx->palette;
        Sprite_Attach(effect, sChaos2SpriteSheet, tiles, effect->basePalette);
        effect->animFrame = 0x4A;
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
            effect->accelX.whole = (RandomNumberGenerator() & 0x3FF) - 0x200;
            EntitySetVelocityFromAngle(effect, angle, speed);
            effect->drawFunc = Sprite_DrawEntity;
        }
    }
}

/* Chaos2SpinningDebris_Update, Chaos2_SpawnShard, EnemyChaos2Update. */

/* Graphics of the parent (unk_504 points at it) used by its shards. */
struct ShardGfx {
    s32 gfx;
    s32 palette;
};

/**
 * @brief A6D88 | Update of a spinning projectile: angle unk_4FC += accelX,
 * move, rotate with its affine matrix affineIndex; once off screen release
 * the matrix (ObjAffine_Free) and delete.
 */
void Chaos2SpinningDebris_Update(struct EwramData_EntityData *entity)
{
    entity->ext.chaosDebris.angle += entity->accelX.whole;
    entity->posX.whole += entity->velX.whole;
    entity->posY.whole += entity->velY.whole;
    ObjAffine_Set((u16)entity->ext.chaosDebris.angle, 0x10000, 0x10000, entity->flipX, entity->flipY,
                 entity->affineIndex);
    if (EntityIsOutsideScreen(entity, 2))
    {
        ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
    }
}

/**
 * @brief A6DEC | Spawn a shard of parent (slots 0x49..0xCF, update
 * Chaos2Shard_Update) at its position and layer with the parent's graphics
 * (unk_504), a random frame 0x4B..0x4D, random spin accelX and flips,
 * moving at angle/speed; unk_4F4 = parent.
 */
void Chaos2_SpawnShard(struct EwramData_EntityData *parent, s32 angle, s32 speed)
{
    struct EwramData_EntityData *shard = EntityCreateInRange(0x49, 0xCF, Chaos2Shard_Update);
    struct ShardGfx *gfx = (struct ShardGfx *)parent->ext.chaos.data.ptr;

    if (shard != NULL)
    {
        shard->posX.part16.integer = parent->posX.part16.integer;
        shard->posY.part16.integer = parent->posY.part16.integer;
        shard->spriteLayer = parent->spriteLayer;
        {
            s32 tiles = gfx->gfx;

            shard->basePalette = gfx->palette;
            Sprite_Attach(shard, sChaos2SpriteSheet, tiles, shard->basePalette);
        }
        shard->animFrame = RandomNumberGenerator() % 3 + 0x4B;
        shard->enemyId = parent->enemyId;
        shard->ext.chaosChild.link = parent;
        shard->scrollWithBg = 0;
        shard->accelX.whole = (RandomNumberGenerator() & 0x3FF) - 0x200;
        shard->flipX = RandomNumberGenerator() % 2;
        shard->flipY = RandomNumberGenerator() % 2;
        EntitySetVelocityFromAngle(shard, angle, speed);
        shard->drawFunc = Sprite_DrawEntity;
    }
}

/**
 * @brief A6ED4 | Chaos (second form) update: outside state 3 react to the
 * player in range (Entity_IsInRect -> Player_TrackEntity) and Enemy_UpdateHitFlash; then
 * states 0..3 (Chaos2_UpdateIntro / Chaos2_UpdateAttack / Chaos2_UpdateIdle / Chaos2_UpdateDefeat),
 * animation and Enemy_CheckHitPlayer.
 */
void EnemyChaos2Update(struct EwramData_EntityData *entity)
{
    u8 unused[0x30]; /* stack frame reserved by the original */

    if (entity->state != 3)
    {
        if (Entity_IsInRect(entity, -0x10, 0, 0x110, 0xB0))
            Player_TrackEntity(entity);
        if (entity->state != 3)
            Enemy_UpdateHitFlash(entity);
    }
    switch (entity->state)
    {
        case 0:
            Chaos2_UpdateIntro(entity);
            break;
        case 1:
            Chaos2_UpdateAttack(entity);
            break;
        case 2:
            Chaos2_UpdateIdle(entity);
            break;
        case 3:
            Chaos2_UpdateDefeat(entity);
            break;
    }
    Anim_Update(entity);
    Enemy_CheckHitPlayer(entity);
}

/* Chaos-fight helper entities 0x080A6F58..0x080A712B. */

/**
 * @brief A6F58 | Reset fields 0x4FC..0x504 of the linked entity (unk_504)
 * and store sequence in its 0x508.
 */
void Chaos_StartPaletteSequence(struct EwramData_EntityData *entity, s32 sequence)
{
    struct EwramData_EntityData *controller = entity->ext.chaos.data.ptr;

    controller->ext.raw.unk_4FC.whole = 0;
    controller->ext.raw.unk_500.whole = 0;
    controller->ext.raw.unk_504.whole = 0;
    controller->ext.raw.unk_508.whole = sequence;
}

/**
 * @brief A6F68 | Player_ApplyContactDamage with (attack, 0, 0, 0) at the entity position.
 */
void Chaos2Part_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}

/**
 * @brief A6F94 | Same as Chaos2Part_DamagePlayerOnContact.
 */
void Chaos2Shot_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}

/**
 * @brief A6FC0 | Switch to state 9 with a 60 frame timer, set 0x53C bit 5
 * and play sound 0x67.
 */
void Chaos2SummonedPart_OnContact(struct EwramData_EntityData *entity)
{
    entity->state = 9;
    entity->subState = 0;
    entity->timer = 0x3C;
    entity->hidden = 1;
    PlaySong(0x67);
}

/**
 * @brief A6FE4 | Follow enemyTarget at the offset 0x4F8/0x4FC, which itself
 * moves with velocity and acceleration; delete when the timer timer ends.
 */
void Chaos2Spark_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData **target = &gEwramData->playerState.enemyTarget;

    entity->posX.whole = (*target)->posX.whole + entity->ext.chaosShot.offsetX;
    entity->posY.whole = (*target)->posY.whole + entity->ext.chaosShot.offsetY;
    entity->velX.whole += entity->accelX.whole;
    entity->velY.whole += entity->accelY.whole;
    entity->ext.chaosShot.offsetX += entity->velX.whole;
    entity->ext.chaosShot.offsetY += entity->velY.whole;
    Anim_Update(entity);
    if ((s8)--entity->timer <= 0)
        EntityDelete(entity);
}

/**
 * @brief A7044 | Same as Chaos2Part_DamagePlayerOnContact.
 */
void Chaos2SnakeHead_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}

/**
 * @brief A7070 | Same as Chaos2Part_DamagePlayerOnContact.
 */
void Chaos2SnakeSegment_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}

/**
 * @brief A709C | Player_ApplyContactDamage with (attack, 0x10, 0, 1) at the entity position.
 */
void Chaos2SnakeShot_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 0x10;
    contactDamage.unused = 0;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief A70CC | Delete the entity when 0x53D bit 0 is set, else animate it.
 */
void Chaos2Effect_Update(struct EwramData_EntityData *entity)
{
    if (entity->animDone)
        EntityDelete(entity);
    else
        Anim_Update(entity);
}

/**
 * @brief A70F0 | Move by the velocity; once outside the screen (margin set 2)
 * release the affine slot in 0x53C bits 0-4 and delete the entity.
 */
void Chaos2Shard_Update(struct EwramData_EntityData *entity)
{
    entity->posX.whole += entity->velX.whole;
    entity->posY.whole += entity->velY.whole;
    if (EntityIsOutsideScreen(entity, 2))
    {
        ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
    }
}

/* Chaos second form: load graphics, palettes and animation data. */

u8 EnemyChaos2Create(struct EwramData_EntityData *entity)
{
    struct ChaosData *data = (struct ChaosData *)entity->ext.chaos.data.ptr;
    s32 graphics = ObjGfx_LoadResource(sChaos2Gfx, 0);
    if (graphics < 0)
        return 0;
    {
        entity->basePalette = ObjPalette_FindOrLoadVariant(sChaosPalette, 0);
        data->graphics = graphics;
        data->palette = ObjPalette_FindOrLoadVariant(sChaosPalette, 2);
        data->palette2 = ObjPalette_FindOrLoadVariant(sChaosPalette, 3);
        Chaos_StartPaletteSequence(entity, (s32)sChaos2PaletteSequence0);
        Sprite_Attach(entity, sChaos2SpriteSheet, graphics, entity->basePalette);
        entity->drawFunc = Sprite_DrawEntity;
        return 1;
    }
}

void Chaos2_UpdateIdle(struct EwramData_EntityData *entity)
{
}
