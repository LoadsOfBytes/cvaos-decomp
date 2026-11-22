/* Thrown hand weapon of an enemy: follows the owner, flies off in an arc once released. */
#include "code_08009A0.h"
#include "worm_entity.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_080211F0.h"
#include "structs/ewram.h"

#include "structs/entity_render_flags.h"


extern s32 Entity_IsTimeStopped(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayerSharedHistory(void *, struct EwramData_EntityData *, struct EwramData_EntityData *);
extern void Worm_StepCrawl(struct EwramData_EntityData *entity, s32 direction, s32 distance, s32 blocked);
extern void Worm_FollowGround(struct EwramData_EntityData *entity);
extern void Effect_SpawnRandom(s32, s32, s32, s32, s32);
extern u16 EntityAngleTo(struct EwramData_EntityData *, struct EwramData_EntityData *);
extern void Worm_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void PoisonWorm_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void Worm_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit);
extern u8 sWormGfx[], sWormSpriteSheet[];

#define ABS(x) ((x) < 0 ? -(x) : (x))

void WormSegment_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *target = WORM_PREVIOUS(entity);
    struct EwramData_EntityData *owner = WORM_ROOT(entity);
    u8 buf[0x10];
    s32 angle = 0;
    s32 amp = 0;

    entity->palette = owner->palette;
    if (entity->state != 0 && owner->state != 3)
    {
        if (owner->unk_510.part8.hitFlashTimer != 0 || Entity_IsTimeStopped(owner) != 0)
        {
            if (owner->state == 3)
                return;
            Player_QueueEntity(entity, 0);
            Hit_CheckWithPlayerSharedHistory(buf, entity, owner);
            return;
        }
        Player_QueueEntity(entity, 1);
    }
    switch (entity->state)
    {
    case 0:
        if (WORM_AGGRESSIVE(owner) != 0)
            goto delete;
        {
            s32 tiles = ObjGfx_LoadResource(sWormGfx, 0);
            Sprite_Attach(entity, sWormSpriteSheet, tiles, owner->unk_510.part8.basePalette);
        }
        entity->unk_510.part8.basePalette = owner->unk_510.part8.basePalette;
        entity->flipX = owner->flipX;
        if (WORM_SEGMENT_INDEX(entity) != 4)
            Anim_StartIndexed(entity, 1, 0, 1);
        else
            Anim_StartIndexed(entity, 2, 0, 1);
        Anim_Update(entity);
        entity->spriteLayer = 0x11;
        if (*((u8 *)gEwramData + 0x8E) & 0x40)
        {
            entity->state = 4;
            break;
        }
        if (WORM_VARIANT(owner) == 0)
            Hit_Enable(entity, 8, Worm_DamagePlayerOnContact, (s32)Worm_OnHit);
        else
            Hit_Enable(entity, 8, PoisonWorm_DamagePlayerOnContact, (s32)Worm_OnHit);
        HitBox_SetBodyRaw(entity, 0x1010F8F8);
        HitBox_SetAttackRaw(entity, 0x1010F8F8);
        entity->state++;
    case 1:
        Worm_StepCrawl(entity, WORM_CRAWL_DIR(owner), target->timer, 1);
        Worm_FollowGround(entity);
        if (!entity->flipX)
            angle = 0x8000;
        if (WORM_AGGRESSIVE(owner) != 0)
        {
            entity->state = 2;
            entity->timer = 0x38;
            entity->accelY.whole = angle;
            entity->velX.whole = (WORM_SEGMENT_INDEX(entity) << 15) - 0x6000;
            if (gEwramData->playerState.player->posX.whole > entity->posX.whole)
                entity->velX.whole = -entity->velX.whole;
            entity->velY.whole = -0x30000;
        }
        break;
    case 2:
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
        entity->velY.whole += 0x2000;
        if (entity->flipX)
            entity->accelY.whole -= 0x100;
        else
            entity->accelY.whole += 0x100;
        angle = entity->accelY.whole;
        if ((entity->timer & 7) == 0)
            Effect_SpawnRandom(entity->posX.whole, entity->posY.whole, 0x100000, 0x100000, entity->spriteLayer - 2);
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
    case 3:
    delete:
        if (ENTITY_RENDER_FLAGS16(entity) & 0x101F)
            ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
        return;
    }
    if (entity->state == 4)
    {
        if (!entity->flipX)
            angle = 0x8000;
        if (WORM_VARIANT(owner) != 0)
        {
            amp = ABS(Sin((u16)WORM_WAVE_PHASE(entity)) * 0x6000 / 0x10000);
            WORM_WAVE_PHASE(entity) += 0x200;
        }
        ObjAffine_Set((u16)EntityAngleTo(entity, target) + angle, 0x10000, amp + 0x10000, entity->flipX, 0, entity->affineIndex);
        Anim_Update(entity);
    }
    else
    {
        if (WORM_VARIANT(owner) != 0)
        {
            amp = ABS(Sin((u16)WORM_WAVE_PHASE(entity)) * 0x6000 / 0x10000);
            WORM_WAVE_PHASE(entity) += 0x200;
        }
        ObjAffine_Set((u16)EntityAngleTo(entity, target) + angle, 0x10000, amp + 0x10000, entity->flipX, 0, entity->affineIndex);
        if (owner->state != 3)
            Hit_CheckWithPlayerSharedHistory(buf, entity, owner);
        Anim_Update(entity);
    }
}
