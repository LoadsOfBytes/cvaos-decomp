#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

extern void Effect_SpawnParticleAnim0(s32 x, s32 y, s32 vx, s32 vy, s32 layer);
extern s32 Entity_GetVisibilityPattern(s32 step);
extern void Biphron_SpawnDebris(struct EwramData_EntityData *entity, s32 x, s32 y, s32 layer, s32 mode);

#define FRAME(e) (*(u16 *)&(e)->animStep)

/**
 * @brief 99DB8 | Enemy attack animation: on frame 1 spawn two dust puffs
 * (Effect_SpawnParticleAnim0), on frame 6 start an 0x50-frame phase that every 16th frame
 * (while timer > 0x2F) throws a debris piece (Biphron_SpawnDebris, sound 0x6E)
 * and blinks before deleting itself.
 */
void Biphron_UpdateDeath(struct EwramData_EntityData *entity)
{
    s32 x, y;

    switch (entity->subState)
    {
    case 0:
        if (FRAME(entity) == 1)
        {
            Effect_SpawnParticleAnim0(entity->posX.whole, entity->posY.whole, 0, -0x6000, entity->spriteLayer - 1);
            x = entity->posX.whole + (RandomNumberGenerator() & 0x7FFFF) - 0x40000;
            y = entity->posY.whole;
            Effect_SpawnParticleAnim0(x, y, 0, -0x4000, entity->spriteLayer - 1);
            entity->subState++;
        }
        break;
    case 1:
        if (FRAME(entity) == 6)
        {
            entity->subState = 2;
            entity->timer = 0x50;
        }
        break;
    case 2:
        if (entity->timer > 0x2F && (entity->timer & 0xF) == 0)
        {
            x = entity->flipX ? 0x80000 : -0x80000;
            x = x + entity->posX.whole + (RandomNumberGenerator() & 0xFFFFF) - 0x80000;
            y = entity->posY.whole - 0x80000;
            Biphron_SpawnDebris(entity, x, y, entity->spriteLayer - 1, 4);
            PlaySong(0x6E);
        }
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
        if ((s8)--entity->timer <= 0)
            entity->deleteRequested = 1;
        break;
    }
}
