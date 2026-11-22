/* Headhunter (entity type 0x6A): update. */
#include "code_08009A0.h"
#include "headhunter_entity.h"
#include "code_08013960.h"
#include "code_08033CAC.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "code/code_080211F0.h"
#include "structs/ewram.h"

struct SingleFrameAnimation {
    u16 count;
    u16 format;
    u8 frame;
    u8 duration;
    u16 unk_6;
};
extern struct SingleFrameAnimation sHeadhunterFrameAnim;
extern u8 sHeadhunterSpriteSheet[];
extern struct AnimScript sHeadhunterAnimScript0;
extern struct AnimScript sHeadhunterAnimScript1;
extern struct AnimScript sHeadhunterAnimScript2;
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Hit_EnableContact(struct EwramData_EntityData *, s32, void *, void *);
extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *);
extern void Entity_SnapToGround(struct EwramData_EntityData *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *);
extern s32 Anim_GetMaxBoxCount(struct EwramData_EntityData *);
extern s32 Entity_IsTimeStopped(struct EwramData_EntityData *);
extern struct EwramData_EntityData *Entity_CreateObject(u32 *);
extern void Headhunter_UpdateMovement(struct EwramData_EntityData *);
extern void Headhunter_UpdateAttack(struct EwramData_EntityData *);
extern void Headhunter_UpdateDeath(struct EwramData_EntityData *);
extern u8 Headhunter_LoadGraphics(struct EwramData_EntityData *);
extern void HeadhunterTransformationPart_Update(struct EwramData_EntityData *);
extern void Headhunter_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void Headhunter_OnHit(struct EwramData_EntityData *);

#define KIND(e) (*(s8 *)&HEADHUNTER_KIND((e)))

/* Headhunter: dispatch AI (0), attacks (1), defeat (2) and the frozen
 * defeated form (3); bit 1 of unk_4FE overrides the frame with unk_4FF,
 * bit 0 disables the contact damage. */
/**
 * @brief Dispatch movement (0), attacks (1), defeat (2) and bestiary
 * animation (3). Bit 1 of unk_4FE overrides the animation frame;
 * bit 0 disables contact checks. Defeat bypasses time-stop gating.
 */
void EnemyHeadhunterUpdate(struct EwramData_EntityData *entity)
{
    u8 collision[48];
    struct SingleFrameAnimation animation = sHeadhunterFrameAnim;
    if (entity->state != 2 && Entity_IsInRect(entity, -16, 0, 272, 176))
        Player_TrackEntity(entity);
    *(s8 *)&HEADHUNTER_FLAGS(entity) &= ~3;
    if (entity->state != 2) Enemy_UpdateHitFlash(entity);
    if (Entity_IsTimeStopped(entity) && entity->state != 2)
    {
        Hit_CheckWithPlayer(collision, entity);
        return;
    }
    switch (entity->state)
    {
    case 0: Headhunter_UpdateMovement(entity); break;
    case 1: Headhunter_UpdateAttack(entity); break;
    case 2: Headhunter_UpdateDeath(entity); break;
    case 3:
        Anim_Update(entity);
        return;
    }
    {
        u8 *initial = &entity->animFrame;
        u8 previous = *initial;
        s32 override = HEADHUNTER_FLAGS(entity) & 2;
        u8 *frame = initial;
        if (override)
        {
            Anim_Start(entity, (struct AnimScript *)&animation, 2, 1);
            animation.frame = HEADHUNTER_FORCED_FRAME(entity);
            Anim_Update(entity);
        }
        else Anim_Update(entity);
        if (previous != *frame) Entity_UpdateBoxes(entity);
    }
    if (entity->state != 2 && !(HEADHUNTER_FLAGS(entity) & 1))
        Hit_CheckWithPlayer(collision, entity);
}
