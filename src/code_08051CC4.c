/* Spawns two debris pieces (animFrame = 3, mirrored) and four random ones (animFrame 4-5)
 * around the parent. */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "structs/ewram.h"
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern void Object14Debris_Update(struct EwramData_EntityData *);
extern u8 sObject14Gfx[], sObject14Palette[], sObject14SpriteSheet[];

/**
 * @brief Spawn the debris pieces (Object14Debris_Update) of object 0x14 around
 * the parent.
 */
void Object14_SpawnDebris(struct EwramData_EntityData *parent)
{
    struct EwramData_EntityData *entity;
    s32 i;

    for (i = 0; i <= 1; i++)
    {
        entity = EntityCreateInRange(0x49, 0xCF, (u32 *)Object14Debris_Update);
        if (entity != NULL)
        {
            Entity_LoadGraphics(entity, sObject14Gfx, sObject14Palette, 0, sObject14SpriteSheet);
            entity->animFrame = 3;
            entity->posX.part16.integer = (s16)parent->posX.part16.integer + (i * 16 - 16);
            entity->posY.part16.integer = parent->posY.part16.integer;
            entity->velX.whole = (i << 17) + ((RandomSigned(8) << 12) + ({ s32 k_ = 0x10000; -k_; }));
            entity->velY.whole = (RandomSigned(0x10) << 12) - 0x30000;
            entity->accelY.whole = 0x2000;
            entity->spriteLayer = parent->spriteLayer;
            entity->flipX = i;
            entity->scrollWithBg = 1;
        }
    }
    for (i = 0; i < 4; i++)
    {
        entity = EntityCreateInRange(0x49, 0xCF, (u32 *)Object14Debris_Update);
        if (entity != NULL)
        {
            Entity_LoadGraphics(entity, sObject14Gfx, sObject14Palette, 0, sObject14SpriteSheet);
            { register u32 k_ asm("r1"); s32 rn_ = RandomNumberGenerator(); k_ = 1; entity->animFrame = (rn_ & k_) + 4; } /* FAKEMATCH: pinned register */
            entity->posX.part16.integer = parent->posX.part16.integer + RandomSigned(0x10);
            entity->posY.part16.integer = parent->posY.part16.integer + RandomSigned(0x10);
            entity->velX.whole = RandomSigned(0x10) << 12;
            entity->velY.whole = (RandomSigned(0x20) << 12) - 0x30000;
            entity->accelY.whole = 0x2000;
            entity->spriteLayer = parent->spriteLayer;
            entity->flipX = RandomNumberGenerator() & 1;
            entity->flipY = RandomNumberGenerator() & 1;
            entity->scrollWithBg = 1;
        }
    }
}
