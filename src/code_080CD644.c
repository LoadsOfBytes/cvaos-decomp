/* Alastor behavior dispatch and collision/animation refresh. */
#include "code/code_080211F0.h"
#include "structs/ewram.h"
extern void Alastor_UpdateBestiaryPreview(struct EwramData_EntityData *);
extern void Alastor_UpdateDeath(struct EwramData_EntityData *);
extern s32 Enemy_IsStunned(struct EwramData_EntityData *);
extern u8 Entity_IsTargetWithinRange(struct EwramData_EntityData *,s32);
extern void Hit_CheckWithPlayer(void *,struct EwramData_EntityData *);
extern void Alastor_UpdateAppear(struct EwramData_EntityData *);
extern void Alastor_UpdateApproach(struct EwramData_EntityData *);
extern void Alastor_UpdateAttackSelection(struct EwramData_EntityData *);
extern void Alastor_UpdateRushAttack(struct EwramData_EntityData *);
extern void Alastor_UpdatePatternAttack(struct EwramData_EntityData *);
extern void Alastor_UpdateAimedCharge(struct EwramData_EntityData *);
extern void Alastor_UpdateSpinningLeap(struct EwramData_EntityData *);
extern void Alastor_UpdateRecoil(struct EwramData_EntityData *);
extern void Alastor_UpdateHitBoxes(struct EwramData_EntityData *);
void EnemyAlastorUpdate(struct EwramData_EntityData *entity)
{
    u8 buffer[16];
    s32 previous=entity->unk_500.part16.unk_500;
    s32 facing=entity->flipX;
    if (entity->state==8)
    {
        Alastor_UpdateBestiaryPreview(entity);
        return;
    }
    if (entity->state==0xFF)
    {
        Alastor_UpdateDeath(entity);
        return;
    }
    Player_TrackEntity(entity);
    if (Enemy_IsStunned(entity))
    {
        if (!Entity_IsTargetWithinRange(entity,0x500000) && entity->state==0)
            return;
        if (!(entity->unk_4FC.part16.unk_4FC & 0x20) && entity->state!=0)
        {
            Hit_CheckWithPlayer(buffer,entity);
            return;
        }
    }
    switch (entity->state)
    {
        case 0: Alastor_UpdateAppear(entity); break;
        case 1: Alastor_UpdateApproach(entity); break;
        case 2: Alastor_UpdateAttackSelection(entity); break;
        case 3: Alastor_UpdateRushAttack(entity); break;
        case 4: Alastor_UpdatePatternAttack(entity); break;
        case 5: Alastor_UpdateAimedCharge(entity); break;
        case 6: Alastor_UpdateSpinningLeap(entity); break;
        case 7: Alastor_UpdateRecoil(entity); break;
    }
    if (entity->unk_500.part16.unk_500!=previous || entity->flipX!=facing)
        Alastor_UpdateHitBoxes(entity);
    if (!(entity->unk_4FC.part16.unk_4FC & 0x20))
        Hit_CheckWithPlayer(buffer,entity);
}
