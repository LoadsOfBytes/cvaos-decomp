/*
 * Harpy: body, feathers and death feathers. The last part of the range also holds the entry points of Siren.
 */
#include "core.h"
#include "graphics.h"
#include "structs/ewram.h"
#include "sound.h"
#include "entity_util.h"
#include "player_core.h"
#include "structs/entity_render_flags.h"
#include "gameplay.h"

/* Harpy setup randomizes its animation phase before enabling movement. */
extern u8 Harpy_LoadGraphics(struct EwramData_EntityData *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *);
extern void Hit_EnableContact(struct EwramData_EntityData *,s32,void *,void *);
void Harpy_DamagePlayerOnContact(struct EwramData_EntityData *entity);
void Harpy_OnHit(struct EwramData_EntityData *entity,struct EwramData_EntityData *attacker,u16 *info);
void EnemyHarpyCreate(struct EwramData_EntityData *entity)
{
    u8 demo;
    if(!Harpy_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity,0,0,1);
    entity->animTimer=RandomNumberGenerator()%16;
    entity->animStep=RandomNumberGenerator()%5;
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    demo=*(u8 *)((u8 *)gEwramData+0x8E) & 0x40;
    if(demo)
        entity->state=4;
    else
    {
        entity->spriteLayer=3;
        Hit_EnableContact(entity,8,Harpy_DamagePlayerOnContact,Harpy_OnHit);
        Entity_UpdateBoxes(entity);
        entity->state=demo;
        entity->subState=10;
        entity->stage=demo;
    }
}

/* Ground waiting, animation-timed wingbeats and the transition into flight. */
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern s32 Entity_GetTargetDistanceX(struct EwramData_EntityData *);
extern s32 Target_GetPixelY(struct EwramData_EntityData *);
extern s32 Enemy_IsDamaged(struct EwramData_EntityData *);
extern void EntityDampVelocityX(struct EwramData_EntityData *,s32);
#define FRAME_PAIR(e) (*(u16 *)&(e)->animStep)
static inline s32 TargetDistanceX(struct EwramData_EntityData *entity)
{
    s32 distance=(s16)Entity_GetTargetDistanceX(entity);
    if(distance<0) distance=-distance;
    return distance;
}
/**
 * @brief Approach state machine (state 0): face the target and wait until it is
 * within 89 px horizontally or the Harpy is damaged, then fly around it and
 * start the attack (state 1).
 */
void Harpy_UpdateApproach(struct EwramData_EntityData *entity)
{
    s32 state=entity->subState;
    switch(state)
    {
        case 0:
            if(FRAME_PAIR(entity)==0)
                entity->flipX=Entity_IsTargetRight(entity);
            if(TargetDistanceX(entity)<=89 || Enemy_IsDamaged(entity))
            {
                entity->subState=10;
                entity->stage=state;
                entity->timer=state;
                entity->velX.whole=state;
                entity->velY.whole=state;
                entity->accelX.whole=state;
                entity->accelY.whole=state;
            }
            break;
        case 1:
            entity->flipX=Entity_IsTargetRight(entity);
            EntityDampVelocityX(entity,12);
            EntityAccelerateAndMove(entity,0x80000);
            if(FRAME_PAIR(entity)==1)
            {
                u32 beat=entity->stage;
                if((u8)beat==0)
                {
                    entity->velY.whole=-0x20000;
                    entity->timer++;
                    entity->stage=beat+1;
                    PlaySong(0x8B);
                }
                else if((u8)beat==1)
                {
                    entity->velY.whole-=0x20000;
                    entity->timer++;
                    entity->stage=beat+1;
                    PlaySong(0x8B);
                }
            }
            if(entity->timer>1 && entity->velY.whole>0)
            {
                entity->flipX=Entity_IsTargetRight(entity);
                entity->state=1;
                entity->subState=0;
            }
            break;
        case 10:
        {
            u16 *frame;
            u32 mask=0xFFFF;
            u32 current=(u16)(FRAME_PAIR(entity) & mask);
            frame=(u16 *)&entity->animStep;
            if(!current)
                entity->flipX=Entity_IsTargetRight(entity);
            if((mask &= *frame)==1)
            {
                s32 targetY;
                s16 *height;
                s32 zero,one;
                s32 limit;
                if(entity->flipX)
                    entity->velX.whole=0x14000;
                else
                    entity->velX.whole=-0x14000;
                targetY=(s16)Target_GetPixelY(entity);
                limit=entity->posY.part16.integer;
                limit+=32;
                targetY-=limit;
                height=&entity->posY.part16.integer;
                if(targetY<0)
                    entity->velY.whole=-0x1A000;
                else
                    entity->velY.whole=-0xC000;
                entity->accelY.whole=0x1000;
                zero=0;
                one=1;
                entity->stage=one;
                if(TargetDistanceX(entity)<=80 &&
                   (s16)Target_GetPixelY(entity)-*height>31 &&
                   (s16)Target_GetPixelY(entity)-*height<=64)
                {
                    Anim_StartIndexed(entity,1,0,1);
                    entity->subState=one;
                    entity->stage=zero;
                    entity->timer=zero;
                    entity->velY.whole=-0x20000;
                    entity->accelY.whole=0x1800;
                    return;
                }
                PlaySong(0x8B);
            }
            if(entity->stage)
            {
                if(*(u8 *)frame==1)
                    entity->accelY.whole=0;
                else
                    entity->accelY.whole=0x1000;
            }
            EntityDampVelocityX(entity,32);
            EntityAccelerateAndMove(entity,0x80000);
            break;
        }
    }
}

/* Aerial approach, braking, volleys and the return to ordinary flight. */
extern s32 Target_GetX(struct EwramData_EntityData *);
extern s32 Target_GetY(struct EwramData_EntityData *);
extern s32 Entity_AimVelocityAtPoint(struct EwramData_EntityData *,s32,s32,s32);
extern s32 Target_IsWithinDistance(struct EwramData_EntityData *,s32);
extern void EntityMove(struct EwramData_EntityData *);
extern void Harpy_ThrowFeather(struct EwramData_EntityData *,s32);
extern void Harpy_ThrowWavyFeather(struct EwramData_EntityData *,s32);
static inline s32 Magnitude(s32 value)
{
    if(value<0) value=-value;
    return value;
}
/**
 * @brief Attack state machine (state 1): wind up (animation 2, sound 0x87), then
 * throw a fan of feathers - three straight ones (Harpy_ThrowFeather kinds
 * 0 / 2 / 4, sound 0x89), three wavy ones (Harpy_ThrowWavyFeather) or two
 * straight ones (kinds 1 / 3); then back to state 0.
 */
void Harpy_UpdateAttack(struct EwramData_EntityData *entity)
{
    s32 song;
    switch(entity->subState)
    {
        case 0:
        {
            u32 random=RandomNumberGenerator() & 1;
            if(random)
            {
                struct BgLayerState *camera;
                Anim_StartIndexed(entity,2,0,0);
                PlaySong(0x87);
                entity->subState++;
                camera=&gEwramData->bgInfo[1];
                entity->ext.harpy.diveX=Target_GetX(entity)+camera->xPos.whole;
                entity->ext.harpy.diveY=Target_GetY(entity)+camera->yPos.whole-0x180000;
                entity->ext.harpy.startY=entity->posY.whole;
            }
            else
            {
                Anim_StartIndexed(entity,1,0,1);
                entity->subState=10;
                entity->stage=random;
                entity->timer=random;
            }
            break;
        }
        case 1:
            if(Entity_AimVelocityAtPoint(entity,entity->ext.harpy.diveX,entity->ext.harpy.diveY,0x30000))
            {
                entity->subState++;
                Anim_StartIndexed(entity,1,0,1);
                if(entity->velY.whole>0) entity->accelY.whole=-0x1000;
                else entity->accelY.whole=0x100;
                EntityDampVelocityY(entity,8);
                entity->timer=0;
            }
            EntityMove(entity);
            break;
        case 2:
        {
            s32 result;
            EntityAccelerateAndMove(entity,0x30000);
            EntityDampVelocityX(entity,64);
            result=Target_IsWithinDistance(entity,0x780000);
            if(!result)
            {
                entity->subState++;
                entity->accelY.whole=result;
            }
            else if(++entity->timer>59)
            {
                entity->subState++;
                entity->accelY.whole=0;
            }
            break;
        }
        case 3:
            entity->flipX=Entity_IsTargetRight(entity);
            EntityDampVelocityX(entity,8);
            EntityDampVelocityY(entity,8);
            EntityMove(entity);
            if(Magnitude(entity->velX.whole)<=0x7FFF && Magnitude(entity->velY.whole)<=0x7FFF)
            {
                entity->state=0;
                entity->subState=10;
                entity->stage=0;
                Anim_StartIndexed(entity,0,0,1);
            }
            break;
        case 10:
            if(FRAME_PAIR(entity)==1 && ++entity->timer>3)
            {
                if(entity->enemyId==56)
                {
                    Harpy_ThrowFeather(entity,0);
                    Harpy_ThrowFeather(entity,2);
                    Harpy_ThrowFeather(entity,4);
                    PlaySong(0x89);
                    goto advance;
                }
                Harpy_ThrowWavyFeather(entity,0);
                Harpy_ThrowWavyFeather(entity,1);
                Harpy_ThrowWavyFeather(entity,2);
                song=0x137;
                goto play_song;
            }
            break;
        case 11:
            if(FRAME_PAIR(entity)==1)
            {
                if(entity->enemyId==56)
                {
                    Harpy_ThrowFeather(entity,1);
                    Harpy_ThrowFeather(entity,3);
                    song=0x89;
play_song:
                    PlaySong(song);
                }
advance:
                entity->subState++;
                entity->timer=0;
            }
            break;
        case 12:
            if(FRAME_PAIR(entity)==1)
            {
                s32 zero;
                u8 timer=++entity->timer;
                zero=0;
                if(timer>2)
                {
                    Anim_StartIndexed(entity,0,0,1);
                    entity->state=zero;
                    entity->subState=10;
                    entity->stage=zero;
                }
            }
            break;
    }
}

/* Retreat from a hit, turn back toward the target and resume flight. */
/**
 * @brief Recoil state machine (state 3): fly up and away from the target, slowing
 * down, then resume.
 */
void Harpy_UpdateRecoil(struct EwramData_EntityData *entity)
{
    switch(entity->subState)
    {
        case 0:
            entity->flipX=Entity_IsTargetRight(entity)^1;
            if(entity->flipX)
                entity->velX.whole=0x30000;
            else
                entity->velX.whole=-0x30000;
            entity->velY.whole=-0x30000;
            entity->subState++;
            break;
        case 1:
            if(entity->velY.whole>-0x14000)
            {
                entity->flipX=Entity_IsTargetRight(entity);
                Anim_StartIndexed(entity,1,0,1);
                entity->subState++;
            }
        case 2:
            EntityDampVelocityX(entity,12);
            EntityDampVelocityY(entity,12);
            EntityMove(entity);
            if(entity->velY.whole>-0x2000)
            {
                entity->state=0;
                entity->subState=10;
                entity->stage=0;
                Anim_StartIndexed(entity,0,0,1);
            }
            break;
    }
}

/* Lethal hits shed four effects; selected attack kinds force a recovery state. */
extern s32 Hit_ResolveDamage(struct EwramData_EntityData *,struct EwramData_EntityData *,u16 *,s32);
extern void Entity_ShowDamage(struct EwramData_EntityData *,s32,s32);
extern void Enemy_DropRewards(struct EwramData_EntityData *);
extern void Harpy_SpawnDeathFeather(struct EwramData_EntityData *,s32,s32);
void Harpy_OnHit(struct EwramData_EntityData *entity,struct EwramData_EntityData *attacker,u16 *info)
{
    struct EntityHit *attack=&attacker->hit;
    s32 damage=Hit_ResolveDamage(entity,attacker,info,(s16)Player_CalculateAttackPower(info[2]));
    s32 zero;
    entity->hitFlashTimer=8;
    {
        u16 hp=entity->hp;
        damage=(u32)damage<<16;
        damage>>=16;
        hp-=damage;
        zero=0;
        entity->hp=hp;
    }
    Entity_ShowDamage(entity,damage,0);
    if((s16)entity->hp<=0)
    {
        Enemy_DropRewards(entity);
        PlaySong(0x1C3);
        Anim_StartIndexed(entity,3,0,1);
        entity->palette=entity->basePalette;
        entity->timer=40;
        entity->state=2;
        entity->subState=zero;
        Harpy_SpawnDeathFeather(entity,entity->posX.whole,entity->posY.whole);
        Harpy_SpawnDeathFeather(entity,entity->posX.whole,entity->posY.whole);
        Harpy_SpawnDeathFeather(entity,entity->posX.whole,entity->posY.whole);
        Harpy_SpawnDeathFeather(entity,entity->posX.whole,entity->posY.whole);
    }
    else if((u8)(attack->type-3)<=1)
    {
        Anim_StartIndexed(entity,3,0,1);
        Anim_Update(entity);
        Entity_UpdateBoxes(entity);
        entity->state=3;
        entity->subState=zero;
    }
}

/* Defeat fragments inherit their owner's sprite palette and scatter randomly. */
extern void HarpyDeathFeather_Update(struct EwramData_EntityData *);
extern u8 sHarpyGfx[],sHarpySpriteSheet[];
/**
 * @brief Spawn a loose feather at (x, y) with a random velocity (four on death).
 */
void Harpy_SpawnDeathFeather(struct EwramData_EntityData *owner,s32 x,s32 y)
{
    struct EwramData_EntityData *fragment=EntityCreateInRange(0x49,0xCF,(u32 *)HarpyDeathFeather_Update);
    if(fragment)
    {
        s32 zero;
        s32 graphics;
        fragment->posX.whole=x;
        fragment->posY.whole=y;
        fragment->velX.whole=(RandomNumberGenerator() & 0x3FFFF)-0x20000;
        fragment->velY.whole=(RandomNumberGenerator() & 0x3FFFF)-0x20000;
        zero=0;
        fragment->accelX.whole=zero;
        fragment->accelY.whole=zero;
        fragment->spriteLayer=owner->spriteLayer-1;
        graphics=ObjGfx_Find(sHarpyGfx);
        Sprite_Attach(fragment,sHarpySpriteSheet,graphics,owner->basePalette);
        fragment->animFrame=8;
        fragment->flipX=RandomNumberGenerator()%2;
        fragment->scrollWithBg=1;
        fragment->drawFunc=(u32 *)Sprite_DrawEntity;
        fragment->timer=zero;
    }
}

/* Fragments slow down, start falling, blink and expire. */
extern s32 Entity_GetVisibilityPattern(s32);
/**
 * @brief Loose feather: slow down, then drift down for 40 frames while blinking
 * out.
 */
void HarpyDeathFeather_Update(struct EwramData_EntityData *entity)
{
    s32 state=entity->state;
    switch(state)
    {
        case 0:
            EntityDampVelocityX(entity,12);
            EntityDampVelocityY(entity,12);
            EntityMove(entity);
            if(Magnitude(entity->velX.whole)<=0xFFF && Magnitude(entity->velY.whole)<=0xFFF)
            {
                entity->state++;
                entity->animFrame=8;
                entity->velX.whole=state;
                entity->velY.whole=0x4000;
            }
            break;
        case 1:
            EntityMove(entity);
            if(++entity->timer>39)
                EntityDelete(entity);
            entity->hidden=Entity_GetVisibilityPattern(40-entity->timer);
            break;
    }
}

/* The caller selects one of five valid directions (0..4) for a spread shot. */
extern void HarpyFeather_Update(struct EwramData_EntityData *);
extern u8 HarpyFeather_LoadGraphics(struct EwramData_EntityData *,struct EwramData_EntityData *);
extern void HarpyFeather_EnableHitBoxes(struct EwramData_EntityData *);
extern void EntitySetVelocityFromAngle(struct EwramData_EntityData *,s32,s32);
void Harpy_ThrowFeather(struct EwramData_EntityData *owner,s32 kind)
{
    struct EwramData_EntityData *projectile=EntityCreateInRange(0x1B,0x48,(u32 *)HarpyFeather_Update);
    if(projectile)
    {
        u32 angle;
        u32 flipped;
        if(!HarpyFeather_LoadGraphics(owner,projectile))
            EntityDelete(projectile);
        projectile->scrollWithBg=1;
        HarpyFeather_EnableHitBoxes(projectile);
        projectile->spriteLayer=owner->spriteLayer+1;
        ENTITY_CREATOR(projectile)=owner;
        projectile->flipX=owner->flipX;
        if(owner->flipX)
            projectile->posX.part16.integer=owner->posX.part16.integer+1;
        else
            projectile->posX.part16.integer=owner->posX.part16.integer-1;
        projectile->posY.part16.integer=owner->posY.part16.integer+2;
        switch(kind)
        {
            case 0:
                angle=0x4000;
                projectile->animFrame=12;
                break;
            case 1:
                flipped=*(u8 *)(ENTITY_RENDER_FLAGS_ADDRESS(projectile)) & 0x40;
                angle=0x5000;
                if(flipped) angle=0x3000;
                projectile->animFrame=11;
                break;
            case 2:
                flipped=*(u8 *)(ENTITY_RENDER_FLAGS_ADDRESS(projectile)) & 0x40;
                angle=0x6000;
                if(flipped) angle=0x2000;
                projectile->animFrame=10;
                break;
            case 3:
                flipped=*(u8 *)(ENTITY_RENDER_FLAGS_ADDRESS(projectile)) & 0x40;
                angle=0x7000;
                if(flipped) angle=0x1000;
                projectile->animFrame=9;
                break;
            case 4:
                flipped=*(u8 *)(ENTITY_RENDER_FLAGS_ADDRESS(projectile)) & 0x40;
                angle=0x8000;
                if(flipped) angle=0;
                projectile->animFrame=8;
                break;
        }
        projectile->ext.harpyFeather.angle=angle;
        EntitySetVelocityFromAngle(projectile,angle,0x30000);
        projectile->spawnIndex=owner->spawnIndex;
        projectile->basePalette=owner->basePalette;
        projectile->attack=owner->attack;
    }
}

/* The caller selects one of three valid directions (0..2) for a spread shot. */
extern void HarpyWavyFeather_Update(struct EwramData_EntityData *);
void Harpy_ThrowWavyFeather(struct EwramData_EntityData *owner,s32 kind)
{
    struct EwramData_EntityData *projectile=EntityCreateInRange(0x1B,0x48,(u32 *)HarpyWavyFeather_Update);
    if(projectile)
    {
        u32 angle;
        u32 flipped;
        if(!HarpyFeather_LoadGraphics(owner,projectile))
            EntityDelete(projectile);
        projectile->scrollWithBg=1;
        HarpyFeather_EnableHitBoxes(projectile);
        projectile->spriteLayer=owner->spriteLayer+1;
        ENTITY_CREATOR(projectile)=owner;
        projectile->flipX=owner->flipX;
        if(owner->flipX)
            projectile->posX.part16.integer=owner->posX.part16.integer+1;
        else
            projectile->posX.part16.integer=owner->posX.part16.integer-1;
        projectile->posY.part16.integer=owner->posY.part16.integer+2;
        switch(kind)
        {
            case 0:
                flipped=*(u8 *)(ENTITY_RENDER_FLAGS_ADDRESS(projectile)) & 0x40;
                angle=0x438E;
                if(flipped) angle=0x3C72;
                projectile->animFrame=14;
                break;
            case 1:
                flipped=*(u8 *)(ENTITY_RENDER_FLAGS_ADDRESS(projectile)) & 0x40;
                angle=0x6000;
                if(flipped) angle=0x2000;
                projectile->animFrame=13;
                break;
            case 2:
                flipped=*(u8 *)(ENTITY_RENDER_FLAGS_ADDRESS(projectile)) & 0x40;
                angle=0x7C72;
                if(flipped) angle=0x38E;
                projectile->animFrame=14;
                break;
        }
        projectile->ext.harpyFeather.angle=angle;
        if(projectile->flipX) projectile->timer=24;
        else projectile->timer=8;
        projectile->flipX=0;
        projectile->spawnIndex=owner->spawnIndex;
        projectile->basePalette=owner->basePalette;
        projectile->attack=owner->attack+(owner->attack>>4);
    }
}

/* The projectile's travel angle oscillates over a 32-frame cycle. */
extern void Hit_CheckWithPlayer(void *,struct EwramData_EntityData *);
void HarpyWavyFeather_Update(struct EwramData_EntityData *entity)
{
    s32 bounds[4];
    s32 state=entity->state;
    switch(state)
    {
        case 0:
        {
            u8 timer=++entity->timer;
            if(timer<=15)
                entity->ext.harpyFeather.angle+=0x400;
            else if(timer<=31)
            {
                entity->ext.harpyFeather.angle-=0x400;
                if(timer>30) entity->timer=state;
            }
            EntitySetVelocityFromAngle(entity,entity->ext.harpyFeather.angle,0x20000);
            EntityMove(entity);
            Hit_CheckWithPlayer(bounds,entity);
            if(!EntityIsOutsideScreen(entity,1)) break;
        }
        case 1:
            EntityDelete(entity);
            break;
        case 2:
            EntityDelete(entity);
            break;
    }
}

/* HarpyFeather_EnableHitBoxes (hit boxes). */
extern void HarpyFeather_DamagePlayerOnContact(struct EwramData_EntityData *entity);
void HarpyFeather_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);

/**
 * @brief 76244 | Activate box slot 8 with HarpyFeather_DamagePlayerOnContact / HarpyFeather_OnHit and
 * set both boxes to the bytes of 0x0606FDFD (x -3, y -3, w 6, h 6 - not a
 * ROM address); clear flags bits 2 and 1.
 */
void HarpyFeather_EnableHitBoxes(struct EwramData_EntityData *entity)
{
    u32 box = 0x0606FDFD;

    Hit_Enable(entity, 8, HarpyFeather_DamagePlayerOnContact, (s32)HarpyFeather_OnHit);
    HitBox_SetBody(entity, &box);
    HitBox_SetAttack(entity, &box);
    entity->hit.flags &= ~4;
    entity->hit.flags &= ~2;
}

/* Harpy and Siren share graphics but select different palette variants. */
extern s32 ObjPalette_FindOrLoadVariant(void *,s32);
extern u8 sHarpyGfx[],sHarpyPalette[],sHarpySpriteSheet[];
u8 Harpy_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 graphics=ObjGfx_LoadResource(sHarpyGfx,0);
    if(graphics<0) return 0;
    if(entity->enemyId==56)
        entity->basePalette=ObjPalette_FindOrLoadVariant(sHarpyPalette,0);
    else
        entity->basePalette=ObjPalette_FindOrLoadVariant(sHarpyPalette,1);
    Sprite_Attach(entity,sHarpySpriteSheet,graphics,entity->basePalette);
    entity->drawFunc=(u32 *)Sprite_DrawEntity;
    return 1;
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Harpy_DamagePlayerOnContact = Object28_OnPlayerContact */

/**
 * @brief 762F0 | Same as Object28_OnPlayerContact.
 */
void Harpy_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 0;
    contactDamage.unused = 0;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/* Harpy knockback and the small projectile's movement/cleanup. */
extern void Enemy_UpdateFragmentDeath(struct EwramData_EntityData *);
/**
 * @brief Death state (state 2): fall away from the target under gravity.
 */
void Harpy_UpdateDeath(struct EwramData_EntityData *entity)
{
    switch(entity->subState)
    {
        case 0:
            entity->flipX=Entity_IsTargetRight(entity);
            if(entity->flipX) entity->velX.whole=-0x10000;
            else entity->velX.whole=0x10000;
            entity->velY.whole=-0x20000;
            { s16 zero=0; entity->accelX.whole=zero; }
            entity->accelY.whole=0x2000;
            entity->subState++;
        case 1:
            EntityAccelerateAndMove(entity,0x80000);
            Enemy_UpdateFragmentDeath(entity);
            break;
    }
}
void HarpyFeather_Update(struct EwramData_EntityData *entity)
{
    s32 bounds[4];
    switch(entity->state)
    {
        case 0:
            EntityMove(entity);
            Hit_CheckWithPlayer(bounds,entity);
            if(!EntityIsOutsideScreen(entity,1)) break;
        case 1:
            EntityDelete(entity);
            break;
        case 2:
            EntityDelete(entity);
            break;
    }
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   HarpyFeather_LoadGraphics = MudmanShot_LoadGraphics */

extern u8 sHarpyGfx[];
extern u8 sHarpySpriteSheet[];

/**
 * @brief 763DC | Same as MudmanShot_LoadGraphics.
 */
u8 HarpyFeather_LoadGraphics(struct EwramData_EntityData *entity, struct EwramData_EntityData *projectile)
{
    s32 gfx = ObjGfx_Find(sHarpyGfx);

    if (gfx < 0)
        return 0;
    Sprite_Attach(projectile, sHarpySpriteSheet, gfx, entity->basePalette);
    Anim_StartIndexed(projectile, 5, 0, 1);
    Anim_Update(projectile);
    projectile->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/* Report a projectile hit at its current position and enter cleanup. */
void HarpyFeather_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage hit;
    s32 zero;
    hit.attack=entity->attack;
    zero=0;
    hit.attributes=0;
    hit.unused=0;
    hit.reaction=0;
    hit.sourceX=entity->posX.whole;
    hit.sourceY=entity->posY.whole;
    Player_ApplyContactDamage(&hit);
    entity->state=2;
    entity->subState=zero;
}

/* HarpyFeather_OnHit. */
extern u16 Player_CalculateAttackPower(u16 attackCategory);

/**
 * @brief 76464 | Hit: call Player_CalculateAttackPower on the damage value (result
 * unused) and go to state 1.
 */
void HarpyFeather_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    Player_CalculateAttackPower(hit[2]);
    entity->state = 1;
    entity->subState = 0;
}

/* Harpy/Siren dispatch share movement states and collision processing. */
extern s32 Enemy_PreUpdate(struct EwramData_EntityData *);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *);
void EnemyHarpyUpdate(struct EwramData_EntityData *entity)
{
    s32 scratch[4];
    if(Enemy_PreUpdate(entity)) return;
    switch(entity->state)
    {
        case 0: Harpy_UpdateApproach(entity); break;
        case 1: Harpy_UpdateAttack(entity); break;
        case 2: Harpy_UpdateDeath(entity); break;
        case 3: Harpy_UpdateRecoil(entity); break;
        case 4: Anim_Update(entity); return;
    }
    Entity_AnimateWithBoxes(entity);
    Enemy_CheckHitPlayer(entity);
}
void EnemySirenUpdate(struct EwramData_EntityData *entity)
{
    EnemyHarpyUpdate(entity);
}
void EnemySirenCreate(struct EwramData_EntityData *entity)
{
    EnemyHarpyCreate(entity);
}
