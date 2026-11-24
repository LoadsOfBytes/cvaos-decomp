/*
 * Quezlcoatl: head, segments and death pieces.
 */
#include "core.h"
#include "player_core.h"
#include "entity_util.h"
#include "structs/ewram.h"
#include "structs/entity_render_flags.h"
#include "gameplay.h"
#include "sound.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "hitbox.h"
#include "obj_gfx.h"
#include "obj_palette.h"
#include "data/data_118B48.h"
#include "damage_numbers.h"
#include "entity_motion.h"

extern s32 Hit_ResolveDamage(struct EwramData_EntityData *entity, s32 other, u16 *hit, s32 inputDamage);

void Quezlcoatl_UpdateFly(struct EwramData_EntityData *);
void Quezlcoatl_UpdatePreview(struct EwramData_EntityData *);
void Quezlcoatl_SpawnDeathPiece(struct EwramData_EntityData *,s32,s32,s32);
u8 Quezlcoatl_LoadGraphics(struct EwramData_EntityData *);
void QuezlcoatlSegment_Update(struct EwramData_EntityData *);
void QuezlcoatlSegment_EnableHitBoxes(struct EwramData_EntityData *);
void Quezlcoatl_DamagePlayerOnContact(struct EwramData_EntityData *entity);
void Quezlcoatl_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
void QuezlcoatlSegment_DamagePlayerOnContact(struct EwramData_EntityData *entity);
void QuezlcoatlPiece_Update(struct EwramData_EntityData *);

/* Quezlcoatl dispatch and staged destruction of the parent entity. */
void EnemyQuezlcoatlUpdate(struct EwramData_EntityData *entity)
{
    s32 bounds[4];
    if(entity->state!=2 && Entity_IsInRect(entity,-16,0,272,176))
        Player_TrackEntity(entity);
    if(Enemy_IsStunned(entity))
    {
        if(entity->state!=2) Hit_CheckWithPlayer((struct HitResult *)bounds,entity);
        return;
    }
    switch(entity->state)
    {
        case 0: Quezlcoatl_UpdateFly(entity); break;
        case 1: break;
        case 2:
            if(entity->subState==0)
            {
                u32 flipped;
                s32 offset;
                entity->subState++;
                flipped=*(u8 *)(ENTITY_RENDER_FLAGS_ADDRESS(entity))&0x40;
                offset=0x80000;
                if(flipped) offset=-0x80000;
                Quezlcoatl_SpawnDeathPiece(entity,entity->posX.whole+offset,entity->posY.whole-0x80000,18);
                Quezlcoatl_SpawnDeathPiece(entity,entity->posX.whole+offset,entity->posY.whole-0x80000,11);
                Effect_SpawnAt(entity->posX.whole,entity->posY.whole,entity->spriteLayer-1);
                Effect_SpawnRandom(entity->posX.whole,entity->posY.whole,0x80000,0x80000,entity->spriteLayer-1);
                Effect_SpawnRandom(entity->posX.whole,entity->posY.whole,0x80000,0x80000,entity->spriteLayer-1);
                Effect_SpawnRandom(entity->posX.whole,entity->posY.whole,0x80000,0x80000,entity->spriteLayer-1);
                entity->hidden=1;
            }
            else if(entity->subState==1)
            {
                entity->subState++;
                if(entity->ext.quezlcoatl.history) Heap_Free((void *)entity->ext.quezlcoatl.history);
            }
            else entity->deleteRequested=1;
            break;
        case 3: Quezlcoatl_UpdatePreview(entity); return;
    }
    Enemy_CheckHitPlayer(entity);
}

/* Quetzalcoatl: allocate the displacement history and create fifteen following body segments. */

void EnemyQuezlcoatlCreate(struct EwramData_EntityData *entity)
{
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    u32 box = 0x0C10F8F8;
    s32 index;
    u8 demo;

    if (!Quezlcoatl_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    entity->animFrame = 12;
    entity->ext.quezlcoatl.history = (s32)Heap_Alloc(60);
    if (!entity->ext.quezlcoatl.history)
    {
        EntityDelete(entity);
        return;
    }
    Enemy_ResetFromTemplate(entity);
    if (!gEwramData->gameState.bestiaryPreview)
        Entity_SnapToGround(entity);
    entity->scrollWithBg = 1;
    entity->ext.quezlcoatl.cameraX = camera->xPos.part16.integer;
    entity->ext.quezlcoatl.cameraY = camera->yPos.part16.integer;
    index = 14;
    do
    {
        struct EwramData_EntityData *part = Entity_CreateObject(QuezlcoatlSegment_Update);
        s32 graphics;

        if (!part)
            goto fail;
        part->spriteLayer = entity->spriteLayer + 1;
        graphics = ObjGfx_Find(sQuezlcoatlWhiteDragonGfx);
        Sprite_Attach(part, sQuezlcoatlWhiteDragonSpriteSheet, graphics, entity->basePalette);
        if (!gEwramData->gameState.bestiaryPreview)
            QuezlcoatlSegment_EnableHitBoxes(part);
        part->enemyId = entity->enemyId;
        part->basePalette = entity->basePalette;
        part->scrollWithBg = 0;
        part->drawFunc = Sprite_DrawEntity;
        ENTITY_OWNER(part) = entity;
        part->ext.quezlcoatlSegment.index = index;
        part->timer = index;
        part->attack = entity->attack;
        part->ext.quezlcoatl.cameraX = entity->ext.quezlcoatl.cameraX;
        part->ext.quezlcoatl.cameraY = entity->ext.quezlcoatl.cameraY;
        part->posX.whole = entity->posX.whole;
        part->posY.whole = entity->posY.whole - 0x40000;
        if (index <= 5)
            part->animFrame = 3;
        else if (index <= 9)
            part->animFrame = 15;
        else if (index <= 13)
            part->animFrame = 16;
        else if (index <= 14)
            part->animFrame = 17;
        continue;
    fail:
        /* the two failures above are separate copies; jump2 cross-jumps them into this one */
        EntityDelete(entity);
        return;
    } while (--index >= 0);
    demo = *(u8 *)((u8 *)gEwramData + 0x8E) & 0x40;
    if (demo)
    {
        entity->state = 3;
        return;
    }
    Hit_Enable(entity, 8, Quezlcoatl_DamagePlayerOnContact, Quezlcoatl_OnHit);
    HitBox_SetBody(entity, &box);
    HitBox_SetAttack(entity, &box);
    entity->flipX = Entity_IsTargetRight(entity);
    entity->state = demo;
}

/* Repeated leaps record motion for the body and turn at ledges. */
/**
 * @brief Flight state machine (state 0): fly across the room toward the target,
 * turning at the room edges (spawnParam0 or 15 px margin) with a wave motion,
 * and record the position history that the body segments follow.
 */
void Quezlcoatl_UpdateFly(struct EwramData_EntityData *entity)
{
    u8 (*history)[2]=(u8 (*)[2])entity->ext.quezlcoatl.history;
    s32 x,y,i,turn,collision;
    switch(entity->subState)
    {
        case 0: entity->subState=1;
        case 1:
            if(!(entity->ext.quezlcoatl.bounceCount&1)) entity->flipX=Entity_IsTargetRight(entity);
            if(!entity->spawnParam0) turn=Entity_GetHorizontalRoomEdge(entity,15);
            else turn=Entity_GetHorizontalRoomEdge(entity,entity->spawnParam0);
            if(turn==1) { entity->flipX=1; entity->ext.quezlcoatl.bounceCount=0; }
            else if(turn==2) { entity->flipX=0; entity->ext.quezlcoatl.bounceCount=0; }
            if(entity->flipX) entity->velX.whole=0x20000;
            else entity->velX.whole=-0x20000;
            entity->velY.whole=-0x40000;
            entity->accelY.whole=0x2800;
            entity->subState++;
            entity->timer=0;
            for(i=119;i>0;i--)
            {
                history[i][0]=history[i-1][0];
                history[i][1]=history[i-1][1];
            }
            history[0][0]=0; history[0][1]=0;
            break;
        case 2:
            x=(s16)entity->posX.part16.integer;
            y=entity->posY.part16.integer;
            if((u8)(entity->timer++-19)<=14) EntityDampVelocityX(entity,32);
            collision=Entity_MoveWithCollision(entity,0x10000,-0x100000,0x60000,8);
            x=(s16)entity->posX.part16.integer-x;
            y=entity->posY.part16.integer-y;
            for(i=119;i>0;i--)
            {
                history[i][0]=history[i-1][0];
                history[i][1]=history[i-1][1];
            }
            history[0][0]=x; history[0][1]=y;
            if(collision&4)
            {
                entity->ext.quezlcoatl.bounceCount++;
                entity->subState=1; entity->animFrame=12;
                Effect_SpawnRisingParticle(entity->posX.whole,entity->posY.whole,entity->spriteLayer-1,1);
                if(!EntityIsOutsideScreen(entity,6)) PlaySong(0x7C);
            }
            else
            {
                if((u32)(entity->velY.whole+0x1FFFF)<=0x3FFFE) entity->animFrame=13;
                else entity->animFrame=12;
            }
            break;
    }
}

/* Demo flight records a pair of byte displacements for each frame. */
/**
 * @brief Bestiary preview state (state 3): swing between x 24 and 119 px with the
 * same wave motion, recording the position history for the segments.
 */
void Quezlcoatl_UpdatePreview(struct EwramData_EntityData *entity)
{
    u8 (*history)[2]=(u8 (*)[2])entity->ext.quezlcoatl.history;
    s32 x,y,i;
    switch(entity->subState)
    {
        case 0: entity->subState=1;
        case 1:
            x=(s16)entity->posX.part16.integer;
            if(x<=24) entity->flipX=1;
            else if(x>119) entity->flipX=0;
            if(entity->flipX) entity->velX.whole=0x8000;
            else entity->velX.whole=-0x8000;
            entity->velY.whole=-0x20000;
            entity->accelY.whole=0x1800;
            entity->subState++;
            entity->timer=0;
            for(i=119;i>0;i--)
            {
                history[i][0]=history[i-1][0];
                history[i][1]=history[i-1][1];
            }
            history[0][0]=0;
            history[0][1]=0;
            break;
        case 2:
            x=(s16)entity->posX.part16.integer;
            y=entity->posY.part16.integer;
            EntityAccelerateAndMove(entity,0x40000);
            x=(s16)entity->posX.part16.integer-x;
            y=entity->posY.part16.integer-y;
            for(i=119;i>0;i--)
            {
                history[i][0]=history[i-1][0];
                history[i][1]=history[i-1][1];
            }
            history[0][0]=x;
            history[0][1]=y;
            if(entity->posY.part16.integer<=70)
            {
                if((u32)(entity->velY.whole+0xFFFF)<=0x1FFFE) entity->animFrame=13;
                else entity->animFrame=12;
            }
            else { entity->subState=1; entity->animFrame=12; }
            break;
    }
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Quezlcoatl_OnHit = WingedSkeleton_OnHit */

/**
 * @brief 7829C | Same as WingedSkeleton_OnHit.
 */
void Quezlcoatl_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    s32 damage = Hit_ResolveDamage(entity, attacker, hit, (s16)Player_CalculateAttackPower(hit[2]));

    entity->hitFlashTimer = 8;
    {
        u16 hp = entity->hp;

        // (s16)damage, written so that agbcc sign-extends after the HP load
        damage <<= 16;
        damage >>= 16;
        entity->hp = hp - damage;
    }
    Entity_ShowDamage(entity, (s16)damage, 0);
    if ((s16)entity->hp <= 0)
    {
        Enemy_DropRewards(entity);
        PlaySong(0x6B);
        entity->palette = entity->basePalette;
        entity->timer = 0x28;
        entity->state = 2;
        entity->subState = 0;
    }
}

/* Follow delayed head displacements, then detach and fall when it dies. */
#define SEGMENT(e) (*(s8 *)&e->ext.whiteDragonSegment.index)
/**
 * @brief Body segment update: follow the owner (type 0x1A) through its position
 * history with its visibility and palette; delete when the owner is gone.
 */
void QuezlcoatlSegment_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner=ENTITY_OWNER(entity);
    struct BgLayerState *camera=&gEwramData->bgInfo[1];
    s32 bounds[4];
    if(entity->state==0)
    {
        s8 *history;
        if(!owner || !owner->updateFunc || owner->enemyId!=26) goto fail;
        history=(s8 *)owner->ext.quezlcoatl.history;
        entity->hidden=owner->hidden;
        entity->palette=owner->palette;
        entity->timer++;
        if(!Enemy_IsStunnedNoFlash(owner))
        {
            entity->velX.part16.integer=history[SEGMENT(entity)*8+8]+(entity->ext.quezlcoatl.cameraX-camera->xPos.part16.integer);
            entity->velY.part16.integer=history[SEGMENT(entity)*8+9]+(entity->ext.quezlcoatl.cameraY-camera->yPos.part16.integer);
            entity->posX.part16.integer+=entity->velX.part16.integer;
            entity->posY.part16.integer+=entity->velY.part16.integer;
            entity->ext.quezlcoatl.cameraX=camera->xPos.part16.integer;
            entity->ext.quezlcoatl.cameraY=camera->yPos.part16.integer;
            if(history[SEGMENT(entity)*8]>0) entity->flipX=1;
            else if(history[SEGMENT(entity)*8]<0) entity->flipX=0;
            if(SEGMENT(entity)<=5)
            {
                if(entity->timer&8) entity->animFrame=4;
                else entity->animFrame=3;
            }
        }
        else
        {
            entity->posX.part16.integer+=entity->ext.quezlcoatl.cameraX-camera->xPos.part16.integer;
            entity->posY.part16.integer+=entity->ext.quezlcoatl.cameraY-camera->yPos.part16.integer;
            entity->ext.quezlcoatl.cameraX=camera->xPos.part16.integer;
            entity->ext.quezlcoatl.cameraY=camera->yPos.part16.integer;
        }
        if(!(entity->timer&7) && !gEwramData->gameState.bestiaryPreview) Hit_CheckWithPlayer((struct HitResult *)bounds,entity);
        if(owner->state==2)
        {
            entity->state=1;
            entity->subState=0;
            entity->accelX.whole=0;
            entity->accelY.whole=0x2000;
            entity->timer=RandomNumberGenerator()%4+60;
            entity->velX.part16.integer=history[SEGMENT(entity)*8+8];
            entity->velY.part16.integer=history[SEGMENT(entity)*8+9];
        }
        return;
fail:
        EntityDelete(entity);
    }
    else
    {
        if(entity->subState==0) { entity->subState++; entity->scrollWithBg=1; }
        EntityAccelerateAndMove(entity,0x70000);
        entity->hidden=Entity_GetVisibilityPattern(entity->timer);
        if((s8)--entity->timer<=0) entity->deleteRequested=1;
    }
}

/* QuezlcoatlSegment_EnableHitBoxes (hit boxes). */

/**
 * @brief 78544 | Activate box slot 8 with QuezlcoatlSegment_DamagePlayerOnContact and set both boxes to
 * the bytes of 0x0808FCFC (x -4, y -4, w 8, h 8 - not a ROM address);
 * clear flags bit 1, set bit 2.
 */
void QuezlcoatlSegment_EnableHitBoxes(struct EwramData_EntityData *entity)
{
    u32 box = 0x0808FCFC;

    Hit_Enable(entity, 8, QuezlcoatlSegment_DamagePlayerOnContact, 0);
    HitBox_SetBody(entity, &box);
    HitBox_SetAttack(entity, &box);
    entity->hit.flags &= ~2;
    entity->hit.flags |= 4;
}

/* Falling debris released by a body segment. */
/**
 * @brief Spawn a falling death piece (frame) at (x, y) with the owner's velocity
 * plus a random part (QuezlcoatlPiece_Update).
 */
void Quezlcoatl_SpawnDeathPiece(struct EwramData_EntityData *owner,s32 x,s32 y,s32 frame)
{
    struct EwramData_EntityData *effect=EntityCreateInRange(0x49,0xCF,QuezlcoatlPiece_Update);
    if(effect)
    {
        s32 graphics;
        
        effect->posX.whole=x;
        effect->posY.whole=y;
        effect->flipX=owner->flipX;
        effect->spriteLayer=owner->spriteLayer;
        graphics=ObjGfx_Find(sQuezlcoatlWhiteDragonGfx);
        Sprite_Attach(effect,sQuezlcoatlWhiteDragonSpriteSheet,graphics,owner->basePalette);
        effect->animFrame=frame;
        effect->velX.whole=owner->velX.whole+(RandomNumberGenerator()&0x7FFF)-0x4000;
        effect->velY.whole=(RandomNumberGenerator()&0x7FFF)-0x24000;
        effect->accelY.whole=0x2000;
        effect->timer=RandomNumberGenerator()%4+60;
        effect->scrollWithBg=1;
        effect->drawFunc=Sprite_DrawEntity;
    }
}

/* Falling fragments blink, emit periodic particles and expire. */
/**
 * @brief Death piece update: fall, blink, spawn an effect every 8 frames and
 * delete when the timer ends.
 */
void QuezlcoatlPiece_Update(struct EwramData_EntityData *entity)
{
    EntityAccelerateAndMove(entity,0x70000);
    entity->hidden=Entity_GetVisibilityPattern(entity->timer);
    if(!(entity->timer&7))
        Effect_SpawnAt(entity->posX.whole,entity->posY.whole,entity->spriteLayer-1);
    if((s8)entity->timer--<=0) EntityDelete(entity);
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Quezlcoatl_LoadGraphics = Object32_LoadGraphics
 *   Quezlcoatl_DamagePlayerOnContact = Enemy_DamagePlayerOnContact
 *   QuezlcoatlSegment_DamagePlayerOnContact = Enemy_DamagePlayerOnContact */

/**
 * @brief 786B0 | Same as Object32_LoadGraphics.
 */
u8 Quezlcoatl_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sQuezlcoatlWhiteDragonGfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sQuezlcoatlWhiteDragonPalette, 1);
    Sprite_Attach(entity, sQuezlcoatlWhiteDragonSpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 786FC | Same as Enemy_DamagePlayerOnContact.
 */
void Quezlcoatl_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}

/**
 * @brief 78728 | Same as Enemy_DamagePlayerOnContact.
 */
void QuezlcoatlSegment_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}
