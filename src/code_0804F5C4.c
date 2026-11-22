/* Object07_SpawnParallaxSprites. */
#include "code_08009A0.h"
#include "structs/ewram.h"

struct Offset16 { u16 x, y; };
extern const u8 sUnk_085255DC[];
extern const u16 sUnk_085255C4[];
extern void Object07ParallaxSprite_Update(struct EwramData_EntityData *entity);

/**
 * @brief Object 0x07 type 0: create four background sprites from
 * sUnk_085255DC, placed by the camera position scaled with the room
 * size plus the variant offset.
 */
void Object07_SpawnParallaxSprites(struct EwramData_EntityData *entity)
{
    struct BgMapMetadata *meta = gEwramData->bgInfo[1].pBgMetadata;
    s32 i;

    for (i = 0; i <= 3; i++)
    {
        struct EwramData_EntityData *e = EntityCreateInRange(0x49, 0xCF, (u32 *)Object07ParallaxSprite_Update);
        if (e != NULL)
        {
            const u8 *entry;
            struct BgLayerState *cam;
            { const u8 *tbl = sUnk_085255DC; entry = tbl + i * 4; }
            e->unk_4FC.part8.unk_4FC = entry[2];
            {
                s32 x = entry[0] + 0x50;
                cam = &gEwramData->bgInfo[1];
                e->posX.part16.integer = x - ((s16)cam->xPos.part16.integer * meta->widthCells) / 64
                    + sUnk_085255C4[entity->unk_514.part16.spawnParam1 * 2];
            }
            e->posY.part16.integer = entry[1] - ((s16)cam->yPos.part16.integer * meta->heightCells) / 64
                + (sUnk_085255C4 + 1)[entity->unk_514.part16.spawnParam1 * 2];
            e->accelX.whole = 0x80;
        }
    }
}
