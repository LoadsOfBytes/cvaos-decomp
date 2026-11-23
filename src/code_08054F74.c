#include "code_08009A0.h"
#include "save_point_entity.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "structs/entity_render_flags.h"
extern void SavePoint_UpdateOrbitLight(struct EwramData_EntityData *);
extern u8 sSaveWarpRoomSpriteSheet[];

struct PartInit
{
    s32 anim;
    s32 palette;
    void *update;
    s32 layer;
};

static inline void AssignPalette(struct EwramData_EntityData *e, struct EwramData_EntityData *owner) { e->affineIndex = owner->ext.raw.unk_4F8.part8.unk_4FB; }
/**
 * @brief After a successful save, spawn eight lights (update
 * SavePoint_UpdateOrbitLight) around the save point. Return 1 when any
 * was created.
 */
s32 SavePoint_SpawnOrbitLights(struct EwramData_EntityData *parent)
{
    struct EwramData_EntityData *entity;
    s32 created = 0;
    struct PartInit parts[1] = {
        { 2, parent->ext.savePoint.frameBase + 3, SavePoint_UpdateOrbitLight, 0x11 },
    };
    s32 i;
    s32 j;

    j = 0;
    for (i = 0; i <= 7; i++)
    {
        entity = EntityCreateInRange(0x1B, 0x48, (u32 *)EntityUpdateNothing);
        if (entity == NULL)
            continue;
        entity->posX.whole = parent->posX.whole;
        entity->posY.whole = parent->posY.whole;
        entity->scrollWithBg = 1;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        Sprite_Attach(entity, sSaveWarpRoomSpriteSheet, parent->ext.savePoint.graphics, parts[j].palette);
        Anim_StartIndexed(entity, parts[j].anim, 0, 1);
        entity->spriteLayer = parts[j].layer;
        entity->updateFunc = (u32 *)parts[j].update;
        entity->ext.savePointPart.owner = parent;
        entity->ext.savePointLight.index = i;
        entity->ext.savePointLight.radius = 0x80000;
        entity->ext.savePointLight.phase = 0;
        SAVE_POINT_LIGHT_SPIN(entity) = 0;
        entity->state = 0;
        entity->timer = 2;
        AssignPalette(entity, parent);
        entity->affine = 1;
        created = 1;
    }
    return created;
}
