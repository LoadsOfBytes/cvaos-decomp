/*
 * Werejaguar: body and death effect. The last part of the range holds the entry points of Weretiger.
 */
#include "core.h"
#include "player_core.h"
#include "graphics.h"
#include "entity_util.h"
#include "structs/ewram.h"
#include "sound.h"
#include "gameplay.h"

/* Werejaguar dispatch, collision handling and animation change notification. */
extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *);
extern s32 Entity_IsTimeStopped(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *,struct EwramData_EntityData *);
extern void Werejaguar_UpdateWalk(struct EwramData_EntityData *);
extern void Werejaguar_UpdateAttack(struct EwramData_EntityData *);
extern void Werejaguar_UpdateDeath(struct EwramData_EntityData *);
extern void Werejaguar_UpdateRecoil(struct EwramData_EntityData *);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *);
void EnemyWerejaguarUpdate(struct EwramData_EntityData *entity)
{
    s32 bounds[12];
    if(entity->state!=2)
    {
        if(EntityIsOutsideScreen(entity,4)) return;
        if(Entity_IsInRect(entity,-16,0,272,176)) Player_TrackEntity(entity);
    }
    if(EntityIsOutsideScreen(entity,4)) entity->hidden=1;
    else entity->hidden=0;
    entity->ext.werejaguar.contactKind=0;
    entity->ext.werejaguar.manualAnim=0;
    entity->animFlags&=0xFD;
    if(entity->state!=2) Enemy_UpdateHitFlash(entity);
    if(Entity_IsTimeStopped(entity) && entity->state!=2) goto collision;
    if(entity->hitFlashTimer && entity->state!=3 && entity->state!=2)
    {
collision:
        Hit_CheckWithPlayer(bounds,entity);
        return;
    }
    switch(entity->state)
    {
        case 0: Werejaguar_UpdateWalk(entity); break;
        case 1: Werejaguar_UpdateAttack(entity); break;
        case 2: Werejaguar_UpdateDeath(entity); break;
        case 3: Werejaguar_UpdateRecoil(entity); break;
        case 4: Anim_Update(entity); return;
    }
    if((s8)entity->ext.werejaguar.manualAnim==0)
    {
        u8 frame=entity->animFrame;
        Anim_Update(entity);
        if(frame!=entity->animFrame) Entity_UpdateBoxes(entity);
    }
    Enemy_CheckHitPlayer(entity);
}

extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern s32 Target_IsInRect(s16, s16, s16, s16);
extern s32 Enemy_IsDamaged(struct EwramData_EntityData *);
extern s32 Entity_MoveWithCollision(struct EwramData_EntityData *, s32, s32, s32, s32);
extern s16 Target_GetPixelX(struct EwramData_EntityData *entity);

static inline s32 Dist(struct EwramData_EntityData *entity)
{
    s32 d = Target_GetPixelX(entity) - (s16)entity->posX.part16.integer;
    if (d < 0)
        d = -d;
    return d;
}

/**
 * @brief 78880 | Hopping enemy AI: waits on the ground until the player is in
 * the 0xA0 box (or Enemy_IsDamaged), then (state 1) jumps at the player in
 * animation frame 3 – straight when Player_GetFocusedActionFlags reports a matching input,
 * otherwise a short hop (1/4) or a back jump, faster when far – and attacks
 * (state = 1) when the player is close; state 2 waits for the landing,
 * state 3 plays sound 0x85 during the attack animation.
 */
void Werejaguar_UpdateWalk(struct EwramData_EntityData *entity)
{
    s32 state = entity->subState;
    s32 res;
    s32 speed;
    s32 speed2;
    s32 accel;
    s32 h;
    s32 d;
    s32 v;

    switch (state)
    {
    case 0:
        entity->flipX = Entity_IsTargetRight(entity);
        entity->velX.whole = state;
        entity->accelX.whole = state;
        entity->velY.whole = 0x20000;
        entity->accelY.whole = 0x2800;
        d = Entity_MoveWithCollision(entity, 0x20000, -0x200000, 0x80000, 8);
        if (!(d & 4))
            break;
        if (Target_IsInRect(entity->posX.part16.integer - 0x50, entity->posY.part16.integer - 0x50, 0xA0, 0xA0) || Enemy_IsDamaged(entity))
        {
            entity->subState = 1;
            Anim_StartIndexed(entity, 6, 0, 1);
        }
        break;
    case 1:
        res = EntityIsOutsideScreen(entity, 4);
        if (res)
        {
            Anim_StartIndexed(entity, 0, 0, 1);
            entity->subState = 0;
            break;
        }
        entity->flipX = Entity_IsTargetRight(entity);
        entity->velX.whole = res;
        entity->accelX.whole = res;
        speed = 0x20000;
        entity->velY.whole = speed;
        accel = 0x2800;
        entity->accelY.whole = accel;
        d = Entity_MoveWithCollision(entity, speed, -0x200000, 0x80000, 8);
        if (!(d & 4))
            break;
        if (entity->animStep != 3)
            break;
        h = entity->enemyId == 0x4B ? 0x28 : 0x78;
        if (Target_IsInRect(entity->posX.part16.integer - 0x3C, entity->posY.part16.integer - h, 0x78, h * 2))
        {
            Anim_StartIndexed(entity, 0, 0, 1);
            entity->state = state;
            entity->subState = res;
            entity->timer = res;
            entity->velX.whole = res;
            entity->accelX.whole = res;
            entity->velY.whole = speed;
            entity->accelY.whole = accel;
            break;
        }
        if ((Player_GetFocusedActionFlags() & 0x360) && Dist(entity) <= 0x4F)
        {
            if (entity->flipX)
                entity->velX.whole = -0x10000;
            else
                entity->velX.whole = 0x10000;
            entity->accelX.whole = 0;
            entity->velY.whole = -0x18000;
            entity->accelY.whole = 0x3000;
            entity->subState++;
            entity->animFlags |= 2;
            break;
        }
        else
        {
            if (!(RandomNumberGenerator() & 3) && Dist(entity) <= 0x4F)
            {
                if (RandomNumberGenerator() % 3 != 0)
                {
                    if (entity->flipX)
                        entity->velX.whole = -0x8000;
                    else
                        entity->velX.whole = 0x8000;
                }
                else
                {
                    Anim_StartIndexed(entity, 3, 0, 0);
                    entity->subState = 3;
                    break;
                }
            }
            else
            {
                if (entity->flipX)
                    entity->velX.whole = 0x10000;
                else
                    entity->velX.whole = -0x10000;
            }
            if (Dist(entity) > 0x63)
                entity->velX.whole += entity->velX.whole / 2;
        }
        entity->accelX.whole = 0;
        entity->velY.whole = -0x18000;
        entity->accelY.whole = 0x3000;
        entity->subState++;
        entity->animFlags |= 2;
        break;
    case 2:
        entity->animFlags |= 2;
        speed2 = 0x20000;
        d = Entity_MoveWithCollision(entity, speed2, -0x200000, 0x80000, 8);
        if (d & 4)
        {
            entity->animFlags &= ~2;
            entity->animTimer = 0xFA;
            entity->velX.whole = 0;
            entity->accelX.whole = 0;
            entity->velY.whole = speed2;
            entity->accelY.whole = 0x2800;
            entity->subState = 1;
            Anim_StartIndexed(entity, 6, 0, 1);
        }
        break;
    case 3:
        if (!entity->animTimer && (entity->animStep == 1 || entity->animStep == 3))
            PlaySong(0x85);
        if (entity->animDone)
        {
            Anim_StartIndexed(entity, 6, 0, 1);
            entity->subState = 1;
        }
        break;
    }
}

/* Werejaguar_UpdateAttack, decompiled from the original assembly: hopping enemy AI
 * (ids 0x4B / 0x54), sibling of Werejaguar_UpdateWalk. The shared failure tail
 * (fail:) and the state 10 attack start sit where the original keeps their
 * cross-jumped copies; the hit test results use a function-level d. */
extern void Afterimage_Create(struct EwramData_EntityData *, s32, s32, s32);

#define HOP_FLAG (entity->ext.raw.unk_4F8.bytes[3])
#define ROLL (entity->ext.raw.unk_4F8.bytes[2])

void Werejaguar_UpdateAttack(struct EwramData_EntityData *entity)
{
    s32 r;
    s32 speed;
    s32 zero;
    s32 d;

    switch (entity->subState)
    {
    case 0:
        entity->flipX = Entity_IsTargetRight(entity);
        speed = 0x20000;
        d = Entity_MoveWithCollision(entity, speed, -0x200000, 0x80000, 8);
        if (!(d & 4))
            break;
        if (entity->timer++ > 0x1D || (Player_GetFocusedActionFlags() & 0x360))
        {
            entity->timer = 0;
            entity->subState++;
            entity->ext.werejaguar.contactHit = 0;
            entity->ext.werejaguar.manualAnim = 1;
            entity->ext.werejaguar.unusedFlag = 0;
            entity->animFrame = 0x10;
            Entity_UpdateBoxes(entity);
            entity->accelX.whole = 0;
            entity->velY.whole = 0x40000;
            entity->accelY.whole = 0x2800;
            if (entity->flipX)
                entity->velX.whole = speed;
            else
                entity->velX.whole = -0x20000;
            break;
        }
        if (entity->enemyId != 0x54)
            break;
        if (Target_IsInRect(entity->posX.part16.integer - 0x3C, entity->posY.part16.integer - 0x78, 0x78, 0x6E))
        {
            Anim_StartIndexed(entity, 4, 0, 0);
            entity->subState = 10;
        }
        break;
    case 1:
        entity->ext.werejaguar.manualAnim = 1;
        r = Entity_MoveWithCollision(entity, 0x40000, -0x200000, 0x80000, 8) & 4;
        if (!r)
            goto fail;
        if (entity->timer++ <= 0x13)
        {
            if (entity->timer <= 3)
                break;
            if (Dist(entity) > 0x14)
                break;
        }
        entity->subState = 2;
        entity->velX.whole = 0;
        entity->animFrame = 0;
        entity->timer = 0;
        Entity_UpdateBoxes(entity);
        break;
    case 2:
        if (entity->timer++ <= 5)
            break;
        r = Target_IsInRect(entity->posX.part16.integer - 0x14, entity->posY.part16.integer - 0x28, 0x28, 0x50);
        if (r != 0)
        {
            if (Dist(entity) <= 0xC)
                goto close;
            entity->subState = 3;
            if (entity->enemyId == 0x4B)
            {
                if (RandomNumberGenerator() % 3)
                    Anim_StartIndexed(entity, 3, 0, 0);
                else if (!(RandomNumberGenerator() & 1))
                    Anim_StartIndexed(entity, 1, 0, 0);
                else
                    Anim_StartIndexed(entity, 2, 0, 0);
            }
            else
                Anim_StartIndexed(entity, 3, 0, 0);
            entity->ext.werejaguar.manualAnim = 0;
            break;
        }
        entity->ext.werejaguar.manualAnim = r;
        Anim_StartIndexed(entity, 6, 0, 1);
        entity->state = r;
        entity->subState = 1;
        break;
    case 3:
        if (Dist(entity) <= 0xC)
        {
        close:
            entity->timer = 0;
            entity->subState = 6;
            entity->ext.werejaguar.manualAnim = 1;
            entity->animFrame = 0x10;
            Entity_UpdateBoxes(entity);
            entity->accelX.whole = 0;
            entity->velY.whole = 0x40000;
            entity->accelY.whole = 0x2800;
            if (entity->flipX)
                entity->velX.whole = -0x20000;
            else
                entity->velX.whole = 0x20000;
            break;
        }
        if (entity->animDone)
        {
            s32 hop = HOP_FLAG;
            if (hop != 0)
            {
                if (entity->enemyId == 0x4B)
                {
                    entity->subState = 5;
                    entity->timer = 0;
                    Anim_StartIndexed(entity, 0, 0, 0);
                    break;
                }
                entity->timer = 0;
                entity->subState = 0xD;
                entity->ext.werejaguar.manualAnim = 1;
                entity->animFrame = 0x10;
                Entity_UpdateBoxes(entity);
                entity->accelX.whole = 0;
                {
                    s32 v = 0x20000;
                    entity->velY.whole = v;
                    entity->accelY.whole = 0x2800;
                    if (entity->flipX)
                        entity->velX.whole = -0x20000;
                    else
                        entity->velX.whole = v;
                }
                break;
            }
            Anim_StartIndexed(entity, 6, 0, 1);
            entity->state = hop;
            entity->subState = 1;
            entity->ext.werejaguar.manualAnim = hop;
            break;
        }
        entity->ext.werejaguar.contactKind = 1;
        if (entity->animTimer == 0 && (entity->animStep == 1 || entity->animStep == 3 || entity->animStep == 5))
            PlaySong(0x85);
        break;
    case 4:
        if (entity->animDone)
        {
            Anim_StartIndexed(entity, 6, 0, 1);
            entity->state = 0;
            entity->subState = 1;
            break;
        }
        if (entity->animStep == 5)
            entity->ext.werejaguar.contactKind = 2;
        else
            entity->ext.werejaguar.contactKind = 1;
        if (entity->animTimer == 0 && (entity->animStep == 1 || entity->animStep == 3 || entity->animStep == 5))
            PlaySong(0x85);
        break;
    case 5:
        if (entity->timer++ > 7)
        {
            if (entity->enemyId == 0x4B)
            {
                if (ROLL == 0 && !(RandomNumberGenerator() & 3))
                {
                    ROLL = 1;
                    if (RandomNumberGenerator() % 3)
                        Anim_StartIndexed(entity, 3, 0, 0);
                    else if (!(RandomNumberGenerator() & 1))
                        Anim_StartIndexed(entity, 1, 0, 0);
                    else
                        Anim_StartIndexed(entity, 2, 0, 0);
                    entity->subState = 3;
                    break;
                }
                if (RandomNumberGenerator() & 1)
                    Anim_StartIndexed(entity, 1, 0, 1);
                else
                    Anim_StartIndexed(entity, 2, 0, 1);
                entity->subState = 4;
                break;
            }
            Anim_StartIndexed(entity, 4, 0, 0);
            entity->subState = 10;
            break;
        }
        entity->ext.werejaguar.contactKind = 1;
        break;
    case 6:
        zero = 0;
        entity->ext.werejaguar.manualAnim = 1;
        r = Entity_MoveWithCollision(entity, 0x40000, -0x200000, 0x80000, 8) & 4;
        if (!r)
            goto fail;
        if (entity->timer++ <= 0xB)
            break;
        entity->subState = 2;
        entity->velX.whole = zero;
        entity->animFrame = zero;
        entity->timer = zero;
        Entity_UpdateBoxes(entity);
        break;
    case 10:
        if (entity->animStep != 1)
            break;
        entity->subState++;
        entity->accelX.whole = 0;
        entity->velY.whole = 0;
        entity->accelY.whole = 0;
        if (entity->flipX)
            entity->velX.whole = 0x100000;
        else
            entity->velX.whole = -0x100000;
        EntityAccelerateAndMove(entity, 0x100000);
        entity->ext.werejaguar.contactKind = 3;
        if (entity->flipX)
            entity->velX.whole = 0x10000;
        else
            entity->velX.whole = -0x10000;
        entity->velY.whole = -0x40000;
        entity->accelY.whole = 0x3000;
        PlaySong(0x86);
        Afterimage_Create(entity, 0x38, 0xC, 1);
        Afterimage_Create(entity, 0x38, 6, 0);
        break;
    case 11:
        {
            s32 z = 0;
            s32 v;
            entity->ext.werejaguar.contactKind = 3;
            if (entity->animStep == 3)
            {
                entity->animFlags |= 2;
                entity->ext.werejaguar.contactKind = z;
            }
            v = 0x20000;
            d = Entity_MoveWithCollision(entity, v, -0x200000, 0x80000, 8);
            if (!(d & 4))
                break;
            entity->animFlags &= ~2;
            entity->animTimer = 0xFA;
            entity->velX.whole = z;
            entity->accelX.whole = z;
            entity->velY.whole = v;
            entity->accelY.whole = 0x2800;
            entity->subState++;
            entity->timer = z;
            break;
        }
    case 12:
        if (entity->timer++ <= 0x1D)
            break;
        Anim_StartIndexed(entity, 6, 0, 1);
        entity->state = 0;
        entity->subState = 1;
        break;
    case 13:
        zero = 0;
        entity->ext.werejaguar.manualAnim = 1;
        r = Entity_MoveWithCollision(entity, 0x40000, -0x200000, 0x80000, 8) & 4;
        if (!r)
            goto fail;
        if (entity->timer++ <= 3)
            break;
        entity->subState = 5;
        entity->timer = zero;
        Anim_StartIndexed(entity, 0, 0, 0);
        entity->ext.werejaguar.manualAnim = zero;
        break;
    fail:
        entity->state = r;
        entity->subState = r;
        Anim_StartIndexed(entity, 0, 0, 1);
        entity->ext.werejaguar.manualAnim = r;
        break;
    }
}

/* Recoil away from the target and return to the grounded idle state. */
extern s32 Entity_MoveWithCollision(struct EwramData_EntityData *,s32,s32,s32,s32);
/**
 * @brief Recoil state (state 3): hop back from the target and resume on landing.
 */
void Werejaguar_UpdateRecoil(struct EwramData_EntityData *entity)
{
    switch(entity->subState)
    {
        case 0:
            entity->flipX=Entity_IsTargetRight(entity);
            if(entity->flipX) entity->velX.whole=-0x18000;
            else entity->velX.whole=0x18000;
            entity->accelX.whole=0;
            entity->velY.whole=-0x24000;
            entity->accelY.whole=0x3000;
            entity->subState++;
        case 1:
            if(Entity_MoveWithCollision(entity,0x40000,-0x200000,0x80000,8)&4)
            {
                s32 zero=0;
                entity->state=zero;
                entity->subState=1;
                Anim_StartIndexed(entity,6,0,1);
                entity->velX.whole=zero;
                entity->accelX.whole=zero;
                entity->velY.whole=0x20000;
                entity->accelY.whole=0x2800;
            }
            break;
    }
}

/* Dying Werejaguar emits shrinking particles and flashes its palette. */
extern s32 Entity_GetVisibilityPattern(s32);
extern void Werejaguar_SpawnDeathEffect(s32,s32,s32,s32);
/**
 * @brief Death state (state 2, also used by the Werewolf): spiral effects every 4
 * frames (Werejaguar_SpawnDeathEffect), flash and request deletion when the
 * timer ends.
 */
void Werejaguar_UpdateDeath(struct EwramData_EntityData *entity)
{
    if(!(entity->timer&3)) Werejaguar_SpawnDeathEffect(entity->posX.whole,entity->posY.whole-0x40000,entity->spriteLayer,entity->timer*0x666);
    if(Entity_GetVisibilityPattern(entity->timer)) entity->palette=5;
    else entity->palette=entity->basePalette;
    if((s8)--entity->timer<=0) entity->deleteRequested=1;
}

/* Select attack damage and knockback, then use the current frame's hitbox. */
extern void AGBAssert(const char *,int,const char *,int);
extern const char sAssertFileLtInline[],sAssertExprOpdId[];
static inline struct SpriteSheet *GetSprite(s16 slot)
{
    if(!(slot>=0 && slot<240)) AGBAssert(sAssertFileLtInline,94,sAssertExprOpdId,1);
    return gEwramData->spriteSlots[slot].sheet;
}
void Werejaguar_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct EntityHit *attack=&entity->hit;
    struct PlayerContactDamage hit;
    u8 *power;
    s32 kind;
    hit.attack=entity->attack;
    power=&entity->attack;
    hit.attributes=0; hit.unused=0; hit.reaction=0;
    hit.sourceX=entity->posX.whole; hit.sourceY=entity->posY.whole;
    attack->type=8;
    kind=(s8)entity->ext.werejaguar.contactKind;
    switch(kind)
    {
        case 0: hit.reaction=kind; Player_ApplyContactDamage(&hit); break;
        case 1:
            attack->type=9;
            hit.reaction=0; hit.attack=*power*3/4;
            Player_ApplyContactDamage(&hit);
            break;
        case 2:
        {
            s32 push;
            hit.reaction=3; hit.attack=*power;
            Player_ApplyContactDamage(&hit);
            push=-0x30000;
            if(gEwramData->playerState.player->posX.whole>entity->posX.whole) push=-push;
            Player_SetKnockbackVelocity(push,-0x30000);
            break;
        }
        case 3:
        {
            s32 push;
            hit.attack=*power+*power/4; hit.reaction=kind;
            Player_ApplyContactDamage(&hit);
            push=-0x30000;
            if(gEwramData->playerState.player->posX.whole>entity->posX.whole) push=-push;
            Player_SetKnockbackVelocity(push,-0x60000);
            break;
        }
    }
    entity->ext.werejaguar.contactHit=1;
    HitBox_SetAttack(entity,GetSprite(entity->spriteSlot)->frames[entity->animFrame].boxes);
}

/* Apply damage and choose full destruction or a short reaction to selected attacks. */
extern s32 Hit_ResolveDamage(struct EwramData_EntityData *,struct EwramData_EntityData *,u16 *,s32);
extern void Entity_ShowDamage(struct EwramData_EntityData *,s32,s32);
extern void Enemy_DropRewards(struct EwramData_EntityData *);
void Werejaguar_OnHit(struct EwramData_EntityData *entity,struct EwramData_EntityData *attacker,u16 *info)
{
    s32 power=Player_CalculateAttackPower(info[2]);
    struct EntityHit *attack=&attacker->hit;
    s32 damage=Hit_ResolveDamage(entity,attacker,info,(s16)power);
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
    entity->ext.werejaguar.manualAnim=zero;
    if((s16)entity->hp<=0)
    {
        Enemy_DropRewards(entity);
        PlaySong(0x6E);
        Anim_StartIndexed(entity,5,0,0);
        entity->palette=entity->basePalette;
        entity->timer=40;
        entity->state=2;
        entity->subState=zero;
    }
    else if(entity->state==1 && entity->subState!=0 && (u8)(attack->type-3)<=1)
    {
        Anim_StartIndexed(entity,5,0,1);
        entity->state=3;
        entity->subState=zero;
    }
}

/* Allocate a rising death particle and save its screen and camera origin. */
extern void WerejaguarDeathEffect_Update(struct EwramData_EntityData *);
#define FIXED_INTEGER(x) ((x)<0 ? -(-(x)>>16) : (x)>>16)
/**
 * @brief Spawn a death effect at (x, y) on layer that circles from angle
 * (WerejaguarDeathEffect_Update).
 */
void Werejaguar_SpawnDeathEffect(s32 x,s32 y,s32 layer,s32 angle)
{
    struct EwramData_EntityData *effect=EntityCreateInRange(0x49,0xCF,(u32 *)WerejaguarDeathEffect_Update);
    struct BgLayerState *camera=&gEwramData->bgInfo[1];
    if(effect)
    {
        effect->posX.whole=x; effect->posY.whole=y;
        effect->flipX=RandomNumberGenerator()%2;
        effect->spriteLayer=layer;
        effect->ext.werejaguarEffect.angle=angle;
        effect->ext.werejaguarEffect.layer=layer;
        effect->ext.werejaguarEffect.x=FIXED_INTEGER(x);
        effect->ext.werejaguarEffect.cameraX=camera->xPos.part16.integer;
        effect->ext.werejaguarEffect.y=FIXED_INTEGER(y);
        effect->ext.werejaguarEffect.cameraY=camera->yPos.part16.integer;
    }
}

/* A spiralling particle rises from a saved origin, independently of the camera. */
extern u8 sCommonObjectGfx[],sCommonObjectSpriteSheet[];
void WerejaguarDeathEffect_Update(struct EwramData_EntityData *entity)
{
    struct BgLayerState *camera=&gEwramData->bgInfo[1];
    switch(entity->state)
    {
        case 0:
        {
            s32 graphics=ObjGfx_Find(sCommonObjectGfx);
            Sprite_Attach(entity,sCommonObjectSpriteSheet,graphics,7);
            Anim_StartIndexed(entity,0,0,0);
            entity->scrollWithBg=0;
            entity->drawFunc=(u32 *)Sprite_DrawEntity;
            entity->state++;
            break;
        }
        case 1:
        {
            s32 x,y;
            entity->ext.werejaguarEffect.angle+=0x1000;
            x=Sin(entity->ext.werejaguarEffect.angle+0x4000)*16;
            y=Sin(entity->ext.werejaguarEffect.angle)*2;
            entity->ext.werejaguarEffect.y-=2;
            entity->posX.whole=(entity->ext.werejaguarEffect.x<<16)+x-(((s16)camera->xPos.part16.integer-entity->ext.werejaguarEffect.cameraX)<<16);
            entity->posY.whole=(entity->ext.werejaguarEffect.y<<16)+y-(((s16)camera->yPos.part16.integer-entity->ext.werejaguarEffect.cameraY)<<16);
            if(y>0) entity->spriteLayer=entity->ext.werejaguarEffect.layer+1;
            else entity->spriteLayer=entity->ext.werejaguarEffect.layer-1;
            break;
        }
    }
    if(Anim_Update(entity)==3) EntityDelete(entity);
}

/* Werejaguar and Weretiger share graphics and select different palette rows. */
extern s32 ObjPalette_FindOrLoadVariant(void *,s32);
extern u8 sWerejaguarGfx[],sWerejaguarPalette[],sWerejaguarSpriteSheet[];
u8 Werejaguar_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 graphics=ObjGfx_LoadResource(sWerejaguarGfx,0);
    if(graphics<0) return 0;
    if(entity->enemyId==75)
        entity->basePalette=ObjPalette_FindOrLoadVariant(sWerejaguarPalette,0);
    else
        entity->basePalette=ObjPalette_FindOrLoadVariant(sWerejaguarPalette,1);
    Sprite_Attach(entity,sWerejaguarSpriteSheet,graphics,entity->basePalette);
    entity->drawFunc=(u32 *)Sprite_DrawEntity;
    return 1;
}

/* EnemyWerejaguarCreate. */
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *entity);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *entity);
extern void Entity_SnapToGround(struct EwramData_EntityData *entity);
extern s32 Hit_EnableContact(struct EwramData_EntityData *entity, s32 type, void *onHit, s32 onHurt);

/**
 * @brief 796F0 | Werejaguar create: load the graphics (delete the enemy
 * when that fails), start the animation and set up the hit boxes; state 4
 * instead when bestiaryPreview is set.
 */
void EnemyWerejaguarCreate(struct EwramData_EntityData *entity)
{
    if (!Werejaguar_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 4;
        return;
    }
    Hit_EnableContact(entity, 8, Werejaguar_DamagePlayerOnContact, (s32)Werejaguar_OnHit);
    Entity_UpdateBoxes(entity);
    Hit_SetTimerOverride(3, 6);
    Entity_SnapToGround(entity);
    entity->state = 0;
}

/* Werejaguar dispatch, collision handling and animation change notification. */
void EnemyWeretigerUpdate(struct EwramData_EntityData *entity)
{
    s32 bounds[12];
    Player_TrackEntity(entity);
    if(EntityIsOutsideScreen(entity,4)) entity->hidden=1;
    else entity->hidden=0;
    entity->ext.werejaguar.contactKind=0;
    entity->ext.werejaguar.manualAnim=0;
    entity->animFlags&=0xFD;
    if(entity->state!=2) Enemy_UpdateHitFlash(entity);
    if(Entity_IsTimeStopped(entity) && entity->state!=2) goto collision;
    if(entity->hitFlashTimer && entity->state!=3 && entity->state!=2)
    {
collision:
        Hit_CheckWithPlayer(bounds,entity);
        return;
    }
    switch(entity->state)
    {
        case 0: Werejaguar_UpdateWalk(entity); break;
        case 1: Werejaguar_UpdateAttack(entity); break;
        case 2: Werejaguar_UpdateDeath(entity); break;
        case 3: Werejaguar_UpdateRecoil(entity); break;
        case 4: Anim_Update(entity); return;
    }
    if((s8)entity->ext.werejaguar.manualAnim==0)
    {
        u8 frame=entity->animFrame;
        Anim_Update(entity);
        if(frame!=entity->animFrame) Entity_UpdateBoxes(entity);
    }
    if(entity->state!=2) Hit_CheckWithPlayer(bounds,entity);
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   EnemyWeretigerCreate = EnemyWerejaguarCreate */

/**
 * @brief 79874 | Same as EnemyWerejaguarCreate.
 */
void EnemyWeretigerCreate(struct EwramData_EntityData *entity)
{
    if (!Werejaguar_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 4;
        return;
    }
    Hit_EnableContact(entity, 8, Werejaguar_DamagePlayerOnContact, (s32)Werejaguar_OnHit);
    Entity_UpdateBoxes(entity);
    Hit_SetTimerOverride(3, 6);
    Entity_SnapToGround(entity);
    entity->state = 0;
}
