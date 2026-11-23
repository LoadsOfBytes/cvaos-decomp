/* Stolas attack substates and return to the movement pose. */
#include <string.h>
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/entity_template.h"
extern void Stolas_SpawnCastEffect(struct EwramData_EntityData *, s32, s32);
extern void Stolas_SpawnBeamSegment(struct EwramData_EntityData *, s32, s32);
extern void Stolas_SpawnBeamImpact(struct EwramData_EntityData *, s32, s32);
extern void Stolas_AttachJoint(struct EwramData_EntityData *, s32, s32, s32, s32);
extern void Stolas_InterpolateJointsInOrder(struct EwramData_EntityData *, s8 *);
extern void EntityMoveX(struct EwramData_EntityData *);
extern const s8 sUnk_081190AC[6];
/* Six 0x30-byte joints, then three child slots and their template IDs. */
struct StolasParts {
    struct EntityJointPart joints[6];
    struct EwramData_EntityData *children[3];
    s32 ids[3];
};
#define POSE(e) (e->ext.joint.pose)
#define FLAGS(e) (e->ext.stolas.flags)
#define FRAME(e) (e->ext.stolas.heldFrame)
#define DISTANCE(e) (e->ext.stolas.beamLength)
#define TYPE_A(e) ((e)->unk_514.part16.spawnParam0)
#define TYPE_B(e) ((e)->unk_514.part16.spawnParam1)
/**
 * @brief Attack state machine (state 1): cast effect (Stolas_SpawnCastEffect),
 * a column of five beam segments in front (Stolas_SpawnBeamSegment) and at
 * frame 73 the impact (Stolas_SpawnBeamImpact); then back to state 0.
 */
void Stolas_UpdateAttack(struct EwramData_EntityData *entity)
{
    s8 order[6];
    /* Function scope preserves the original order of spilled pointer slots. */
    struct EwramData_EntityData *child;
    memcpy(order, sUnk_081190AC, sizeof(order));
    switch (entity->subState)
    {
    case 0:
        entity->subState++;
        entity->timer = 0;
        Stolas_SpawnCastEffect(entity, entity->flipX ? 0xC0000 : -0xC0000, -0x2E0000);
        /* fall through */
    case 1:
        FLAGS(entity) |= 1;
        FRAME(entity) = 13;
        if (entity->timer++ > 59)
        {
            entity->timer = 0;
            entity->subState++;
            FLAGS(entity) = (u8)FLAGS(entity) & 0xFE;
            Anim_StartIndexed(entity, 2, 0, 0);
            Stolas_SpawnBeamSegment(entity, DISTANCE(entity), 1);
            Stolas_SpawnBeamSegment(entity, DISTANCE(entity), -31);
            Stolas_SpawnBeamSegment(entity, DISTANCE(entity), -63);
            Stolas_SpawnBeamSegment(entity, DISTANCE(entity), -95);
            Stolas_SpawnBeamSegment(entity, DISTANCE(entity), -127);
            PlaySong(0x11F);
        }
        break;
    case 2:
        if (++entity->timer == 73) Stolas_SpawnBeamImpact(entity, DISTANCE(entity), 1);
        if (entity->timer == 112)
        {
            child = EntityCreateInRange(0x1B, 0x48, (u32 *)EntityUpdateNothing);
            if (child != NULL)
            {
                struct StolasParts *parts = (struct StolasParts *)entity->ext.joint.parts;
                s32 type;
                s32 index;
                if ((RandomNumberGenerator() & 127) <= 31)
                {
                    if (TYPE_B(entity) >= 0) type = TYPE_B(entity);
                    else type = TYPE_A(entity);
                }
                else type = TYPE_A(entity);
                /* Add the scaled index before each array-field offset. Rewriting
                 * these as direct indexed fields changes the address arithmetic. */
                for (index = 0; index < 3; index++)
                {
                    if (((struct StolasParts *)((u8 *)parts + index * 4))->children[0] == NULL)
                    {
                        ((struct StolasParts *)((u8 *)parts + index * 4))->children[0] = child;
                        ((struct StolasParts *)((u8 *)parts + index * 4))->ids[0] = type;
                        break;
                    }
                }
                if (entity->flipX)
                    child->posX.part16.integer = DISTANCE(entity) + entity->posX.part16.integer;
                else
                {
                    u16 distance = DISTANCE(entity);
                    child->posX.part16.integer = entity->posX.part16.integer - distance;
                }
                child->posY.part16.integer = entity->posY.part16.integer;
                child->flipX = entity->flipX;
                child->scrollWithBg = 1;
                child->unk_518.part8.spawnIndex = 0;
                child->unk_518.part8.enemyId = type;
                child->drawFunc = (u32 *)Sprite_DrawEntity;
                {
                    struct EntityTemplate *template = &sEnemyData[type];
                    child->updateFunc = template->updateFunc;
                    template->createFunc(child);
                }
            }
        }
        if (entity->animDone)
        {
            entity->state = 0;
            entity->timer = 36;
            entity->subState = 1;
            FLAGS(entity) = (u8)FLAGS(entity) & 0xFE;
            Anim_StartIndexed(entity, 1, 0, 0);
            POSE(entity) = 1;
            Stolas_AttachJoint(entity, 1, 0, 0x14800, 0x80000);
            Stolas_AttachJoint(entity, 2, 1, 0x16800, 0xd0000);
            Stolas_AttachJoint(entity, 4, 3, 0x13000, 0x80000);
            Stolas_AttachJoint(entity, 5, 4, 0x13800, 0xd0000);
            if (entity->flipX) entity->velX.whole = 0xD0000 / entity->timer;
            else entity->velX.whole = -0xD0000 / entity->timer;
            entity->velY.whole = 0x10000;
            entity->accelY.whole = 0x2800;
        }
        break;
    case 10:
        entity->subState++;
            POSE(entity) = 7;
            Stolas_AttachJoint(entity, 1, 0, 0x17000, 0x80000);
            Stolas_AttachJoint(entity, 2, 1, 0x17000, 0xd0000);
            Stolas_AttachJoint(entity, 4, 3, 0x13000, 0x80000);
            Stolas_AttachJoint(entity, 5, 4, 0x13000, 0xd0000);
        entity->timer = 12;
        if (entity->flipX) entity->velX.whole = (0x100000 / entity->timer);
        else entity->velX.whole = (-0x100000 / entity->timer);
        entity->ext.joint.hurtMask |= 4;
        break;
    case 11:
        Stolas_InterpolateJointsInOrder(entity, order);
        EntityMoveX(entity);
        if ((s8)--entity->timer <= 0)
        {
            entity->timer = 30;
            entity->subState++;
        }
        break;
    case 12:
        Stolas_InterpolateJointsInOrder(entity, order);
        if ((s8)--entity->timer <= 0)
        {
            entity->timer = 30;
            entity->subState++;
            POSE(entity) = 5;
            Stolas_AttachJoint(entity, 1, 0, 0x12800, 0x80000);
            Stolas_AttachJoint(entity, 2, 1, 0x13000, 0xd0000);
            Stolas_AttachJoint(entity, 4, 3, 0x14000, 0x80000);
            Stolas_AttachJoint(entity, 5, 4, 0x15000, 0xd0000);
            if (entity->flipX) entity->velX.whole = -0xE0000 / entity->timer;
            else entity->velX.whole = 0xE0000 / entity->timer;
        }
        break;
    case 13:
        Stolas_InterpolateJointsInOrder(entity, order);
        EntityMoveX(entity);
        if ((s8)--entity->timer <= 6)
        {
            entity->timer = 12;
            entity->subState++;
            if (entity->flipX) entity->velX.whole = (-0x20000 / entity->timer);
            else entity->velX.whole = (0x20000 / entity->timer);
        }
        break;
    case 14:
        Stolas_InterpolateJointsInOrder(entity, order);
        EntityMoveX(entity);
        if ((s8)--entity->timer <= 0)
        {
            s32 one;
            entity->state = 0;
            /* Retain this local to order the state/timer stores as in ROM. */
            one = 1;
            entity->timer = 36;
            entity->subState = one;
            POSE(entity) = one;
            Stolas_AttachJoint(entity, 1, 0, 0x14800, 0x80000);
            Stolas_AttachJoint(entity, 2, 1, 0x16800, 0xd0000);
            Stolas_AttachJoint(entity, 4, 3, 0x13000, 0x80000);
            Stolas_AttachJoint(entity, 5, 4, 0x13800, 0xd0000);
            if (entity->flipX) entity->velX.whole = 0xD0000 / entity->timer;
            else entity->velX.whole = -0xD0000 / entity->timer;
            entity->velY.whole = 0x10000;
            entity->accelY.whole = 0x2800;
            entity->ext.joint.hurtMask &= ~4;
        }
        break;
    }
}
