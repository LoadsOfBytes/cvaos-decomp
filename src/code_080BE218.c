#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "syscalls.h"

extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern s32 Entity_GetRoomPointDistance(struct EwramData_EntityData *entity, s32 param_1, s32 param_2);

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
        entity->unk_4FC.part16.unk_4FE[0] = ArcTan2(entity->posX.part16.integer - 0x100, entity->posY.part16.integer - 0x80);
    {
        struct BgLayerState *camera = &gEwramData->bgInfo[1];
        entity->posX.whole -= camera->xPos.whole;
        entity->posY.whole -= camera->yPos.whole;
    }
        entity->unk_504.whole = Entity_GetRoomPointDistance(entity, 0x1000000, 0x800000);
        entity->unk_500.whole = 0x10000;
        entity->unk_508.whole = -0x155;
        entity->unk_510.whole = -(entity->unk_504.whole / 0xC0);
        entity->state = 1;
    case 1:
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
        entity->state = 2;
    case 2:
        if (entity->unk_50C.part16.unk_50C <= 0x5FF)
            entity->unk_50C.part16.unk_50C += 0x10;
        entity->unk_4FC.part16.unk_4FC += entity->unk_50C.part16.unk_50C;
        entity->unk_4FC.part16.unk_4FE[0] += entity->unk_50C.part16.unk_50C;
        entity->unk_500.whole += entity->unk_508.whole;
        if (entity->unk_500.whole < 0)
            entity->unk_500.whole = 0;
        if (entity->unk_504.whole > 0)
        {
            entity->unk_504.whole += entity->unk_510.whole;
            break;
        }
    case 3:
        ObjAffine_Free(entity->affineIndex);
    delete:
        EntityDelete(entity);
        return;
    }
    {
        s32 s = Sin(entity->unk_4FC.part16.unk_4FE[0] + 0x4000);
        s32 r = entity->unk_504.whole;
        entity->velX.whole = (s >> 8) * (r >> 8);
    }
    {
        s32 s = Sin(entity->unk_4FC.part16.unk_4FE[0]);
        s32 r = entity->unk_504.whole;
        entity->velY.whole = (s >> 8) * (r >> 8);
    }
    {
        struct BgLayerState *camera;
        u16 *velXInt = &entity->velX.part16.integer;
        camera = &gEwramData->bgInfo[1];
        entity->posX.part16.integer = *velXInt - (u16)(camera->xPos.part16.integer - 0x100);
        entity->posY.part16.integer = entity->velY.part16.integer - (s16)(camera->yPos.part16.integer - 0x80);
    }
    ObjAffine_Set(entity->unk_4FC.part16.unk_4FC, entity->unk_500.whole, entity->unk_500.whole,
                 entity->flipX, 0, entity->affineIndex);
}
