/* Great Armor update. */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *);
extern s32 Entity_ClampXToRoom(struct EwramData_EntityData *, s32);
extern void Heap_Free(void *);
extern s32 Entity_IsTimeStopped(struct EwramData_EntityData *);
extern void Effect_SpawnRisingParticle(s32, s32, s32, s32);
extern void GreatArmor_UpdateWalk(struct EwramData_EntityData *);
extern void GreatArmor_UpdateAttack(struct EwramData_EntityData *);

struct ArmorBox {
    s32 x:8;
    s32 y:8;
    u32 w:8;
    u32 h:8;
};

#define TO_PIXELS(d) ((d) < 0 ? -(-(d) >> 16) : (d) >> 16)
/* set by the AI when the shield arm (part 4) is up: makes box 2 harmless */
#define GUARD(e) (*(s8 *)&(e)->unk_504.part8.unk_505)

/* Great Armor: dispatch AI (0) / attacks (1), crumble when killed (2,
 * dust, free the parts), frozen (3); the second box follows part 4. */
void EnemyGreatArmorUpdate(struct EwramData_EntityData *entity)
{
    u8 collision[48];
    struct ArmorBox box;
    s32 *parts; /* GreatArmorPart viewed as 10 words (needed for the addressing) */

    GUARD(entity) = 0;
    if (entity->state != 2)
    {
        if (EntityIsOutsideScreen(entity, 6)) return;
        if (Entity_IsInRect(entity, -16, 0, 272, 176)) Player_TrackEntity(entity);
        if (entity->state != 2) Enemy_UpdateHitFlash(entity);
    }
    if (Entity_IsTimeStopped(entity) && entity->state != 2)
        goto boxes;
    switch (entity->state)
    {
    case 0:
        GreatArmor_UpdateWalk(entity);
        break;
    case 1:
        GreatArmor_UpdateAttack(entity);
        break;
    case 2:
        if (entity->timer >= 121 && entity->timer <= 214 && (entity->timer & 3) == 0)
        {
            s32 x = RandomNumberGenerator() % 0x280000 - 0x140000;
            RandomNumberGenerator();
            Effect_SpawnRisingParticle(entity->posX.whole + x, entity->posY.whole, entity->spriteLayer - 1, 1);
        }
        if (--entity->timer <= 1) entity->deleteRequested = 1;
        if (entity->deleteRequested && entity->unk_500.greatArmorParts != NULL)
            Heap_Free(entity->unk_500.greatArmorParts);
        return;
    case 3:
        return;
    }
boxes:
    parts = (s32 *)entity->unk_500.greatArmorParts;
    if (entity->flipX)
        box.x = -TO_PIXELS(parts[4 * 10 + 0]) - 8;
    else
        box.x = TO_PIXELS(parts[4 * 10 + 0]) - 8;
    box.y = TO_PIXELS(parts[4 * 10 + 1]) - 8;
    box.w = 0x18;
    box.h = box.y < 0 ? -box.y : box.y;
    if (!gEwramData->gameState.bestiaryPreview)
    {
        HitBox_SetAttack(entity, &box);
        HitBox_SetBody(entity, &box);
    }
    if (entity->state != 2)
    {
        if (GUARD(entity)) entity->hit.flags |= 4;
        else entity->hit.flags &= ~4;
        Hit_CheckWithPlayer(collision, entity);
    }
    Entity_ClampXToRoom(entity, 1);
    if ((RandomNumberGenerator() & 0x7F) <= 3)
        entity->unk_4FC.part8.unk_4FE = 0;
}
