#include "code/code_08039340.h"
#include "code_08009A0.h"
#include "code_08001194.h"
#include "structs/ewram.h"

extern u8 Object37_LoadGraphics(struct EwramData_EntityData *);
extern void Object37_SpawnChild(struct EwramData_EntityData *);

/* Keep the entity and room coordinate reads in their original order. */
static inline s32 EntityRoomX(struct EwramData_EntityData *entity, struct EwramData **ram)
{
    s32 x = (s16)entity->posX.part16.integer;
    struct BgLayerState *bg = &(*ram)->bgInfo[1];
    return x + (s16)bg->xPos.part16.integer;
}

static inline s32 EntityRoomY(struct EwramData_EntityData *entity, struct EwramData **ram)
{
    s32 y = (s16)entity->posY.part16.integer;
    struct BgLayerState *bg = &(*ram)->bgInfo[1];
    return y + (s16)bg->yPos.part16.integer;
}

static inline s32 Abs(s32 value)
{
    if (value < 0)
        value = -value;
    return value;
}

/* Initialize object 0x37, including its five collision probes. */
void Object37Create(struct EwramData_EntityData *entity)
{
    struct EwramData **ewram = &gEwramData;
    u16 graphics;
    s32 scale;
    s32 x, y;
    if (!Object37_LoadGraphics(entity))
        goto fail;
    entity->animFrame = 1;
    entity->flipX = 0;
    entity->spriteLayer = 9;
    graphics = ObjAffine_Alloc(1);
    if ((s16)graphics < 0)
    {
    fail:
        EntityDelete(entity);
        return;
    }
    entity->affineIndex = graphics;
    entity->affine = 1;
    scale = 0x10000;
    ObjAffine_Set(0, scale, scale, entity->flipX, entity->flipY, entity->affineIndex);
    entity->hit.flags |= 4;
    entity->hit.flags |= 2;
    entity->state = 0;
    x = entity->posX.whole - (*ewram)->playerState.player->posX.whole;
    if (Abs(x) < 0x300000)
    {
        entity->unk_4FC.whole = 0x3000;
        entity->subState = 1;
        ObjAffine_Set(0x3000, scale, scale, entity->flipX, entity->flipY, entity->affineIndex);
        entity->unk_500.whole = 0;
    }
    else
    {
        entity->unk_500.whole = 1;
        x = EntityRoomX(entity, ewram);
        y = EntityRoomY(entity, ewram);
        x -= 0x10;
        Collision_SetBlock(x, y + 8);
        Collision_SetBlock(x, y - 8);
        Collision_SetBlock(x, y - 0x18);
        Collision_SetBlock(x, y - 0x28);
        Collision_SetBlock(x, y - 0x38);
    }
    Object37_SpawnChild(entity);
}
