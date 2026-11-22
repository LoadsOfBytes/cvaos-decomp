/* Slime particle creation with bounded entity allocation. */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern void SlimeParticle_Update(struct EwramData_EntityData *);
extern u8 sSlimeGfx[], sSlimePalette[], sSlimeSpriteSheet[];

/**
 * @brief Spawn up to count slime particles with randomized position, frame
 * and facing and velocity biased by vx/vy. Stop on entity or graphics
 * allocation failure; remove a child whose graphics failed to load.
 */
void Slime_SpawnParticles(struct EwramData_EntityData *entity, s32 count, s32 vx, s32 vy)
{
    s32 index;
    struct EwramData_EntityData *child;
    index = 0;
    if (index >= count) return;
    do
    {
        child = EntityCreateInRange(0x49, 0xCF, (u32 *)SlimeParticle_Update);
        if (child != NULL)
        {
            if (!Entity_LoadGraphics(child, sSlimeGfx, sSlimePalette, (u8)entity->unk_514.part16.spawnParam1, sSlimeSpriteSheet))
            {
                EntityDelete(child);
                return;
            }
            child->animFrame = (RandomNumberGenerator() & 3) + 7;
            child->spriteLayer = 0x11;
            child->flipX = RandomNumberGenerator() & 1;
            child->flipY = RandomNumberGenerator() & 1;
            child->scrollWithBg = 1;
            {
                s32 random = RandomSigned(0x10);
                u16 *position = &entity->posX.part16.integer;
                s32 x = *position;
                x += random;
                child->posX.part16.integer = x;
            }
            {
                s32 random = RandomSigned(0x10);
                u16 *position = (u16 *)&entity->posY.part16.integer;
                s32 y = *position;
                y += random;
                child->posY.part16.integer = y;
            }
            child->velX.whole = vx + RandomSigned(0x20000);
            child->velY.whole = RandomSigned(0x10000) + vy;
            child->accelY.whole = 0x2000;
        }
        else return;
        index++;
    } while (index < count);
}
