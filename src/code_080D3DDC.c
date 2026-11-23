/* GrahamAttacker_UpdateDeath: matched (plain --x == 0xFF / ++x == 0x20: the QImode 255 mask is shared
   by CSE and spilled to the stack, as in the original). */
#include "code_08009A0.h"
#include "graham_entity.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s16 sGrahamAttackerPositions[32][2];
extern void *memcpy(void *, const void *, unsigned long);
extern void GrahamDeathSpark_Update(struct EwramData_EntityData *);
extern void Effect_SpawnRisingParticle(s32, s32, s32, s32);

static inline void SpawnSpark(struct EwramData_EntityData *entity, s32 dx, s32 dy, u8 pri)
{
    struct EwramData_EntityData *effect;
    s32 x, y, priority;
    x = (dx << 16) + entity->posX.whole;
    y = (dy << 16) + entity->posY.whole;
    y -= 0x460000;
    priority = pri - 1;

    effect = EntityCreateInRange(0x49, 0xCF, (u32 *)GrahamDeathSpark_Update);
    if (effect != NULL)
    {
        effect->posX.whole = x;
        effect->posY.whole = y;
        effect->flipX = RandomNumberGenerator() % 2;
        effect->spriteLayer = priority;
    }
}
static inline void SpawnSpark2(struct EwramData_EntityData *entity, s32 dx, s32 dy, u8 pri)
{
    struct EwramData_EntityData *effect;
    s32 x, y, priority;
    x = (dx << 16) + entity->posX.whole;
    y = (dy << 16) + entity->posY.whole;
    y -= 0x460000;
    priority = pri - 1;

    effect = EntityCreateInRange(0x49, 0xCF, (u32 *)GrahamDeathSpark_Update);
    if (effect != NULL)
    {
        effect->posX.whole = x;
        effect->posY.whole = y;
        effect->flipX = RandomNumberGenerator() % 2;
        effect->spriteLayer = priority;
    }
}

/**
 * @brief Remove the linked attack effect, wait for owner flags and emit
 * 32 table-positioned sparks/particles followed by a final burst.
 * Request deletion; retain pointer-test order and byte-counter forms.
 */
void GrahamAttacker_UpdateDeath(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *part = &gEwramData->entityData[entity->ext.grahamScene.childIndex];
    struct EwramData_EntityData *owner = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    s16 positions[32][2];

    memcpy(positions, sGrahamAttackerPositions, sizeof(positions));
    switch (entity->subState)
    {
    case 0:
        Anim_StartIndexed(entity, 3, 0, 1);
        if (part->enemyId == entity->enemyId && part != NULL)
            part->deleteRequested = 1;
        entity->objMode = 1;
        entity->timer = 2;
        entity->subState++;
        entity->stage = 0;
    case 1:
        if (!(owner->enemyFlags & 2) || owner == NULL || (owner->ext.graham.flags & 0x20))
        {
            entity->subState++;
            entity->stage = 0;
        }
        break;
    case 2:
        {
        if (--entity->timer == 0xFF)
        {
            PlaySong(0x6E);
            SpawnSpark2(entity, positions[entity->stage][0], positions[entity->stage][1], entity->spriteLayer);
            Effect_SpawnRisingParticle(entity->posX.whole + (positions[entity->stage][0] << 16),
                entity->posY.whole + ((positions[entity->stage][1] - 0x46) << 16), entity->spriteLayer - 1, 0);
            if (++entity->stage == 0x20)
            {
                entity->subState++;
                entity->stage = 0;
            }
            else
                entity->timer = 2;
        }
        }
        break;
    case 3:
        if (entity->stage == 0)
        {
            entity->timer = 0x20;
            entity->stage++;
        }
        if (--entity->timer == 0xFF)
        {
            entity->subState++;
            entity->stage = 0;
        }
        break;
    case 4:
        {
            s32 i;
            for (i = 0; i < 32; i += 4)
            {
                PlaySong(0x77);
                SpawnSpark(entity, positions[i][0], positions[i][1], entity->spriteLayer);
            }
            entity->deleteRequested = 1;
        }
        break;
    }
}
