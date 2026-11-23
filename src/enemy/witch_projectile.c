/*
 * Witch ground projectile, shared by the witch enemies. Kept apart from student_witch.c: the callers there pass full-width offsets to Witch_SpawnGroundProjectile (u16 parameters).
 */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

/* Witch projectile setup, movement and spawn helper. */
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern void Enemy_OnContact(struct EwramData_EntityData *);
extern void WitchGroundProjectile_OnHit(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern struct EwramData_EntityData *Entity_CreateObject(void *);
extern u8 sWitchGfx[], sWitchPalette[], sWitchSpriteSheet[], sWitchGroundProjectileWalkProfile[];

/**
 * @brief Shared ground projectile: accelerate along terrain, burst on an
 * obstacle or cleared HP, and remove offscreen. Substate bits select
 * animation and hit behavior.
 */
void WitchGroundProjectile_Update(struct EwramData_EntityData *entity)
{
    u32 box[4];
    s32 flags;
    s32 hp;
    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, sWitchGfx, sWitchPalette, 0, sWitchSpriteSheet)) goto remove;
        if (!(entity->subState & 1)) Anim_StartIndexed(entity, 0xB, 0, 1);
        else Anim_StartIndexed(entity, 6, 0, 1);
        entity->spriteLayer = 0x13;
        Hit_Enable(entity, 8, Enemy_OnContact, (s32)WitchGroundProjectile_OnHit);
        HitBox_SetBodyRaw(entity, 0x1018F0F4);
        HitBox_SetAttackRaw(entity, 0x1018F0F4);
        entity->accelY.whole = 0x2000;
        entity->velY.whole = 0;
        if (entity->subState & 2) entity->hit.flags |= 2;
        entity->hp = 1;
        entity->state = 1;
        /* fall through */
    case 1:
        if (entity->ext.witchGround.speed <= 0x1FFFF) entity->ext.witchGround.speed += 0x2000;
        flags = Entity_Walk(entity, entity->ext.witchGround.speed, sWitchGroundProjectileWalkProfile, 8);
        if (flags & 8)
        {
            if (entity->stage == 0)
            {
                entity->stage = 1;
                PlaySong(0x68);
            }
            if (flags & 3) entity->state = 2;
        }
        hp = (s16)entity->hp;
        if (hp == 0) entity->state = 2;
        break;
    case 2:
        Effect_SpawnShared(entity->posX.whole, entity->posY.whole, 0, 7);
        goto remove;
    }
    if (EntityIsOutsideScreen(entity, 3))
    {
remove:
        EntityDelete(entity);
        return;
    }
    if (entity->state != 2) Hit_CheckWithPlayer(box, entity);
    Anim_Update(entity);
}

extern struct EwramData_EntityData *Entity_CreateObject(void *);
static inline s32 Add16(s16 a, u16 b) { return a + b; }

static inline void FollowParent(struct EwramData_EntityData *child, struct EwramData_EntityData *owner, u16 x_, u16 y_)
{
    u16 x = x_;
    s16 y = y_;
    child->flipX = owner->flipX;
    if (child->flipX) x = (-((u32)x << 16)) >> 16;
    child->posX.part16.integer = Add16(x, owner->posX.part16.integer);
    child->posY.part16.integer = (s32)y + owner->posY.part16.integer;
}

/* Spawn an attached effect at a mirrored offset, with half the owner value. */
/**
 * @brief Spawn the shared ground projectile at a mirrored offset with half
 * the owner damage value; copy the supplied mode into its substate.
 */
void Witch_SpawnGroundProjectile(struct EwramData_EntityData *owner, u16 x, u16 y, s32 state)
{
    struct EwramData_EntityData *child = Entity_CreateObject((u32 *)WitchGroundProjectile_Update);
    if (child != NULL)
    {
        child->attack = owner->attack >> 1;
        FollowParent(child, owner, x, y);
        child->subState = state;
        child->scrollWithBg = 1;
    }
}
