/* Object 08: persistent flag, per-kind graphics and a centred hit box. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"
struct DebrisGraphics { void *graphics, *palette, *sprite; };
struct Object08Kind { u16 graphicsIndex; u8 palette, frame; u8 pad[2]; u8 width, height, layer, behavior; u16 pad_A; };
extern struct DebrisGraphics sBreakableBlockGraphics[];
extern struct Object08Kind sBreakableBlockVariants[];
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern void BreakableBlock_SetCollision(struct EwramData_EntityData *);
void Object08Create(struct EwramData_EntityData *entity)
{
    struct Object08Kind *kind;
    struct DebrisGraphics *graphics;
    u32 box;
    if (entity->unk_514.part16.spawnParam1 != 0 && (gEwramData->gameState.gameFlagsTail[(entity->unk_514.part16.spawnParam1) >> 5] & (1 << ((entity->unk_514.part16.spawnParam1) & 31)))) goto fail;
    kind = &sBreakableBlockVariants[entity->unk_514.part16.spawnParam0];
    graphics = &sBreakableBlockGraphics[kind->graphicsIndex];
    entity->flipX = 0;
    entity->spriteLayer = kind->layer;
    if (!Entity_LoadGraphics(entity, graphics->graphics, graphics->palette, kind->palette, graphics->sprite))
    {
fail:
        EntityDelete(entity);
        return;
    }
    entity->animFrame = kind->frame;
    box = ((-kind->width / 2) & 255) + (((-kind->height / 2) & 255) << 8) + (kind->width << 16) + (kind->height << 24);
    entity->unk_4F8.part8.unk_4F8 = kind->behavior;
    BreakableBlock_SetCollision(entity);
    Hit_Enable(entity, 8, NULL, 0);
    HitBox_SetBodyRaw(entity, box);
    entity->hit.flags |= 2;
    entity->unk_518.part16.hp = 3;
}
