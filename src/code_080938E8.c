#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern s32 MedusaHead_LoadGraphics(struct EwramData_EntityData *entity);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *entity);
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *entity);
extern void MedusaHead_DamagePlayerOnContact(struct EwramData_EntityData *entity);
extern void MedusaHead_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);

/**
 * @brief 938E8 | Medusa Head create: deleted when MedusaHead_LoadGraphics fails;
 * frozen time gives state 4; otherwise palette 3, box slot 8 (MedusaHead_DamagePlayerOnContact
 * / MedusaHead_OnHit) with box 0x0A0AFBFB; with unk_516 bit 1 it is the hidden
 * spawner (state 3, boxes off), else state 0 facing Entity_IsTargetRight with velX
 * 0xC000..0xFFFF towards it, accelY 0xC00 and a random wave phase
 * (unk_504..506).
 */
void EnemyMedusaHeadCreate(struct EwramData_EntityData *entity)
{
    u32 box = 0x0A0AFBFB;
    u32 r;

    if ((u8)MedusaHead_LoadGraphics(entity) == 0)
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
    entity->spriteLayer = 3;
    Hit_Enable(entity, 8, MedusaHead_DamagePlayerOnContact, (s32)MedusaHead_OnHit);
    HitBox_SetAttack(entity, &box);
    HitBox_SetBody(entity, &box);
    entity->hit.flags &= ~4;
    entity->hit.flags &= ~2;
    if ((s16)(entity->unk_514.part16.spawnParam1 & 2))
    {
        entity->state = 3;
        entity->hidden = 1;
        entity->hit.flags |= 4;
        entity->hit.flags |= 2;
        return;
    }
    entity->state = 0;
    entity->flipX = Entity_IsTargetRight(entity);
    if (entity->flipX)
        entity->velX.whole = (RandomNumberGenerator() & 0x3FFF) + 0xC000;
    else
        entity->velX.whole = -0xC000 - (RandomNumberGenerator() & 0x3FFF);
    entity->accelY.whole = 0xC00;
    r = RandomNumberGenerator() & 1;
    entity->unk_504.part8.unk_504 = r;
    entity->unk_504.part8.unk_505 = 0;
    r = (RandomNumberGenerator() & 0xF) + 0x2D;
    entity->unk_504.part8.unk_506 = r;
    entity->unk_504.part8.unk_505 = entity->unk_504.part8.unk_506 / 2;
}
