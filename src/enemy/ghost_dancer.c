/*
 * Ghost Dancer (create, graphics, update, unused state, movement, death, hits, bestiary preview) and the Skeleton create / update.
 * Built without -f2003-patch (GhostDancer_UpdateUnusedState needs it off; the rest compiles the same either way).
 */
#include "graphics.h"
#include "structs/ewram.h"
#include "core.h"
#include "sound.h"
#include "player_core.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "math_anim.h"
#include "oam.h"
#include "obj_gfx.h"
#include "obj_palette.h"
#include "skeleton.h"
#include "data/data_118B48.h"
#include "hitbox.h"
#include "damage_numbers.h"
#include "entity_hit.h"
#include "bg_and_collision.h"


u8 GhostDancer_LoadGraphics(struct EwramData_EntityData *);
void GhostDancer_DamagePlayerOnContact(struct EwramData_EntityData *entity);
void GhostDancer_OnHit(struct EwramData_EntityData *entity,s32 hit,u16 *info);
void GhostDancer_UpdateBestiaryPreview(struct EwramData_EntityData *);
void GhostDancer_UpdateMovement(struct EwramData_EntityData *);
void GhostDancer_UpdateDeath(struct EwramData_EntityData *);

void EnemyGhostDancerCreate(struct EwramData_EntityData *input)
{
    /* agbcc otherwise assigns r4 to the zero kept across Enemy_ResetFromTemplate,
     * exchanging r4/r5 throughout this function. Preserve the ROM allocation. */
    register struct EwramData_EntityData *entity asm("r4") = input; /* FAKEMATCH: pinned register */
    if (GhostDancer_LoadGraphics(entity))
    {
        u8 flags;
        entity->spriteLayer = 3;
        Enemy_ResetFromTemplate(entity);
        entity->state = 1;
        entity->stage = 0;
        entity->subState = 0;
        /* The byte temporary retains the original narrowing and provides the
         * zero halfword written below when this test is false. */
        flags = ((u8 *)gEwramData)[0x8E] & 0x40;
        if (flags)
            entity->state = 5;
        else
        {
            entity->ext.ghostDancer.phase = flags;
            entity->ext.ghostDancer.cycles = 0;
            entity->ext.ghostDancer.side = 1;
            entity->objMode = 1;
            entity->ext.ghostDancer.mode = 0;
            entity->ext.ghostDancer.timer = 0;
            Entity_EnableBoxes(entity, GhostDancer_DamagePlayerOnContact, GhostDancer_OnHit);
        }
    }
}

/* Ghost Dancer's renderer and initial visibility/collision state. */
/**
 * @brief Find or load graphics and the selected palette variant, attach
 * the sprite and start animation 0. Delete and return 0 on failure;
 * return 1 on success.
 */
u8 GhostDancer_LoadGraphics(struct EwramData_EntityData *entity)
{
    u8 *palette=sGhostDancerPalette;
    u8 *animation=sGhostDancerSpriteSheet;
    u8 *graphics=sGhostDancerGfx;
    s32 slot=ObjGfx_Find(graphics);
    if(slot<0)
    {
        slot=ObjGfx_LoadResource(graphics,0);
        if(slot<0)
        {
            EntityDelete(entity);
            return 0;
        }
    }
    entity->basePalette=ObjPalette_FindOrLoadVariant(palette,(u8)entity->spawnParam0);
    if(entity->basePalette==255)
    {
        EntityDelete(entity);
        return 0;
    }
    if(!Sprite_Attach(entity,animation,slot,entity->basePalette))
    {
        EntityDelete(entity);
        return 0;
    }
    if(!Anim_StartIndexed(entity,0,0,1))
    {
        EntityDelete(entity);
        return 0;
    }
    Anim_Update(entity);
    entity->drawFunc=Sprite_DrawEntity;
    return 1;
    EntityDelete(entity);
    return 0;
}

/* Visibility flashes alternate around movement, damage and death processing. */
#define MODE(e) ((e)->ext.ghostDancer.mode)
#define TIMER(e) ((e)->ext.ghostDancer.timer)
void EnemyGhostDancerUpdate(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    if(entity->state==5)
    {
        GhostDancer_UpdateBestiaryPreview(entity);
        return;
    }
    entity->hidden=EntityIsOutsideScreen(entity,4);
    if(entity->state!=3)
    {
        s32 mode=MODE(entity);
        switch(mode)
        {
            case 0:
                entity->objMode=0;
                break;
            case 1:
                if(mode &= TIMER(entity))
                    entity->objMode=0;
                else
                    entity->objMode=1;
                if((s8)TIMER(entity)++>32)
                {
                    MODE(entity)++;
                    TIMER(entity)=0;
                }
                break;
            case 2:
                entity->objMode=1;
                break;
            case 3:
                if(TIMER(entity) & 1)
                    entity->objMode=1;
                else
                    entity->objMode=0;
                if((s8)TIMER(entity)++>32)
                {
                    MODE(entity)=0;
                    TIMER(entity)=0;
                }
                break;
        }
    }
    if(Enemy_IsStunned(entity))
    {
        if(entity->state!=3)
            Hit_CheckWithPlayer(&result,entity);
        return;
    }
    switch(entity->state)
    {
        case 1:
        case 2:
            GhostDancer_UpdateMovement(entity);
            break;
        case 3:
            GhostDancer_UpdateDeath(entity);
            break;
    }
    Entity_AnimateWithBoxes(entity);
    if(entity->state!=3)
        Hit_CheckWithPlayer(&result,entity);
}

/* Alternate movement helper reacts to the player's distance and random pauses. */
static inline s32 HorizontalDistance(struct EwramData_EntityData *entity)
{
    s32 value=(s16)Entity_GetTargetDistanceX(entity);
    if(value<0) value=-value;
    return value;
}
/**
 * @brief Unreferenced movement state between the Ghost Dancer functions:
 * reverse the velocity, then react when the target is above within 160
 * px and horizontally close.
 */
void GhostDancer_UpdateUnusedState(struct EwramData_EntityData *entity)
{
    switch(entity->subState)
    {
        case 0:
            if(!entity->flipX)
                entity->velX.whole=-entity->velX.whole;
            entity->velX.whole=-entity->velX.whole;
            Anim_StartIndexed(entity,0,0,1);
            entity->subState++;
            break;
        case 1:
        {
            u8 *animation;
            if((s16)Entity_GetTargetDistanceY(entity)<0 && (s16)Entity_GetTargetDistanceY(entity)>-160 && HorizontalDistance(entity)<=127)
            {
                u32 frame=entity->animScriptIndex;
                animation=&entity->animScriptIndex;
                if(frame!=1) Anim_StartIndexed(entity,1,0,1);
            }
            else
            {
                u32 frame=entity->animScriptIndex;
                animation=&entity->animScriptIndex;
                if(frame!=0) Anim_StartIndexed(entity,0,0,1);
            }
            if((s16)Entity_GetTargetDistanceY(entity)<0 && (s16)Entity_GetTargetDistanceY(entity)>-160 && HorizontalDistance(entity)<=95)
            {
                entity->state=2;
                entity->subState=0;
                break;
            }
            if(!(RandomNumberGenerator() & 127) && *animation==0)
            {
                Anim_StartIndexed(entity,1,0,1);
                entity->subState++;
            }
            break;
        }
        case 2:
            if((*(u32 *)&entity->animFlags & 0xFFFFFF00)==0x100)
            {
                Anim_StartIndexed(entity,0,0,1);
                entity->subState--;
            }
            break;
    }
}

/* Horizontal and vertical sine motion, alternating frames at turning points. */
#define PHASE(e) ((e)->ext.ghostDancer.phase)
/**
 * @brief Alive states (1/2): move with horizontal and vertical sine phases,
 * swap animations at horizontal turning points and advance visibility
 * modes at half-period boundaries.
 */
void GhostDancer_UpdateMovement(struct EwramData_EntityData *entity)
{
    s32 state=entity->subState;
    switch(state)
    {
        case 0:
            Anim_StartIndexed(entity,0,0,1);
            entity->velX.whole=state;
            entity->velY.whole=state;
            if((s16)Entity_GetTargetDistanceX(entity)>0)
                entity->flipX=0;
            else
                entity->flipX=1;
            if(!entity->flipX)
                entity->velX.whole=-entity->velX.whole;
            PHASE(entity)=0;
            entity->ext.ghostDancer.cycles=0;
            entity->timer=0;
            entity->subState++;
            break;
        case 1:
            entity->velX.whole=(u32)Sin(PHASE(entity)+0x4000)<<1;
            entity->posX.whole+=entity->velX.whole;
            entity->posY.whole+=entity->velY.whole+Sin(PHASE(entity)*4);
            PHASE(entity)+=128;
            if(!PHASE(entity))
                entity->ext.ghostDancer.cycles++;
            if(!entity->velX.whole)
                Anim_StartIndexed(entity,(u16)(1-entity->animScriptIndex),0,1);
            { s32 phase=PHASE(entity);
            s32 period=0x8000;
            if(phase%period==0)
            { u32 mode=(u8)entity->ext.ghostDancer.mode;
            if(!(state &= mode))
            {
                entity->ext.ghostDancer.side=1-entity->ext.ghostDancer.side;
                entity->ext.ghostDancer.mode=mode+1;
                entity->ext.ghostDancer.timer=0;
            }
            }
            }
            break;
    }
}

/* Play the defeated animation with the entity's palette and delete at its end. */
/**
 * @brief Death state (3): play animation 2 with the restored palette and
 * request deletion when the animation is done.
 */
void GhostDancer_UpdateDeath(struct EwramData_EntityData *entity)
{
    switch(entity->subState)
    {
        case 0:
            Anim_StartIndexed(entity,2,0,0);
            entity->palette=entity->basePalette;
            entity->objMode=1;
            entity->state=3;
            entity->subState++;
            entity->objMode=1;
        case 1:
            if(entity->animDone)
                entity->deleteRequested=1;
            break;
    }
}

/* Apply damage, flash the sprite, then enter the defeated animation. */
/**
 * @brief Resolve and display damage with hit flash; on defeat drop rewards,
 * restore the palette, play the death sound and start death state 3.
 */
void GhostDancer_OnHit(struct EwramData_EntityData *entity,s32 hit,u16 *info)
{
    s32 damage=Hit_ResolveDamage(entity,(struct EwramData_EntityData *)hit,info,Player_CalculateAttackPower(info[2]));
    entity->hitFlashTimer=8;
    { u16 hp=entity->hp;
        damage=(u32)damage<<16;
        damage>>=16;
        entity->hp=hp-damage; }
    Entity_ShowDamage(entity,damage,0);
    if((s16)entity->hp<=0)
    {
        Enemy_DropRewards(entity);
        entity->palette=entity->basePalette;
        entity->state=3;
        entity->subState=0;
        PlaySong(0x6D);
        GhostDancer_UpdateDeath(entity);
    }
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   GhostDancer_UpdateBestiaryPreview = Imp_UpdateState4Animation
 *   GhostDancer_DamagePlayerOnContact = Enemy_DamagePlayerOnContact */

/**
 * @brief Bestiary state (5): start animation 0 once, then update it.
 */
void GhostDancer_UpdateBestiaryPreview(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
        case 0:
            Anim_StartIndexed(entity, 0, 0, 1);
            entity->subState++;
        case 1:
            Anim_Update(entity);
            break;
    }
}

/**
 * @brief Apply stored contact damage with zero effect type and flags at
 * the entity position.
 */
void GhostDancer_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}

/* Skeleton enemy: graphics, child entity and patrol range around the spawn X. */

void EnemySkeletonCreate(struct EwramData_EntityData *entity)
{
    void *palette = sSkeletonWaiterSkeletonPalette;
    void *sprite = sSkeletonSpriteSheet;
    u8 *graphics = sSkeletonGfx;
    s32 affine;
    s32 x;
    s32 zero;
    s32 zero2;
    struct EwramData_EntityData *child;

    if (!Entity_LoadGraphics(entity, graphics, palette, 0, sprite))
    {
        EntityDelete(entity);
        return;
    }
    entity->ext.skeleton.flags |= 1;
    zero = 0;
    if (Entity_CreateLinkedAndHide(entity, SkeletonLinkedPart_Update) == NULL)
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    if (((u8 *)gEwramData)[0x8E] & 0x40)
    {
        entity->state = 5;
        entity->stage = zero;
        entity->subState = zero;
        return;
    }
    affine = ObjAffine_Alloc(1);
    if (affine < 0)
    {
        entity->deleteRequested = 1;
        return;
    }
    zero2 = 0;
    *(u16 *)&entity->ext.skeleton.affine = affine;
    entity->affineIndex = affine;
    entity->affine = 0;
    entity->spriteLayer = 0x11;
    
    entity->state = zero2;
    entity->stage = zero2;
    entity->subState = zero2;
    Enemy_ResetFromTemplate(entity);
    Entity_EnableBoxes(entity, Enemy_OnContact, Skeleton_OnHit);
    Entity_SnapToGround(entity);
    x = GetEntityRoomXPositionIntegerWide(entity);
    entity->ext.skeleton.homeX = x;
    entity->ext.skeleton.patrolRight = x + 0x20;
    entity->ext.skeleton.patrolLeft = x - 0x20;
    child = Entity_CreateLinkedAndHide(entity, SkeletonAttachedPart_Update);
    if (child == NULL)
        return;
    child->hidden = 0;
    entity->ext.skeleton.flags |= 4;
    if (RandomNumberGenerator() & 1)
        entity->state = 1;
    else
        entity->state = 1;
    entity->stage = zero2;
    entity->subState = zero2;
}

/* EnemySkeletonUpdate. */
/* Skeleton update dispatch and affine rotation. */

void EnemySkeletonUpdate(struct EwramData_EntityData *entity)
{
    if (!entity->flipX)
        entity->ext.skeleton.spinAngle += -0x708;
    else
        entity->ext.skeleton.spinAngle += 0x708;
    ObjAffine_Set(entity->ext.skeleton.spinAngle, 0x10000, 0x10000, 0, 0, entity->ext.skeleton.affine);
    if (entity->state == 5)
        Skeleton_UpdateState5Animation(entity);
    else if (entity->state == 0xFF)
        Skeleton_UpdateDeath(entity);
    else if (Enemy_PreUpdate(entity) == 0)
    {
        entity->ext.skeleton.unusedDelay = RandomNumberGenerator() % 80 + 40;
        switch (entity->state)
        {
        case 0: Skeleton_UpdatePatrol(entity); break;
        case 1: Skeleton_UpdateCombatMovement(entity); break;
        case 2: Skeleton_UpdateIdle(entity); break;
        case 3: Skeleton_UpdateAttackPreparation(entity); break;
        case 4: Skeleton_UpdateThrow(entity); break;
        }
        Entity_AnimateWithBoxes(entity);
        Enemy_CheckHitPlayer(entity);
    }
}
