#include "code_08009A0.h"
#include "joint_entity.h"
#include "structs/ewram.h"
#include "string.h"
extern const u8 sGolemDelay[4];
extern void Golem_UpdateAllParts(struct EwramData_EntityData *);
extern void Golem_SpawnIdleEffect(struct EwramData_EntityData *, s32, s32);

/* Separate cases preserve the original cross-jumps between timed phases. */
/**
 * @brief Animation-only state 3 selected by the bestiary flag at
 * EWRAM offset 0x8E bit 6. Cycle timed joint poses and the Flesh Golem
 * idle effect before returning to the shared pose loop.
 */
void Golem_UpdateBestiaryPreview(struct EwramData_EntityData *entity)
{
    u8 delay[4];
    memcpy(delay, sGolemDelay, 4);
    switch (entity->subState) {
    case 0:
        if ((u8)entity->timer++ > 59) {
            entity->subState++;
            entity->timer = delay[entity->unk_508.whole];
            JOINT_POSE(entity) = 2;
        }
        break;
    case 1:
        Golem_UpdateAllParts(entity);
        if ((s8)--entity->timer <= 0) {
            entity->timer = delay[entity->unk_508.whole];
            entity->subState++;
            JOINT_POSE(entity) = 2;
        }
        break;
    case 2:
        Golem_UpdateAllParts(entity);
        if ((s8)--entity->timer <= 0) {
            entity->timer = 40;
            entity->subState++;
            JOINT_POSE(entity) = 2;
        }
        break;
    case 3:
        if ((s8)--entity->timer <= 0) {
            entity->timer = delay[entity->unk_508.whole];
            entity->subState++;
            JOINT_POSE(entity) = 3;
        }
        break;
    case 4:
        Golem_UpdateAllParts(entity);
        if ((s8)--entity->timer <= 0) {
            entity->timer = delay[entity->unk_508.whole];
            entity->subState++;
            JOINT_POSE(entity) = 4;
        }
        break;
    case 5:
        Golem_UpdateAllParts(entity);
        if ((s8)--entity->timer <= 0) {
            entity->timer = 40;
            entity->subState++;
            JOINT_POSE(entity) = 4;
        }
        break;
    case 6:
        if ((s8)--entity->timer <= 0) {
            entity->timer = delay[entity->unk_508.whole];
            entity->subState = 1;
            JOINT_POSE(entity) = 1;
        }
        break;
    case 20:
        if ((u8)entity->timer++ > 59) {
            entity->subState++;
            entity->timer = 20;
            JOINT_POSE(entity) = 6;
        }
        break;
    case 21:
        Golem_UpdateAllParts(entity);
        if ((s8)--entity->timer <= 0) {
            entity->timer = 4;
            entity->subState++;
            JOINT_POSE(entity) = 8;
        }
        break;
    case 22:
        Golem_UpdateAllParts(entity);
        if ((s8)--entity->timer <= 0) {
            entity->timer = 4;
            entity->subState++;
            JOINT_POSE(entity) = 6;
        }
        break;
    case 24:
        Golem_UpdateAllParts(entity);
        if ((s8)--entity->timer <= 0) {
            entity->subState++;
            entity->timer = 20;
            JOINT_POSE(entity) = 5;
        }
        break;
    case 25:
        Golem_UpdateAllParts(entity);
        if ((s8)--entity->timer <= 0) {
            entity->timer = 4;
            entity->subState++;
            JOINT_POSE(entity) = 9;
        }
        break;
    case 26:
        Golem_UpdateAllParts(entity);
        if ((s8)--entity->timer <= 0) {
            entity->timer = 4;
            entity->subState++;
            JOINT_POSE(entity) = 5;
        }
        break;
    case 28:
        Golem_UpdateAllParts(entity);
        if ((s8)--entity->timer <= 0) {
            entity->subState++;
            entity->timer = 20;
            JOINT_POSE(entity) = 0;
        }
        break;
    case 29:
        Golem_UpdateAllParts(entity);
        if ((s8)--entity->timer <= 0) {
            entity->timer = 4;
            entity->subState++;
            JOINT_POSE(entity) = 10;
        }
        break;
    case 30:
        Golem_UpdateAllParts(entity);
        if ((s8)--entity->timer <= 0) {
            entity->timer = 4;
            entity->subState++;
            JOINT_POSE(entity) = 0;
        }
        break;
    case 23:
    case 27:
    case 31:
        Golem_UpdateAllParts(entity);
        if ((s8)--entity->timer <= 0) {
            entity->timer = 20;
            entity->subState++;
        }
        break;
    case 32:
        Golem_UpdateAllParts(entity);
        if ((s8)--entity->timer <= 0) {
            entity->timer = delay[entity->unk_508.whole];
            entity->subState = 2;
            JOINT_POSE(entity) = 2;
        }
        break;
    case 40:
        if ((s8)entity->unk_4FC.part8.unk_4FD > 1) {
            entity->timer = delay[entity->unk_508.whole];
            entity->subState = 2;
            JOINT_POSE(entity) = 2;
        } else {
            s32 r = RandomNumberGenerator() % 60;
            if (r == 0) {
                entity->subState++;
                Golem_SpawnIdleEffect(entity, 0, 0);
                entity->timer = r;
                entity->unk_4FC.part8.unk_4FD++;
            }
        }
        break;
    case 41:
        if ((u8)entity->timer++ > 59) {
            s32 zero = 0;
            entity->subState = 40;
            entity->timer = zero;
        }
        break;
    }
}
