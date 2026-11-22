#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern void EntitySetVelocityFromAngle(struct EwramData_EntityData *, s32, s32);
extern void EntityMove(struct EwramData_EntityData *entity);
extern s32 Tile_Classify(s16 x, s16 y);
extern s32 Entity_GetVisibilityPattern(s32 step);
extern s32 Hit_CheckWithPlayer(void *res, struct EwramData_EntityData *b);

#define FIXED_TO_INT(v) ((v) < 0 ? -(-(v) >> 16) : (v) >> 16)

/**
 * @brief A1740 | Homing projectile: turn the angle unk_502 by 0x100 per frame
 * towards 0x4000 (straight down), fly at 0x30000; it leaves a debris state
 * (10) when the probe 8 px ahead hits a wall, and expires after 0xF0 frames.
 * Drawn rotated by the angle (affine slot affineIndex).
 */
void GladiatorShot_Update(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    s32 x, y;
    u32 d;

    switch (entity->state)
    {
    case 0:
        if ((u16)entity->unk_500.part16.unk_502 > 0x4000)
            entity->unk_500.part16.unk_502 -= 0x100;
        else if ((u16)entity->unk_500.part16.unk_502 < 0x4000)
            entity->unk_500.part16.unk_502 += 0x100;
        EntitySetVelocityFromAngle(entity, (u16)entity->unk_500.part16.unk_502, 0x30000);
        EntityMove(entity);
        x = FixedMultiply(Sin((u16)entity->unk_500.part16.unk_502 + 0x4000), 0x80000);
        y = FixedMultiply(Sin((u16)entity->unk_500.part16.unk_502), 0x80000);
        x = FIXED_TO_INT(x + entity->posX.whole + bg->xPos.whole);
        y = FIXED_TO_INT(y + entity->posY.whole + bg->yPos.whole);
        if (Tile_Classify(x, y) != 1)
        {
            entity->state = 10;
            entity->timer = 0x78;
        }
        if (entity->timer++ > 0xF0)
        {
            ObjAffine_Free(entity->affineIndex);
            EntityDelete(entity);
            return;
        }
        Anim_Update(entity);
        Hit_CheckWithPlayer(collision, entity);
        break;
    case 1:
        ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
        return;
    case 10:
        entity->timer--;
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
        if (entity->timer == 0)
        {
            ObjAffine_Free(entity->affineIndex);
            EntityDelete(entity);
            return;
        }
        Anim_Update(entity);
        Hit_CheckWithPlayer(collision, entity);
        break;
    }
    d = -0x8000;
    ObjAffine_Set((u16)entity->unk_500.part16.unk_502 + d, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
}
