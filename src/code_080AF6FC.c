/* Enemy helpers decompiled from assembly. */
#include "structs/ewram.h"
#include "skull_archer_entity.h"
#include "waiter_skeleton_entity.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code_08009A0.h"
#include "code/code_08060B98.h"
#include "code/code_08040A38.h"
#include "code_080D73B8.h"

extern s32 Enemy_TakeDamage(struct EwramData_EntityData *, s32, u16 *);
extern void EntityMoveX(struct EwramData_EntityData *);
extern void Enemy_DropRewards(struct EwramData_EntityData *);
extern void Entity_SetVelocityXFromFacing(struct EwramData_EntityData *, s32);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Effect_SpawnRandom(s32, s32, s32, s32, s32);
extern void Entity_FaceTarget(struct EwramData_EntityData *);
extern s32 Target_IsNearEntity(struct EwramData_EntityData *, u16, u16);
extern s32 sUnk_08528614[];
extern struct EwramData_EntityData *Entity_CreateObject(u32 *);
extern void SkullArcherArrow_OnHit(struct EwramData_EntityData *);
extern u8 sSkullArcherGfx[], sSkullArcherSpriteSheet[];
extern u8 sSkullArcherWalkProfile[], sSkullArcherPalette[];
extern s32 Entity_GetTargetDirection(struct EwramData_EntityData *);
extern s16 Entity_GetTargetDistance(struct EwramData_EntityData *);
extern void Effect_UpdateSpriteDebris(struct EwramData_EntityData *);
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, u8 *, void *, u8, void *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *);
extern void Entity_SnapToGround(struct EwramData_EntityData *);
extern void Entity_EnableBoxes(struct EwramData_EntityData *, void *, void *);
extern void Enemy_OnContact(struct EwramData_EntityData *);
extern void SkullArcher_OnHit(struct EwramData_EntityData *, struct EwramData_EntityData *, u16 *);

void WaiterSkeleton_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    if (Enemy_TakeDamage(entity, attacker, hit))
    {
        Enemy_DropRewards(entity);
        entity->palette = entity->unk_510.part8.basePalette;
        entity->timer = 0x28;
        entity->state = 3;
        entity->stage = 0;
        entity->subState = 0;
    }
}

/* Notify the parent when its low unk_4F4 byte still matches the child's. */
void WaiterSkeletonThrownTray_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage effect;
    struct EwramData_EntityData *parent = WAITER_TRAY_OWNER(entity);
    effect.attributes = 0;
    effect.unused = 0;
    effect.attack = entity->attack;
    effect.reaction = 0;
    effect.sourceX = entity->posX.whole;
    effect.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&effect);
    if (ENTITY_INDEX(parent) == ENTITY_INDEX(entity))
        WAITER_SKELETON_FLAGS(parent) |= 8;
}

void WaiterSkeletonThrownTray_OnHit(struct EwramData_EntityData *entity)
{
    entity->unk_518.part16.hp = 0;
}

/* Record the parent and initial relative position for the child effect. */
void SkullArcherArrow_DamagePlayerOnContact(struct EwramData_EntityData *entity, struct EwramData_EntityData *parent)
{
    struct PlayerContactDamage effect;
    effect.attributes = 0;
    effect.unused = 0;
    effect.attack = entity->attack;
    effect.reaction = 1;
    effect.sourceX = entity->posX.whole;
    effect.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&effect);
    entity->hit.flags |= 2;
    entity->hit.flags |= 4;
    SKULL_ARCHER_ARROW_OWNER(entity) = parent;
    entity->accelX.whole = entity->posX.whole - parent->posX.whole;
    entity->accelY.whole = entity->posY.whole - parent->posY.whole;
    entity->state = 1;
    entity->timer = 0x1E;
}

/* Projectile: move or follow its parent, then expire or break on impact. */
void SkullArcherArrow_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = SKULL_ARCHER_ARROW_OWNER(entity);
    u8 collision[16];
    if (entity->state != 2 && (s16)entity->unk_518.part16.hp <= 0)
        entity->state = 2;
    if (gEwramData->gameState.timeStopTimer != 0 && entity->state != 2)
        goto collision;
    if (EntityIsOutsideScreen(entity, 4))
    {
        EntityDelete(entity);
        return;
    }
    switch (entity->state)
    {
        case 0:
            Entity_SetVelocityXFromFacing(entity, 0x30000);
            EntityMoveX(entity);
        collision:
            Hit_CheckWithPlayer(collision, entity);
            break;
        case 1:
            entity->posX.whole = parent->posX.whole + entity->accelX.whole;
            entity->posY.whole = parent->posY.whole + entity->accelY.whole;
            if (entity->timer != 0)
            {
                if (--entity->timer == 10)
                    entity->drawSkip = 1;
            }
            else
                entity->deleteRequested = 1;
            break;
        case 2:
            Effect_SpawnRandom(entity->posX.whole + (entity->velX.whole << 3),
                         entity->posY.whole - 0x160000, 0x40000, 0x40000,
                         entity->spriteLayer - 1);
            EntityDelete(entity);
            break;
    }
}

void SkullArcher_UpdateIdle(struct EwramData_EntityData *entity)
{
    Entity_FaceTarget(entity);
    if (entity->subState == 0 && Target_IsNearEntity(entity, 240, 35))
    {
        s32 index = SKULL_ARCHER_PATTERN_STEP(entity) / 2;
        if (++SKULL_ARCHER_PATTERN_STEP(entity) > 4)
            SKULL_ARCHER_PATTERN_STEP(entity) = 0;
        entity->state = 1;
        entity->stage = 0;
        entity->subState = 0;
        SKULL_ARCHER_PATTERN(entity) = sUnk_08528614[index];
        Anim_StartIndexed(entity, index + 2, 0, 0);
    }
}

/* Emit an arrow when the next four-byte pattern entry matches the
 * animation frame; after the animation, wait before the next state. */
void SkullArcher_UpdateShoot(struct EwramData_EntityData *entity)
{
    s8 *pattern = (s8 *)SKULL_ARCHER_PATTERN(entity);
    switch (entity->subState)
    {
        case 0:
            if (pattern[0] == entity->animStep && entity->animTimer == 0)
            {
                struct EwramData_EntityData *arrow = Entity_CreateObject((u32 *)SkullArcherArrow_Update);
                if (arrow != NULL)
                {
                    s32 graphics;
                    PlaySong(0x85);
                    graphics = ObjGfx_Find(sSkullArcherGfx);
                    Sprite_Attach(arrow, sSkullArcherSpriteSheet, graphics, entity->unk_510.part8.basePalette);
                    arrow->animFrame = 0x19;
                    arrow->flipX = entity->flipX;
                    arrow->drawFunc = (u32 *)Sprite_DrawEntity;
                    arrow->spriteLayer = entity->spriteLayer;
                    arrow->posX.part16.integer = entity->posX.part16.integer;
                    arrow->posY.part16.integer = pattern[1] + entity->posY.part16.integer;
                    arrow->scrollWithBg = 1;
                    arrow->attack = entity->attack;
                    arrow->unk_518.part16.hp = 1;
                    Hit_Enable(arrow, 10, SkullArcherArrow_DamagePlayerOnContact, (s32)SkullArcherArrow_OnHit);
                    HitBox_SetAttackRaw(arrow, 0x0202E8EE);
                    HitBox_SetBodyRaw(arrow, 0x0202E8EE);
                    arrow->hit.flags &= ~4;
                }
                pattern += 4;
                SKULL_ARCHER_PATTERN(entity) = (s32)pattern;
            }
            if (entity->animFlags & 4)
            {
                entity->subState = 1;
                entity->timer = 0x20;
            }
            break;
        case 1:
            if (entity->timer != 0)
                entity->timer--;
            else if (entity->unk_514.part16.spawnParam0 == 0)
            {
                entity->state = 0;
                entity->stage = 0;
                entity->subState = 0;
                Anim_StartIndexed(entity, 0, 0, 1);
            }
            else
            {
                entity->state = 2;
                entity->stage = 0;
                entity->subState = 0;
                Anim_StartIndexed(entity, 1, 0, 1);
            }
            break;
    }
}


/**
 * @brief AFA9C | Skull Archer patrol: substate 0 walks (Entity_Walk,
 * turning every 128 frames) until the target is in front (Entity_GetTargetDirection
 * 1 when facing right, -1 when facing left) and closer than 80 pixels;
 * substate 1 backs off until 100 pixels away and then shoots (state 1,
 * one of the patterns in sUnk_08528614, sprite 2 + pattern).
 * Only matches without -f2003-patch. */
void SkullArcher_UpdatePatrol(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
        case 0:
            if (entity->timer++ > 128)
            {
                entity->timer = 0;
                entity->flipX ^= 1;
            }
            Entity_Walk(entity, 0x4000, sSkullArcherWalkProfile, 12);
            if (entity->flipX && (s8)Entity_GetTargetDirection(entity) == 1)
                goto close;
            if (!entity->flipX && (s8)Entity_GetTargetDirection(entity) == -1)
                goto close;
            break;
        close:
            if (Entity_GetTargetDistance(entity) <= 79)
                entity->subState = 1;
            break;
        case 1:
            Entity_Walk(entity, -0xC000, sSkullArcherWalkProfile, 13);
            if (Entity_GetTargetDistance(entity) > 99)
            {
                s32 index = SKULL_ARCHER_PATTERN_STEP(entity) / 2;
                if (++SKULL_ARCHER_PATTERN_STEP(entity) > 4)
                    SKULL_ARCHER_PATTERN_STEP(entity) = 0;
                entity->state = 1;
                entity->stage = 0;
                entity->subState = 0;
                SKULL_ARCHER_PATTERN(entity) = sUnk_08528614[index];
                Anim_StartIndexed(entity, index + 2, 0, 0);
            }
            break;
    }
}
