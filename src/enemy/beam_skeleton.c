/*
 * Beam Skeleton: create, update dispatcher, patrol, approach, idle, fire and the beam.
 * Built without -f2003-patch (BeamSkeleton_UpdatePatrol needs it off; the rest compiles the same either way).
 */
#include "core.h"
#include "graphics.h"
#include "structs/ewram.h"
#include "sound.h"
#include "entity_util.h"

/* Beam Skeleton creation and update dispatcher. */
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *entity,u8 *graphics,void *palette,u8 slot,void *sprite);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *entity);
extern void Entity_SnapToGround(struct EwramData_EntityData *entity);
extern void Entity_EnableBoxes(struct EwramData_EntityData *entity,void *callback,s32 hitCallback);
extern void Enemy_OnContact(struct EwramData_EntityData *entity);
extern void BeamSkeleton_OnHit(struct EwramData_EntityData *entity);
extern s32 GetEntityRoomXPositionInteger(struct EwramData_EntityData *entity);
extern void BeamSkeleton_UpdateBestiaryPreview(struct EwramData_EntityData *entity);
extern void BeamSkeleton_UpdateDeath(struct EwramData_EntityData *entity);
extern void BeamSkeleton_UpdateFire(struct EwramData_EntityData *entity);
extern void BeamSkeleton_UpdatePatrol(struct EwramData_EntityData *entity);
extern void BeamSkeleton_UpdateApproach(struct EwramData_EntityData *entity);
extern void BeamSkeleton_UpdateIdle(struct EwramData_EntityData *entity);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *entity);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *entity);
extern s32 Enemy_PreUpdate(struct EwramData_EntityData *entity);
extern u8 sBeamSkeletonPalette[];
extern u8 sBeamSkeletonSpriteSheet[];
extern u8 sBeamSkeletonGfx[];

static inline void SetAffine(struct EwramData_EntityData *e,s32 index) { e->affineIndex = index; }
void EnemyBeamSkeletonCreate(struct EwramData_EntityData *entity)
{
    void *palette = sBeamSkeletonPalette;
    void *sprite = sBeamSkeletonSpriteSheet;
    u8 *graphics = sBeamSkeletonGfx;
    u8 display;
    u16 affine;
    s32 x;
    if (!Entity_LoadGraphics(entity,graphics,palette,0,sprite))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity,0,0,1);
    Anim_Update(entity);
    /* Read the whole flag byte: this caller retains mask 0x40 in display. */
    display = ((u8 *)&gEwramData->gameState)[0x2E] & 0x40;
    if (display != 0)
    {
        entity->state = 4;
        entity->stage = 0;
        entity->subState = 0;
        return;
    }
    affine = ObjAffine_Alloc(1);
    if ((s16)affine < 0)
    {
        EntityDelete(entity);
        return;
    }
    SetAffine(entity,affine);
    entity->affine = 0;
    ObjAffine_Set(0,0x10000,0x10000,entity->flipX,display,entity->affineIndex);
    entity->spriteLayer = 17;
    Enemy_ResetFromTemplate(entity);
    entity->state = display;
    entity->stage = display;
    entity->subState = display;
    Entity_EnableBoxes(entity,Enemy_OnContact,(s32)BeamSkeleton_OnHit);
    Entity_SnapToGround(entity);
    x = GetEntityRoomXPositionInteger(entity);
    entity->ext.beamSkeleton.homeX = x;
    entity->ext.beamSkeleton.patrolRight = x + 64;
    entity->ext.beamSkeleton.patrolLeft = x - 64;
}

void EnemyBeamSkeletonUpdate(struct EwramData_EntityData *entity)
{
    if (entity->state == 4)
        BeamSkeleton_UpdateBestiaryPreview(entity);
    else if (entity->state == 0xFF)
        BeamSkeleton_UpdateDeath(entity);
    else if (entity->state == 3)
    {
        BeamSkeleton_UpdateFire(entity);
        Entity_AnimateWithBoxes(entity);
        Enemy_CheckHitPlayer(entity);
    }
    else if (!Enemy_PreUpdate(entity))
    {
        entity->ext.beamSkeleton.unusedDelay = RandomNumberGenerator() % 80 + 40;
        switch (entity->state)
        {
            case 0: BeamSkeleton_UpdatePatrol(entity); break;
            case 1: BeamSkeleton_UpdateApproach(entity); break;
            case 2: BeamSkeleton_UpdateIdle(entity); break;
        }
        Entity_AnimateWithBoxes(entity);
        Enemy_CheckHitPlayer(entity);
    }
}

/* Beam Skeleton patrol and selection of the attack state. */
extern s32 Entity_Walk(struct EwramData_EntityData *entity,s32 speed,void *profile,s32 flags);
extern s32 Entity_ClampXToRoom(struct EwramData_EntityData *entity,s32 mode);
extern s32 Target_IsNearEntity(struct EwramData_EntityData *entity,s32 width,s32 height);
extern s16 sBeamSkeletonWalkProfile[];

/**
 * @brief Patrol state (0): walk within the stored range, turn at terrain
 * or room bounds and enter approach state 1 near the target.
 */
void BeamSkeleton_UpdatePatrol(struct EwramData_EntityData *entity)
{
    s32 blocked = 0;
    u16 left = entity->ext.beamSkeleton.patrolLeft;
    u16 right = entity->ext.beamSkeleton.patrolRight;

    switch (entity->subState)
    {
        case 0:
            if (entity->animScriptIndex != 1)
                Anim_StartIndexed(entity,1,0,1);
            entity->subState++;
            break;
        case 1:
        {
            u16 distance = entity->ext.beamSkeleton.homeX - GetEntityRoomXPositionInteger(entity);
            if (entity->flipX)
            {
                if ((s16)right < GetEntityRoomXPositionInteger(entity) || (s16)distance < -40)
                    entity->flipX = 0;
            }
            else
            {
                if ((s16)left > GetEntityRoomXPositionInteger(entity) || (s16)distance > 40)
                    entity->flipX = 1;
            }
            if (Entity_Walk(entity,0x6000,sBeamSkeletonWalkProfile,12) & 3)
                blocked = 1;
            if (Entity_ClampXToRoom(entity,1))
                blocked = 1;
            if (blocked)
                entity->flipX = 1 - entity->flipX;
            if (Target_IsNearEntity(entity,192,128))
            {
                entity->velX.whole = 0;
                entity->state = 1;
                entity->stage = 0;
                entity->subState = 0;
            }
            break;
        }
    }
}

/* Beam Skeleton approach and attack selection. */
extern s32 Entity_GetTargetDistanceY(struct EwramData_EntityData *entity);
extern s32 Entity_GetTargetDistanceX(struct EwramData_EntityData *entity);
extern void Entity_FaceTarget(struct EwramData_EntityData *entity);
extern s32 Entity_IsTargetInCenteredBox(struct EwramData_EntityData *entity,u16 width,u16 height);
#define ABS(x) ((x) < 0 ? -(x) : (x))

/**
 * @brief Approach state (1): vary target-relative movement and wait, then
 * select firing state 3 when aligned or idle state 2 from distance
 * and randomness. Return to patrol when the vertical distance is too large.
 */
void BeamSkeleton_UpdateApproach(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
        case 0:
        {
            s32 distance = (s16)Entity_GetTargetDistanceY(entity);
            if (ABS(distance) > 32)
            {
                entity->state = 0;
                entity->stage = 0;
                entity->subState = 0;
                return;
            }
            entity->ext.beamSkeleton.cooldown = RandomNumberGenerator() % 32;
            entity->subState++;
            entity->stage = 0;
        }
        case 1:
            Entity_FaceTarget(entity);
            entity->timer = 20;
            entity->ext.beamSkeleton.walkSpeed = RandomNumberGenerator() % 0x4000 + 0x8000;
            if (Entity_IsTargetInCenteredBox(entity,RandomNumberGenerator() % 128 + 64,128))
                entity->ext.beamSkeleton.walkSpeed = -entity->ext.beamSkeleton.walkSpeed;
            if (entity->animScriptIndex != 1)
                Anim_StartIndexed(entity,1,0,1);
            entity->subState++;
            entity->stage = 0;
        case 2:
            if (entity->ext.beamSkeleton.cooldown != 0)
                entity->ext.beamSkeleton.cooldown--;
            if (entity->ext.beamSkeleton.walkSpeed > 0)
                entity->ext.beamSkeleton.walkSpeed -= entity->ext.beamSkeleton.walkSpeed >> 4;
            else
                entity->ext.beamSkeleton.walkSpeed += ABS(entity->ext.beamSkeleton.walkSpeed) >> 4;
            if (entity->timer == 0)
            {
                if (entity->ext.beamSkeleton.cooldown == 0 && Entity_IsTargetInCenteredBox(entity,100,40))
                {
                    entity->state = 3;
                    entity->stage = 0;
                    entity->subState = 0;
                    return;
                }
                entity->ext.beamSkeleton.fireUrge++;
                if (RandomNumberGenerator() % 40 < entity->ext.beamSkeleton.fireUrge)
                {
                    s32 distance = (s16)Entity_GetTargetDistanceX(entity);
                    if (ABS(distance) > 48)
                    {
                        entity->ext.beamSkeleton.fireUrge = 0;
                        if (RandomNumberGenerator() % 50 <= 14)
                        {
                            entity->state = 2;
                            entity->stage = 0;
                            entity->subState = 0;
                        }
                        return;
                    }
                    entity->ext.beamSkeleton.fireUrge >>= 1;
                }
                entity->subState--;
                entity->stage = 0;
            }
            else
                entity->timer--;
            Entity_Walk(entity,entity->ext.beamSkeleton.walkSpeed,sBeamSkeletonWalkProfile,12);
            break;
    }
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   BeamSkeleton_UpdateIdle = Skeleton_UpdateIdle */

/**
 * @brief Idle state (2): play animation 0 for a 60-update countdown, then
 * return to approach state 1.
 */
void BeamSkeleton_UpdateIdle(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
        case 0:
            if (entity->animScriptIndex)
                Anim_StartIndexed(entity, 0, 0, 1);
            entity->timer = 0x3C;
            entity->subState++;
        case 1:
            if (--entity->timer == 0)
            {
                entity->state = 1;
                entity->stage = 0;
                entity->subState = 0;
            }
            break;
    }
}

/* Beam Skeleton firing sequence and vertical affine scale. */
extern void BeamSkeleton_CreateBeam(struct EwramData_EntityData *entity);
#define ANIM(e,n,loop) do { if ((e)->animScriptIndex != (n)) Anim_StartIndexed((e),(n),0,(loop)); } while (0)
#define SCALE(e) (*(u16 *)&(e)->ext.raw.unk_504.part16.unk_504)

static inline u16 FullScale(void) { return 0xFFFF; }
/**
 * @brief Firing state (3): sequence animations and shared vertical affine
 * scale, create beam entities at frame 19 and maintain the beam lifetime
 * flag until recovery. Preserve countdown-underflow and fallthrough behavior.
 */
void BeamSkeleton_UpdateFire(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
        case 0:
            ANIM(entity,2,0);
            if (entity->animDone)
            {
                entity->subState = 1;
                entity->stage = 0;
            }
            break;
        case 1:
            switch (entity->stage)
            {
                case 0:
                    ANIM(entity,3,1);
                    entity->timer = 32;
                    entity->stage++;
                    SCALE(entity) = 0;
                    ObjAffine_Set(0,0x10000,0,entity->flipX,0,entity->affineIndex);
                case 1:
                    if (--entity->timer == 0xFF)
                    {
                        entity->ext.beamSkeleton.firing |= 1;
                        entity->subState = 2;
                        entity->stage = 0;
                    }
                    break;
            }
            break;
        case 2:
            switch (entity->stage)
            {
                case 0:
                    SCALE(entity) = FullScale();
                    ANIM(entity,4,0);
                case 1:
                    if (entity->animFrame == 19)
                    {
                        BeamSkeleton_CreateBeam(entity);
                        entity->subState = 3;
                        entity->stage = 0;
                    }
                    break;
            }
            break;
        case 3:
            switch (entity->stage)
            {
                case 0:
                    ANIM(entity,5,1);
                    entity->timer = 42;
                    entity->stage++;
                case 1:
                {
                    s32 scale = SCALE(entity) + 0x2000;
                    s32 zero;
                    if (scale > 0xFFFF)
                        scale = 0xFFFF;
                    zero = 0;
                    SCALE(entity) = scale;
                    ObjAffine_Set(0,0x10000,SCALE(entity),entity->flipX,zero,entity->affineIndex);
                    if (--entity->timer == 0xFF)
                        entity->stage++;
                    break;
                }
                case 2:
                {
                    s32 scale = SCALE(entity) - 0x400;
                    s32 zero;
                    if (scale < 0)
                        scale = 0;
                    zero = 0;
                    SCALE(entity) = scale;
                    ObjAffine_Set(0,0x10000,SCALE(entity),entity->flipX,zero,entity->affineIndex);
                    if (SCALE(entity) == 0)
                    {
                        entity->timer = 8;
                        entity->subState = 4;
                        entity->stage = 0;
                    }
                    break;
                }
            }
            break;
        case 4:
            entity->ext.beamSkeleton.firing &= 0xFE;
            if (--entity->timer == 0xFF)
            {
                entity->state = 1;
                entity->stage = 0;
                entity->subState = 0;
            }
            break;
    }
}

extern void BeamSkeletonBeamSprite_Update(struct EwramData_EntityData *entity);
extern void BeamSkeletonBeam_Update(struct EwramData_EntityData *entity);
extern s8 ObjPalette_FindOrLoad(void *palette);
extern void Hit_Enable(struct EwramData_EntityData *entity,s32 mode,void *callback,s32 flags);
extern void HitBox_SetAttackRaw(struct EwramData_EntityData *entity,s32 flags);

/**
 * @brief Create a stationary beam sprite and moving contact entity using
 * the owner graphics and affine index. Preserve the distinct 1.5 and
 * integer 3/2 damage expressions and all allocation-failure behavior.
 */
void BeamSkeleton_CreateBeam(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *beam = EntityCreateInRange(0x49,0xCF,(u32 *)BeamSkeletonBeamSprite_Update);
    s32 i;
    if (beam != NULL)
    {
        s32 graphics;
        void *sprite;
        beam->flipX = entity->flipX;
        beam->posX.whole = -0xB0000;
        beam->posY.whole = -0x110000;
        if (beam->flipX)
            beam->posX.whole = -beam->posX.whole;
        beam->posX.whole += entity->posX.whole;
        beam->posY.whole += entity->posY.whole;
        beam->ext.beamSkeletonBeam.ownerIndex = ENTITY_INDEX(entity);
        beam->scrollWithBg = 1;
        beam->drawFunc = (u32 *)Sprite_DrawEntity;
        sprite = sBeamSkeletonSpriteSheet;
        graphics = Sprite_GetTileSlot(entity);
        Sprite_Attach(beam,sprite,graphics,ObjPalette_FindOrLoad(sBeamSkeletonPalette));
        beam->animFrame = 21;
        beam->objMode = 1;
        beam->spriteLayer = 18;
        beam->affineIndex = entity->affineIndex;
        beam->affine = 1;
        beam->attack = entity->attack * 1.5;
        beam->defense = entity->defense;
        beam->shrinkPartOffsets = 1;
    }
    for (i = 0; i < 1; i++)
    {
        beam = EntityCreateInRange(0x49,0xCF,(u32 *)BeamSkeletonBeam_Update);
        if (beam != NULL)
        {
            s32 graphics;
            beam->flipX = entity->flipX;
            beam->posX.whole = -0xB0000;
            beam->posY.whole = -0x110000;
            if (beam->flipX)
                beam->posX.whole = -beam->posX.whole;
            beam->posX.whole += entity->posX.whole;
            beam->posY.whole += entity->posY.whole;
            beam->ext.beamSkeletonBeam.ownerIndex = ENTITY_INDEX(entity);
            beam->scrollWithBg = 1;
            beam->drawFunc = (u32 *)Sprite_DrawEntity;
            graphics = Sprite_GetTileSlot(entity);
            Sprite_Attach(beam,sBeamSkeletonSpriteSheet,graphics,ObjPalette_FindOrLoad(sBeamSkeletonPalette));
            beam->animFrame = 22;
            beam->spriteLayer = 9;
            beam->affineIndex = entity->affineIndex;
            beam->attack = entity->attack * 3 / 2;
            beam->defense = entity->defense;
            Hit_Enable(beam,8,Enemy_OnContact,0);
            HitBox_SetAttackRaw(beam,0x0C08FAFC);
            beam->hit.flags |= 4;
            beam->timer = i;
        }
    }
}

/* Beam Skeleton moving beam and sound maintenance. */
extern void Hit_CheckWithPlayer(void *buffer,struct EwramData_EntityData *entity);
extern void Sound_PlayTracked(s32 id);
extern void Sound_StopTracked(s32 id);

static inline void ResetBeam(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = &gEwramData->entityData[entity->ext.beamSkeletonBeam.ownerIndex];
    entity->posX.whole = -0xB0000;
    if (entity->flipX)
        entity->posX.whole = -entity->posX.whole;
    entity->posX.whole += owner->posX.whole;
    entity->animFrame = 22;
    entity->velX.whole = 0xC0000;
    if (!entity->flipX)
    {
        entity->velX.whole = -entity->velX.whole;
        entity->accelX.whole = -entity->accelX.whole;
    }
}

/**
 * @brief Move and reset the contact beam relative to its owner, toggle
 * affine display from owner scale and maintain tracked sound 0x66.
 * Request deletion when the owner beam flag clears or the owner is hidden;
 * the shared affine slot remains owned by the enemy.
 */
void BeamSkeletonBeam_Update(struct EwramData_EntityData *entity)
{
    u8 buffer[16];
    struct EwramData_EntityData *owner = &gEwramData->entityData[entity->ext.beamSkeletonBeam.ownerIndex];
    if ((u16)owner->ext.beamSkeleton.beamLevel > 0x400)
        Hit_CheckWithPlayer(buffer,entity);
    switch (entity->state)
    {
        case 0:
            ResetBeam(entity);
            entity->state++;
        case 1:
            if (--entity->timer == 0xFF)
            {
                entity->timer = 2;
                entity->state++;
            }
            break;
        case 2:
            if ((u16)owner->ext.beamSkeleton.beamLevel > 0xFFFE)
            {
                if (--entity->timer == 0xFF)
                {
                    entity->state++;
                    entity->timer = 1;
                    entity->affine = 1;
                }
            }
            break;
        case 3:
            if (--entity->timer == 0xFF && entity->animFrame == 22)
                entity->animFrame = 23;
            entity->posX.whole += entity->velX.whole;
            entity->velX.whole += entity->accelX.whole;
            if (ABS((s16)entity->posX.part16.integer - (s16)owner->posX.part16.integer) > 92)
            {
                ResetBeam(entity);
                entity->timer = 1;
            }
            break;
    }
    if ((u16)owner->ext.beamSkeleton.beamLevel == 0xFFFF)
    {
        entity->affine = 0;
        ((u8 *)entity)[0x59] &= 0x7F;
    }
    else
    {
        entity->hit.flags |= 2;
        entity->affine = 1;
        ((u8 *)entity)[0x59] |= 0x80;
    }
    if (!(owner->ext.beamSkeleton.firing & 1) || owner->hidden)
    {
        Sound_StopTracked(0x66);
        entity->deleteRequested = 1;
    }
    else
    {
        if (entity->ext.beamSkeletonBeam.counter & 3)
        {
            u16 *sounds = gEwramData->trackedSoundIds;
            s32 i;
            for (i=0;i<4;i++,sounds++)
                if (*sounds == 0x66)
                    break;
            if (i==4)
                Sound_PlayTracked(0x66);
        }
        entity->ext.beamSkeletonBeam.counter++;
    }
}

/**
 * @brief Toggle the stationary beam sprite affine mode from owner scale;
 * request deletion when the owner beam flag clears or the owner is hidden.
 * Do not free the shared owner affine slot.
 */
void BeamSkeletonBeamSprite_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = &gEwramData->entityData[entity->ext.beamSkeletonBeam.ownerIndex];
    if ((u16)owner->ext.beamSkeleton.beamLevel > 0xFFFE)
    {
        entity->affine = 0;
        ((u8 *)entity)[0x59] &= 0x7F;
    }
    else
    {
        entity->affine = 1;
        ((u8 *)entity)[0x59] |= 0x80;
    }
    if (!(owner->ext.beamSkeleton.firing & 1) || owner->hidden)
        entity->deleteRequested = 1;
}

/* Beam Skeleton: seven fragments on death. */
extern void Effect_UpdateSpriteDebris(struct EwramData_EntityData *entity);
extern s32 Entity_GetTargetDirection(struct EwramData_EntityData *entity);
extern s8 sBeamSkeletonDeathPieceOffsets[][6];
#define INDEX(e) (e->ext.beamSkeletonPiece.index)

extern void BeamSkeleton_SpawnDeathPieces(struct EwramData_EntityData *entity);
extern void Enemy_DropRewards(struct EwramData_EntityData *entity);

/**
 * @brief Death state (0xFF): spawn the seven pieces once, drop rewards,
 * restore the palette, free the owner affine slot and request deletion.
 */
void BeamSkeleton_UpdateDeath(struct EwramData_EntityData *entity)
{
    entity->hidden = 1;
    switch (entity->subState)
    {
        case 0:
            BeamSkeleton_SpawnDeathPieces(entity);
            Enemy_DropRewards(entity);
            entity->palette = entity->basePalette;
            entity->timer = 50;
            entity->state = 0xFF;
            entity->subState++;
        case 1:
            ObjAffine_Free(entity->affineIndex);
            entity->deleteRequested = 1;
            break;
    }
}

/**
 * @brief Hide the owner and spawn up to seven table-configured sprite
 * fragments, counting them in the owner field and selecting direction
 * relative to the target.
 */
void BeamSkeleton_SpawnDeathPieces(struct EwramData_EntityData *entity)
{
    s32 i;
    entity->hidden = 1;
    for (i = 0; i < 7; i++)
    {
        struct EwramData_EntityData *part = EntityCreateInRange(0x49,0xCF,(u32 *)Effect_UpdateSpriteDebris);
        if (part != NULL)
        {
            s32 graphics;
            s32 x;
            part->ext.beamSkeletonPiece.ownerIndex = ENTITY_INDEX(entity);
            INDEX(part) = i;
            entity->ext.beamSkeleton.deathPieces++;
            part->spriteLayer = entity->spriteLayer;
            part->scrollWithBg = 1;
            graphics = Sprite_GetTileSlot(entity);
            Sprite_Attach(part,sBeamSkeletonSpriteSheet,graphics,ObjPalette_FindOrLoad(sBeamSkeletonPalette));
            part->drawFunc = (u32 *)Sprite_DrawEntity;
            part->posX.whole = entity->posX.whole;
            part->posY.whole = entity->posY.whole;
            x = sBeamSkeletonDeathPieceOffsets[INDEX(part)][0];
            part->posY.part16.integer += sBeamSkeletonDeathPieceOffsets[INDEX(part)][1];
            part->animFrame = sBeamSkeletonDeathPieceOffsets[INDEX(part)][2];
            part->velX.whole = (u8)sBeamSkeletonDeathPieceOffsets[INDEX(part)][3] << 12;
            part->velY.whole = -(sBeamSkeletonDeathPieceOffsets[INDEX(part)][4] << 12) - 16;
            part->accelY.whole = 0x3000;
            part->timer = sBeamSkeletonDeathPieceOffsets[INDEX(part)][5];
            part->flipX = entity->flipX;
            if ((s8)Entity_GetTargetDirection(entity) > 0)
            {
                part->velX.whole = -ABS(part->velX.whole);
                x = -ABS(x);
            }
            part->posX.part16.integer += x;
        }
    }
}

/* Alternate Beam Skeleton fragment update. */
static inline s32 TooLow(s32 y) { return y > 170; }

/**
 * @brief Unreferenced per-piece update that sets up a Beam Skeleton death piece
 * from sBeamSkeletonDeathPieceOffsets (the table of BeamSkeleton_UpdateDeath) on the
 * owner's graphics and lets it fly.
 */
void BeamSkeletonDeathPiece_UpdateUnused(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = &gEwramData->entityData[entity->ext.beamSkeletonPiece.ownerIndex];
    switch (entity->state)
    {
        case 0:
        {
            s32 graphics;
            s32 x;
            u16 *y;
            void *sprite;
            entity->spriteLayer = owner->spriteLayer;
            entity->scrollWithBg = 1;
            sprite = sBeamSkeletonSpriteSheet;
            graphics = Sprite_GetTileSlot(owner);
            Sprite_Attach(entity,sprite,graphics,ObjPalette_FindOrLoad(sBeamSkeletonPalette));
            entity->drawFunc = (u32 *)Sprite_DrawEntity;
            entity->posX.whole = owner->posX.whole;
            entity->posY.whole = owner->posY.whole;
            x = sBeamSkeletonDeathPieceOffsets[INDEX(entity)][0];
            y = &entity->posY.part16.integer;
            *y = sBeamSkeletonDeathPieceOffsets[INDEX(entity)][1] + *y + 16;
            entity->animFrame = sBeamSkeletonDeathPieceOffsets[INDEX(entity)][2];
            entity->velX.whole = (u8)sBeamSkeletonDeathPieceOffsets[INDEX(entity)][3] << 12;
            entity->velY.whole = -(sBeamSkeletonDeathPieceOffsets[INDEX(entity)][4] << 12) - 16;
            entity->timer = sBeamSkeletonDeathPieceOffsets[INDEX(entity)][5];
            entity->flipX = owner->flipX;
            if ((s8)Entity_GetTargetDirection(owner) > 0)
            {
                entity->velX.whole = -ABS(entity->velX.whole);
                x = -ABS(x);
            }
            entity->posX.part16.integer += x;
            entity->state++;
            break;
        }
        case 1:
        {
            s32 done = 0;
            if (--entity->timer == 0)
                done = 1;
            if (TooLow((s16)entity->posY.part16.integer))
                done = 2;
            if (done)
            {
                owner->ext.beamSkeleton.deathPieces--;
                entity->deleteRequested = 1;
            }
            break;
        }
    }
    entity->posX.whole += entity->velX.whole;
    entity->posY.whole += entity->velY.whole;
    entity->velY.whole += 0x3000;
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   BeamSkeleton_UpdateBestiaryPreview = Imp_UpdateState4Animation */

/**
 * @brief Bestiary state (4): start animation 0 once, then update it.
 */
void BeamSkeleton_UpdateBestiaryPreview(struct EwramData_EntityData *entity)
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

/* Beam Skeleton hit callback and Needles create/idle behavior. */
extern s32 Enemy_TakeDamage(struct EwramData_EntityData *entity);
extern void Needles_OnHit(struct EwramData_EntityData *entity);
extern s32 HitBox_SetBodyRaw(struct EwramData_EntityData *entity,s32 flags);
extern u8 sNeedlesPalette[];
extern u8 sNeedlesSpriteSheet[];
extern u8 sNeedlesGfx[];

/**
 * @brief Apply enemy damage; on defeat enter state 0xFF, immediately run
 * the death helper and play the death sound.
 */
void BeamSkeleton_OnHit(struct EwramData_EntityData *entity)
{
    if (Enemy_TakeDamage(entity))
    {
        entity->state = 0xFF;
        entity->stage = 0;
        entity->subState = 0;
        BeamSkeleton_UpdateDeath(entity);
        PlaySong(0x6B);
    }
}

void EnemyNeedlesCreate(struct EwramData_EntityData *entity)
{
    void *palette = sNeedlesPalette;
    void *sprite = sNeedlesSpriteSheet;
    u8 *graphics = sNeedlesGfx;
    s32 zero;
    if (!Entity_LoadGraphics(entity,graphics,palette,0,sprite))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity,0,0,1);
    entity->spriteLayer = 17;
    zero = 0;
    Enemy_ResetFromTemplate(entity);
    entity->flipX = 0;
    entity->state = 1;
    entity->stage = zero;
    entity->subState = zero;
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 3;
        return;
    }
    Hit_Enable(entity,8,Enemy_OnContact,(s32)Needles_OnHit);
    HitBox_SetAttackRaw(entity,0x1010F8F8);
    HitBox_SetBodyRaw(entity,0x1010F8F8);
    if (entity->spawnParam0 != 0)
        entity->ext.needles.bobPhase = RandomNumberGenerator() % 0xFFFF;
}

/**
 * @brief Alive state: pause and resume animation with random countdowns.
 * The configured moving variant also bobs vertically and updates X motion.
 */
void Needles_UpdateIdle(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
        case 0:
            if (entity->animScriptIndex != 0)
                Anim_StartIndexed(entity,0,0,1);
            entity->animDone = 1;
            entity->subState = 1;
            entity->stage = 0;
        case 1:
            switch (entity->stage)
            {
                case 0:
                    if (entity->animDone)
                    {
                        entity->stage = 1;
                        entity->timer = RandomNumberGenerator() % 128;
                        entity->animDone = 0;
                        entity->animFlags |= 2;
                    }
                    break;
                case 1:
                    if (--entity->timer == 0xFF)
                    {
                        entity->stage = 0;
                        entity->animFlags &= 0xFD;
                    }
                    break;
            }
            break;
        default:
            return;
    }
    if (entity->spawnParam0 != 0)
    {
        s32 vy = Sin((u16)entity->ext.needles.bobPhase + 0x4000) / 4;
        entity->ext.needles.bobPhase += 0x400;
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += vy;
        entity->velX.whole += entity->accelX.whole;
        entity->velY.whole = vy + entity->accelY.whole;
    }
}

/* Needles 0x080CB6BC..0x080CB94B (Needles_UpdateBestiaryPreview / Needles_OnHit are copies of the Blue Crow
 * BlueCrow_UpdateBestiaryPreview / BlueCrow_OnHit). */

/* Five 6-byte needle records for Needles_SpawnDeathParts (a struct would be padded to
 * 8 bytes by agbcc): offset x, offset y, animFrame, velX / 0x1000,
 * -velY / 0x1000, timer (timer). */
enum {
    NEEDLE_OFFSET_X,
    NEEDLE_OFFSET_Y,
    NEEDLE_UNK_2,
    NEEDLE_VEL_X,
    NEEDLE_VEL_Y,
    NEEDLE_TIMER
};

extern const s8 sNeedlesDeathParts[5][6];

void NeedlesPart_Update(struct EwramData_EntityData *entity);
void Needles_UpdateDeath(struct EwramData_EntityData *entity);
void Needles_UpdateBestiaryPreview(struct EwramData_EntityData *entity);

/**
 * @brief Spawn up to five table-configured needle parts with mirrored
 * velocity and offsets; count the live parts in the owner field.
 */
void Needles_SpawnDeathParts(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child;
    const s8 *needle;
    s32 offsetX;
    s32 i;

    for (i = 0; i < 5; i++)
    {
        child = EntityCreateInRange(0x49, 0xCF, (u32 *)NeedlesPart_Update);
        if (child == NULL)
            continue;
        needle = sNeedlesDeathParts[i];
        entity->ext.needlesDeath.parts++;
        ENTITY_PARENT_INDEX(child) = ENTITY_INDEX(entity);
        child->spriteLayer = entity->spriteLayer;
        child->basePalette = entity->basePalette;
        child->flipX = entity->flipX;
        Sprite_Attach(child, sNeedlesSpriteSheet, Sprite_GetTileSlot(entity), child->basePalette);
        child->scrollWithBg = 1;
        child->drawFunc = (u32 *)Sprite_DrawEntity;
        child->posX.whole = entity->posX.whole;
        child->posY.whole = entity->posY.whole;
        offsetX = needle[NEEDLE_OFFSET_X];
        child->posY.part16.integer += needle[NEEDLE_OFFSET_Y];
        child->animFrame = needle[NEEDLE_UNK_2];
        child->velX.whole = needle[NEEDLE_VEL_X] << 12;
        child->velY.whole = -(needle[NEEDLE_VEL_Y] << 12);
        child->timer = needle[NEEDLE_TIMER];
        if (child->flipX)
        {
            child->velX.whole = -child->velX.whole;
            offsetX = -offsetX;
        }
        child->posX.part16.integer += offsetX;
    }
}

/**
 * @brief Move a needle part under gravity. On bounds, timer-underflow or
 * bottom-Y completion, decrement the owner part count and request deletion.
 */
void NeedlesPart_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];

    if (!Entity_IsInRect(entity, -0x80, -0x40, 0x1F0, 0x120) || (u8)--entity->timer == 0xFF
        || entity->posY.part16.integer > 0xAA)
    {
        parent->ext.needlesDeath.parts--;
        entity->deleteRequested = 1;
    }
    else
    {
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
        entity->velY.whole += 0x3000;
    }
}

/**
 * @brief CB854 | Needles update: state 3 -> Needles_UpdateBestiaryPreview, state 2 ->
 * Needles_UpdateDeath, otherwise Needles_UpdateIdle unless Enemy_PreUpdate says otherwise.
 */
void EnemyNeedlesUpdate(struct EwramData_EntityData *entity)
{
    if (entity->state == 3)
    {
        Needles_UpdateBestiaryPreview(entity);
    }
    else if (entity->state == 2)
    {
        Needles_UpdateDeath(entity);
    }
    else if (!Enemy_PreUpdate(entity))
    {
        Needles_UpdateIdle(entity);
        Anim_Update(entity);
        Enemy_CheckHitPlayer(entity);
    }
}

/**
 * @brief Death state (2): hide, drop rewards and spawn the five parts.
 * Request deletion after the live-part count reaches zero.
 */
void Needles_UpdateDeath(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
        case 0:
            PlaySong(0x70);
            entity->hidden = 1;
            Enemy_DropRewards(entity);
            entity->palette = entity->basePalette;
            Needles_SpawnDeathParts(entity);
            entity->subState++;
        case 1:
            if (entity->ext.needlesDeath.parts == 0)
                entity->deleteRequested = 1;
            break;
    }
}

/**
 * @brief Bestiary state (3): select animation 0 once, then update it.
 */
void Needles_UpdateBestiaryPreview(struct EwramData_EntityData *entity)
{
    if (entity->subState == 0)
    {
        if (entity->animScriptIndex != 0)
            Anim_StartIndexed(entity, 0, 0, 1);
        entity->subState++;
    }
    Anim_Update(entity);
}

/**
 * @brief Apply enemy damage; on defeat enter death state 2 with substate
 * counters reset. Reward drops belong to the death helper.
 */
void Needles_OnHit(struct EwramData_EntityData *entity)
{
    if (Enemy_TakeDamage(entity))
    {
        entity->state = 2;
        entity->stage = 0;
        entity->subState = 0;
    }
}
