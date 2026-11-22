/* Spawn a falling fragment at (x, y) with velocity (vx, vy), gravity
 * 0x2000, lifetime 60 and the given frame; graphics slot and palette come
 * from the shared data block at owner->unk_4F8 (see Legion_LoadGraphics). */
#include "code_08009A0.h"
#include "particle_effect.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void Effect_UpdateTimedParticle(struct EwramData_EntityData *);
extern u8 sLegionSpriteSheet[];

void Legion_SpawnDebris(struct EwramData_EntityData *owner, s32 x, s32 y, s32 vx, s32 vy, s32 frame)
{
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, (u32 *)Effect_UpdateTimedParticle);
    s32 *data = (s32 *)owner->unk_4F8.unk_4F8_entity;
    s32 graphics;

    if (entity == NULL)
        return;
    entity->posX.whole = x;
    entity->posY.whole = y;
    entity->velX.whole = vx;
    entity->velY.whole = vy;
    entity->accelY.whole = 0x2000;
    entity->timer = 0x3C;
    PARTICLE_FLAGS(entity) = 4;
    entity->spriteLayer = owner->spriteLayer - 1;
    graphics = data[0];
    entity->unk_510.part8.basePalette = data[9];
    entity->palette = data[9];
    Sprite_Attach(entity, sLegionSpriteSheet, graphics, entity->unk_510.part8.basePalette);
    entity->animFrame = frame;
    entity->animMode = 2;
    entity->scrollWithBg = 1;
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
}
