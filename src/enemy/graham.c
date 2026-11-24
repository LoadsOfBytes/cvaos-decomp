/*
 * Graham (boss): scene and defeat scene, combat body, overlays, side parts, attackers, first form, spread / shot / spiral projectiles and breakup.
 */
#include "core.h"
#include "graham_entity.h"
#include "in_game.h"
#include "music.h"
#include "player_level.h"
#include "structs/ewram.h"
#include "graphics.h"
#include "sound.h"
#include "graham_projectile.h"
#include "gameplay.h"
#include "map_screen.h"
#include "map_flags.h"
#include "gba/dma.h"
#include "structs/main.h"
#include "structs/entity_render_flags.h"
#include <string.h>
#include "player_core.h"
#include "boss_defeat.h"
#include "effects.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "math_anim.h"
#include "oam.h"
#include "obj_gfx.h"
#include "obj_palette.h"
#include "palette.h"
#include "shop_update.h"
#include "data/data_118B48.h"
#include "data/data_4F1444.h"
#include "entity_inline.h"
#include "hitbox.h"
#include "damage_numbers.h"
#include "red_souls_b.h"

extern s32 Entity_AngleToPoint(struct EwramData_EntityData *,s32,s32);
extern s32 Player_GetZoneAroundX(s32,s32,s32);
extern u8 sEndingRoomDefinition[];
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, u8 *, void *, u8, void *);
extern void Entity_FaceTarget(struct EwramData_EntityData *);
extern void *memcpy(void *, const void *, unsigned long);
extern s32 Hit_ResolveDamage(struct EwramData_EntityData *entity, s32 other, u16 *hit, s32 inputDamage);

s32 Graham_CreateSceneObject(struct EwramData_EntityData *,s32);
struct EwramData_EntityData *Graham_CreateCombatBody(struct EwramData_EntityData *owner);
struct EwramData_EntityData *GrahamFirstForm_Create(struct EwramData_EntityData *owner);
void GrahamSceneProjectile_Update(struct EwramData_EntityData *);
void GrahamSceneProjectile_OnContact(struct EwramData_EntityData *entity);
void Graham_SpawnBreakupEffect(s32,s32);
void GrahamCombatBody_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
void GrahamCombatBody_Update(struct EwramData_EntityData *);
struct EwramData_EntityData *Graham_CreateBodyOverlay(struct EwramData_EntityData *);
s32 GrahamPart_UpdatePaletteSequence(struct EwramData_EntityData *,s32);
void GrahamBodyOverlay_Update(struct EwramData_EntityData *);
struct EwramData_EntityData *Graham_CreateSidePart(struct EwramData_EntityData *, s32);
struct EwramData_EntityData *Graham_CreateAttacker(struct EwramData_EntityData *);
void GrahamSidePart_Update(struct EwramData_EntityData *);
void GrahamAttacker_OnHit(struct EwramData_EntityData *, s32, u16 *);
void GrahamAttacker_Update(struct EwramData_EntityData *);
void GrahamAttacker_UpdateDeath(struct EwramData_EntityData *);
struct EwramData_EntityData *GrahamAttacker_CreateAttackEffect(struct EwramData_EntityData *);
void GrahamDeathSpark_Update(struct EwramData_EntityData *);
void GrahamAttackEffect_Update(struct EwramData_EntityData *);
void GrahamFirstForm_OnHit(struct EwramData_EntityData *, s32, u16 *);
void GrahamFirstForm_Update(struct EwramData_EntityData *);
void GrahamFirstForm_UpdateTransition(struct EwramData_EntityData *);
void GrahamFirstForm_UpdateAttacks(struct EwramData_EntityData *);
void GrahamFirstForm_UpdateAlternateAttack(struct EwramData_EntityData *);
s32 Graham_SpawnSpreadProjectiles(struct EwramData_EntityData *entity);
s32 Graham_CreateShotPair(struct EwramData_EntityData *owner, s32 low);
s32 Graham_SpawnSpiralProjectiles(struct EwramData_EntityData *owner, s32 reverse);
void GrahamTransitionPart_Update(struct EwramData_EntityData *);
s32 GrahamPart_TakeDamage(struct EwramData_EntityData *, s32, u16 *);
void GrahamSpreadProjectile_Update(struct EwramData_EntityData *);
void GrahamSpreadProjectile_DamagePlayerOnContact(struct EwramData_EntityData *entity);
void GrahamSpreadProjectile_OnHit(struct EwramData_EntityData *entity);
void GrahamShotPart_Update(struct EwramData_EntityData *);
void GrahamSpiralProjectile_Update(struct EwramData_EntityData *);
void Graham_UpdateState2(struct EwramData_EntityData *);
void Graham_UpdateBestiaryPreview(struct EwramData_EntityData *);
void Graham_UpdateState2(struct EwramData_EntityData *entity);
void Graham_UpdateBestiaryPreview(struct EwramData_EntityData *entity);
s32 GrahamPart_TakeDamage(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);

/* Graham encounter setup, including character/event dependent scene paths. */
void EnemyGrahamCreate(struct EwramData_EntityData *entity)
{
    if(gEwramData->gameState.stateFlags & 0x10000)
    {
        entity->deleteRequested=1;
        return;
    }
    {
        struct BgLayerState *camera=&gEwramData->bgInfo[1];
        entity->posX.whole=0x780000-camera->xPos.whole;
        entity->posY.whole=0x8F0000-camera->yPos.whole;
    }
    if(entity->spawnParam0==1)
    {
        if(!gEwramData->gameState.bestiaryPreview && gEwramData->playerStatus.currentCharacter!=1)
        {
            s32 zero;
            entity->ext.graham.flags|=0x80;
            zero=0;
            if(gEwramData->gameState.bossEventFlags & 1)
            {
                Graham_CreateSceneObject(entity,5);
                entity->state=2;
                entity->stage=zero;
                entity->subState=zero;
                return;
            }
            Game_SetMenusEnabled(0);
            entity->state=1;
            entity->stage=zero;
            entity->subState=zero;
            return;
        }
    }
    if(!gEwramData->gameState.bestiaryPreview)
    {
        Game_SetMenusEnabled(1);
        Player_SetSelectedTarget(entity);
        gEwramData->gameState.stateFlags|=2;
    }
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer=20;
    if(gEwramData->gameState.bestiaryPreview)
    {
        s32 zero;
        entity->posX.part16.integer-=56;
        zero=0;
        entity->posY.part16.integer-=64;
        Graham_CreateCombatBody(entity);
        entity->state=0xD0;
        entity->stage=zero;
        entity->subState=zero;
        return;
    }
    if((gEwramData->gameState.bossEventFlags & 1) && gEwramData->gameState.bossRushMode==0)
    {
        EntityDelete(entity);
        return;
    }
    Music_SetCurrent(19);
    GrahamFirstForm_Create(entity);
    entity->state=3;
    entity->stage=0;
    entity->subState=0;
}

/* Create a Graham scene object and set its event bit. */
/**
 * @brief Create the indexed scene object 0x20 when owner/global flags
 * allow it, store slot links and set its event bit. Return 1 on success
 * or 0 when gated or allocation fails.
 */
s32 Graham_CreateSceneObject(struct EwramData_EntityData *entity,s32 index)
{
    struct GrahamSceneObject *info=&sGrahamSceneObjects[index];
    struct EwramData_EntityData *object;
    s32 facing;
    if(!(entity->ext.graham.flags & 0x80))
        return 0;
    if(gEwramData->gameState.stateFlags & 0x10)
        return 0;
    object=EntityCreateInRange(0x1B,0x48,EntityUpdateNothing);
    if(!object)
        return 0;
    entity->ext.grahamScene.objectIndex=ENTITY_INDEX(object);
    ENTITY_PARENT_INDEX(object)=ENTITY_INDEX(entity);
    if(index==0 || index==5)
    {
        s32 x=info->x<<16;
        struct BgLayerState *bg=&gEwramData->bgInfo[1];
        object->posX.whole=x-bg->xPos.whole;
        object->posY.whole=(info->y<<16)-bg->yPos.whole;
    }
    else
    {
        object->posX.whole=entity->posX.whole;
        object->posY.whole=entity->posY.whole;
    }
    facing=0;
    if(object->posX.whole<gEwramData->playerState.player->posX.whole)
        facing=1;
    object->flipX=facing;
    object->scrollWithBg=1;
    object->spawnParam0=info->param1;
    object->spawnParam1=info->param2;
    object->spawnIndex=index;
    object->enemyId=32;
    object->drawFunc=Sprite_DrawEntity;
    object->updateFunc=Object20Update;
    Object20Create(object);
    gEwramData->gameState.roomEntitySpawn.spawnedFlags[index>>5] |= 1 << (index & 31);
    return 1;
}

/* Graham scene setup, player-input pause and return after the scene finishes. */
/**
 * @brief Root intro state (1): gate menus, create the temporary scene
 * helper, pause player input and start scene entry 0, then reinitialize
 * the encounter on scene completion. Preserve allocation/pointer assumptions.
 */
void Graham_UpdateIntro(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *object=&gEwramData->entityData[(u8)entity->ext.grahamScene.objectIndex];
    struct EwramData_EntityData *helper=&gEwramData->entityData[entity->ext.grahamScene.helperIndex];
    Game_SetMenusEnabled(0);
    switch(entity->stage)
    {
        case 0:
        {
            struct BgLayerState *camera;
            helper=Entity_CreateFromObjectTable(4);
            entity->ext.grahamScene.helperIndex=ENTITY_INDEX(helper);
            helper->flipX=1;
            camera=&gEwramData->bgInfo[1];
            helper->posX.part16.integer=120-camera->xPos.part16.integer;
            helper->posY.part16.integer=191-camera->yPos.part16.integer;
            entity->timer=80;
            entity->stage++;
            break;
        }
        case 1:
            SetPlayerInput(2,0);
            if(--entity->timer==255)
                entity->stage++;
            break;
        case 2:
            if(!(gEwramData->gameState.stateFlags & 0x100))
            {
                entity->hidden=1;
                helper->deleteRequested=1;
                Graham_CreateSceneObject(entity,0);
                entity->stage++;
            }
            break;
        case 3:
            if(!object)
            {
                entity->spawnParam0=0;
                EnemyGrahamCreate(entity);
            }
            else if(object->state==128)
            {
                entity->spawnParam0=0;
                gEwramData->gameState.stateFlags&=~0x40000;
                EnemyGrahamCreate(entity);
            }
            break;
    }
}

/* Wait for the attached entity to signal the transition to the next phase. */
/**
 * @brief Root state (3): wait for flag 4, create the later combat body,
 * then watch its defeat flag. Drop root rewards and enter defeat
 * scene state 0xF0 when that linked body is defeated.
 */
void Graham_UpdatePhaseMonitor(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child=&gEwramData->entityData[entity->ext.grahamScene.childIndex];
    s32 state=entity->subState;
    switch(state)
    {
        case 0:
            if(entity->ext.graham.flags & 4)
            {
                PlaySong(0x1DD);
                Graham_CreateCombatBody(entity);
                entity->subState=1;
                entity->stage=state;
            }
            break;
        case 1:
            if(child->enemyId==entity->enemyId && child && (child->enemyFlags & 2))
            {
                Enemy_DropRewards(entity);
                entity->state=0xF0;
                entity->stage=0;
                entity->subState=0;
            }
            break;
    }
}

/* Graham projectile: sprite, collision, direction and optional homing. */
/**
 * @brief Create an animated scene projectile at pixel coordinates with
 * randomized initial angle, optional homing/contact callback and
 * mode-dependent lifetime. Return the entity or NULL.
 */
struct EwramData_EntityData *Graham_CreateSceneProjectile(s32 xInput,s32 yInput,s32 homing)
{
    u16 x=xInput;
    u16 y=yInput;
    struct EwramData_EntityData *effect=EntityCreateInRange(0x49,0xCF,GrahamSceneProjectile_Update);
    if(effect)
    {
        s32 zero;
        s32 graphics;
        s32 targetX,targetY;
        u8 *kind;
        u8 *layer=&effect->spriteLayer;
        zero=0;
        *layer=zero+3;
        graphics=ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(effect,sCommonObjectSpriteSheet,graphics,5);
        Anim_StartIndexed(effect,6,zero,1);
        Anim_Update(effect);
        effect->stage=1;
        effect->drawFunc=Sprite_DrawEntity;
        if(homing)
        {
            Hit_Enable(effect,11,GrahamSceneProjectile_OnContact,0);
            effect->hit.flags|=4;
            HitBox_SetAttackRaw(effect,0x0C0CFAFA);
        }
        else
        {
            effect->hit.flags|=2;
            effect->hit.flags|=4;
        }
        {
        s32 turn;
        u16 *positionX=&effect->posX.part16.integer;
        turn=0;
        *positionX=x;
        effect->posY.part16.integer=y;
        effect->scrollWithBg=1;
        effect->group=64;
        kind=&effect->ext.grahamShot.homing;
        *kind=homing;
        targetX=gEwramData->playerState.player->posX.whole;
        targetY=gEwramData->playerState.player->posY.whole;
        targetY-=((RandomNumberGenerator()&0x7F)<<12);
        targetY-=0x80000;
        effect->ext.grahamShot.angle=Entity_AngleToPoint(effect,targetX,targetY);
        effect->ext.grahamShot.angle=(effect->ext.grahamShot.angle+(RandomNumberGenerator()&0xFFFF))&0xFFFF;
        effect->ext.grahamShot.turn=turn;
        effect->ext.grahamShot.speed=0x30000;
        if(*kind)
            effect->timer=200;
        else
            effect->timer=16;
        }
    }
    return effect;
}

/* Homing turn rate and speed ramp, followed by a timed projectile lifetime. */
/**
 * @brief Ramp turn rate and speed, optionally steer toward the player,
 * move and check contact. Request deletion after the configured
 * lifetime and final eight-update countdown.
 */
void GrahamSceneProjectile_Update(struct EwramData_EntityData *entity)
{
    s32 bounds[4];
    if(entity->state<=1)
    {
        if(entity->ext.grahamShot.homing)
        {
            struct EwramData_EntityData *player=gEwramData->playerState.player;
            entity->ext.grahamShot.angle=Entity_StepAngleTowardPoint(entity,player->posX.whole,player->posY.whole-0x100000,
                                                entity->ext.grahamShot.angle,entity->ext.grahamShot.turn);
        }
        entity->ext.grahamShot.turn+=42;
        if(entity->ext.grahamShot.turn>0x4000)
            entity->ext.grahamShot.turn=0x4000;
        entity->ext.grahamShot.speed+=0x1800;
        if(entity->ext.grahamShot.speed>0x80000)
            entity->ext.grahamShot.speed=0x80000;
        { s32 sine=Sin(entity->ext.grahamShot.angle+0x4000);
            s32 speed=entity->ext.grahamShot.speed;
            entity->velX.whole=(sine>>8)*(speed>>8); }
        { s32 sine=Sin(entity->ext.grahamShot.angle);
            s32 speed=entity->ext.grahamShot.speed;
            entity->velY.whole=(sine>>8)*(speed>>8); }
        entity->posX.whole+=entity->velX.whole;
        entity->posY.whole+=entity->velY.whole;
    }
    Hit_CheckWithPlayer((struct HitResult *)bounds,entity);
    switch(entity->state)
    {
        case 0:
            if(--entity->timer==0)
            {
                entity->timer=8;
                entity->state++;
            }
            break;
        case 1:
            if(--entity->timer==0)
                entity->deleteRequested=1;
            break;
        default:
            entity->timer++;
            break;
    }
    if(entity->stage)
        Anim_Update(entity);
}

/* Animated effect follows an owner with fixed X/Y offsets. */
/**
 * @brief Animate an effect following its owner pointer at fixed offsets.
 * Request deletion when the owner pointer is NULL or animation completes.
 */
void GrahamPlayerEffect_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner=entity->ext.particleAttach.parent;
    if(!owner) { entity->deleteRequested=1; return; }
    {
        switch(entity->state)
        {
            case 0:
            {
                s32 graphics;
                if((RandomNumberGenerator()&3)==0)
                    PlaySong(0x16D);
                graphics=ObjGfx_Find(sCommonObjectGfx);
                Sprite_Attach(entity,sCommonObjectSpriteSheet,graphics,entity->palette);
                Anim_StartIndexed(entity,(u8)entity->ext.particleAttach.anim,0,0);
                entity->drawFunc=Sprite_DrawEntity;
                entity->scrollWithBg=1;
                entity->state++;
            }
            case 1:
                entity->posX.whole=owner->posX.whole+entity->accelX.whole;
                entity->posY.whole=owner->posY.whole+entity->accelY.whole;
                break;
        }
        if(Anim_Update(entity)==3)
            entity->deleteRequested=1;
    }

}

/* Select one of five emitters and add a small random pixel offset. */
/**
 * @brief Cycle through five table emitters, add randomized pixel offsets
 * and create one scene projectile. Select homing from the owner timer
 * modulo six; advance the emitter counter.
 */
void Graham_SpawnSceneProjectile(struct EwramData_EntityData *entity)
{
    struct GrahamSpawnTable positions=sGrahamSpawnTable;
    u16 counter=entity->ext.grahamEmitter.counter;
    u16 index=counter%5;
    u16 x=positions.coordinate[index*2];
    s32 *ys=&positions.coordinate[1];
    u16 y;
    ys+=index*2;
    y=*ys;
    entity->ext.grahamEmitter.counter=counter+1;
    if((u8)(entity->timer%6)==0)
    {
        s32 random=RandomNumberGenerator();
        s32 finalX=x+16;
        s32 finalY;
        s32 mask=31;
        finalX-=(random&mask);
        finalX=(u32)finalX<<16;
        finalX>>=16;
        random=RandomNumberGenerator();
        finalY=y+16;
        finalY-=(random&mask);
        finalY=(u32)finalY<<16;
        finalY>>=16;
        Graham_CreateSceneProjectile(finalX,finalY,1);
    }
    else
    {
        s32 random=RandomNumberGenerator();
        s32 finalX=x+16;
        s32 finalY;
        s32 mask=31;
        finalX-=(random&mask);
        finalX=(u32)finalX<<16;
        finalX>>=16;
        random=RandomNumberGenerator();
        finalY=y+16;
        finalY-=(random&mask);
        finalY=(u32)finalY<<16;
        finalY>>=16;
        Graham_CreateSceneProjectile(finalX,finalY,0);
    }
}

/* Graham defeat scene: dialogue, graphics cleanup, particles and exit event.
 *: case 3 tails written per branch (CSE reuses the
 * zero register, cross-jump merges them), case 4 re-reads object->subState
 * (GCSE copies), constant 170 in a variable for the player-side compare. */
#define FLAGS(e) (e->ext.graham.flags)
/**
 * @brief Root defeat state (0xF0): coordinate dialogue objects, palette/
 * graphics cleanup, timed effects and character-dependent exit events.
 * Gate menus and player input through the existing scene progression.
 */
void Graham_UpdateDefeatScene(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *object=&gEwramData->entityData[(u8)entity->ext.grahamScene.objectIndex];
    if(gEwramData->gameState.stateFlags & 0x10) return;
    switch(entity->subState)
    {
        case 0:
        {
            s32 inner=entity->stage;
            switch(inner)
            {
                case 0:
                    if(gEwramData->gameState.bossRushMode==0)
                    {
                        u32 *flags=&gEwramData->gameState.stateFlags;
                        *flags|=0x20;
                        DMA_FILL_32(3,inner,(u8 *)VRAM_BASE+0x4000,0x2000);
                        *flags|=0x10000;
                        if(FLAGS(entity)&8)
                        {
                            if(Graham_CreateSceneObject(entity,3))
                                goto start_dialogue;
                        }
                        else if(Graham_CreateSceneObject(entity,7))
                        {
start_dialogue:
                            entity->stage++;
                            goto mark_dialogue;
                        }
                    }
                    entity->subState++;
                    entity->stage=inner;
mark_dialogue:
                    FLAGS(entity)|=0x10;
                    break;
                case 1:
                    if(object && (!object->updateFunc || object->enemyId!=32))
                    {
                        entity->subState++;
                        entity->stage=0;
                    }
                    break;
            }
            break;
        }
        case 1:
            FLAGS(entity)|=0x20;
            entity->timer=140;
            if(gEwramData->playerStatus.currentCharacter==1)
                PlaySong(0x1DE);
            else
                PlaySong(0x1DB);
            entity->subState++;
            entity->stage=0;
            break;
        case 2:
            if(entity->timer==32)
                Graham_SpawnBreakupEffect(entity->posX.whole,entity->posY.whole-0x80000);
            if(--entity->timer==255)
            {
                entity->subState++;
                entity->stage=0;
                FLAGS(entity)|=0x40;
                gEwramData->gameState.stateFlags&=~2;
            }
            break;
        case 3:
            if(entity->stage==0)
            {
                s32 i=0;
                u8 *graphics=sGrahamGfx2;
                for(;i<4;i++)
                {
                    gEwramData->gameState.sharedObjPaletteSources[i]=0;
                    gEwramData->gameState.sharedObjPaletteVariants[i]=0;
                }
                ObjGfx_Unload(ObjGfx_Find(graphics));
                ObjGfx_Unload(ObjGfx_Find(sGrahamGfx3));
                gEwramData->gameState.bossEventFlags|=1;
                PlaySong(0x1010);
                entity->timer=64;
                entity->stage++;
            }
            if(--entity->timer==255)
            {
                FLAGS(entity)|=0x40;
                gEwramData->gameState.stateFlags&=~2;
                if(gEwramData->gameState.bossRushMode==0)
                {
                    if(gEwramData->playerStatus.currentCharacter==1)
                    {
                        entity->subState=6;
                        entity->stage=0;
                    }
                    else if(FLAGS(entity)&8)
                    {
                        if(Graham_CreateSceneObject(entity,4))
                        {
                            struct EwramData_EntityData *projectile=Graham_CreateSceneProjectile((s16)entity->posX.part16.integer,
                                                                     (s16)(entity->posY.part16.integer-16),1);
                            entity->ext.grahamScene.defeatProjectileIndex=ENTITY_INDEX(projectile);
                            gEwramData->gameState.stateFlags&=~0x10000;
                            entity->subState++;
                            entity->stage=0;
                        }
                        else
                        {
                            entity->subState+=2;
                            entity->stage=0;
                        }
                    }
                    else if(Graham_CreateSceneObject(entity,1))
                    {
                        entity->subState++;
                        entity->stage=0;
                        Sound_PlayTracked(0x16C);
                    }
                    else
                    {
                        entity->subState+=2;
                        entity->stage=0;
                    }
                }
                else
                {
                    entity->subState+=2;
                    entity->stage=0;
                }
            }
            break;
        case 4:
            if(object)
            {
                if(FLAGS(entity)&8)
                {
                    struct EwramData_EntityData *projectile=&gEwramData->entityData[(u16)entity->ext.grahamScene.defeatProjectileIndex];
                    if(!projectile->updateFunc && object->subState==3)
                        object->timer=(u32)projectile->updateFunc;
                    if((u8)(entity->timer%3)==0)
                    {
                        if(object->subState<=6 || (object->subState==7 && object->stage<=1))
                        {
                            if(object->subState==7 && object->stage==1)
                                entity->timer=7;
                            else if(object->subState<=4)
                                entity->timer=7;
                            Graham_SpawnSceneProjectile(entity);
                        }
                    }
                    entity->timer++;
                }
                if(object->state==128)
                {
                    u16 flag=FLAGS(entity)&8;
                    if(flag)
                    {
                        entity->subState++;
                        entity->stage=0;
                    }
                    else
                    {
                        entity->subState+=2;
                        entity->stage=flag;
                    }
                }
            }
            break;
        case 5:
            if(entity->stage==0)
            {
                Player_SetSelectedTarget(0);
                gEwramData->gameState.stateFlags&=~2;
                entity->stage++;
            }
            Game_SetMenusEnabled(1);
            break;
        case 6:
        {
            s32 inner;
            s32 jitterX,jitterY;
            SetPlayerInput(1,0);
            inner=entity->stage;
            if(inner==0)
            {
                Sound_StopTracked(0x16C);
                Sound_PlayTracked(0x105);
                entity->timer=inner;
                BgPalette_StartColorFade(31,31,31,0x10000,inner,255);
                ObjPalette_StartColorFade(31,31,31,0x10000,inner,255);
                entity->stage++;
            }
            else if(inner==1)
            {
                if(entity->timer==100)
                    PlaySong(0x1041);
                if(!BgPalette_GetPendingFades())
                    entity->stage++;
            }
            else if(inner==2)
            {
                if(gEwramData->playerStatus.currentCharacter==0)
                    gEwramData->gameState.stateFlags|=0x2000;
                else
                    gEwramData->gameState.stateFlags|=0x8000;
                gEwramData->startPauseEnabled=0;
                gEwramData->gameState.stateFlags|=0x800000;
                RoomJump_SetDestination(sEndingRoomDefinition,0,0,0,0);
                RoomJump_Begin(&gEwramData->gameState);
                Player_SetSelectedTarget(0);
                gEwramData->gameState.stateFlags&=~2;
                gDisplayRegisters.bldCnt=0;
                entity->stage++;
            }
            jitterX=(s8)(RandomNumberGenerator()%3);
            jitterY=(s8)(RandomNumberGenerator()%3);
            ScreenShake_RequestOffset(2,jitterX,jitterY);
            break;
        }
    }
    if(gEwramData->gameState.bossRushMode==0)
    {
        if(!(FLAGS(entity)&8) && entity->subState>3)
        {
            u32 timer=entity->timer;
            if((u8)(timer%3)==0)
            {
                entity->timer=7;
                Graham_SpawnSceneProjectile(entity);
                entity->timer=timer;
            }
            entity->timer++;
        }
        if((u8)(entity->subState-2)<=1 && gEwramData->gameState.bossRushMode==0)
        {
            s32 input=0;
            s32 x=170;
            if((s8)Player_GetZoneAroundX(x,60,30)==1)
            {
                s32 playerX=(s16)gEwramData->playerState.player->posX.part16.integer;
                input=x<playerX ? 32 : 16;
            }
            if(input==0)
                gEwramData->playerState.player->flipX=1;
            SetPlayerInput(1,input);
        }
    }
    if(entity->subState!=5)
        Game_SetMenusEnabled(0);
    else
        Game_SetMenusEnabled(1);
}

/* Create the linked combat body and its subordinate parts.
 * The special 0xD0 path, as in the
 * original, does not define a return value. */

/**
 * @brief Create the later linked combat body, set combat fields and root
 * slot link, then create its subordinate parts. Preserve the bestiary
 * branch whose return value is unspecified in the matching source.
 */
struct EwramData_EntityData *Graham_CreateCombatBody(struct EwramData_EntityData *owner)
{
    u8 *gfx2 = sGrahamPalette2;
    u8 *gfx3 = sGrahamSpriteSheet2;
    u8 *gfx1 = sGrahamGfx2;
    struct EwramData_EntityData *e = Entity_CreateObject(GrahamCombatBody_Update);
    if (e == NULL) return NULL;
    if (!Entity_LoadGraphics(e, gfx1, gfx2, 0, gfx3))
    {
        EntityDelete(e);
        return NULL;
    }
    ENTITY_PARENT_INDEX(e) = ENTITY_INDEX(owner);
    e->spriteLayer = owner->spriteLayer;
    e->flipX = owner->flipX;
    e->posX.whole = owner->posX.whole;
    e->posY.whole = owner->posY.whole;
    e->scrollWithBg = 1;
    e->enemyId = owner->enemyId;
    e->hp = 2000;
    e->mp = owner->mp;
    e->attack = owner->attack;
    e->defense = 110;
    e->enemyFlags |= 4;
    owner->ext.grahamScene.childIndex = ENTITY_INDEX(e);
    Anim_StartIndexed(e, 1, 0, 1);
    Anim_Update(e);
    Graham_CreateBodyOverlay(e);

    if (gEwramData->gameState.bestiaryPreview)
    {
        e->state = 0xD0;
        e->stage = 0;
        e->subState = 0;
    }
    else
    {
        Entity_EnableBoxes(e, NULL, GrahamCombatBody_OnHit);
        e->hit.flags |= 2;
        return e;
    }
}

/* Linked combat body mirrors event flags and transitions to its fade state. */
/**
 * @brief Mirror root identity/event flags, run the palette entry sequence
 * and animate/check contact while active. On root defeat flags enter
 * the fade state or request deletion. Bestiary only animates.
 */
void GrahamCombatBody_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner=&gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    if(entity->state==0xD0)
    {
        Anim_Update(entity);
        return;
    }
    if(owner)
    {
        if(owner->enemyId!=entity->enemyId)
            entity->deleteRequested=1;
        entity->ext.graham.flags=owner->ext.graham.flags;
        if(entity->ext.graham.flags & 0x40)
            entity->deleteRequested=1;
        if(entity->deleteRequested)
            return;
    }
    {
        s32 state=entity->state;
        switch(state)
        {
            case 0:
                if(GrahamPart_UpdatePaletteSequence(entity,1))
                {
                    entity->state=1;
                    entity->stage=state;
                    entity->subState=state;
                }
                break;
            case 1:
                if(entity->ext.graham.flags & 0x10)
                {
                    entity->state=0xE0;
                    entity->stage=0;
                    entity->subState=0;
                }
                Anim_Update(entity);
                Enemy_CheckHitPlayer(entity);
                break;
            case 0xE0:
                if(entity->ext.graham.flags & 0x20)
                    entity->objMode=1;
                break;
        }
    }
}

/* Create the combat body overlay, two side parts and a linked attacker.
 * The special 0xD0 path, as in the
 * original, does not define a return value. */

/**
 * @brief Create the body overlay with shared combat fields, create two
 * side parts and the linked attacker, and store side pointers. Preserve
 * the bestiary branch with its unspecified return value.
 */
struct EwramData_EntityData *Graham_CreateBodyOverlay(struct EwramData_EntityData *owner)
{
    u8 *gfx2 = sGrahamPalette2;
    u8 *gfx3 = sGrahamSpriteSheet2;
    u8 *gfx1 = sGrahamGfx2;
    struct EwramData_EntityData *e = Entity_CreateObject(GrahamBodyOverlay_Update);
    if (e == NULL) return NULL;
    if (!Entity_LoadGraphics(e, gfx1, gfx2, 0, gfx3))
    {
        EntityDelete(e);
        return NULL;
    }
    ENTITY_PARENT_INDEX(e) = ENTITY_INDEX(owner);
    e->spriteLayer = owner->spriteLayer + 1;
    e->flipX = owner->flipX;
    e->posX.whole = owner->posX.whole;
    e->posY.whole = owner->posY.whole;
    e->scrollWithBg = 1;
    e->hp = owner->hp;
    e->mp = owner->mp;
    e->attack = owner->attack;
    e->defense = owner->defense;
    e->enemyFlags |= 1;
    e->enemyFlags |= 4;
    e->enemyId = owner->enemyId;
    Anim_StartIndexed(e, 0, 0, 1);
    Anim_Update(e);
    e->ext.graham.sidePart0 = (s32)Graham_CreateSidePart(e, 0);
    GRAHAM_SIDE_PART_1(e) = (s32)Graham_CreateSidePart(e, 1);
    Graham_CreateAttacker(e);

    if (gEwramData->gameState.bestiaryPreview)
    {
        e->state = 0xD0;
        e->stage = 0;
        e->subState = 0;
    }
    else
    {

        return e;
    }
}

/* Linked part mirrors its parent, fades, and coordinates the two limb sounds. */
/* The original reads the adjacent mode/substate bytes as one halfword. */
#define STATE_WORD(e) (*(u16 *)&(e)->state)
/**
 * @brief Mirror body flags, run its palette sequence and coordinate
 * sounds from both side parts using combined state/substate reads.
 * Enter the fade state when owner defeat flags are set.
 */
void GrahamBodyOverlay_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner=&gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    u32 mode=entity->state;
    if(mode==0xD0)
    { Anim_Update(entity); return; }
    if(owner)
    {
        s32 zero;
        u16 flags;
        if(owner->enemyId!=entity->enemyId)
        { entity->deleteRequested=1; return; }
        zero=0;
        entity->ext.graham.flags=owner->ext.graham.flags;
        if((entity->ext.graham.flags & 0x10) && mode<=0xDF)
        {
            entity->enemyFlags|=2;
            entity->state=0xE0;
            entity->stage=zero;
            entity->subState=zero;
        }
    }
    {
        s32 state=entity->state;
        switch(state)
        {
            case 0:
                if(GrahamPart_UpdatePaletteSequence(entity,0))
                {
                    entity->state=1;
                    entity->stage=state;
                    entity->subState=state;
                }
                break;
            case 1:
                if(entity->ext.graham.flags & 0x10)
                {
                    entity->state=0xE0;
                    entity->stage=0;
                    entity->subState=0;
                }
                else
                    Anim_Update(entity);
                break;
            case 0xE0:
                if(entity->ext.graham.flags & 0x20)
                    entity->objMode=1;
                return;
        }
    }
    {
        struct EwramData_EntityData *first=(void *)entity->ext.graham.sidePart0;
        struct EwramData_EntityData *second;
        if(first->stage==8)
        {
            u32 mask=0xFFFF;
            u32 firstState=(u16)(STATE_WORD(first)&mask);
            if(firstState==0x102)
            {
                second=(void *)GRAHAM_SIDE_PART_1(entity);
                if(second->stage==8 && (mask &= STATE_WORD(second))==firstState)
                    PlaySong(0x10E);
                else
                    PlaySong(0x10F);
                return;
            }
        }
        second=(void *)GRAHAM_SIDE_PART_1(entity);
        if(second->stage==8 && STATE_WORD(second)==0x102)
            PlaySong(0x10F);
    }
}

/* Linked part creation.
 * The original defines its return value only on allocation failure and the
 * ordinary success path. Graphics failure and the special 0xD0 mode
 * leave the return value unspecified; preserve those original paths. */

/**
 * @brief Create a side-selected linked part with graphics and contact
 * boxes. Preserve the unspecified return on graphics failure and
 * bestiary branches; ordinary gameplay success returns the part.
 */
struct EwramData_EntityData *Graham_CreateSidePart(struct EwramData_EntityData *owner, s32 side)
{
    u8 *gfx2 = sGrahamPalette2;
    u8 *gfx3 = sGrahamSideSpriteSheet;
    u8 *gfx1 = sGrahamGfx3;
    struct EwramData_EntityData *e = Entity_CreateObject(GrahamSidePart_Update);
    if (e == NULL)
        return NULL;
    if (!Entity_LoadGraphics(e, gfx1, gfx2, 0, gfx3))
    {
        EntityDelete(e);
        return;
    }
    ENTITY_PARENT_INDEX(e) = ENTITY_INDEX(owner);
    e->spriteLayer = owner->spriteLayer - 3;
    e->flipX = side;
    e->posX.whole = owner->posX.whole;
    e->posY.whole = owner->posY.whole;
    e->scrollWithBg = 1;
    e->enemyFlags |= 4;
    e->attack = owner->attack;
    e->enemyId = owner->enemyId;
    e->ext.grahamPart.side = side;
    Anim_StartIndexed(e, 0, 0, 1);
    Anim_Update(e);
    if (gEwramData->gameState.bestiaryPreview)
    {
        e->state = 0xD0;
        e->stage = 0;
        e->subState = 0;
    }
    else
    {
        Entity_EnableBoxes(e, Enemy_OnContact, 0);
        e->hit.flags |= 4;
        return e;
    }
}

/* Child entity that follows its owner: copies the owner flags and runs a
 * timed rise/wait state machine (wait tables sUnk_08119580..). */

/**
 * @brief Mirror owner flags and run palette entry, table-timed waits and
 * a 16-step vertical motion pattern with effects and contact checks.
 * Preserve table copies, halfword event reads and pinned registers.
 */
void GrahamSidePart_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    struct Wait16 rise = sUnk_08119580;
    s32 m = 0x10;
    struct Wait8 waitA = sUnk_081195C0;
    struct Wait8 waitB = sUnk_081195E0;
    u32 mode = entity->state;
    s32 x, y, dx, r;

    if (mode == 0xD0)
    {
        Anim_Update(entity);
        return;
    }
    if (owner)
    {
        register s32 zero asm("r6"); /* FAKEMATCH: zero in r6 */
        register u32 flags asm("r1"); /* FAKEMATCH: flags in r1 */
        if (owner->enemyId != entity->enemyId)
        {
            entity->deleteRequested = 1;
            return;
        }
        
        flags = (u16)owner->ext.graham.flags;
        zero = 0;
        entity->ext.graham.flags = flags;
        if ((flags & m) && mode <= 0xDF)
        {
            entity->state = 0xE0;
            entity->stage = zero;
            entity->subState = zero;
        }
    }
    {
        s32 state = entity->state;
        switch (state)
        {
        case 0:
            if (GrahamPart_UpdatePaletteSequence(entity, 1))
            {
                entity->state = 1;
                entity->stage = state;
                entity->subState = state;
            }
            break;
        case 1:
            switch (entity->subState)
            {
            case 0:
                if (entity->stage == 0)
                {
                    if (entity->animScriptIndex)
                        Anim_StartIndexed(entity, 0, 0, 1);
                    entity->timer = 0x80;
                    entity->stage++;
                }
                goto wait;
            case 1:
            {
                if (entity->ext.grahamPart.side)
                    entity->timer = waitA.t[entity->ext.grahamPart.step];
                else
                    entity->timer = waitB.t[entity->ext.grahamPart.step];
                if (++entity->ext.grahamPart.step != 1 && entity->ext.grahamPart.step > 7)
                {
                    entity->ext.grahamPart.step = 0;
                    entity->ext.grahamPart.side = 1 - entity->ext.grahamPart.side;
                }
                entity->subState++;
                entity->stage = 0;
            }
            case 2:
            wait:
                if (--entity->timer == 0xFF)
                {
                    entity->subState++;
                    entity->stage = 0;
                }
                break;
            case 3:
            {
                u16 busy = *(u16 *)&owner->animStep;
                if (busy == 0 && owner)
                {
                    entity->state = 2;
                    entity->stage = busy;
                    entity->subState = busy;
                    if (entity->animScriptIndex != 1)
                        Anim_StartIndexed(entity, 1, 0, 0);
                }
                break;
            }
            }
            break;
        case 2:
            switch (entity->subState)
            {
            case 0:
                entity->velY.whole = 0x10000;
                entity->stage = 0;
                entity->subState++;
                entity->ext.grahamPart.flash = 16;
            case 1:
                entity->timer = rise.t[entity->stage] - 1;
                entity->subState++;
            case 2:
                if (entity->ext.grahamPart.flash)
                {
                    if ((s16)(entity->ext.grahamPart.flash % 3) == 0)
                    {
                        dx = -0x380000;
                        if (!entity->flipX)
                            dx = -dx;
                        r = RandomNumberGenerator();
                        x = entity->posX.whole + dx;
                        x += -0x40000;
                        x += r & 0x7FFFF;
                        r = RandomNumberGenerator();
                        y = entity->posY.whole;
                        y += -0x40000;
                        y += r & 0x7FFFF;
                        Effect_CreateFromTable(x, y, 0, -0x2000, 4, 9);
                    }
                    entity->ext.grahamPart.flash--;
                }
                if (--entity->timer == 0xFF)
                {
                    entity->posY.whole += entity->velY.whole;
                    if ((u8)++entity->stage == m)
                    {
                        entity->ext.grahamPart.flash = 0;
                        if (entity->animScriptIndex)
                            Anim_StartIndexed(entity, 0, 0, 1);
                        entity->state = 1;
                        entity->stage = 0;
                        entity->subState = 1;
                    }
                    else
                    {
                        if ((entity->stage % 4) == 0)
                            entity->velY.whole = -entity->velY.whole;
                        entity->subState--;
                    }
                }
                break;
            }
            break;
        case 0xE0:
            if (entity->ext.graham.flags & 0x20)
                entity->objMode = 1;
            return;
        }
    }
    if (entity->state)
    {
        Entity_AnimateWithBoxes(entity);
        Enemy_CheckHitPlayer(entity);
    }
}

/* Create the linked attacker with doubled health/parameter 512.
 * The special 0xD0 path, as in the
 * original, does not define a return value. */

/**
 * @brief Create the linked attacker with doubled owner HP and parameter
 * 0x512, shared identity and its own hit callback. Preserve the
 * bestiary branch with its unspecified return value.
 */
struct EwramData_EntityData *Graham_CreateAttacker(struct EwramData_EntityData *owner)
{
    u8 *gfx2 = sGrahamPalette2;
    u8 *gfx3 = sGrahamSpriteSheet2;
    u8 *gfx1 = sGrahamGfx3;
    struct EwramData_EntityData *e = Entity_CreateObject(GrahamAttacker_Update);
    if (e == NULL) return NULL;
    if (!Entity_LoadGraphics(e, gfx1, gfx2, 0, gfx3))
    {
        EntityDelete(e);
        return NULL;
    }
    ENTITY_PARENT_INDEX(e) = ENTITY_INDEX(owner);
    e->spriteLayer = owner->spriteLayer - 2;
    e->flipX = owner->flipX;
    e->posX.whole = owner->posX.whole;
    e->posY.whole = owner->posY.whole;
    e->scrollWithBg = 1;
    e->enemyFlags |= 1;
    e->enemyFlags |= 4;
    e->enemyId = owner->enemyId;
    e->hp = (s16)owner->hp * 2;
    e->mp = owner->mp * 2;
    e->attack = owner->attack;
    e->defense = owner->defense * 2 / 3;
    Anim_StartIndexed(e, 2, 0, 1);
    Anim_Update(e);

    if (gEwramData->gameState.bestiaryPreview)
    {
        e->state = 0xD0;
        e->stage = 0;
        e->subState = 0;
    }
    else
    {
        Entity_EnableBoxes(e, NULL, GrahamAttacker_OnHit);
        e->hit.flags |= 2;
        return e;
    }
}

#define STATE(e, value) { (e)->state = value; (e)->stage = 0; (e)->subState = 0; }

/**
 * @brief Wait, move toward target X and sequence an attack effect with
 * timed recovery. Dispatch its death routine at state 0xF0 and mirror
 * owner defeat progression; bestiary only animates.
 */
void GrahamAttacker_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    u32 mode = entity->state;
    if (mode == 0xD0) { Anim_Update(entity); return; }
    if (mode == 0xF0) { GrahamAttacker_UpdateDeath(entity); return; }
    if (owner != NULL)
    {
        if (owner->enemyId != entity->enemyId)
        { entity->deleteRequested = 1; return; }
        if ((owner->ext.graham.flags & 0x10) && mode <= 0xDF)
            STATE(entity, 0xE0);
    }
    {
        s32 state = entity->state;
        switch (state)
        {
        case 0:
            if (entity->subState == 0)
            {
                entity->subState++;
                entity->timer = 0x40;
                entity->objMode = 1;
            }
            if (--entity->timer == 0xFF)
            {
                entity->objMode = 0;
                entity->state = 1;
                entity->stage = state;
                entity->subState = state;
            }
            break;
        case 1:
            if (entity->subState == 0)
            {
                s32 distance;
                Anim_StartIndexed(entity, 2, 0, 1);
                Entity_FaceTarget(entity);
                if (Entity_GetTargetOffsetX(entity) < 0)
                    distance = -Entity_GetTargetOffsetX(entity);
                else
                    distance = Entity_GetTargetOffsetX(entity);
                if (entity->flipX)
                {
                    entity->velX.whole = 0x30000;
                    entity->accelX.whole = 0;
                }
                else
                {
                    entity->velX.whole = -0x30000;
                    entity->accelX.whole = 0;
                }
                entity->timer = distance / (entity->velX.whole < 0 ? -entity->velX.whole : entity->velX.whole);
                entity->subState++;
                entity->stage = 0;
            }
            if (--entity->timer == 0xFF)
                STATE(entity, 2)
            else
            {
                entity->posX.whole += entity->velX.whole;
                entity->posY.whole += entity->velY.whole;
                entity->velX.whole += entity->accelX.whole;
                entity->velY.whole += entity->accelY.whole;
            }
            break;
        case 2:
            switch (entity->subState)
            {
            case 0:
                switch (entity->stage)
                {
                case 0:
                    if (entity->animScriptIndex != 5) Anim_StartIndexed(entity, 5, 0, 0);
                    entity->stage++;
                case 1:
                    if (entity->animDone)
                    {
                        PlaySong(0x111);
                        entity->subState++;
                        entity->stage = 0;
                    }
                    break;
                }
                break;
            case 1:
                switch (entity->stage)
                {
                case 0:
                    if (entity->animScriptIndex != 3) Anim_StartIndexed(entity, 3, 0, 1);
                    GrahamAttacker_CreateAttackEffect(entity);
                    entity->timer = 0x80;
                    entity->stage++;
                case 1:
                    if (entity->timer == 0x60) PlaySong(0x110);
                    if (--entity->timer == 0xFF) STATE(entity, 1);
                    break;
                }
                break;
            }
            break;
        case 0xE0:
            STATE(entity, 0xF0);
            return;
        }
    }
    Entity_AnimateWithBoxes(entity);
    Enemy_CheckHitPlayer(entity);
}

/* GrahamAttacker_UpdateDeath: matched (plain --x == 0xFF / ++x == 0x20: the QImode 255 mask is shared
   by CSE and spilled to the stack, as in the original). */

static inline void SpawnSpark(struct EwramData_EntityData *entity, s32 dx, s32 dy, u8 pri)
{
    struct EwramData_EntityData *effect;
    s32 x, y, priority;
    x = (dx << 16) + entity->posX.whole;
    y = (dy << 16) + entity->posY.whole;
    y -= 0x460000;
    priority = pri - 1;

    effect = EntityCreateInRange(0x49, 0xCF, GrahamDeathSpark_Update);
    if (effect != NULL)
    {
        effect->posX.whole = x;
        effect->posY.whole = y;
        effect->flipX = RandomNumberGenerator() % 2;
        effect->spriteLayer = priority;
    }
}
static inline void SpawnSpark2(struct EwramData_EntityData *entity, s32 dx, s32 dy, u8 pri)
{
    struct EwramData_EntityData *effect;
    s32 x, y, priority;
    x = (dx << 16) + entity->posX.whole;
    y = (dy << 16) + entity->posY.whole;
    y -= 0x460000;
    priority = pri - 1;

    effect = EntityCreateInRange(0x49, 0xCF, GrahamDeathSpark_Update);
    if (effect != NULL)
    {
        effect->posX.whole = x;
        effect->posY.whole = y;
        effect->flipX = RandomNumberGenerator() % 2;
        effect->spriteLayer = priority;
    }
}

/**
 * @brief Remove the linked attack effect, wait for owner flags and emit
 * 32 table-positioned sparks/particles followed by a final burst.
 * Request deletion; retain pointer-test order and byte-counter forms.
 */
void GrahamAttacker_UpdateDeath(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *part = &gEwramData->entityData[entity->ext.grahamScene.childIndex];
    struct EwramData_EntityData *owner = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    s16 positions[32][2];

    memcpy(positions, sGrahamAttackerPositions, sizeof(positions));
    switch (entity->subState)
    {
    case 0:
        Anim_StartIndexed(entity, 3, 0, 1);
        if (part->enemyId == entity->enemyId && part != NULL)
            part->deleteRequested = 1;
        entity->objMode = 1;
        entity->timer = 2;
        entity->subState++;
        entity->stage = 0;
    case 1:
        if (!(owner->enemyFlags & 2) || owner == NULL || (owner->ext.graham.flags & 0x20))
        {
            entity->subState++;
            entity->stage = 0;
        }
        break;
    case 2:
        {
        if (--entity->timer == 0xFF)
        {
            PlaySong(0x6E);
            SpawnSpark2(entity, positions[entity->stage][0], positions[entity->stage][1], entity->spriteLayer);
            Effect_SpawnRisingParticle(entity->posX.whole + (positions[entity->stage][0] << 16),
                entity->posY.whole + ((positions[entity->stage][1] - 0x46) << 16), entity->spriteLayer - 1, 0);
            if (++entity->stage == 0x20)
            {
                entity->subState++;
                entity->stage = 0;
            }
            else
                entity->timer = 2;
        }
        }
        break;
    case 3:
        if (entity->stage == 0)
        {
            entity->timer = 0x20;
            entity->stage++;
        }
        if (--entity->timer == 0xFF)
        {
            entity->subState++;
            entity->stage = 0;
        }
        break;
    case 4:
        {
            s32 i;
            for (i = 0; i < 32; i += 4)
            {
                PlaySong(0x77);
                SpawnSpark(entity, positions[i][0], positions[i][1], entity->spriteLayer);
            }
            entity->deleteRequested = 1;
        }
        break;
    }
}

/* Create a linked part and copy the owner's position and combat fields. */

/**
 * @brief Create an animated damaging effect linked to the attacker with
 * five-quarters owner damage. Return it or NULL after failure cleanup.
 * Preserve the dead-register instruction and multiply/shift lifetime.
 */
struct EwramData_EntityData *GrahamAttacker_CreateAttackEffect(struct EwramData_EntityData *owner)
{
    u8 *palette = sGrahamPalette2;
    u8 *animation = sGrahamSpriteSheet2;
    u8 *graphics = sGrahamGfx3;
    struct EwramData_EntityData *part = Entity_CreateObject(GrahamAttackEffect_Update);
    if (part == NULL)
        return NULL;
    if (Entity_LoadGraphics(part, graphics, palette, 1, animation))
    {
    ENTITY_PARENT_INDEX(part) = ENTITY_INDEX(owner);
    part->spriteLayer = owner->spriteLayer - 1;
    part->flipX = owner->flipX;
    part->posX.whole = owner->posX.whole;
    part->posY.whole = owner->posY.whole;
    part->scrollWithBg = 1;
    owner->ext.grahamScene.childIndex = ENTITY_INDEX(part);
    { u8 v_ = part->enemyFlags; { register u32 z_ asm("r7"); asm("" : "=r"(z_) : "0"(0)); } part->enemyFlags = v_ | 1; } /* FAKEMATCH: dead "movs r7, #0" of the original (graphics = 0) */

    part->enemyFlags |= 4;
    part->enemyId = owner->enemyId;
    { s32 m_ = owner->attack * 5; s32 s_ = m_ >> 2; asm("" :: "r"(m_)); part->attack = s_; } /* FAKEMATCH: m_ outlives the shift, so the result gets its own reg */
    Anim_StartIndexed(part, 4, 0, 1);
    Anim_Update(part);
    Entity_EnableBoxes(part, Entity_OnPlayerContactDamage, 0);
    part->hit.flags |= 4;
    return part;
    }
    else
    { EntityDelete(part); return NULL; }
}

/**
 * @brief Create the initial linked combat entity, copy root identity and
 * configure its HP, graphics and callbacks. Return the entity in the
 * gameplay branch or NULL on allocation/graphics failure. Preserve the
 * bestiary branch with its existing unspecified return value.
 */
struct EwramData_EntityData *GrahamFirstForm_Create(struct EwramData_EntityData *owner)
{
    u8 *gfx2 = sGrahamPalette1;
    u8 *gfx3 = sGrahamSpriteSheet1;
    u8 *gfx1 = sGrahamGfx1;
    struct EwramData_EntityData *e = Entity_CreateObject(GrahamFirstForm_Update);
    if (e == NULL) return NULL;
    if (!Entity_LoadGraphics(e, gfx1, gfx2, 0, gfx3))
    {
        EntityDelete(e);
        return NULL;
    }
    ENTITY_PARENT_INDEX(e) = ENTITY_INDEX(owner);
    e->spriteLayer = owner->spriteLayer - 1;
    e->flipX = owner->flipX;
    e->posX.whole = owner->posX.whole;
    e->posY.whole = owner->posY.whole;
    e->scrollWithBg = 1;
    e->state = 3;
    e->stage = 0;
    e->subState = 0;
    e->enemyFlags |= 4;
    e->hp = 1500;
    e->mp = owner->mp / 2;
    e->attack = 60;
    e->defense = 100;
    e->enemyId = owner->enemyId;
    e->spawnIndex = owner->spawnIndex;
    e->enemyFlags = owner->enemyFlags | 4;
    Anim_StartIndexed(e, 0, 0, 1);
    Anim_Update(e);
    if (gEwramData->gameState.bestiaryPreview)
    {
        e->state = 0xD0;
        e->stage = 0;
        e->subState = 0;
    }
    else
    {
        Entity_SnapToGround(e);
        Entity_EnableBoxes(e, Enemy_OnContact, GrahamFirstForm_OnHit);
        return e;
    }
}

/* Graham core: vanish/reappear palette sequence, reposition, then combat. */
#define SET_PALETTE(n) ObjPalette_LoadRange(sGrahamPalette1,n,1,entity->palette)
/**
 * @brief Dispatch first-form teleport/reappearance state 3, attacks in
 * state 6/0xE0 and transition state 5. Apply palette events, target
 * facing, hit-box animation and contact checks; bestiary only animates.
 */
void GrahamFirstForm_Update(struct EwramData_EntityData *entity)
{
    if(entity->state==0xD0)
    { Anim_Update(entity); return; }
    if(entity->state==5)
    { GrahamFirstForm_UpdateTransition(entity); return; }
    switch(entity->state)
    {
        case 3:
        {
            s32 state=entity->subState;
            switch(state)
            {
                case 0:
                    if(entity->animScriptIndex!=0)
                        Anim_StartIndexed(entity,0,0,1);
                    if(entity->ext.graham.flags & 2)
                    {
                        entity->hp=1500;
                        entity->subState+=2;
                        entity->stage=0;
                    }
                    else
                    {
                        entity->timer=16;
                        entity->subState++;
                        entity->stage=state;
                    }
                    break;
                case 1:
                    if(--entity->timer==255)
                    {
                        entity->subState++;
                        entity->stage=0;
                    }
                    break;
                case 2:
                    switch(entity->stage)
                    {
                        case 0:
                            entity->posY.part16.integer-=32;
                            entity->stage++;
                        case 1:
                        {
                            u8 *frame;
                            if(entity->animScriptIndex!=6)
                                Anim_StartIndexed(entity,6,0,0);
                            frame=&entity->animStep;
                            if(*frame==2) SET_PALETTE(3);
                            if(*frame==3) SET_PALETTE(2);
                            if(*frame==4) SET_PALETTE(1);
                            if(*frame==21) SET_PALETTE(0);
                            if(entity->animDone)
                            {
                                Entity_FaceTarget(entity);
                                if(entity->ext.graham.flags & 2)
                                {
                                    struct BgLayerState *camera=&gEwramData->bgInfo[1];
                                    entity->posX.whole=0x780000-camera->xPos.whole;
                                }
                                else
                                {
                                    u32 x=RandomNumberGenerator()%160;
                                    struct BgLayerState *camera;
                                    x+=40;
                                    camera=&gEwramData->bgInfo[1];
                                    x-=camera->xPos.part16.integer;
                                    entity->posX.part16.integer=x;
                                }
                                entity->stage++;
                                SET_PALETTE(1);
                            }
                            break;
                        }
                        case 2:
                        {
                            u8 *animation=&entity->animScriptIndex;
                            u8 *frame;
                            if(*animation!=5)
                                Anim_StartIndexed(entity,5,0,0);
                            frame=&entity->animStep;
                            if(*frame<=14)
                                Entity_FaceTarget(entity);
                            if(*frame==19) SET_PALETTE(2);
                            if(*frame==20) SET_PALETTE(3);
                            if(*frame==21) SET_PALETTE(0);
                            if(entity->animDone)
                            {
                                SET_PALETTE(0);
                                if(*animation!=0)
                                    Anim_StartIndexed(entity,0,0,1);
                                entity->posY.part16.integer+=32;
                                entity->stage++;
                                entity->timer=state;
                            }
                            break;
                        }
                        case 3:
                            if(--entity->timer==255)
                            {
                                u32 flags=*(volatile u16 *)&entity->ext.graham.flags; /* FAKEMATCH: volatile read forces the reload of the original */
                                s32 mask=flags & state;
                                if(mask)
                                {
                                    entity->state=0xE0;
                                    entity->stage=0;
                                    entity->subState=0;
                                }
                                else
                                {
                                    entity->state=6;
                                    entity->stage=mask;
                                    entity->subState=mask;
                                }
                            }
                            break;
                    }
                    break;
            }
            break;
        }
        case 6:
            GrahamFirstForm_UpdateAttacks(entity);
            break;
        case 0xE0:
            GrahamFirstForm_UpdateAlternateAttack(entity);
            break;
    }
    Entity_AnimateWithBoxes(entity);
    Enemy_CheckHitPlayer(entity);
}

/* Graham attack selection, animation turns and timed projectile volleys. */
/**
 * @brief Attack state (6): turn toward the target, choose spread or paired
 * projectile volleys with timed recovery and optional mid-volley turns,
 * then return to teleport state 3.
 */
void GrahamFirstForm_UpdateAttacks(struct EwramData_EntityData *entity)
{
    s32 remaining;
    switch(entity->subState)
    {
        case 0:
        {
            s32 facing=Entity_IsFacingTarget(entity);
            if(!facing)
            {
                entity->subState=1;
                entity->stage=facing;
            }
            else
            {
                entity->subState=2;
                entity->stage=0;
            }
            break;
        }
        case 1:
            if(entity->animScriptIndex!=7)
                Anim_StartIndexed(entity,7,0,0);
            if(entity->animDone)
            {
                entity->flipX=1-entity->flipX;
                entity->subState=2;
                entity->stage=0;
            }
            break;
        case 2:
            if(entity->animScriptIndex!=2)
                Anim_StartIndexed(entity,2,0,0);
            if(entity->animDone)
            {
                entity->subState=3;
                entity->stage=0;
            }
            break;
        case 3:
        {
            s32 random;
            if(entity->animScriptIndex!=1)
                Anim_StartIndexed(entity,1,0,1);
            random=RandomNumberGenerator() & 1;
            if(!random)
            {
                entity->timer=16;
                PlaySong(0x1D9);
                entity->subState=4;
                entity->stage=random;
            }
            else
            {
                random=RandomNumberGenerator()%3;
                if(!random)
                {
                    PlaySong(0x1D7);
                    entity->timer=random;
                    entity->subState=5;
                    entity->stage=4;
                }
                else
                {
                    PlaySong(0x1D8);
                    entity->timer=0;
                    entity->subState=6;
                    entity->stage=2;
                    if(!(RandomNumberGenerator() & 1))
                        entity->ext.grahamTurn.volleySide=0;
                    else
                        entity->ext.grahamTurn.volleySide=1;
                }
            }
            break;
        }
        case 4:
            if(--entity->timer==255)
            {
                PlaySong(0x7A);
                Graham_SpawnSpreadProjectiles(entity);
                entity->timer=64;
                entity->subState=8;
                entity->stage=0;
            }
            break;
        case 5:
            if(--entity->timer==255)
            {
                remaining=entity->stage;
                if(!remaining) goto finish_volley;
                Graham_CreateShotPair(entity,0);
                entity->timer=24;
                entity->stage--;
            }
            else if(entity->timer<=7 && entity->stage && !Entity_IsFacingTarget(entity))
            {
                entity->ext.grahamTurn.resume=5;
                entity->subState=7;
            }
            break;
        case 6:
            if(--entity->timer==255)
            {
                remaining=entity->stage;
                if(!remaining)
                {
finish_volley:
                    entity->subState=8;
                    entity->stage=remaining;
                    entity->timer=32;
                    break;
                }
                if(entity->ext.grahamTurn.volleySide)
                {
                    if(remaining==2) goto shot_one;
                    Graham_CreateShotPair(entity,0);
                    goto shot_done;
                }
                if(remaining==2) goto shot_zero;
shot_one:
                Graham_CreateShotPair(entity,1);
                goto shot_done;
shot_zero:
                Graham_CreateShotPair(entity,0);
shot_done:
                entity->timer=48;
                entity->stage--;
            }
            else if(entity->timer<=7 && entity->stage && !Entity_IsFacingTarget(entity))
            {
                entity->ext.grahamTurn.resume=6;
                entity->subState=7;
            }
            break;
        case 7:
        {
            u8 *animation=&entity->animScriptIndex;
            if(*animation!=7)
                Anim_StartIndexed(entity,7,0,1);
            if(entity->animDone)
            {
                if(*animation!=1)
                    Anim_StartIndexed(entity,1,0,1);
                entity->flipX=1-entity->flipX;
                entity->subState=entity->ext.grahamTurn.resume;
            }
            break;
        }
        case 8:
            if(--entity->timer==255)
            {
                entity->state=3;
                entity->stage=0;
                entity->subState=0;
            }
            break;
    }
}

/* Alternate attack repeats a projectile pattern with an alternating selector. */
/**
 * @brief Alternate attack state (0xE0): face the target and periodically
 * spawn the spiral pattern with alternating direction.
 */
void GrahamFirstForm_UpdateAlternateAttack(struct EwramData_EntityData *entity)
{
    s32 state=entity->subState;
    switch(state)
    {
        case 0:
            if(entity->animScriptIndex!=1)
                Anim_StartIndexed(entity,1,0,1);
            entity->subState++;
            entity->stage=state;
            entity->timer=state;
            break;
        case 1:
            Entity_FaceTarget(entity);
            if(--entity->timer==255)
            {
                if((state &= entity->stage) && !(RandomNumberGenerator() & 3))
                    PlaySong(0x1D9);
                else
                    PlaySong(0x10C);
                Graham_SpawnSpiralProjectiles(entity,entity->stage & 1);
                entity->timer=160;
                entity->stage++;
            }
            break;
    }
}

/* Link the animated overhead object back to the boss. */
/**
 * @brief Create the animated transition part above the first form and
 * store both owner/child links. Preserve the existing allocation assumption.
 */
void GrahamFirstForm_CreateTransitionPart(struct EwramData_EntityData *entity)
{
    u8 *sprite=sGrahamPalette2;
    u8 *animation=sGrahamFirstFormSpriteSheet;
    u8 *graphics=sGrahamFirstFormGfx;
    struct EwramData_EntityData *effect=Entity_CreateObject(GrahamTransitionPart_Update);
    if(!Entity_LoadGraphics(effect,graphics,sprite,5,animation))
    {
        EntityDelete(effect);
        return;
    }
    ENTITY_PARENT_INDEX(effect)=ENTITY_INDEX(entity);
    effect->spriteLayer=entity->spriteLayer-1;
    effect->flipX=entity->flipX;
    effect->posX.whole=entity->posX.whole;
    effect->posY.whole=entity->posY.whole-0x300000;
    effect->scrollWithBg=1;
    effect->hidden=1;
    entity->ext.grahamScene.childIndex=ENTITY_INDEX(effect);
    Anim_StartIndexed(effect,1,1,0);
    Anim_Update(effect);
}

/* Linked scene part: wait for the scene object, replace graphics,
 * animate the transition and coordinate its partner. */

/**
 * @brief Transition state (5): coordinate scene objects, replace graphics
 * and animate the transformation with a linked part. Set root flag 4
 * when that part finishes, free graphics and wait for its removal.
 */
void GrahamFirstForm_UpdateTransition(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *part = &gEwramData->entityData[entity->ext.grahamScene.childIndex];
    struct EwramData_EntityData *owner = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    struct EwramData_EntityData *object = &gEwramData->entityData[(u8)owner->ext.grahamScene.objectIndex];
    switch (entity->subState)
    {
    case 0:
        switch (entity->stage)
        {
        case 0:
            entity->timer = 0x20;
            entity->stage++;
        case 1:
            if (--entity->timer == 0xFF)
                entity->stage++;
            break;
        case 2:
            {
                s32 created;
                if (owner->ext.graham.flags & 8)
                    created = Graham_CreateSceneObject(owner, 2);
                else
                    created = Graham_CreateSceneObject(owner, 6);
                if (created)
                    entity->stage++;
                else
                    entity->stage += 2;
            }
            if (entity->animScriptIndex != 0)
                Anim_StartIndexed(entity, 0, 0, 1);
            entity->timer = 0x40;
            break;
        case 3:
            if (object != NULL && (!object->updateFunc || object->enemyId != 0x20))
            {
                entity->subState = 1;
                entity->stage = 0;
            }
            break;
        case 4:
            if (--entity->timer == 0xFF)
            {
                entity->subState = 1;
                entity->stage = 0;
            }
            break;
        }
        break;
    case 1:
        {
            s32 i;
            s32 graphics;
            u8 *palette, *animation, *graphicsData;
            s8 slot;
            for (i = 0; i <= 3; i++)
            {
                gEwramData->gameState.sharedObjPaletteSources[i] = 0;
                gEwramData->gameState.sharedObjPaletteVariants[i] = 0;
            }
            Sprite_Free(entity, 1);
            graphicsData = sGrahamFirstFormGfx;
            palette = sGrahamPalette2;
            animation = sGrahamFirstFormSpriteSheet;
            graphics = ObjGfx_LoadResource(graphicsData, 1);
            if (graphics >= 0)
            {
                slot = ObjPalette_FindOrLoadVariant(palette, 6);
                if (slot >= 0)
                {
                    entity->basePalette = slot;
                    Sprite_Attach(entity, animation, graphics, entity->basePalette);
                    entity->drawFunc = Sprite_DrawEntity;
                }
            }
            Anim_StartIndexed(entity, 0, 1, 0);
            Anim_Update(entity);
            entity->subState = 2;
            entity->stage = 0;
            GrahamFirstForm_CreateTransitionPart(entity);
            entity->posY.part16.integer -= 0x1A;
            PlaySong(0x1DC);
        }
        break;
    case 2:
        if (entity->animStep == 0x11)
            ObjPalette_LoadRange(sGrahamPalette2, 7, 1, entity->palette);
        if (entity->animStep == 0x19)
        {
            PlaySong(0xFE);
            ObjPalette_LoadRange(sGrahamPalette2, 8, 1, entity->palette);
        }
        if (entity->animStep == 0x21)
            ObjPalette_LoadRange(sGrahamPalette2, 5, 1, entity->palette);
        if (entity->animStep <= 0x17 && entity->animTimer == 0)
            entity->posY.part16.integer--;
        if (entity->animStep == 0x29)
            part->hidden = 0;
        if (entity->animDone)
        {
            entity->hidden = 1;
            entity->subState = 3;
            entity->stage = 0;
        }
        break;
    case 3:
        if (part->animDone)
        {
            owner->ext.graham.flags |= 4;
            entity->subState = 4;
            entity->stage = 0;
            Sprite_Free(entity, 1);
        }
        return;
    case 4:
        if (!part->updateFunc)
            entity->deleteRequested = 1;
        return;
    }
    if (!entity->hidden)
        Anim_Update(entity);
}

/* Hit callback with an equipped-souls check. */

#define SET_STATE(e, s)                     \
    {                                       \
        (e)->state = (s);                 \
        (e)->stage = 0;                   \
        (e)->subState = 0;                   \
    }

/* 0 when exactly this red/blue/yellow soul set is equipped */
static inline s32 SoulsDiffer(u8 red, u8 blue, u8 yellow)
{
    struct PlayerStatus *equip = &gEwramData->playerStatus;
    s32 differ = 0;

    if (equip->equippedRedSoul != red)
        return 1;
    if (equip->equippedBlueSoul != blue)
        return 1;
    if (equip->equippedYellowSoul != yellow)
        differ = 1;
    return differ;
}

/**
 * @brief Apply linked-part damage. First HP exhaustion sets flag 2 and
 * restarts teleport; later exhaustion starts transition state 5. On
 * that transition, equipment soul IDs 0x2C/2/7 set root flag 8.
 */
void GrahamFirstForm_OnHit(struct EwramData_EntityData *entity, s32 hit, u16 *data)
{
    struct EwramData_EntityData *owner = &gEwramData->entityData[(u8)ENTITY_PARENT_INDEX(entity)];

    if (GrahamPart_TakeDamage(entity, hit, data))
    {
        if (!(entity->ext.graham.flags & 2))
        {
            entity->ext.graham.flags |= 2;
            SET_STATE(entity, 3);
        }
        else
        {
            SET_STATE(entity, 5);
            if (!SoulsDiffer(0x2C, 2, 7))
                owner->ext.graham.flags |= 8;
        }
    }
    PlaySong(0x1DA);
}

/* Spawn of three linked parts. */

/**
 * @brief Create three damaging projectiles at the owner with indexed
 * launch slopes. Return success. Preserve the existing uninitialized
 * array entries and full-array cleanup on allocation/graphics failure.
 */
s32 Graham_SpawnSpreadProjectiles(struct EwramData_EntityData *entity)
{
    /* graphics pointers are loaded before the loop in the original */
    u8 *gfx2 = sGrahamPalette1;
    u8 *gfx3 = sGrahamSpriteSheet1;
    u8 *gfx1 = sGrahamGfx1;
    struct EwramData_EntityData *parts[3];
    struct EwramData_EntityData **p;
    s32 ok = 1;
    s32 i;

    i = 0;
    p = parts;
    while (i < 3)
    {
        *p = Entity_CreateObject(GrahamSpreadProjectile_Update);
        if (*p == NULL)
        {
            ok = 0;
            break;
        }
        if (!Entity_LoadGraphics(*p, gfx1, gfx2, 4, gfx3))
        {
            ok = 0;
            goto fail;
        }
        ENTITY_PARENT_INDEX((*p)) = ENTITY_INDEX(entity);
        (*p)->spriteLayer = entity->spriteLayer - 1;
        (*p)->flipX = 1 - entity->flipX;
        (*p)->posX.whole = entity->posX.whole;
        (*p)->posY.whole = entity->posY.whole - 0x200000;
        (*p)->scrollWithBg = 1;
        (*p)->attack = 0x46;
        (*p)->ext.grahamSpread.index = i;
        Anim_StartIndexed(*p, 8, 0, 1);
        Anim_Update(*p);
        Entity_EnableBoxes(*p++, GrahamSpreadProjectile_DamagePlayerOnContact, GrahamSpreadProjectile_OnHit);
        i++;
    }
    if (!ok)
    {
    fail:
        for (p = parts, i = 2; i >= 0; p++, i--)
        {
            if (*p != NULL)
                EntityDelete(*p);
        }
    }
    return ok;
}

/* Three launch slopes share acceleration and cleanup when their owner dies. */
/**
 * @brief Launch along one of three slopes and check player contact.
 * On owner defeat, hit state or screen exit, spawn an effect and
 * request removal.
 */
void GrahamSpreadProjectile_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner=&gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    s32 state;
    if((s16)owner->hp<=0)
    {
        entity->hit.flags|=2;
        entity->state=0xF0;
    }
    state=entity->state;
    switch(state)
    {
        case 0:
        {
            s32 velocity=0x18000;
            entity->velX.whole=velocity;
            if(entity->flipX)
                entity->velX.whole=-velocity;
            switch((u8)entity->ext.grahamSpread.index)
            {
                case 0: entity->velY.whole=-0x3800; break;
                case 1: entity->velY.whole=state; break;
                case 2: entity->velY.whole=0x3800; break;
            }
            entity->state++;
            entity->stage=0;
            entity->subState=0;
        }
        case 1:
            entity->posX.whole+=entity->velX.whole;
            entity->posY.whole+=entity->velY.whole;
            entity->velX.whole+=entity->accelX.whole;
            entity->velY.whole+=entity->accelY.whole;
            if(EntityIsOutsideScreen(entity,4))
            {
                entity->state=0xF0;
                entity->stage=0;
                entity->subState=0;
            }
            break;
        case 0xF0:
            Effect_SpawnAt(entity->posX.whole,entity->posY.whole,entity->spriteLayer-1);
            entity->deleteRequested=1;
            return;
    }
    Anim_Update(entity);
    Enemy_CheckHitPlayer(entity);
}

/* Spawn the two child parts of this enemy (one plain, one with an affine matrix); delete both on failure. */

/**
 * @brief Create a plain firing effect and delayed affine shot at the
 * selected height. Return success; retain all existing cleanup bounds
 * and array-entry assumptions, including the failure loop as written.
 */
s32 Graham_CreateShotPair(struct EwramData_EntityData *owner, s32 low)
{
    u8 *graphics = sGrahamGfx1;
    u8 *pal = sGrahamSpriteSheet1;
    s32 success = 1;
    s32 i = 0;
    struct EwramData_EntityData *parts[2];
    struct EwramData_EntityData **p = parts;
    while (i <= 1)
    {
        { struct EwramData_EntityData *spawned = Entity_CreateObject(GrahamShotPart_Update);
        *p = spawned;
        if (!spawned) { success = 0; break; }
        if (!Entity_LoadGraphics(spawned, graphics, sGrahamShotPalette, 0, pal))
        { success = 0; goto fail; } }
        ENTITY_PARENT_INDEX((*p)) = ENTITY_INDEX(owner);
        (*p)->spriteLayer = owner->spriteLayer - 1;
        (*p)->flipX = owner->flipX;
        (*p)->posX.whole = owner->posX.whole;
        if (low)
            (*p)->posY.whole = owner->posY.whole - 0x100000;
        else
            (*p)->posY.whole = owner->posY.whole - 0x200000;
        (*p)->scrollWithBg = 1;
        (*p)->attack = 0x50;
        (*p)->ext.grahamShot.partIndex = i;
        if ((u8)(*p)->ext.grahamShot.partIndex == 0)
            Anim_StartIndexed(*p, 4, 0, 0);
        else
        {
            s32 matrix = ObjAffine_Alloc(1);
            register s32 scale asm("r2"); /* FAKEMATCH: keeps 0x10000 from being hoisted out of the loop by loop.c */
            if (matrix < 0)
                success = 0;
            else
            {
                (*p)->affineIndex = matrix;
                (*p)->affine = 1;
                scale = 0x10000;
                ObjAffine_Set(scale, scale, scale, 0, 0, (*p)->affineIndex);
            }
            Anim_StartIndexed(*p, 3, 0, 1);
            (*p)->hidden = 1;
        }
        Anim_Update(*p);
        Entity_EnableBoxes(*p, Enemy_OnContact, 0);
        (*p++)->hit.flags |= 4;
        i++;
    }
     if (!success)
    {
    fail:
        p = parts;
        i = 2;
        do
        {
            if (*p != NULL) EntityDelete(*p);
            p++;
            i--;
        } while (i >= 0);
    }
    return success;
}

/* Delayed rotating projectile accelerates and leaves a randomized trail. */
#define ANGLE(e) ((e)->ext.raw.unk_500.part16.unk_500)
#define STEP(e) ((e)->ext.raw.unk_500.part16.unk_502)
/**
 * @brief Index 0 animates the firing effect; index 1 waits, then reveals
 * a rotating accelerating projectile with randomized trail effects.
 * Release the affine slot in its removal state.
 */
void GrahamShotPart_Update(struct EwramData_EntityData *entity)
{
    if((u8)entity->ext.grahamSpread.index==0)
    {
        if(entity->animDone)
        {
            PlaySong(0x10D);
            entity->deleteRequested=1;
            return;
        }
    }
    else
    {
        switch(entity->state)
        {
            case 0:
                entity->hidden=1;
                entity->timer=26;
                if(!entity->flipX)
                    entity->posX.part16.integer-=10;
                else
                    entity->posX.part16.integer+=10;
                entity->state++;
                entity->stage=0;
                entity->subState=0;
            case 1:
                if(--entity->timer==255)
                {
                    if(entity->flipX)
                    {
                        entity->velX.whole=0x2000;
                        entity->accelX.whole=0xE00;
                    }
                    else
                    {
                        entity->velX.whole=-0x2000;
                        entity->accelX.whole=-0xE00;
                    }
                    entity->state++;
                    entity->stage=0;
                    entity->subState=0;
                    entity->hidden=0;
                }
                break;
            case 2:
            {
                s16 zero;
                s32 step,scale,angle;
                entity->posX.whole+=entity->velX.whole;
                entity->posY.whole+=entity->velY.whole;
                entity->velX.whole+=entity->accelX.whole;
                entity->velY.whole+=entity->accelY.whole;
                zero=0;
                step=zero+0x2C0;
                STEP(entity)=step;
                if(!entity->flipX)
                    STEP(entity)=-step;
                ANGLE(entity)+=STEP(entity);
                angle=(s16)ANGLE(entity);
                scale=0x10000;
                ObjAffine_Set(angle,scale,scale,entity->flipX,0,entity->affineIndex);
                if((u8)(entity->timer%10)==0)
                {
                    s32 offset=(10-RandomNumberGenerator()%20)<<16;
                    s32 x=entity->posX.whole;
                    s32 y=entity->posY.whole-offset;
                    u8 *layer=&entity->spriteLayer;
                    Effect_SpawnAt(x,y,*layer-1);
                    Effect_SpawnRisingParticle(entity->posX.whole,entity->posY.whole-offset,*layer-1,0);
                }
                entity->timer++;
                if(EntityIsOutsideScreen(entity,4))
                {
                    entity->state=0xF0;
                    entity->stage=zero;
                    entity->subState=zero;
                }
                break;
            }
            case 0xF0:
                ObjAffine_Free(entity->affineIndex);
                entity->deleteRequested=1;
                return;
        }
    }
    if(!entity->hidden)
    {
        Anim_Update(entity);
        Enemy_CheckHitPlayer(entity);
    }
}

/**
 * @brief Create three hidden affine projectiles with indexed orbit phase
 * and optional reversed rotation. Return success, preserving original
 * allocation and failure-cleanup behavior.
 */
s32 Graham_SpawnSpiralProjectiles(struct EwramData_EntityData *owner, s32 reverse)
{
    u8 *animation = sGrahamSpriteSheet1;
    s32 success = 1;
    s32 i;
    struct EwramData_EntityData *parts[6];
    struct EwramData_EntityData **p;
    owner->ext.grahamSpiral.count = 3;
    i = 0;
    p = parts;
    while (i < owner->ext.grahamSpiral.count)
    {
        *p = Entity_CreateObject(GrahamSpiralProjectile_Update);
        if (!*p) { success = 0; break; }
        if (!Entity_LoadGraphics(*p, sGrahamGfx1, sGrahamShotPalette, 0, animation))
        { success = 0; goto fail; }
        ENTITY_PARENT_INDEX((*p)) = ENTITY_INDEX(owner);
        (*p)->spriteLayer = owner->spriteLayer - 1;
        (*p)->flipX = owner->flipX;
        (*p)->posX.whole = owner->posX.whole;
        (*p)->posY.whole = owner->posY.whole - 0x200000;
        (*p)->scrollWithBg = 1;
        (*p)->ext.grahamSpiral.count = owner->ext.grahamSpiral.count;
        (*p)->attack = 0x50;
        (*p)->ext.grahamSpiral.index = i;
        GRAHAM_SPIRAL_SPIN((*p)) = 0xAA;
        (*p)->ext.grahamSpiral.rotationSpeed = 0x2C0;
        if (reverse)
        {
            GRAHAM_SPIRAL_SPIN((*p)) = -GRAHAM_SPIRAL_SPIN((*p));
            (*p)->ext.grahamSpiral.rotationSpeed = -(*p)->ext.grahamSpiral.rotationSpeed;
        }
        {
            s32 matrix = ObjAffine_Alloc(1);
            if (matrix < 0)
                success = 0;
            else
            {
                (*p)->affineIndex = matrix;
                (*p)->affine = 1;
                ObjAffine_Set(0x10000, 0, 0, 0, 0, (*p)->affineIndex);
            }
        }
        Anim_StartIndexed(*p, 3, 0, 1);
        (*p)->hidden = 1;
        Anim_Update(*p);
        Entity_EnableBoxes(*p, Enemy_OnContact, 0);
        (*p++)->hit.flags |= 4;
        i++;
    }
    if (!success)
    {
    fail:
        i = 0;
        if (i < owner->ext.grahamSpiral.count)
        {
            p = parts;
            do
            {
                if (*p != NULL) EntityDelete(*p);
                p++;
                i++;
            } while (i < owner->ext.grahamSpiral.count);
        }
    }
    return success;
}

/* GrahamSpiralProjectile_Update: matched (index/sin/cos/radius shared at function level,
   SCALE <= 0, angle loaded before adding 0x4000). */
#define SCALE(e) (e->ext.grahamSpiral.scale)
#define RADIUS(e) (e->ext.grahamSpiral.radius)
#undef ANGLE
#define ANGLE(e) (e->ext.grahamSpiral.angle)

/**
 * @brief Grow an affine projectile, then move in an expanding spiral
 * relative to the owner with trail effects. Shrink on owner transition
 * or free the slot on removal. Preserve the nonzero initialization
 * test, parent test ordering and shared temporaries needed for matching.
 */
void GrahamSpiralProjectile_Update(struct EwramData_EntityData *entity)
{
    s32 index, sin, cos, radius;
    struct EwramData_EntityData *owner = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    if (entity->state != 0xF0 && entity->state != 4 && owner->state == 5 && owner != NULL)
    {
        entity->hit.flags |= 2;
        STATE(entity, 4);
    }
    {
        s32 state = entity->state;
        switch (state)
        {
        case 0:
            *(u8 *)(ENTITY_RENDER_FLAGS_ADDRESS(entity)) &= 0xDF;
            entity->timer = 0x1A;
            entity->state = 2;
            entity->stage = state;
            entity->subState = state;
        case 1:
            if (--entity->timer == 0xFF)
            {
                STATE(entity, 2);
                entity->hidden = 0;
            }
            break;
        case 2:
            /* The original tests for nonzero, so the first ordinary update
             * skips this block; do not reverse this condition. */
            if (entity->subState != 0)
            {

                RADIUS(entity) = 0x200000;
                ANGLE(entity) = 0;
                index = (u8)entity->ext.grahamSpiral.index;
                sin = Sin(index * (0x10000 / entity->ext.grahamSpiral.count));
                cos = Sin(index * (0x10000 / entity->ext.grahamSpiral.count) + ({ s32 a_ = ANGLE(entity); a_ + 0x4000; }));
                radius = GRAHAM_SPIRAL_RADIUS_PIXELS(entity);
                sin *= radius;
                cos *= radius;
                entity->posX.whole = owner->posX.whole + cos;
                entity->posY.whole = owner->posY.whole + ({ s32 t = sin - 0x400000; t; }) + 0x200000;
                entity->subState++;
            }
            if (SCALE(entity) <= 0xFFFF) SCALE(entity) += 0x800;
            ObjAffine_Set(entity->ext.grahamSpiral.rotation, SCALE(entity), SCALE(entity), entity->flipX, 0, entity->affineIndex);
            if (SCALE(entity) == 0x10000)
            {
                STATE(entity, 3);
                PlaySong(0x10D);
                entity->timer = 0;
            }
            break;
        case 3:
            {

                index = (u8)entity->ext.grahamSpiral.index;
                sin = Sin(index * (0x10000 / entity->ext.grahamSpiral.count) + ANGLE(entity));
                cos = Sin(index * (0x10000 / entity->ext.grahamSpiral.count) + ({ s32 a_ = ANGLE(entity); a_ + 0x4000; }));
                radius = RADIUS(entity) >> 16;
                sin *= radius;
                cos *= radius;
                entity->posX.whole = owner->posX.whole + cos;
                entity->posY.whole = sin + owner->posY.whole - 0x200000;
                RADIUS(entity) += 0x8000;
                ANGLE(entity) += GRAHAM_SPIRAL_SPIN(entity);
                if (SCALE(entity) <= 0xFFFF) SCALE(entity) += 0x800;
                entity->ext.grahamSpiral.rotation += entity->ext.grahamSpiral.rotationSpeed;
                ObjAffine_Set(entity->ext.grahamSpiral.rotation, 0x10000, 0x10000, entity->flipX, 0, entity->affineIndex);
                if (EntityIsOutsideScreen(entity, 4)) STATE(entity, 0xF0);
                if (!(entity->timer & 0xF) && entity->timer > 0x20)
                {
                    s32 offset = (10 - RandomNumberGenerator() % 20) << 16;
                    Effect_SpawnAt(entity->posX.whole, entity->posY.whole - offset, entity->spriteLayer - 1);
                    Effect_SpawnRisingParticle(entity->posX.whole, entity->posY.whole - offset, entity->spriteLayer - 1, 0);
                }
                entity->timer++;
            }
            break;
        case 4:
            if (entity->timer++ & 2) entity->palette = 4;
            else entity->palette = entity->basePalette;
            SCALE(entity) -= 0x800;
            if (SCALE(entity) >= 0)
            {
                if (SCALE(entity) <= 0) STATE(entity, 0xF0);
                ObjAffine_Set(entity->ext.grahamSpiral.rotation, SCALE(entity), SCALE(entity), entity->flipX, 0, entity->affineIndex);
            }
            break;
        case 0xF0:
            ObjAffine_Free(entity->affineIndex);
            entity->deleteRequested = 1;
            return;
        }
    }
    Anim_Update(entity);
    if (!entity->hidden && entity->state > 2 && entity->state != 4)
        Enemy_CheckHitPlayer(entity);
}

/* Burst of particles in timed waves from a 4x15 offset table (BossDefeat_EmitParticleWavesUnused).
 * The timer is read through volatile like the original (fresh ldrb). */
/**
 * @brief Unreferenced defeat effect next to GrahamBreakupFragment_Update: emit
 * BossDefeatParticle_Update particles in timed waves from the 4x15
 * offset table sBossDefeatBurstOffsets.
 */
void BossDefeat_EmitParticleWavesUnused(struct EwramData_EntityData *entity)
{
    struct BurstOffset offsets[4][15];
    memcpy(offsets, sBossDefeatBurstOffsets, sizeof(offsets));
    switch (entity->state)
    {
    case 0:
        entity->timer = 8;
        entity->accelX.whole = 0x20000;
        entity->state++;
    case 1:
        if (entity->timer == 0)
        {
            entity->timer = 0x80;
            entity->state++;
            break;
        }
        entity->timer--;
        break;
    case 2:
        if (!(entity->timer % 32))
        {
            BgPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0, 0x10000, 0x20);
            ObjPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0x4000, 0x10000, 0x10);
        }
        {
            u8 timer = *(vu8 *)&entity->timer;
            if (timer == 0)
            {
                entity->timer = 0x40;
                entity->state++;
                break;
            }
            if ((u8)((u32)*(vu8 *)&entity->timer % 3) == 0 && entity->stage <= 14)
            {
                s32 row, side;
                for (row = 0; row <= 3; row++)
                {
                    if (offsets[row][entity->stage].x != 0xFF)
                    {
                        for (side = 0; side <= 1; side++)
                        {
                            struct EwramData_EntityData *effect;
                            if (row == 0 && side == 0) continue;
                            effect = EntityCreateInRange(0x49, 0xCF, BossDefeatParticle_Update);
                            if (effect != NULL)
                            {
                                effect->scrollWithBg = 1;
                                if (side == 0)
                                    effect->posX.whole = entity->posX.whole + (offsets[row][entity->stage].x << 19);
                                else
                                    effect->posX.whole = entity->posX.whole - (offsets[row][entity->stage].x << 19);
                                effect->posY.whole = entity->posY.whole + (offsets[row][entity->stage].y << 19);
                                effect->velY.whole = 0x4000;
                                effect->accelY.whole -= 0x2000;
                                effect->spriteLayer = 0x11;
                            }
                        }
                    }
                }
                entity->stage++;
                timer = entity->timer;
            }
            entity->accelX.whole -= 0x600;
            entity->timer = timer - 1;
        }
        break;
    case 3:
        if (entity->timer == 0)
        {
            s32 col, row, side;
            for (col = 0; col <= 14; col++)
            {
                for (row = 0; row <= 3; row++)
                {
                    struct BurstOffset *offset;
                    if (((struct BurstOffset *)((u8 *)offsets + (row * 15 * 4 + col * 4)))->x == 0xFF) continue;
                    side = 0;
                    offset = (struct BurstOffset *)((u8 *)offsets + (row * 15 * 4 + col * 4));
                    for (; side <= 1; side++)
                    {
                        struct EwramData_EntityData *effect;
                        if (row == 0 && side == 0) continue;
                        effect = EntityCreateInRange(0x49, 0xCF, BossDefeatParticle_Update);
                        if (effect != NULL)
                        {
                            effect->scrollWithBg = 1;
                            if (side == 0) effect->posX.whole = entity->posX.whole + (offset->x << 19);
                            else effect->posX.whole = entity->posX.whole - (offset->x << 19);
                            effect->posY.whole = entity->posY.whole + (offset->y << 19);
                            effect->velY.whole = 0x4000;
                            effect->accelY.whole -= 0x2000;
                            effect->spriteLayer = 0x11;
                        }
                    }
                }
            }
            entity->timer = 0x40;
            entity->state++;
        }
        else entity->timer--;
        break;
    case 4:
        if (entity->timer == 0)
        {
            BossDefeat_RestoreEffectTiles();
            EntityDelete(entity);
            return;
        }
        entity->timer--;
        break;
    }
    if (!entity->roomActive) BossDefeat_RestoreEffectTiles();
}

/* A timed sine-shaped sweep emits falling particles during the defeat effect. */
/**
 * @brief Unreferenced defeat effect next to GrahamBreakupFragment_Update: a
 * timed sine-shaped sweep that emits falling BossDefeatParticle_Update
 * particles.
 */
void BossDefeat_EmitSweepParticlesUnused(struct EwramData_EntityData *entity)
{
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
                goto tick;
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
            BossDefeat_RestoreEffectTiles();
            EntityDelete(entity);
            return;
    }
    if(!entity->roomActive)
        BossDefeat_RestoreEffectTiles();
}

/* Delayed fragments become visible, accelerate downward and finish animating. */
/**
 * @brief Wait for a random delay while hidden, then reveal, move under
 * acceleration and delete when the fragment animation completes.
 */
void GrahamBreakupFragment_Update(struct EwramData_EntityData *entity)
{
    switch(entity->state)
    {
        case 0:
        {
            s32 graphics=ObjGfx_Find(sCommonObjectGfx);
            Sprite_Attach(entity,sDefeatParticleSpriteSheet,graphics,6);
            Anim_StartIndexed(entity,0,0,0);
            entity->drawFunc=Sprite_DrawEntity;
            entity->spriteLayer=16;
            entity->objMode=1;
            entity->state++;
            entity->timer=RandomNumberGenerator()%32;
            entity->hidden=1;
        }
        case 1:
            if(--entity->timer==255)
            {
                entity->hidden=0;
                entity->state++;
            }
            break;
        case 2:
            EntityAccelerateAndMove(entity,0x80000);
            if(Anim_Update(entity)==3)
                EntityDelete(entity);
            break;
    }
}

/* Mirrored fragment positions form the expanding breakup effect. */
/**
 * @brief Wait, emit mirrored fragments from the 4-by-15 point table with
 * palette flashes, then run the existing effect cleanup callback
 * and remove after its final countdown.
 */
void GrahamBreakup_Update(struct EwramData_EntityData *entity)
{
    struct FragmentPoint positions[4][15];
    memcpy(positions,sGrahamBreakupFragmentPoints,sizeof positions);
    switch(entity->state)
    {
        case 0:
            entity->timer=8;
            entity->accelX.whole=0x20000;
            entity->state++;
        case 1:
            if(entity->timer!=0) goto tick;
            entity->timer=128;
            entity->state+=2;
            break;
        case 2:
        case 3:
        {
            s32 column,row,mirror;
            if((entity->timer % 32)==0)
            {
                BgPalette_StartColorFade(31,31,31,0,0x10000,32);
                ObjPalette_StartColorFade(31,31,31,0x4000,0x10000,16);
            }
            for(column=0;column<15;column++)
            {
                for(row=0;row<4;row++)
                {
                    if(positions[row][column].x!=255)
                    {
                        for(mirror=0;mirror<2;mirror++)
                        {
                            if(row!=0 || mirror!=0)
                            {
                                struct EwramData_EntityData *particle=EntityCreateInRange(0x49,0xCF,GrahamBreakupFragment_Update);
                                if(particle)
                                {
                                    particle->scrollWithBg=1;
                                    if(mirror==0)
                                        particle->posX.whole=entity->posX.whole+(s32)((u32)positions[row][column].x<<19);
                                    else
                                        particle->posX.whole=entity->posX.whole-(s32)((u32)positions[row][column].x<<19);
                                    particle->posY.whole=entity->posY.whole+(s32)((u32)positions[row][column].y<<19);
                                    particle->velY.whole=0x4000;
                                    particle->accelY.whole-=0x2000;
                                    particle->spriteLayer=17;
                                }
                            }
                        }
                    }
                }
            }
            entity->timer=64;
            entity->state++;
            break;
        }
        case 4:
            if(entity->timer!=0) goto tick;
            BossDefeat_RestoreEffectTiles();
            EntityDelete(entity);
            return;
tick:
            entity->timer--;
            break;
    }
    if(!entity->roomActive)
        BossDefeat_RestoreEffectTiles();
}

/* Dispatch Graham's scene, combat, defeat and special presentation states. */
void EnemyGrahamUpdate(struct EwramData_EntityData *entity)
{
    s32 state=entity->state;
    if(state==1)
        Graham_UpdateIntro(entity);
    else if(state==2)
        Graham_UpdateState2(entity);
    else if(state==0xF0)
        Graham_UpdateDefeatScene(entity);
    else if(state==0xD0)
        Graham_UpdateBestiaryPreview(entity);
    /* Preserve the separate state read in the original dispatch. */
    else if(*(volatile u8 *)&entity->state==3) /* FAKEMATCH: volatile read forces the reload of the original */
        Graham_UpdatePhaseMonitor(entity);
}

/* Graham root callbacks and linked combat damage helpers.
 * EnemyGrahamUpdate is now matching C. */

/**
 * @brief Root state 2 callback; intentionally performs no actions.
 */
void Graham_UpdateState2(struct EwramData_EntityData *entity)
{
}

/**
 * @brief Root bestiary state 0xD0 callback; intentionally performs no
 * actions. Linked presentation entities update independently.
 */
void Graham_UpdateBestiaryPreview(struct EwramData_EntityData *entity)
{
}

/**
 * @brief Apply enemy damage; on defeat mark flag 2 and select state 0xE0
 * with substate counters reset.
 */
void GrahamCombatBody_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    if (Enemy_TakeDamage(entity, attacker, hit))
    {
        entity->state = 0xE0;
        entity->stage = 0;
        entity->subState = 0;
        entity->enemyFlags |= 2;
    }
}

/**
 * @brief Apply the shared linked-part damage helper; on defeat select
 * state 0xF0 with substate counters reset.
 */
void GrahamAttacker_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    if (GrahamPart_TakeDamage(entity, attacker, hit))
    {
        entity->state = 0xF0;
        entity->stage = 0;
        entity->subState = 0;
    }
}

/**
 * @brief Select removal state 0xF0 and reset its counters.
 */
void GrahamSpreadProjectile_OnHit(struct EwramData_EntityData *entity)
{
    entity->state = 0xF0;
    entity->stage = 0;
    entity->subState = 0;
}

/**
 * @brief Request deletion when the attack animation is done; otherwise
 * animate with hit boxes and check player contact.
 */
void GrahamAttackEffect_Update(struct EwramData_EntityData *entity)
{
    if (entity->animDone)
    {
        entity->deleteRequested = 1;
    }
    else
    {
        Entity_AnimateWithBoxes(entity);
        Enemy_CheckHitPlayer(entity);
    }
}

/**
 * @brief Resolve damage, apply hit flash, subtract signed HP damage and
 * show its number 64 pixels higher while restoring full position values.
 * Return nonzero when HP is exhausted. Shared by linked combat entities.
 */
s32 GrahamPart_TakeDamage(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    s32 damage = Hit_ResolveDamage(entity, attacker, hit, (s16)Player_CalculateAttackPower(hit[2]));
    s32 oldX;
    s32 oldY;

    entity->hitFlashTimer = 8;
    {
        u16 hp = entity->hp;
        damage = (damage << 16) >> 16;
        entity->hp = hp - damage;
    }
    oldX = entity->posX.whole;
    oldY = entity->posY.whole;
    entity->posY.whole = oldY - 0x400000;
    Entity_ShowDamage(entity, (s16)damage, 0);
    entity->posX.whole = oldX;
    entity->posY.whole = oldY;
    if ((s16)entity->hp <= 0)
        return 1;
    return 0;
}

/* Five cumulative timer thresholds select one of two palette sequences. */
static inline s32 AfterThreshold(s32 index) { return index+1; }
/**
 * @brief Use five cumulative timer thresholds to load one of two palette
 * sequences selected by kind. Advance the substate and return nonzero
 * after the final step; preserve the byte timer comparison and indexing.
 */
s32 GrahamPart_UpdatePaletteSequence(struct EwramData_EntityData *entity,s32 kind)
{
    u8 steps[15];
    s32 finished=0;
    s32 timer;
    s32 index;
    memcpy(steps,sGrahamPartSteps,15);
    timer=entity->timer++;
    index=entity->subState*3;
    if((u8)timer>=steps[index])
    {
        s32 next;
        ObjPalette_LoadRange(sGrahamPalette2,steps[kind+AfterThreshold(index)],1,entity->palette);
        next=entity->subState+1;
        if(next<=4)
            entity->subState=next;
        else
            finished=1;
    }
    return finished;
}

/* Graham_SpawnBreakupEffect. */

/**
 * @brief Create the breakup controller at the supplied fixed-point
 * position and begin palette fades plus the existing display effect
 * setup. Do nothing further if controller allocation fails.
 */
void Graham_SpawnBreakupEffect(s32 x, s32 y)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, GrahamBreakup_Update);

    if (child != NULL)
    {
        BgPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0, 0x10000, 0x20);
        ObjPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0x4000, 0x10000, 0x10);
        BossDefeat_LoadEffectTiles();
        child->posX.whole = x;
        child->posY.whole = y;
        child->scrollWithBg = 1;
        child->roomActive = 1;
    }
}

/* Remove this effect and spawn a follower above the current player. */
/**
 * @brief Request projectile deletion and create the animation-12 effect
 * linked to the current player at a fixed upward offset, if allocation
 * and the player pointer permit it.
 */
void GrahamSceneProjectile_OnContact(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *player;
    struct EwramData_EntityData *effect;
    s32 kind, offset;
    s32 zero;
    entity->deleteRequested = 1;
    player = gEwramData->playerState.player;
    zero = 0;
    kind = 12;
    offset = -0x100000;
    if (player != 0)
    {
        effect = EntityCreateInRange(0x49, 0xCF, GrahamPlayerEffect_Update);
        if (effect != 0)
        {
            effect->ext.particleAttach.anim = kind;
            effect->ext.particleAttach.parent = player;
            effect->palette = 5;
            effect->accelX.whole = zero;
            effect->accelY.whole = offset;
            effect->posX.whole = player->posX.whole;
            effect->posY.whole = player->posY.whole + offset;
            effect->spriteLayer = kind;
        }
    }
}

/* Effects 0x080D6060..0x080D6177 (GrahamDeathSpark_Update from the template RisingEffect_Update). */

/**
 * @brief Attach shared effect graphics, play animation 1 and rise;
 * delete on animation completion.
 */
void GrahamDeathSpark_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
        case 0:
        {
            s32 gfx = ObjGfx_Find(sCommonObjectGfx);

            Sprite_Attach(entity, sCommonObjectSpriteSheet, gfx, 7);
            Anim_StartIndexed(entity, 1, 0, 0);
            entity->scrollWithBg = 1;
            entity->drawFunc = Sprite_DrawEntity;
            entity->state++;
            break;
        }
        case 1:
            entity->posY.whole -= 0x8000;
            break;
    }
    if (Anim_Update(entity) == 3)
        EntityDelete(entity);
}

/**
 * @brief Animate while visible. When the animation is done, advance to
 * the next state, hide and request deletion.
 */
void GrahamTransitionPart_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
        case 0:
            if (entity->animDone)
            {
                entity->state = 1;
                entity->stage = 0;
                entity->subState = 0;
            }
            break;
        case 1:
            entity->hidden = 1;
            entity->deleteRequested = 1;
            break;
    }
    if (!entity->hidden)
        Anim_Update(entity);
}

/**
 * @brief Apply stored damage with effect type 2 and zero flags, then
 * select removal state 0xF0 with counters reset.
 */
void GrahamSpreadProjectile_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attributes = 2;
    contactDamage.unused = 0;
    contactDamage.attack = entity->attack;
    contactDamage.reaction = 0;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
    entity->state = 0xF0;
    entity->stage = 0;
    entity->subState = 0;
}
