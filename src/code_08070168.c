/* Spawn and attach a Killer Fish part in the reserved enemy slots. */
#include "code_08009A0.h"
#include "structs/ewram.h"
extern void KillerFishSwarm_Update(struct EwramData_EntityData *);
extern u8 KillerFishSwarm_LoadGraphics(struct EwramData_EntityData *, struct EwramData_EntityData *);
extern void KillerFishSwarm_EnableHitBoxes(struct EwramData_EntityData *);
static inline void SetRelativePosition(struct EwramData_EntityData *parent, struct EwramData_EntityData *part, s16 x, s16 y)
{
    if (parent->flipX)
        part->posX.part16.integer = parent->posX.part16.integer - x;
    else
        part->posX.part16.integer = parent->posX.part16.integer + x;
    part->posY.part16.integer = parent->posY.part16.integer + y;
}
/**
 * @brief Spawn one swarm fish around parent (EnemyKillerFishCreate spawns
 * unk_516 of them) on a random layer in front of or behind it, with a
 * quarter of its attack (attack >> 2).
 */
void KillerFish_SpawnSwarmFish(struct EwramData_EntityData *parent)
{
    struct EwramData_EntityData *part = EntityCreateInRange(0x1B, 0x48, (u32 *)KillerFishSwarm_Update);
    if (part)
    {
        if (!KillerFishSwarm_LoadGraphics(parent, part)) EntityDelete(part);
        part->scrollWithBg = 1;
        KillerFishSwarm_EnableHitBoxes(part);
        if (RandomNumberGenerator() % 2 == 0)
            part->spriteLayer = parent->spriteLayer + 1;
        else
            part->spriteLayer = parent->spriteLayer - 1;
        ENTITY_CREATOR(part) = parent;
        SetRelativePosition(parent, part, 0, 0);
        part->unk_518.part8.spawnIndex = parent->unk_518.part8.spawnIndex;
        part->unk_510.part8.basePalette = parent->unk_510.part8.basePalette;
        part->attack = parent->attack >> 2;
    }
}
