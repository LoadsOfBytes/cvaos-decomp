/* Belmont dispatch, hit flashing, collision and active hitbox gating. */
#include "code/code_080211F0.h"
#include "structs/ewram.h"
extern void Belmont_UpdateIntro(struct EwramData_EntityData *);
extern void Belmont_UpdateSceneBoundary(struct EwramData_EntityData *);
extern void Belmont_UpdateBestiaryPreview(struct EwramData_EntityData *);
extern void Belmont_UpdateDefeat(struct EwramData_EntityData *);
extern void Belmont_UpdateDodge(struct EwramData_EntityData *);
extern void Belmont_UpdateHitReaction(struct EwramData_EntityData *);
extern void Belmont_EnterCombat(struct EwramData_EntityData *);
extern void Belmont_UpdateCombat(struct EwramData_EntityData *);
extern void Belmont_UpdateMeleeAttack(struct EwramData_EntityData *);
extern void Belmont_UpdateJumpAttack(struct EwramData_EntityData *);
extern void Belmont_UpdateWaveAttack(struct EwramData_EntityData *);
extern void Belmont_UpdateSubweaponAttack(struct EwramData_EntityData *);
extern void Belmont_UpdateJumpSlide(struct EwramData_EntityData *);
extern void Belmont_UpdatePoweredPhase(struct EwramData_EntityData *);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *);
extern s32 Entity_IsInRect(struct EwramData_EntityData *,s16,s16,u16,u16);
extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *);
#define FLASH(e) ((e)->unk_4F8.part8.unk_4F9)
void EnemyBelmontUpdate(struct EwramData_EntityData *entity)
{
    if(entity->state==1) { Belmont_UpdateIntro(entity); return; }
    if(entity->state==13) { Belmont_UpdateSceneBoundary(entity); return; }
    if(entity->state==12) { Belmont_UpdateBestiaryPreview(entity); return; }
    if(entity->state==0xF0) { Belmont_UpdateDefeat(entity); return; }
    if(entity->state==9) { Belmont_UpdateDodge(entity); return; }
    if(entity->state==11) { Belmont_UpdateHitReaction(entity); Entity_AnimateWithBoxes(entity); return; }
    if(!(entity->enemyFlags & 2))
    {
        if(Entity_IsInRect(entity,-16,0,272,176))
            Player_TrackEntity(entity);
        Enemy_UpdateHitFlash(entity);
    }
    if(FLASH(entity)!=0)
    {
        { u8 value=FLASH(entity) & 1; entity->hidden=value; }
        FLASH(entity)--;
    }
    else
        entity->hidden=0;
    switch(entity->state)
    {
        case 3: Belmont_EnterCombat(entity); break;
        case 2: Belmont_UpdateCombat(entity); break;
        case 7: Belmont_UpdateMeleeAttack(entity); break;
        case 4: Belmont_UpdateJumpAttack(entity); break;
        case 5: Belmont_UpdateWaveAttack(entity); break;
        case 6: Belmont_UpdateSubweaponAttack(entity); break;
        case 10: Belmont_UpdateJumpSlide(entity); break;
        case 8: Belmont_UpdatePoweredPhase(entity); break;
    }
    Entity_AnimateWithBoxes(entity);
    if(entity->attack==0)
        entity->hit.flags|=2;
    else
        entity->hit.flags&=0xFD;
    if(entity->state!=11 && FLASH(entity)==0)
        Enemy_CheckHitPlayer(entity);
}
