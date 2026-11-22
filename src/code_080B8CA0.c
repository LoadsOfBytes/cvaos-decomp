/* Witch and Student Witch initialisation and state dispatchers. */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *);
extern void Entity_SnapToGround(struct EwramData_EntityData *);
extern void Enemy_OnContact(struct EwramData_EntityData *);
extern void StudentWitch_OnHit(struct EwramData_EntityData *, s32, u16 *);
extern void Witch_OnHit(struct EwramData_EntityData *, s32, u16 *);
extern void Entity_EnableBoxes(struct EwramData_EntityData *, void *, void *);
extern s32 Hit_EnableContact(struct EwramData_EntityData *, s32, void *, s32);
extern s32 Enemy_IsStunned(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *);
extern void StudentWitch_UpdateWalk(struct EwramData_EntityData *);
extern void StudentWitch_UpdateGroundAttack(struct EwramData_EntityData *);
extern void StudentWitch_UpdateAirAttack(struct EwramData_EntityData *);
extern s32 StudentWitch_UpdateDeath(struct EwramData_EntityData *);
extern void Witch_UpdateWait(struct EwramData_EntityData *);
extern void Witch_UpdateCast(struct EwramData_EntityData *);
extern void Witch_UpdateCharge(struct EwramData_EntityData *);
extern s32 Witch_UpdateDeath(struct EwramData_EntityData *);
extern u8 sWitchGfx[], sWitchPalette[], sWitchSpriteSheet[];

void EnemyStudentWitchCreate(struct EwramData_EntityData *entity)
{
    if (!Entity_LoadGraphics(entity, sWitchGfx, sWitchPalette, 0, sWitchSpriteSheet))
    {
        EntityDelete(entity);
        return;
    }
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer = 0x13;
    Anim_StartIndexed(entity, 7, 0, 1);
    Anim_Update(entity);
    if (gEwramData->gameState.bestiaryPreview) entity->state = 5;
    else
    {
        Hit_EnableContact(entity, 8, Enemy_OnContact, (s32)StudentWitch_OnHit);
        Entity_UpdateBoxes(entity);
        Entity_SnapToGround(entity);
        entity->posY.part16.integer++;
        entity->scrollWithBg = 1;
        entity->state = 0;
    }
}

void EnemyStudentWitchUpdate(struct EwramData_EntityData *entity)
{
    u32 box[4];
    u32 outside = EntityIsOutsideScreen(entity, 4);
    outside = (entity->hidden = outside);
    if (!outside) Player_TrackEntity(entity);
    if (Enemy_IsStunned(entity))
    {
        if (entity->state != 4) Hit_CheckWithPlayer(box, entity);
        return;
    }
    switch (entity->state)
    {
    case 0: StudentWitch_UpdateWalk(entity); break;
    case 3: StudentWitch_UpdateGroundAttack(entity); break;
    case 2: StudentWitch_UpdateAirAttack(entity); break;
    case 4:
        if (StudentWitch_UpdateDeath(entity)) { EntityDelete(entity); return; }
        break;
    case 5:
        Anim_Update(entity);
        return;
    }
    if (entity->state != 4) Hit_CheckWithPlayer(box, entity);
    Entity_AnimateWithBoxes(entity);
}

