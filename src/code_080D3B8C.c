#include "code_080D73B8.h"
#include "graham_entity.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void GrahamAttacker_UpdateDeath(struct EwramData_EntityData *);
extern struct EwramData_EntityData *GrahamAttacker_CreateAttackEffect(struct EwramData_EntityData *);
extern void Entity_FaceTarget(struct EwramData_EntityData *);
extern s32 Entity_GetTargetOffsetX(struct EwramData_EntityData *);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *);
#define STATE(e, value) { (e)->state = value; (e)->stage = 0; (e)->subState = 0; }

/**
 * @brief Wait, move toward target X and sequence an attack effect with
 * timed recovery. Dispatch its death routine at state 0xF0 and mirror
 * owner defeat progression; bestiary only animates.
 */
void GrahamAttacker_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    u32 mode = entity->state;
    if (mode == 0xD0) { Anim_Update(entity); return; }
    if (mode == 0xF0) { GrahamAttacker_UpdateDeath(entity); return; }
    if (owner != NULL)
    {
        if (owner->enemyId != entity->enemyId)
        { entity->deleteRequested = 1; return; }
        if ((owner->ext.graham.flags & 0x10) && mode <= 0xDF)
            STATE(entity, 0xE0);
    }
    {
        s32 state = entity->state;
        switch (state)
        {
        case 0:
            if (entity->subState == 0)
            {
                entity->subState++;
                entity->timer = 0x40;
                entity->objMode = 1;
            }
            if (--entity->timer == 0xFF)
            {
                entity->objMode = 0;
                entity->state = 1;
                entity->stage = state;
                entity->subState = state;
            }
            break;
        case 1:
            if (entity->subState == 0)
            {
                s32 distance;
                Anim_StartIndexed(entity, 2, 0, 1);
                Entity_FaceTarget(entity);
                if (Entity_GetTargetOffsetX(entity) < 0)
                    distance = -Entity_GetTargetOffsetX(entity);
                else
                    distance = Entity_GetTargetOffsetX(entity);
                if (entity->flipX)
                {
                    entity->velX.whole = 0x30000;
                    entity->accelX.whole = 0;
                }
                else
                {
                    entity->velX.whole = -0x30000;
                    entity->accelX.whole = 0;
                }
                entity->timer = distance / (entity->velX.whole < 0 ? -entity->velX.whole : entity->velX.whole);
                entity->subState++;
                entity->stage = 0;
            }
            if (--entity->timer == 0xFF)
                STATE(entity, 2)
            else
            {
                entity->posX.whole += entity->velX.whole;
                entity->posY.whole += entity->velY.whole;
                entity->velX.whole += entity->accelX.whole;
                entity->velY.whole += entity->accelY.whole;
            }
            break;
        case 2:
            switch (entity->subState)
            {
            case 0:
                switch (entity->stage)
                {
                case 0:
                    if (entity->animScriptIndex != 5) Anim_StartIndexed(entity, 5, 0, 0);
                    entity->stage++;
                case 1:
                    if (entity->animDone)
                    {
                        PlaySong(0x111);
                        entity->subState++;
                        entity->stage = 0;
                    }
                    break;
                }
                break;
            case 1:
                switch (entity->stage)
                {
                case 0:
                    if (entity->animScriptIndex != 3) Anim_StartIndexed(entity, 3, 0, 1);
                    GrahamAttacker_CreateAttackEffect(entity);
                    entity->timer = 0x80;
                    entity->stage++;
                case 1:
                    if (entity->timer == 0x60) PlaySong(0x110);
                    if (--entity->timer == 0xFF) STATE(entity, 1);
                    break;
                }
                break;
            }
            break;
        case 0xE0:
            STATE(entity, 0xF0);
            return;
        }
    }
    Entity_AnimateWithBoxes(entity);
    Enemy_CheckHitPlayer(entity);
}
