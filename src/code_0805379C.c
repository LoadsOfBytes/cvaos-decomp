#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "structs/ewram.h"
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern void Sprite_GetPartCenterOffset(struct EwramData_EntityData *, s32, s16 *);
extern void Sprite_GetPartOriginOffset(struct EwramData_EntityData *, s32, s16 *);
extern void Object18Debris_Update(struct EwramData_EntityData *);
extern u8 sObject18Gfx[], sObject18Palette[], sObject18SpriteSheet[];

/**
 * @brief Spawn the four debris pieces (frames 1..4, Object18Debris_Update) of
 * the parent at its part offsets, flung with a share of its speed.
 */
void Object18_SpawnDebris(struct EwramData_EntityData *parent)
{
    struct EwramData_EntityData *entity;
    s32 frame;
    s32 i;
    s16 offset[2];
    s16 *p;

    frame = 1;
    for (i = 0; i <= 3; i++, frame++)
    {
        entity = EntityCreateInRange(0x49, 0xCF, (u32 *)Object18Debris_Update);
        if (entity == NULL)
            return;
        if (!Entity_LoadGraphics(entity, sObject18Gfx, sObject18Palette, 0, sObject18SpriteSheet))
        {
            EntityDelete(entity);
            return;
        }
        entity->animFrame = frame;
        entity->spriteLayer = 0x14;
        entity->flipX = parent->flipX;
        p = offset;
        Sprite_GetPartCenterOffset(parent, i, p);
        entity->posX.part16.integer = parent->posX.part16.integer + offset[0];
        entity->posY.part16.integer = parent->posY.part16.integer + p[1];
        Sprite_GetPartOriginOffset(entity, 0, offset);
        entity->posX.part16.integer += offset[0] / 4;
        entity->posY.part16.integer -= p[1] / 4;
        entity->velX.whole = parent->velX.whole * ((RandomNumberGenerator() & 0xFFFF) + 0x10000);
        entity->velY.whole = -0x20000 - RandomSigned(0x20000);
        entity->accelY.whole = 0x2000;
        entity->scrollWithBg = 1;
    }
}
