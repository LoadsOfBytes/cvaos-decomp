/* Golem debris (816BC/817B0). */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern u8 sCommonObjectGfx[];
extern u8 sCommonObjectSpriteSheet[];
extern s32 Entity_MoveWithCollisionDefault(struct EwramData_EntityData *, s32, s32);
extern void Effect_SpawnFallingPiece(s32, s32, s32, s32, s32, s32);
extern void Hit_CheckWithPlayerSharedHistory(void *, struct EwramData_EntityData *, struct EwramData_EntityData *);
extern void GolemDebris_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void GolemDebris_OnKilled(struct EwramData_EntityData *, s32, u16 *);
void GolemDebris_Update(struct EwramData_EntityData *entity);

/* Throw a piece of debris from the owner at (x, y) with (vx, vy): it hurts
 * the player (box 0x0808FCFC = x -4, y -4, w 8, h 8) and can be destroyed
 * (GolemDebris_DamagePlayerOnContact / GolemDebris_OnKilled). */
void Golem_ThrowDebris(struct EwramData_EntityData *owner, s32 x, s32 y, s32 vx, s32 vy)
{
    u32 box = 0x0808FCFC;
    struct EwramData_EntityData *debris = EntityCreateInRange(0x1B, 0x48, (u32 *)GolemDebris_Update);
    s32 graphics;

    if (debris == NULL)
        return;
    graphics = ObjGfx_Find(sCommonObjectGfx);
    if (graphics < 0)
    {
        EntityDelete(debris);
        return;
    }
    Sprite_Attach(debris, sCommonObjectSpriteSheet, graphics, 6);
    debris->animFrame = 0x42;
    debris->drawFunc = (u32 *)Sprite_DrawEntity;
    Hit_Enable(debris, 10, GolemDebris_DamagePlayerOnContact, (s32)GolemDebris_OnKilled);
    HitBox_SetAttack(debris, &box);
    HitBox_SetBody(debris, &box);
    debris->hit.flags &= ~4;
    debris->hit.flags &= ~2;
    debris->scrollWithBg = 1;
    debris->spriteLayer = owner->spriteLayer;
    debris->unk_4F8.unk_4F8_entity = owner;
    debris->posX.whole = x;
    debris->posY.whole = y;
    debris->velX.whole = vx;
    debris->velY.whole = vy;
    debris->accelY.whole = 0x2000;
    debris->flipX = owner->flipX;
    debris->attack = owner->attack;
}

/* Debris: fall; on hitting a wall or ceiling/floor (bits 2/3) bounce up
 * and break into one piece, on the floor (bits 0/1) bounce back and break
 * into two; vanish off screen. */
void GolemDebris_Update(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    struct EwramData_EntityData *owner = entity->unk_4F8.unk_4F8_entity;
    s32 hit, vx, vy;

    if (entity->state != 0)
        return;
    hit = Entity_MoveWithCollisionDefault(entity, 0x10000, -0x40000);
    if (EntityIsOutsideScreen(entity, 1))
    {
        EntityDelete(entity);
        return;
    }
    if (hit & 0xC)
    {
        entity->velY.whole = -(entity->velY.whole < 0 ? -entity->velY.whole : entity->velY.whole);
        Effect_SpawnFallingPiece(entity->posX.whole, entity->posY.whole, entity->velX.whole, entity->velY.whole, entity->spriteLayer, 2);
        EntityDelete(entity);
        return;
    }
    if (hit & 3)
    {
        entity->velX.whole = (entity->velX.whole >> 2) - entity->velX.whole;
        vx = (RandomNumberGenerator() & 0x1FFFF) - 0x10000;
        vy = (RandomNumberGenerator() & 0x1FFFF) - 0x10000;
        Effect_SpawnFallingPiece(entity->posX.whole, entity->posY.whole, entity->velX.whole + vx, entity->velY.whole + vy, entity->spriteLayer, 1);
        vx = (RandomNumberGenerator() & 0x1FFFF) - 0x10000;
        vy = (RandomNumberGenerator() & 0x1FFFF) - 0x10000;
        Effect_SpawnFallingPiece(entity->posX.whole, entity->posY.whole, entity->velX.whole + vx, entity->velY.whole + vy, entity->spriteLayer, 0);
        EntityDelete(entity);
        return;
    }
    Hit_CheckWithPlayerSharedHistory(collision, entity, owner);
}
