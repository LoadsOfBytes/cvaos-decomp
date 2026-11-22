/* Witch projectile setup, movement and spawn helper. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
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
        entity->unk_518.part16.hp = 1;
        entity->state = 1;
        /* fall through */
    case 1:
        if (entity->unk_4F8.whole <= 0x1FFFF) entity->unk_4F8.whole += 0x2000;
        flags = Entity_Walk(entity, entity->unk_4F8.whole, sWitchGroundProjectileWalkProfile, 8);
        if (flags & 8)
        {
            if (entity->stage == 0)
            {
                entity->stage = 1;
                PlaySong(0x68);
            }
            if (flags & 3) entity->state = 2;
        }
        hp = (s16)entity->unk_518.part16.hp;
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

