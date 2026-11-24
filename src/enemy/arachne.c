/*
 * Arachne: body, legs and effects.
 */
#include "core.h"
#include "player_core.h"
#include "graphics.h"
#include "entity_util.h"
#include "structs/ewram.h"
#include "gameplay.h"
#include "structs/entity_render_flags.h"
#include "entity_hit.h"
#include "sound.h"
#include <string.h>
#include "enemy_common.h"
#include "entity_common.h"
#include "hitbox.h"
#include "math_anim.h"
#include "music.h"
#include "obj_gfx.h"
#include "obj_palette.h"
#include "data/data_118B48.h"
#include "damage_numbers.h"
#include "entity_motion.h"

void Arachne_UpdateWalk(struct EwramData_EntityData *);
void Arachne_UpdateAttack(struct EwramData_EntityData *);
void Arachne_UpdateRecoil(struct EwramData_EntityData *);
void Arachne_UpdateDeath(struct EwramData_EntityData *);
void Arachne_UpdatePreview(struct EwramData_EntityData *entity);
u8 Arachne_LoadGraphics(struct EwramData_EntityData *entity);
void ArachneLegs_Update(struct EwramData_EntityData *entity);
void Arachne_DamagePlayerOnContact(struct EwramData_EntityData *entity);
void Arachne_OnHit(struct EwramData_EntityData *entity,struct EwramData_EntityData *attacker,u16 *info);
void Arachne_SpawnEffect(struct EwramData_EntityData *owner, s32 x, s32 y, s32 animation, s32 lifetime);
void ArachneEffect_Update(struct EwramData_EntityData *);
void ArachneBeam_DamagePlayerOnContact(struct EwramData_EntityData *);

/* Arachne dispatch and screen-dependent sound and collision updates. */
void EnemyArachneUpdate(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    if(entity->state!=2)
    {
        if(EntityIsOutsideScreen(entity,4)) { Sound_StopTracked(0x82); return; }
        if(Entity_IsInRect(entity,-16,0,272,176)) Player_TrackEntity(entity);
    }
    if(EntityIsOutsideScreen(entity,4)) entity->hidden=1;
    else entity->hidden=0;
    if(entity->state!=2) Enemy_UpdateHitFlash(entity);
    if(Entity_IsTimeStopped(entity) && entity->state!=2)
    {
        Hit_CheckWithPlayer(&result,entity);
        return;
    }
    if(entity->hitFlashTimer && entity->state!=3 && entity->state!=2)
    {
        Hit_CheckWithPlayer(&result,entity);
        return;
    }
    switch(entity->state)
    {
        case 0: Arachne_UpdateWalk(entity); break;
        case 1: Arachne_UpdateAttack(entity); break;
        case 3: Arachne_UpdateRecoil(entity); break;
        case 2: Arachne_UpdateDeath(entity); break;
        case 4: Arachne_UpdatePreview(entity); Anim_Update(entity); return;
    }
    Entity_AnimateWithBoxes(entity);
    Enemy_CheckHitPlayer(entity);
}

/**
 * @brief 799FC | Arachne create: six legs Entity_CreateObject(ArachneLegs_Update) with
 * graphics sArachneSpriteSheet, odd ones in front (palette +1, animFrame 0x19) and
 * even ones behind (0x15), leg index in unk_504 and velX (i / 2 * 7) << 16
 * as the spread; frozen time gives state 4, otherwise box slot 8
 * (Hit_EnableContact), Entity_UpdateBoxes / Entity_SnapToGround and state 0. A failed
 * allocation deletes it.
 */
void EnemyArachneCreate(struct EwramData_EntityData *entity)
{
    s32 i;

    if ((u8)Arachne_LoadGraphics(entity) == 0)
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    for (i = 5; i >= 0; i--)
    {
        struct EwramData_EntityData *leg = Entity_CreateObject(ArachneLegs_Update);
        s32 tiles;
        if (leg != NULL)
        {
        if (i & 1)
            leg->spriteLayer = entity->spriteLayer + 1;
        else
            leg->spriteLayer = entity->spriteLayer - 1;
        tiles = ObjGfx_Find(sArachneGfx);
        Sprite_Attach(leg, sArachneSpriteSheet, tiles, entity->basePalette);
        leg->enemyId = entity->enemyId;
        leg->basePalette = entity->basePalette;
        leg->scrollWithBg = 0;
        leg->drawFunc = Sprite_DrawEntity;
        ENTITY_OWNER(leg) = entity;
        leg->ext.arachne.legIndex = i;
        leg->posX.whole = entity->posX.whole;
        leg->posY.whole = entity->posY.whole;
        leg->velX.whole = (i / 2 * 7) << 16;
        if (i & 1)
            leg->animFrame = 0x19;
        else
            leg->animFrame = 0x15;
        continue;
        }
        EntityDelete(entity);
        return;
    }
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 4;
        entity->ext.arachne.gait = 0;
        return;
    }
    Hit_EnableContact(entity, 8, Arachne_DamagePlayerOnContact, Arachne_OnHit);
    Entity_UpdateBoxes(entity);
    Entity_SnapToGround(entity);
    entity->state = 0;
}

/* Idle gait and transitions between approaching, retreating and attacking. */
#define GAIT(e) (e->ext.arachne.gait)
#define WAIT(e) (e->ext.arachne.wait)
/**
 * @brief Walk state machine (state 0): wake when the target is in the 160 x 160
 * box or the Arachne is damaged, then crawl with the gait tables toward or
 * away from it and start the beam attack (state 1).
 */
void Arachne_UpdateWalk(struct EwramData_EntityData *entity)
{
    s32 state=entity->subState;
    s32 phase;
    u32 flipped;
    switch(state)
    {
    case 0:
        entity->flipX=Entity_IsTargetRight(entity);
        if(Target_IsInRect((s16)(entity->posX.part16.integer-80),(s16)(entity->posY.part16.integer-80),160,160) || Enemy_IsDamaged(entity))
        {
            entity->subState=1; GAIT(entity)=1;
            entity->ext.arachne.walkPhase=state;
            entity->ext.arachne.walkSpeed=state;
            entity->velX.whole=state;
            entity->timer=state;
            if(Target_IsInRect((s16)(entity->posX.part16.integer-60),(s16)(entity->posY.part16.integer-80),120,160))
            { GAIT(entity)=2; entity->subState=2; }
            else if(Target_IsInRect((s16)(entity->posX.part16.integer-100),(s16)(entity->posY.part16.integer-80),200,160)) goto vertical;
            else { GAIT(entity)=1; entity->subState=1; }
        }
        break;
    case 1:
    {
        s32 target;
        u32 random;
        if(entity->ext.arachne.walkSpeed<0x10000) entity->ext.arachne.walkSpeed+=0x2000;
        phase=entity->ext.arachne.walkPhase+entity->ext.arachne.walkSpeed;
        entity->ext.arachne.walkPhase=phase;
        if(phase>=0x240000) entity->ext.arachne.walkPhase=phase-0x240000;
        else if(phase<0) entity->ext.arachne.walkPhase=phase+0x240000;
        if(entity->timer++>29) { entity->flipX=Entity_IsTargetRight(entity); entity->timer=0; }
        if(entity->flipX) entity->accelX.whole=0x1000;
        else entity->accelX.whole=-0x1000;
        flipped=*(u8 *)(ENTITY_RENDER_FLAGS_ADDRESS(entity))&0x40;
        if((u8)flipped) { if(entity->velX.whole>=0x8000)entity->accelX.whole=0; }
        else if(entity->velX.whole<=-0x8000)entity->accelX.whole=flipped;
        Entity_MoveWithCollision(entity,0x10000,-0x200000,0x80000,12);
        target=Target_IsInRect((s16)(entity->posX.part16.integer-60),(s16)(entity->posY.part16.integer-80),120,160);
        if(target)
        { GAIT(entity)=2; entity->subState=2; entity->timer=0; entity->velY.whole=0x10000; entity->accelY.whole=0x2800; return; }
        if(Entity_ClampXToRoom(entity,1))
        { GAIT(entity)=2; entity->subState=2; entity->timer=target; entity->velY.whole=0x10000; entity->accelY.whole=0x2800; return; }
        if(WAIT(entity)) WAIT(entity)--;
        else
        {
            random=RandomNumberGenerator()&31;
            if(!random && Target_IsInRect((s16)(entity->posX.part16.integer-80),(s16)(entity->posY.part16.integer-80),160,160))
            { GAIT(entity)=3; entity->state=1; entity->subState=random; entity->timer=random; }
        }
        break;
    }
    case 2:
    {
        u32 random;
        if(entity->ext.arachne.walkSpeed>-0x10000) entity->ext.arachne.walkSpeed-=0x2000;
        phase=entity->ext.arachne.walkPhase+entity->ext.arachne.walkSpeed;
        entity->ext.arachne.walkPhase=phase;
        if(phase>=0x240000) entity->ext.arachne.walkPhase=phase-0x240000;
        else if(phase<0) entity->ext.arachne.walkPhase=phase+0x240000;
        entity->flipX=Entity_IsTargetRight(entity);
        if(entity->flipX) entity->accelX.whole=-0x1000;
        else entity->accelX.whole=0x1000;
        flipped=*(u8 *)(ENTITY_RENDER_FLAGS_ADDRESS(entity))&0x40;
        if((u8)flipped) { if(entity->velX.whole<=-0x8000)entity->accelX.whole=0; }
        else if(entity->velX.whole>=0x8000)entity->accelX.whole=flipped;
        if((Entity_MoveWithCollision(entity,0x10000,-0x200000,0x80000,12)&3) ||
            !Target_IsInRect((s16)(entity->posX.part16.integer-100),(s16)(entity->posY.part16.integer-80),200,160) || Entity_ClampXToRoom(entity,1))
        { GAIT(entity)=1; entity->subState=1; entity->timer=0;
vertical:
            entity->velY.whole=0x10000;
            entity->accelY.whole=0x2800;
            return;
        }
        if(!WAIT(entity))
        {
            random=RandomNumberGenerator()&31;
            if(!random && Target_IsInRect((s16)(entity->posX.part16.integer-80),(s16)(entity->posY.part16.integer-80),160,160))
            { GAIT(entity)=3; entity->state=1; entity->subState=10; entity->timer=random; }
        }
        else WAIT(entity)--;
        break;
    }
    }
}

/* Arachne_UpdateAttack: breath attack (hover states 0 / 10, wind-up state 1, state 2 traces the breath projectile until it hits a wall). */

#undef WAIT
#define WAIT(e) ((e)->ext.arachne.wait)

#define HOVER(entity)                                                       \
    {                                                                       \
        s32 phase;                                                          \
                                                                            \
        if (entity->ext.arachne.walkSpeed < 0)                                 \
            entity->ext.arachne.walkSpeed += 0x2000;                           \
        else if (entity->ext.arachne.walkSpeed > 0)                            \
            entity->ext.arachne.walkSpeed -= 0x2000;                           \
        phase = entity->ext.arachne.walkPhase + entity->ext.arachne.walkSpeed;    \
        entity->ext.arachne.walkPhase = phase;                                 \
        if (phase >= 0x240000)                                              \
            entity->ext.arachne.walkPhase = phase - 0x240000;                  \
        else if (phase < 0)                                                 \
            entity->ext.arachne.walkPhase = phase + 0x240000;                  \
        EntityDampVelocityX(entity, 8);                                     \
        Entity_MoveWithCollision(entity, 0x10000, -0x200000, 0x80000, 12);              \
    }

/**
 * @brief Attack state machine (state 1): rear up and sweep a beam traced from the
 * mouth to the first wall each frame (contact ArachneBeam_DamagePlayerOnContact,
 * sparks via Arachne_SpawnEffect) for 60 frames; then back to state 0.
 */
void Arachne_UpdateAttack(struct EwramData_EntityData *entity)
{
    u32 box = 0x0404FEFE;

    switch (entity->subState)
    {
    case 0:
        HOVER(entity)
        if (entity->timer++ > 7)
        {
            entity->timer = 0;
            entity->subState++;
            Anim_StartIndexed(entity, 1, 0, 0);
        }
        break;
    case 1:
        if (*(ENTITY_RENDER_FLAGS_HIGH_ADDRESS(entity)) & 1)
        {
            entity->timer = 0;
            entity->subState++;
            Anim_StartIndexed(entity, 2, 0, 0);
            if (entity->flipX)
                entity->ext.arachne.attackAngle = 0x2800;
            else
                entity->ext.arachne.attackAngle = 0x5800;
            Sound_PlayTracked(0x82);
        }
        break;
    case 2:
    {
        s32 x, y;
        struct EwramData_EntityData *p;
        u8 *flags58;

        if (!(({ s32 t = entity->timer + 1; entity->timer = t; t; }) & 3))
            Sound_PlayTrackedIfAbsent(0x82);
        x = entity->posX.whole;
        y = entity->posY.whole - 0x2B0000;
        {
            u8 *f58 = ENTITY_RENDER_FLAGS_ADDRESS(entity);
            s32 f = *f58 & 0x40;

            flags58 = f58;
            if (f)
                x += 0xD0000;
            else
                x -= 0xD0000;
        }
        p = EntityCreateInRange(0x1B, 0x48, ArachneEffect_Update);
        if (p != NULL)
        {
            struct HitResult hit;
            s32 r;
            s32 wall;

            Hit_Enable(p, 8, ArachneBeam_DamagePlayerOnContact, 0);
            HitBox_SetAttack(p, &box);
            *((u8 *)p + 0x72) = (*((u8 *)p + 0x72) & 0xFD) | 4;
            p->posX.whole = x;
            p->posY.whole = y;
            { u8 t = *((u8 *)entity + 0x3C); *((u8 *)p + 0x3C) = t; }
            EntitySetVelocityFromAngle(p, entity->ext.arachne.attackAngle, 0x80000);
            RandomNumberGenerator();
            y = 3;
            r = RandomNumberGenerator() & y;
            wall = 4;
            while (!(Entity_MoveWithCollisionDefault(p, 0, 0) & wall))
            {
                s32 adx;

                if (EntityIsOutsideScreen(p, 0))
                    goto delete;
                x = Target_GetX(entity) - p->posX.whole;
                y = Target_GetY(entity) - p->posY.whole;
                adx = x;
                if (x < 0)
                    adx = -x;
                if (adx <= 0x7FFFF && (u32)(y + 0x1FFFF) <= 0x21FFFE)
                    Hit_CheckWithPlayer(&hit, p);
                if (p->timer <= 3 && r == (p->timer & 3))
                {
                    register u32 m asm("r5"); /* FAKEMATCH: pinned register */

                    x = RandomNumberGenerator();
                    m = 0x1FFFF;
                    x = (x & m) - 0x10000;
                    y = (RandomNumberGenerator() & m) - 0x10000;

                    Arachne_SpawnEffect(entity, p->posX.whole + x, p->posY.whole + y, 7, 1);
                }
                p->timer++;
            }
            if (!(entity->timer & 3))
                Arachne_SpawnEffect(entity, p->posX.whole, p->posY.whole, 5, wall);
        delete:
            EntityDelete(p);
        }
        if (*flags58 & 0x40)
            entity->ext.arachne.attackAngle -= 0x60;
        else
            entity->ext.arachne.attackAngle += 0x60;
        if (!(entity->timer & 3))
        {
            s32 rx = RandomNumberGenerator() & 0x3FFFF;
            s32 f;

            y = (RandomNumberGenerator() & 0x3FFFF) - 0x2E0000;
            f = *flags58 & 0x40;
            x = rx - 0xC0000;
            if (f)
                { register s32 k asm("r2") = 0x80000; x = rx + k; } /* FAKEMATCH: pinned register */
            Arachne_SpawnEffect(entity, entity->posX.whole + x, entity->posY.whole + y, 4, 4);
        }
        if (entity->timer > 0x3B)
        {
            entity->state = 0;
            entity->subState = 1;
            entity->timer = 0;
            WAIT(entity) = (RandomNumberGenerator() % 64) + 0x3C;
            entity->velX.whole = 0;
            entity->accelX.whole = 0;
            entity->velY.whole = 0x10000;
            entity->accelY.whole = 0x2800;
            Anim_StartIndexed(entity, 0, 0, 1);
            Sound_StopTracked(0x82);
        }
        break;
    }
    case 10:
        HOVER(entity)
        if (entity->timer++ > 7)
        {
            entity->timer = 0;
            entity->subState = 1;
            Anim_StartIndexed(entity, 1, 0, 0);
        }
        break;
    }
}

/* Recoil reverses gait, accelerates away from the target, and recovers at an obstacle. */
/**
 * @brief Recoil state machine (state 3, after a hit of type 3 / 4): back off from
 * the target, then return to state 0.
 */
void Arachne_UpdateRecoil(struct EwramData_EntityData *entity)
{
    switch(entity->subState)
    {
    case 0:
        entity->ext.arachne.gait=2;
        entity->subState++;
        entity->ext.arachne.walkSpeed=-0x20000;
    case 1:
    {
        s32 phase=entity->ext.arachne.walkPhase+entity->ext.arachne.walkSpeed;
        u32 flipped;
        entity->ext.arachne.walkPhase=phase;
        if(phase>=0x240000) entity->ext.arachne.walkPhase=phase-0x240000;
        else if(phase<0) entity->ext.arachne.walkPhase=phase+0x240000;
        entity->flipX=Entity_IsTargetRight(entity);
        if(entity->flipX) entity->accelX.whole=-0x4000;
        else entity->accelX.whole=0x4000;
        flipped=*(u8 *)(ENTITY_RENDER_FLAGS_ADDRESS(entity))&0x40;
        if((u8)flipped)
        {
            if(entity->velX.whole<=-0x20000) entity->accelX.whole=0;
        }
        else if(entity->velX.whole>=0x20000) entity->accelX.whole=flipped;
        if(entity->animDone && entity->animScriptIndex==3) Anim_StartIndexed(entity,0,0,1);
        if((Entity_MoveWithCollision(entity,0x10000,-0x200000,0x80000,12)&3) ||
           !Target_IsInRect((s16)(entity->posX.part16.integer-100),(s16)(entity->posY.part16.integer-80),200,160) ||
           Entity_ClampXToRoom(entity,1))
        {
            entity->ext.arachne.gait=1;
            entity->state=0;
            entity->subState=1;
            entity->velY.whole=0x10000;
            entity->accelY.whole=0x2800;
            Anim_StartIndexed(entity,0,0,1);
        }
        break;
    }
    }
}

/* Arachne's death flashes the palette, sheds particles and slowly sinks. */
/**
 * @brief Death state (state 2): random effects above it every 4 frames, flash,
 * sink and request deletion when the timer ends.
 */
void Arachne_UpdateDeath(struct EwramData_EntityData *entity)
{
    if(!(entity->timer&3)) Effect_SpawnRandom(entity->posX.whole,entity->posY.whole-0x180000,0x100000,0x100000,entity->spriteLayer-1);
    if(Entity_GetVisibilityPattern(entity->timer)) entity->palette=5;
    else entity->palette=entity->basePalette;
    if((u8)(entity->timer-9)<=80) entity->posY.whole+=0x2000;
    if((s8)--entity->timer<=0) entity->deleteRequested=1;
}

/* Apply damage and choose full destruction or a short reaction to selected attacks. */
/**
 * @brief Hit: resolve, flash, subtract and show the damage; at HP <= 0 stop sound
 * 0x82, drop the rewards and die (song 0x6E, animation 3, 90 frames,
 * state 2); outside the attack a hit of type 3 / 4 starts the recoil
 * (state 3).
 */
void Arachne_OnHit(struct EwramData_EntityData *entity,struct EwramData_EntityData *attacker,u16 *info)
{
    s32 power=Player_CalculateAttackPower(info[2]);
    struct EntityHit *attack=&attacker->hit;
    s32 damage=Hit_ResolveDamage(entity,attacker,info,power);
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
        Sound_StopTracked(0x82);
        Enemy_DropRewards(entity);
        PlaySong(0x6E);
        Anim_StartIndexed(entity,3,0,0);
        entity->palette=entity->basePalette;
        entity->timer=90;
        entity->state=2;
        entity->subState=zero;
    }
    else if(entity->state!=1 && (u8)(attack->type-3)<=1)
    {
        Anim_StartIndexed(entity,3,0,0);
        entity->state=3;
        entity->subState=zero;
    }
}

/* Six legs select delayed gait frames from their owner's phase. */
#define LEG(e) (*(s8 *)&e->ext.arachne.legIndex)
#undef GAIT
#define GAIT(e) (*(s8 *)&e->ext.arachne.gait)
#define FIXED_INTEGER(x) ((x)<0 ? -(-(x)>>16) : (x)>>16)
/**
 * @brief Leg sprite update: follow the owner (type 0x17) with its visibility,
 * palette and facing, picking the frame from the owner's gait (sArachneLegsFrames);
 * delete when the owner is gone.
 */
void ArachneLegs_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner=ENTITY_OWNER(entity);
    u8 scratch[16]; /* Stack workspace reserved by the original routine. */
    u8 frames[36];
    s32 step;
    s32 gait;
    u8 current;
    s8 *ownerGait;
    s8 *index;
    memcpy(frames,sArachneLegsFrames,36);
    if(entity->state==0)
    {
        if(!owner || !owner->updateFunc || owner->enemyId!=23) goto fail;
        entity->hidden=owner->hidden;
        entity->palette=owner->palette;
        entity->flipX=owner->flipX;
        entity->posX.part16.integer=owner->posX.part16.integer;
        entity->posY.part16.integer=owner->posY.part16.integer;
        if(entity->flipX) entity->posX.part16.integer-=entity->velX.part16.integer;
        else entity->posX.part16.integer+=entity->velX.part16.integer;
        gait=GAIT(owner);
        ownerGait=(s8 *)&owner->ext.arachne.gait;
        switch(gait)
        {
        case 0:
            entity->animFrame=21;
            entity->subState=0;
            current=*ownerGait; GAIT(entity)=current;
            break;
        case 1:
            step=FIXED_INTEGER((s32)owner->ext.arachne.walkPhase);
            index=(s8 *)&entity->ext.arachne.legIndex;
            step-=(*index/2)*7;
            if(GAIT(entity)==2) entity->subState=1;
            if(entity->subState==0)
            {
                if(step>=0) { entity->subState=1; current=*ownerGait; GAIT(entity)=current; }
                else step=0;
            }
            else if(step<0) step+=36;
            entity->animFrame=frames[step];
            current=*ownerGait; GAIT(entity)=current;
            break;
        case 2:
            step=FIXED_INTEGER((s32)owner->ext.arachne.walkPhase);
            index=(s8 *)&entity->ext.arachne.legIndex;
            step-=(*index/2)*7;
            if(GAIT(entity)==1) entity->subState=1;
            if(entity->subState==0)
            {
                if((u32)(step-1)>21) step=0;
                else entity->subState=1;
            }
            if(step<0) step+=36;
            entity->animFrame=frames[step];
            current=*ownerGait; GAIT(entity)=current;
            break;
        case 3:
            step=FIXED_INTEGER((s32)owner->ext.arachne.walkPhase);
            index=(s8 *)&entity->ext.arachne.legIndex;
            step-=(*index/2)*7;
            if(step<0)step+=36;
            entity->animFrame=frames[step];
            current=*ownerGait; GAIT(entity)=current;
            break;
        }
        index=(s8 *)&entity->ext.arachne.legIndex;
        if(*index&1) entity->animFrame+=4;
        if(owner->state==2) { entity->state=1; entity->subState=0; entity->timer=90; }
        return;
fail:
        EntityDelete(entity);
    }
    else
    {
        if(entity->subState==0) { entity->subState++; entity->scrollWithBg=1; }
        entity->palette=owner->palette;
        if(LEG(entity)==2) { if(entity->timer&12) entity->animFrame=22; else entity->animFrame=23; }
        else if(LEG(entity)==4) { if(!(entity->timer&24)) entity->animFrame=22; else entity->animFrame=21; }
        else if(LEG(entity)==0) entity->animFrame=23;
        if((s8)--entity->timer<=0) entity->deleteRequested=1;
    }
}

/* Arachne effects inherit the owner's palette, facing and layer. */
/**
 * @brief Spawn an effect at (x, y) with the Arachne graphics: animation, lifetime
 * frames, or until the animation ends when the owner timer is 128.
 */
void Arachne_SpawnEffect(struct EwramData_EntityData *owner,s32 x,s32 y,s32 animation,s32 lifetime)
{
    struct EwramData_EntityData *effect=EntityCreateInRange(0x49,0xCF,ArachneEffect_Update);
    if(effect)
    {
        s32 graphics;
        effect->posX.whole=x; effect->posY.whole=y;
        effect->flipX=owner->flipX;
        effect->spriteLayer=owner->spriteLayer;
        graphics=ObjGfx_Find(sArachneGfx);
        Sprite_Attach(effect,sArachneSpriteSheet,graphics,owner->basePalette);
        if(owner->timer!=128) Anim_StartIndexed(effect,animation,0,1);
        else Anim_StartIndexed(effect,animation,0,0);
        effect->timer=lifetime;
        effect->scrollWithBg=1;
        effect->drawFunc=Sprite_DrawEntity;
    }
}

/* Arachne helpers 0x0807A900..0x0807A9EB. */

/**
 * @brief 7A900 | Animate; with timer == 0x80 delete once the animation
 * ended (0x53D bit 0), otherwise count timer down and delete at 0.
 */
void ArachneEffect_Update(struct EwramData_EntityData *entity)
{
    Anim_Update(entity);
    if (entity->timer == 0x80)
    {
        if (entity->animDone)
            EntityDelete(entity);
    }
    else if ((s8)entity->timer-- <= 0)
    {
        EntityDelete(entity);
    }
}

/**
 * @brief 7A940 | Player_ApplyContactDamage with (attack, 0x200, 0, 0) at the entity
 * position.
 */
void ArachneBeam_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 0x200;
    contactDamage.unused = 0;
    contactDamage.reaction = 0;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief 7A970 | Load the graphics of Arachne; 0 when no slot is free.
 */
u8 Arachne_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sArachneGfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sArachnePalette, 0);
    Sprite_Attach(entity, sArachneSpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = Sprite_DrawEntity;
    return 1;
}

/**
 * @brief Bestiary preview state (state 4): nothing (the caller animates).
 */
void Arachne_UpdatePreview(struct EwramData_EntityData *entity)
{
}

/**
 * @brief 7A9C0 | Player_ApplyContactDamage with (attack, 0, 0, 0) at the entity position
 * (same as Chaos2Part_DamagePlayerOnContact).
 */
void Arachne_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}
