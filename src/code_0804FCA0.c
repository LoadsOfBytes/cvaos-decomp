/* BreakableBlock_SetCollision (breakable block tiles). */
#include "code_08001194.h"
#include "room_object_entity.h"
#include "structs/ewram.h"

/* Per variant: collisionOffsetX/collisionOffsetY and collisionWidth x collisionHeight in pixels. */
struct BreakableBlockVariant {
    u8 pad_0[4];
    s8 collisionOffsetX;
    s8 collisionOffsetY;
    u8 collisionWidth;
    u8 collisionHeight;
    u8 pad_8[0xC - 0x8];
};

extern struct BreakableBlockVariant sBreakableBlockVariants[];

/**
 * @brief 4FCA0 | Call Collision_SetBlock for every 16 px tile of the area of
 * variant unk_514 around the entity: a column (mode 1, mode 2 shifted 16 px
 * left), a row (modes 3 / 4) or the whole rectangle (mode 0); the mode is
 * unk_4F8.
 */
void BreakableBlock_SetCollision(struct EwramData_EntityData *entity)
{
    struct BreakableBlockVariant *def = &sBreakableBlockVariants[entity->unk_514.part16.spawnParam0];
    s32 offsetX = def->collisionOffsetX;
    s32 offsetY = def->collisionOffsetY;
    s32 i;
    s32 j;

    switch ((u8)entity->ext.object08.kind)
    {
        case 2:
            offsetX -= 0x10;
        case 1:
            for (i = -def->collisionHeight / 2; i < def->collisionHeight / 2; i += 0x10)
            {
                s32 x = (s16)entity->posX.part16.integer;
                struct BgLayerState *bg = &gEwramData->bgInfo[1];

                Collision_SetBlock(x + (s16)bg->xPos.part16.integer + offsetX,
                             (s16)entity->posY.part16.integer + (s16)bg->yPos.part16.integer + offsetY - i);
            }
            break;
        case 3:
        case 4:
            for (i = -def->collisionWidth / 2; i < def->collisionWidth / 2; i += 0x10)
            {
                s32 x = (s16)entity->posX.part16.integer;
                struct BgLayerState *bg = &gEwramData->bgInfo[1];

                Collision_SetBlock(x + (s16)bg->xPos.part16.integer + offsetX - i,
                             (s16)entity->posY.part16.integer + (s16)bg->yPos.part16.integer + offsetY);
            }
            break;
        case 0:
            for (i = -def->collisionWidth / 2; i < def->collisionWidth / 2; i += 0x10)
            {
                for (j = -def->collisionHeight / 2; j < def->collisionHeight / 2; j += 0x10)
                {
                    s32 x = (s16)entity->posX.part16.integer;
                    struct BgLayerState *bg = &gEwramData->bgInfo[1];

                    Collision_SetBlock(x + (s16)bg->xPos.part16.integer + offsetX + i,
                                 (s16)entity->posY.part16.integer + (s16)bg->yPos.part16.integer + offsetY + j);
                }
            }
            break;
    }
}
