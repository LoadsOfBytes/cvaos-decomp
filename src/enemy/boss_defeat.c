/*
 * Boss defeat effects (generic, Headhunter, Chaos) and the boss recovery pickup.
 */
#include "boss_defeat.h"
#include "core.h"
#include "graphics.h"
#include "structs/ewram.h"
#include "bg_and_collision.h"
#include "gba/memory.h"
#include "sound.h"
#include "gba.h"
#include "in_game.h"
#include "music.h"
#include "player_level.h"
#include "player_core.h"
#include "gameplay.h"
#include "structs/main.h"
#include "structs/entity_render_flags.h"
#include "effects.h"
#include "entity_common.h"
#include "math_anim.h"
#include "oam.h"
#include "obj_gfx.h"
#include "palette.h"
#include "data/data_118B48.h"
#include "data/data_4F1444.h"
#include "structs/entity_template.h"
#include "entity_inline.h"
#include "hitbox.h"
#include "soul_pickup.h"
#include "entity_hit.h"

void BossRecoveryPickup_Draw(struct EwramData_EntityData *);

/* BossDefeatParticle_Update (effect). */

/**
 * @brief Attach shared effect graphics with palette 8 and layer 0x10,
 * then accelerate and move until animation completion. Used by
 * multiple defeat controllers; preserve initialization fallthrough.
 */
void BossDefeatParticle_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
        case 0:
        {
            s32 gfx = ObjGfx_Find(sCommonObjectGfx);

            Sprite_Attach(entity, sDefeatParticleSpriteSheet, gfx, 8);
            Anim_StartIndexed(entity, 0, 0, 0);
            entity->drawFunc = Sprite_DrawEntity;
            entity->spriteLayer = 0x10;
            entity->objMode = 1;
            entity->state++;
        }
        case 1:
            EntityAccelerateAndMove(entity, 0x80000);
            if (Anim_Update(entity) == 3)
                EntityDelete(entity);
            break;
    }
}

/* Wider sweep variant restores the shared object graphics when it ends. */
static inline void RestoreEffectGraphics(void)
{
    if(gEwramData->gameState.stateFlags & 8)
    {
        u8 *graphics=sCommonObjectGfxTiles;
        s32 offset=ObjTiles_GetSlotTile((u8)ObjGfx_Find(graphics-4));
        u32 *second;
        offset<<=5;
        DmaQueue_IndirectCopy(0x200,(u32 *)graphics,(u32 *)((u8 *)VRAM_OBJ+offset));
        graphics+=0x200;
        second=(u32 *)((u8 *)VRAM_OBJ+0x400+offset);
        DmaQueue_IndirectCopy(0x200,(u32 *)graphics,second);
        gEwramData->gameState.stateFlags&=~8;
    }
}
/**
 * @brief Run the delayed particle sweep and palette cleanup, then
 * restore the common effect tiles and remove. Preserve the shared
 * labels, fixed-point sweep calculation and graphics ownership flag.
 */
void BossDefeatEffect_Update(struct EwramData_EntityData *entity)
{
    s32 timer;
    switch(entity->state)
    {
        case 0:
            entity->timer=8;
            entity->accelX.whole=0x24000;
            entity->state++;
        case 1:
            timer=entity->timer;
            if(timer!=0)
                goto tick;
            goto advance;
        case 2:
            if(entity->timer!=0)
            {
                if((entity->timer & 3)==0)
                {
                    struct EwramData_EntityData *particle=EntityCreateInRange(0x49,0xCF,BossDefeatParticle_Update);
                    if(particle)
                    {
                        s32 random,width,centerX;
                        s32 jitterY,positionY,sweepY;
                        particle->scrollWithBg=1;
                        random=RandomSigned(4);
                        width=entity->accelX.whole;
                        centerX=entity->posX.whole;
                        
                        particle->posX.whole=centerX+(SweepWave(entity)<0 ? random-((-SweepWave(entity))>>16) : (SweepWave(entity)>>16)+random)*width;
                        jitterY=(u32)RandomSigned(16)<<16;
                        jitterY+=0x180000;
                        positionY=entity->posY.whole+jitterY;
                        sweepY=entity->timer<<15;
                        sweepY-=0xC0000;
                        particle->posY.whole=positionY+sweepY;
                        { s32 velocity=0x4000; particle->velY.whole=RandomSigned(velocity)+velocity; }
                        particle->accelY.whole-=0x2000;
                        if((entity->timer & 7)==0)
                            PlaySong(0x77);
                    }
                }
                entity->accelX.whole-=0x400;
                timer=entity->timer;
                entity->timer=timer-1;
                break;
            }
            else
            {
                if(!(gEwramData->gameState.stateFlags & 0x10))
                {
                    BgPalette_StopColorFade();
                    ObjPalette_StopColorFade();
                }
advance:
                entity->timer=64;
                entity->state++;
            }
            break;
        case 3:
            timer=entity->timer;
            if(timer==0) goto finished;
tick:
            entity->timer=timer-1;
            break;
finished:
            RestoreEffectGraphics();
            EntityDelete(entity);
            return;
    }
    if(!entity->roomActive)
        RestoreEffectGraphics();
}

/* BossDefeatEffect_Spawn. */
/* Same as BossDefeat_LoadEffectTiles. */
static inline void BossDefeat_LoadEffectTilesOnce(void)
{
    if (!(gEwramData->gameState.stateFlags & 8))
    {
        u8 *src = sDefeatEffectTiles;
        s32 tileOffset = ObjTiles_GetSlotTile((u8)ObjGfx_Find(sCommonObjectGfx)) * 32;

        DmaQueue_IndirectCopy(0x200, (u32 *)src, (u32 *)(VRAM_BASE + 0x10000 + tileOffset));
        src += 0x200;
        DmaQueue_IndirectCopy(0x200, (u32 *)src, (u32 *)(VRAM_BASE + 0x10400 + tileOffset));
        gEwramData->gameState.stateFlags |= 8;
    }
}

/**
 * @brief Create the shared boss-defeat sweep at the supplied fixed-point
 * position, start white palette fades unless flag 0x10 blocks them,
 * and install its temporary effect tiles through the inline copy.
 */
void BossDefeatEffect_Spawn(s32 x, s32 y)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, BossDefeatEffect_Update);

    if (child == NULL)
        return;
    if (!(gEwramData->gameState.stateFlags & 0x10))
    {
        BgPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0, 0x10000, 0x20);
        ObjPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0x4000, 0x10000, 0x10);
    }
    BossDefeat_LoadEffectTilesOnce();
    child->posX.whole = x;
    child->posY.whole = y;
    child->scrollWithBg = 1;
    child->roomActive = 1;
}

/* A camera-relative rain of particles restores shared object graphics at exit. */
/**
 * @brief Run the Headhunter defeat particle sequence across room-space
 * X 112..384 and Y 128..184, then stop permitted fades and restore
 * the common effect tiles. Retain countdown and ownership behavior.
 */
void HeadhunterDefeatEffect_Update(struct EwramData_EntityData *entity)
{
    struct BgLayerState *camera=&gEwramData->bgInfo[1];
    s32 timer;
    switch(entity->state)
    {
        case 0:
            entity->timer=8;
            entity->accelX.whole=0x20000;
            entity->state++;
        case 1:
            timer=entity->timer;
            if(timer!=0)
            {
                entity->timer=timer-1;
                break;
            }
            timer=96;
            goto advance;
        case 2:
            if(entity->timer!=0)
            {
                if((entity->timer & 3)==0)
                {
                    struct EwramData_EntityData *particle=EntityCreateInRange(0x49,0xCF,BossDefeatParticle_Update);
                    if(particle)
                    {
                        u32 random;
                        s32 x,y;
                        particle->scrollWithBg=1;
                        random=RandomNumberGenerator()%18;
                        x=(random<<4)+112;
                        x-=camera->xPos.part16.integer;
                        particle->posX.part16.integer=x;
                        random=RandomNumberGenerator();
                        y=((random&7)<<3)+128;
                        y-=camera->yPos.part16.integer;
                        particle->posY.part16.integer=y;
                        particle->velY.whole=0x4000;
                        particle->accelY.whole-=0x2000;
                    }
                }
                entity->accelX.whole-=0x600;
                timer=entity->timer;
                entity->timer=timer-1;
                break;
            }
            else
            {
                if(!(gEwramData->gameState.stateFlags & 0x10))
                {
                    BgPalette_StopColorFade();
                    ObjPalette_StopColorFade();
                }
                timer=64;
advance:
                entity->timer=timer;
                entity->state++;
            }
            break;
        case 3:
            timer=entity->timer;
            if(timer!=0)
            {
                entity->timer=timer-1;
                break;
            }
            RestoreEffectGraphics();
            EntityDelete(entity);
            return;
    }
    if(!entity->roomActive)
        RestoreEffectGraphics();
}

/* ChaosDefeatParticle_Update: variant of an already decompiled function, generated with
 * a cloning script (constants adjusted where needed), checked byte for
 * byte. */

/**
 * @brief Attach shared effect graphics with palette 8 and layer 0x19,
 * then accelerate and move until animation completion. Created by
 * the Chaos defeat sweep.
 */
void ChaosDefeatParticle_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
        case 0:
        {
            s32 gfx = ObjGfx_Find(sCommonObjectGfx);

            Sprite_Attach(entity, sDefeatParticleSpriteSheet, gfx, 8);
            Anim_StartIndexed(entity, 0, 0, 0);
            entity->drawFunc = Sprite_DrawEntity;
            entity->spriteLayer = 0x19;
            entity->objMode = 1;
            entity->state++;
        }
        case 1:
            EntityAccelerateAndMove(entity, 0x80000);
            if (Anim_Update(entity) == 3)
                EntityDelete(entity);
            break;
    }
}

/* Alternate falling particles use the original sweep and restore object graphics. */
/**
 * @brief Run the Chaos defeat particle sweep with its original vertical
 * offsets and layer-0x19 particles, then clean up fades and temporary
 * tiles. Preserve the shared countdown labels and fixed-point form.
 */
void ChaosDefeatEffect_Update(struct EwramData_EntityData *entity)
{
    struct EwramData *ewram=gEwramData;
    s32 timer;
    switch(entity->state)
    {
        case 0:
            entity->timer=8;
            entity->accelX.whole=0x20000;
            entity->state++;
        case 1:
            timer=entity->timer;
            if(timer!=0)
                goto tick;
            goto advance;
        case 2:
            if(entity->timer!=0)
            {
                if((entity->timer & 3)==0)
                {
                    struct EwramData_EntityData *particle=EntityCreateInRange(0x49,0xCF,ChaosDefeatParticle_Update);
                    if(particle)
                    {
                        s32 random,width,centerX;
                        s32 jitterY,positionY,sweepY;
                        particle->scrollWithBg=1;
                        random=RandomSigned(4);
                        width=entity->accelX.whole;
                        centerX=entity->posX.whole;
                        
                        particle->posX.whole=centerX+(SweepWave(entity)<0 ? random-((-SweepWave(entity))>>16) : (SweepWave(entity)>>16)+random)*width;
                        jitterY=(u32)RandomSigned(16)<<16;
                        jitterY+=0x180000;
                        positionY=entity->posY.whole+jitterY;
                        sweepY=entity->timer<<14;
                        sweepY-=0x80000;
                        particle->posY.whole=positionY+sweepY;
                        particle->velY.whole=0x4000;
                        particle->accelY.whole-=0x2000;
                        if((entity->timer & 7)==0)
                            PlaySong(0x77);
                    }
                }
                entity->accelX.whole-=0x600;
                timer=entity->timer;
                entity->timer=timer-1;
                break;
            }
            else
            {
                if(!(ewram->gameState.stateFlags & 0x10))
                {
                    BgPalette_StopColorFade();
                    ObjPalette_StopColorFade();
                }
advance:
                entity->timer=64;
                entity->state++;
            }
            break;
        case 3:
            timer=entity->timer;
            if(timer==0) goto finished;
tick:
            entity->timer=timer-1;
            break;
finished:
            RestoreEffectGraphics();
            EntityDelete(entity);
            return;
    }
    if(!entity->roomActive)
        RestoreEffectGraphics();
}

/* An animated particle spirals inward as its angular speed increases. */
#define ANGLE(e) (e->ext.bossPickupParticle.angle)
#define STEP(e) (e->ext.bossPickupParticle.step)
#define RADIUS(e) (e->ext.bossPickupParticle.radius)
#define SPEED(e) (e->ext.bossPickupParticle.speed)
/**
 * @brief Animate a particle around a stored room-space center with
 * accelerating angular motion and shrinking radius. Remove when
 * the radius becomes nonpositive; preserve shifted multiplication.
 */
void BossRecoveryPickupParticle_Update(struct EwramData_EntityData *entity)
{
    struct BgLayerState *camera=&gEwramData->bgInfo[1];
    switch(entity->state)
    {
        case 0:
        {
            s32 graphics=ObjGfx_Find(sCommonObjectGfx);
            Sprite_Attach(entity,sCommonObjectSpriteSheet,graphics,7);
            Anim_Start(entity,sRecoveryPickupParticleAnimScript,0,0);
            entity->drawFunc=Sprite_DrawEntity;
            entity->spriteLayer=10;
            entity->scrollWithBg=1;
            entity->state++;
        }
        case 1:
        {
            s32 sine,offset,radius;
            RADIUS(entity)+=SPEED(entity);
            SPEED(entity)-=0xC00;
            if(RADIUS(entity)<=0)
            {
                EntityDelete(entity);
                return;
            }
            sine=Sin((s16)ANGLE(entity)+0x4000);
            radius=RADIUS(entity);
            offset=(radius>>8)*(sine>>8);
            entity->posX.whole=entity->accelX.whole-camera->xPos.whole+offset;
            sine=Sin((s16)ANGLE(entity));
            radius=RADIUS(entity);
            offset=(radius>>8)*(sine>>8);
            entity->posY.whole=entity->accelY.whole-camera->yPos.whole+offset;
            ANGLE(entity)+=STEP(entity);
            if(!entity->flipX)
            {
                STEP(entity)+=0x40;
                if((s16)STEP(entity)>0x1800)
                    STEP(entity)=0x1800;
            }
            else
            {
                STEP(entity)-=0x40;
                if((s16)STEP(entity)<-0x1800)
                    STEP(entity)=-0x1800;
            }
            break;
        }
    }
    Anim_Update(entity);
}

/* Seven scene stages: appear, spiral particles, fall, trigger and wait. */
#define PHASE(e) (e->ext.bossPickup.phase)
#define LANDED(e) (e->ext.bossPickup.landed)
#define SCALE(e) (*(s32 *)&e->ext.bossPickup.scale)
#define EVENT(e) ((e)->spawnParam0)
/**
 * @brief Reveal and grow the delayed pickup, emit particles, fall and
 * wait for player contact. Restore HP/MP and optional Julius stat
 * growth, wait for displayed values, then free the affine slot.
 */
void BossRecoveryPickup_Update(struct EwramData_EntityData *entity)
{
    struct BgLayerState *camera=&gEwramData->bgInfo[1];
    struct HitResult result;
    switch(entity->state)
    {
        case 0:
            if(entity->timer)
            {
                entity->timer--;
                break;
            }
            entity->state=1;
        case 1:
            Sound_PlayTracked(0x14C);
            entity->hidden=0;
            entity->state=2;
        case 2:
            entity->timer++;
            if(!(entity->timer & 1) && SCALE(entity)<=0x7FFF)
            {
                u16 angle=RandomNumberGenerator();
                struct EwramData_EntityData *particle=EntityCreateInRange(0x49,0xCF,BossRecoveryPickupParticle_Update);
                if(particle)
                {
                    particle->accelX.whole=entity->posX.whole+camera->xPos.whole;
                    particle->accelY.whole=entity->posY.whole+camera->yPos.whole;
                    particle->flipX=RandomNumberGenerator() & 1;
                    particle->ext.bossPickupParticle.angle=angle;
                    particle->ext.bossPickupParticle.radius=RandomSigned(0x80000)+0x380000;
                }
            }
            if(SCALE(entity)==0xF000)
            {
                Sound_StopTracked(0x14C);
                PlaySong(0x14B);
            }
            if(SCALE(entity)<=0xFFFF)
                SCALE(entity)+=0x200;
            else
            {
                entity->hit.flags&=~2;
                entity->state=3;
            }
            break;
        case 3:
            entity->timer++;
            if(!(entity->timer & 1))
            {
                s32 x=entity->posX.whole+(u32)RandomSigned(16)*0x10000;
                s32 y=entity->posY.whole+(u32)RandomSigned(16)*0x10000;
                s32 angle=RandomSigned(0x1000);
                struct EwramData_EntityData *particle=Effect_CreateFromTable(x,y,angle,0,4,9);
                if(particle)
                {
                    particle->velY.whole=RandomSigned(0x4000)-0x10000;
                    particle->accelY.whole=-particle->velY.whole/32;
                    particle->spriteLayer=10;
                }
            }
            if(Sin((s16)PHASE(entity))<0)
                SCALE(entity)=-Sin((s16)PHASE(entity))/3+0x10000;
            else
                SCALE(entity)=Sin((s16)PHASE(entity))/3+0x10000;
            EntityAccelerateAndMoveY(entity,0x80000);
            if(!LANDED(entity))
            {
                s32 x=(s16)entity->posX.part16.integer;
                struct BgLayerState *bg=&gEwramData->bgInfo[1];
                s32 y;
                x+=(s16)bg->xPos.part16.integer;
                y=(s16)entity->posY.part16.integer+(s16)bg->yPos.part16.integer;
                if(Collision_GetPushUp(x,y+57) && (gUnk_03002CB0.collisionProbeFlags & 2))
                {
                    entity->accelY.whole=0;
                    LANDED(entity)=1;
                }
            }
            else
            {
                EntityDampVelocityY(entity,6);
                if(entity->velY.whole<=0xFFF)
                    entity->velY.whole=0;
                if(!entity->velY.whole)
                {
                    if(!(PHASE(entity) & 0x7FFF))
                        PlaySong(0x14A);
                    PHASE(entity)+=0x200;
                    entity->posY.whole+=Sin((s16)PHASE(entity)/4)/4;
                }
            }
            { s32 collision=Hit_CheckWithPlayer(&result,entity);
            collision=(u32)collision<<16;
            collision=(u32)collision>>16;
            collision=(u8)(collision & 1); if(collision)
            {
                entity->state=4;
                PlaySong(0xF8);
                entity->hidden=1;
            }
            }
            break;
        case 4:
        {
            s32 zero;
            if(EVENT(entity))
                Player_ApplyJuliusStatGrowth();
            Player_RestoreHPAndMP();
            zero=0;
            entity->stage=64;
            entity->subState=1;
            Game_SetMenusEnabled(0);
            gEwramData->paused=1;
            entity->timer=zero;
            entity->state=5;
            if(EVENT(entity))
            {
                gEwramData->gameState.bossEventFlags|=1<<EVENT(entity);
                gEwramData->gameState.stateFlags&=~2;
            }
            break;
        }
        case 5:
            entity->subState=0;
            if(gEwramData->gameState.hudHP<gEwramData->playerStatus.currentHP ||
               gEwramData->gameState.hudMP<gEwramData->playerStatus.currentMP)
            {
                if(!(entity->timer & 7))
                    PlaySong(0x140);
                entity->timer++;
            }
            else
                entity->state=6;
            break;
        case 6:
            if(ENTITY_RENDER_FLAGS16(entity) & 0x101F)
                ObjAffine_Free(entity->affineIndex);
            EntityDelete(entity);
            return;
    }
    ObjAffine_Set(0,SCALE(entity),SCALE(entity),entity->flipX,0,entity->affineIndex);
}

/* Create a scene object and update the corresponding persistent event bit. */
/**
 * @brief Update the existing boss-event/music flags and create a hidden
 * delayed recovery pickup with affine scaling and a custom draw
 * callback. Retain mode-specific event storage and failure paths.
 */
void BossRecoveryPickup_Spawn(u16 x,u16 y,s32 event,s32 delay,s32 special)
{
    struct EwramData_EntityData *entity;
    if(gEwramData->playerStatus.currentCharacter)
        Music_SubmitCommandOnce(0x1040);
    if(gEwramData->gameState.bossRushMode)
        gEwramData->gameState.bossEventFlags|=1<<event;
    else if(!special && !gEwramData->playerStatus.currentCharacter)
    {
        gEwramData->gameState.bossEventFlags|=1<<event;
        gEwramData->gameState.stateFlags&=~2;
    }
    entity=EntityCreateInRange(0x49,0xCF,BossRecoveryPickup_Update);
    if(entity)
    {
        u16 sprite=ObjAffine_Alloc(1);
        if((s16)sprite<0)
        {
            EntityDelete(entity);
            return;
        }
        entity->affineIndex=sprite;
        entity->affine=1;
        entity->doubleSize=1;
        {
            s32 graphics=ObjGfx_Find(sCommonObjectGfx);
            Sprite_Attach(entity,sCommonObjectSpriteSheet,graphics,7);
        }
        entity->drawFunc=BossRecoveryPickup_Draw;
        entity->animFrame=0x7C;
        entity->spriteLayer=10;
        entity->scrollWithBg=1;
        entity->posX.part16.integer=x;
        entity->posY.part16.integer=y;
        entity->accelY.whole=0x3000;
        entity->timer=delay;
        Hit_Enable(entity,11,0,0);
        HitBox_SetAttackRaw(entity,0x1010F8F8);
        entity->hit.flags|=2;
        entity->hit.flags|=4;
        entity->hidden=1;
        if(!gEwramData->gameState.bossRushMode && gEwramData->playerStatus.currentCharacter)
            entity->spawnParam0=event;
    }
}

/* BossDefeat_LoadEffectTiles / BossDefeat_RestoreEffectTiles (swap of the common effect tiles). */
/* Common effect graphics; the tiles start 4 bytes in. */
/* Two 0x200-byte tile blocks that temporarily replace them. */

/**
 * @brief If global flag bit 3 is clear, queue two 0x200-byte blocks
 * of temporary defeat tiles over the common effect OBJ graphics
 * and set the bit. Preserve exact VRAM destinations and copy order.
 */
void BossDefeat_LoadEffectTiles(void)
{
    if (!(gEwramData->gameState.stateFlags & 8))
    {
        u8 *src = sDefeatEffectTiles;
        s32 tileOffset = ObjTiles_GetSlotTile((u8)ObjGfx_Find(sCommonObjectGfx)) * 32;

        DmaQueue_IndirectCopy(0x200, (u32 *)src, (u32 *)(VRAM_BASE + 0x10000 + tileOffset));
        src += 0x200;
        DmaQueue_IndirectCopy(0x200, (u32 *)src, (u32 *)(VRAM_BASE + 0x10400 + tileOffset));
        gEwramData->gameState.stateFlags |= 8;
    }
}

/**
 * @brief If global flag bit 3 is set, queue the original common effect
 * tiles back into the two OBJ VRAM blocks and clear the bit.
 */
void BossDefeat_RestoreEffectTiles(void)
{
    if (gEwramData->gameState.stateFlags & 8)
    {
        u8 *src = &sCommonObjectGfx[4];
        s32 tileOffset = ObjTiles_GetSlotTile((u8)ObjGfx_Find(src - 4)) * 32;

        DmaQueue_IndirectCopy(0x200, (u32 *)src, (u32 *)(VRAM_BASE + 0x10000 + tileOffset));
        src += 0x200;
        DmaQueue_IndirectCopy(0x200, (u32 *)src, (u32 *)(VRAM_BASE + 0x10400 + tileOffset));
        gEwramData->gameState.stateFlags &= ~8;
    }
}

/* Headhunter_SpawnDefeatEffect. */
/* Two 0x200-byte tile blocks copied into the slot of sCommonObjectGfx. */

/* Same as BossDefeat_LoadEffectTiles. */

/**
 * @brief Create the room-wide Headhunter defeat controller at the
 * supplied fixed-point position and install temporary tiles through
 * the existing inline copy. Return after allocation failure.
 */
void Headhunter_SpawnDefeatEffect(s32 x, s32 y)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, HeadhunterDefeatEffect_Update);

    if (child == NULL)
        return;
    BossDefeat_LoadEffectTilesOnce();
    child->posX.whole = x;
    child->posY.whole = y;
    child->scrollWithBg = 1;
    child->roomActive = 1;
}

/* Chaos_SpawnDefeatEffect. */
/* Two 0x200-byte tile blocks copied into the slot of sCommonObjectGfx. */

/* Same as BossDefeat_LoadEffectTiles. */

/**
 * @brief Create the Chaos defeat controller at the supplied fixed-point
 * position and install temporary tiles through the existing inline
 * copy. Called by the core defeat scene.
 */
void Chaos_SpawnDefeatEffect(s32 x, s32 y)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, ChaosDefeatEffect_Update);

    if (child == NULL)
        return;
    BossDefeat_LoadEffectTilesOnce();
    child->posX.whole = x;
    child->posY.whole = y;
    child->scrollWithBg = 1;
    child->roomActive = 1;
}

/* SoulPickup_SpawnFromEnemyDefinition / BossRecoveryPickup_Draw. */

/**
 * @brief For Soma under the existing mode guard, create the enemy-definition soul pickup and store its persistence flag and substate two.
 */
void SoulPickup_SpawnFromEnemyDefinition(s32 x_, s32 y_, s32 enemyId, s32 spawnParam)
{
    u16 x = x_;
    u16 y = y_;
    struct EntityTemplate *enemyDef = &sEnemyData[enemyId];
    u8 palette = enemyDef->soulType;
    u8 soulId = enemyDef->soulIndex;

    if (gEwramData->gameState.bossRushMode == 0 && gEwramData->playerStatus.currentCharacter == 0)
    {
        struct EwramData_EntityData *pickup = SoulPickup_Create((s16)x, (s16)y, palette, soulId - 1, 0);

        if (pickup != NULL)
        {
            pickup->spawnParam0 = spawnParam;
            pickup->subState = 2;
        }
    }
}

/**
 * @brief Draw the recovery pickup. While its substate is nonzero,
 * count stage down and then enable menus and clear pause. Preserve
 * this draw-time release relative to HP/MP display recovery.
 */
void BossRecoveryPickup_Draw(struct EwramData_EntityData *entity)
{
    Sprite_Draw(entity);
    if (entity->subState != 0)
    {
        if (entity->stage != 0)
            entity->stage--;
        else
        {
            Game_SetMenusEnabled(1);
            gEwramData->paused = 0;
        }
    }
}
