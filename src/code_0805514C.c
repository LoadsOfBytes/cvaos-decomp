/* Orbiting child: spiral outward around a fixed point (state 0), then orbit and shrink around the owner (state 1). */
#include "code_08009A0.h"
#include "save_point_entity.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s32 Target_GetPixelX(struct EwramData_EntityData *);
extern s32 Target_GetPixelY(struct EwramData_EntityData *);
extern void SaveSparkle_SpawnAround(struct EwramData_EntityData *, s32, s32);

/**
 * @brief Spiral outwards, then orbit inwards onto the player; tell the owner
 * (SAVE_POINT_LIGHTS_OUT_DONE, SAVE_POINT_LIGHTS_IN_DONE) when each phase
 * ends. Emits sparkles.
 */
void SavePoint_UpdateOrbitLight(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner;
    s32 state;
    s32 s;
    s32 c;
    s32 r;
    u32 step;
    s32 rad;

    owner = entity->ext.savePointPart.owner;
    state = entity->state;
    switch (state)
    {
    case 0:
        s = Sin(entity->ext.savePointLight.index << 13);
        c = Sin((entity->ext.savePointLight.index << 13) + 0x4000);
        rad = entity->ext.savePointLight.radius;
        r = rad >> 16;
        s *= r;
        c *= r;
        entity->posY.whole = s + 0x500000;
        entity->posX.whole = c + 0x760000;
        if ((u32)entity->ext.savePointLight.phase <= 0x1DFFFF)
            step = (u32)entity->ext.savePointLight.phase >> 16;
        else
            step = ((u32)entity->ext.savePointLight.phase >> 16) * 3;
        entity->ext.savePointLight.radius = rad + (step << 13);
        entity->ext.savePointLight.phase += 0x10000;
        SAVE_POINT_LIGHT_SPIN(entity) += 0x800;
        if (*(vs32 *)&entity->ext.savePointLight.radius > 0xFFFFFF)
        {
            entity->state++;
            entity->timer = 2;
            entity->spriteLayer = 9;
            entity->ext.savePointLight.radius = 0x1000000;
            SAVE_POINT_LIGHTS_OUT_DONE(owner) = 1;
            entity->ext.savePointLight.phase = 0;
        }
        break;
    case 1:
        s = Sin((entity->ext.savePointLight.index << 13) + entity->ext.savePointLight.phase);
        c = Sin((entity->ext.savePointLight.index << 13) + ({ s32 t_ = entity->ext.savePointLight.phase; t_ + 0x4000; }));
        r = SAVE_POINT_LIGHT_RADIUS_PX(entity);
        s *= r;
        c *= r;
        entity->posX.whole = c + 0x780000;
        entity->posY.whole = s + 0x500000;
        entity->posX.whole += (Target_GetPixelX(entity) << 16) - owner->posX.whole;
        entity->posY.whole = entity->posY.whole + ((Target_GetPixelY(entity) << 16) - owner->posY.whole) + 0x200000;
        entity->ext.savePointLight.radius -= 0x30000;
        entity->ext.savePointLight.phase += 0x200;
        if (entity->ext.savePointLight.radius <= 0)
        {
            SAVE_POINT_LIGHTS_IN_DONE(entity->ext.savePointPart.owner) = state;
            entity->deleteRequested = 1;
        }
        break;
    }
    if ((++entity->timer & 3) == 0)
        SaveSparkle_SpawnAround(entity, 0x10, 0x10);
    Anim_Update(entity);
}
