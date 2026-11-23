#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "syscalls.h"

/*
 * Death debris (spawned when Death is defeated): it spirals into the room
 * point (0x100, 0x80) while shrinking: sprite rotation and orbit angle (both
 * advanced by the growing spin speed), orbit radius and its change per frame,
 * affine scale and its change per frame. Token-identical expansion.
 */
#define DEBRIS_RADIUS_STEP(e) e->unk_510.whole

extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern s32 Entity_GetRoomPointDistance(struct EwramData_EntityData *entity, s32 x, s32 y);

/* Spinning projectile: flies outwards from the screen point (0x100, 0x80)
 * along a spiral, shrinking until its radius reaches zero. */
/**
 * @brief Animate a delayed affine debris part spiraling inward toward room
 * point (256,128) as scale and radius decrease. Release its slot and
 * remove when the radius reaches zero. This entity has no hit callback.
 */
void DeathDebris_Update(struct EwramData_EntityData *entity)
{
    u16 slot;

    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, (void *)0x08119498, (void *)0x0820BC1C, 0, (void *)0x08249C48))
            goto delete;
        slot = ObjAffine_Alloc(1);
        if ((s16)slot < 0)
            goto delete;
        entity->affineIndex = slot;
        entity->affine = 1;
        entity->spriteLayer = 0x13;
        Anim_StartIndexed(entity, entity->subState + 0x14, 2, 0);
        Anim_Update(entity);
        entity->ext.deathDebris.orbitAngle = ArcTan2(entity->posX.part16.integer - 0x100, entity->posY.part16.integer - 0x80);
    {
        struct BgLayerState *camera = &gEwramData->bgInfo[1];
        entity->posX.whole -= camera->xPos.whole;
        entity->posY.whole -= camera->yPos.whole;
    }
        entity->ext.deathDebris.radius = Entity_GetRoomPointDistance(entity, 0x1000000, 0x800000);
        entity->ext.deathDebris.scale = 0x10000;
        entity->ext.deathDebris.scaleStep = -0x155;
        DEBRIS_RADIUS_STEP(entity) = -(entity->ext.deathDebris.radius / 0xC0);
        entity->state = 1;
    case 1:
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
        entity->state = 2;
    case 2:
        if (entity->ext.deathDebris.spin <= 0x5FF)
            entity->ext.deathDebris.spin += 0x10;
        entity->ext.deathDebris.rotation += entity->ext.deathDebris.spin;
        entity->ext.deathDebris.orbitAngle += entity->ext.deathDebris.spin;
        entity->ext.deathDebris.scale += entity->ext.deathDebris.scaleStep;
        if (entity->ext.deathDebris.scale < 0)
            entity->ext.deathDebris.scale = 0;
        if (entity->ext.deathDebris.radius > 0)
        {
            entity->ext.deathDebris.radius += DEBRIS_RADIUS_STEP(entity);
            break;
        }
    case 3:
        ObjAffine_Free(entity->affineIndex);
    delete:
        EntityDelete(entity);
        return;
    }
    {
        s32 s = Sin(entity->ext.deathDebris.orbitAngle + 0x4000);
        s32 r = entity->ext.deathDebris.radius;
        entity->velX.whole = (s >> 8) * (r >> 8);
    }
    {
        s32 s = Sin(entity->ext.deathDebris.orbitAngle);
        s32 r = entity->ext.deathDebris.radius;
        entity->velY.whole = (s >> 8) * (r >> 8);
    }
    {
        struct BgLayerState *camera;
        u16 *velXInt = &entity->velX.part16.integer;
        camera = &gEwramData->bgInfo[1];
        entity->posX.part16.integer = *velXInt - (u16)(camera->xPos.part16.integer - 0x100);
        entity->posY.part16.integer = entity->velY.part16.integer - (s16)(camera->yPos.part16.integer - 0x80);
    }
    ObjAffine_Set(entity->ext.deathDebris.rotation, entity->ext.deathDebris.scale, entity->ext.deathDebris.scale,
                 entity->flipX, 0, entity->affineIndex);
}
