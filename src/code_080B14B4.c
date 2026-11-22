/* Disc Armor disc ring. */
#include "code_08009A0.h"
#include "disc_armor_entity.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, u8 *, void *, u8, void *);
extern void DiscArmorRing_Draw(struct EwramData_EntityData *);
extern u8 sDiscArmorGfx[], sDiscArmorPalette[], sDiscArmorSpriteSheet[];

/**
 * @brief B14B4 | Disc Armor disc ring effect.  State 0 loads the disc graphics,
 * allocates an affine matrix (affineIndex, flag affine), sets the draw
 * callback DiscArmorRing_Draw, animation 7, spriteLayer = 0x14 and scrollWithBg; failure
 * deletes the entity.  State 2 frees the matrix and deletes the entity.
 * Otherwise the ring scale follows the distance unk_504 (capped at 0x600000,
 * divided by 3 and 32): unk_4FC/unk_500 become cos/sin of the angle unk_4F8
 * times the scale and the matrix is rotated/scaled accordingly.
 */
void DiscArmorRing_Update(struct EwramData_EntityData *entity)
{
    s32 dist, scale, y;

    switch (entity->state)
    {
    case 0:
        if (Entity_LoadGraphics(entity, sDiscArmorGfx, sDiscArmorPalette, 0, sDiscArmorSpriteSheet))
        {
            u16 matrix = ObjAffine_Alloc(1);

            if ((s16)matrix >= 0)
            {
                entity->drawFunc = (u32 *)DiscArmorRing_Draw;
                entity->affineIndex = matrix;
                entity->affine = 1;
                entity->spriteLayer = 0x14;
                Anim_StartIndexed(entity, 7, 0, 0);
                Anim_Update(entity);
                entity->scrollWithBg = 1;
                entity->state++;
                break;
            }
        }
        goto delete;
    case 2:
        if (entity->affine)
            ObjAffine_Free(entity->affineIndex);
    delete:
        EntityDelete(entity);
        return;
    }
    dist = DISC_ARMOR_RING_LENGTH(entity);
    if (dist > 0x600000)
        dist = 0x600000;
    dist /= 3;
    scale = dist / 32;
    DISC_ARMOR_RING_STEP_X(entity) = Sin(DISC_ARMOR_RING_ANGLE(entity) + 0x4000) << 5;
    y = Sin(DISC_ARMOR_RING_ANGLE(entity)) << 5;
    DISC_ARMOR_RING_STEP_Y(entity) = y;
    DISC_ARMOR_RING_STEP_X(entity) = (s32)DISC_ARMOR_RING_STEP_X(entity) / 0x10000 * scale;
    DISC_ARMOR_RING_STEP_Y(entity) = y / 0x10000 * scale;
    ObjAffine_Set(DISC_ARMOR_RING_ANGLE(entity), scale, 0x10000, 0, 0, entity->affineIndex);
}
