#include "code_08009A0.h"
#include "head_stack_entity.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "structs/ewram.h"

struct HeadStackDebrisVariant {
    void *graphics;
    void *palette;
    void *spriteSheet;
    u8 firstDebrisFrame;
    u8 lastDebrisFrame;
    u8 pad_E[2];
};

extern const struct HeadStackDebrisVariant sUnk_081193DC[];
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern void HeadStackDebris_Update(struct EwramData_EntityData *entity);

/**
 * @brief Spawn debris frames from the variant table, then hide for eight countdown ticks; return 1 when finished.
 */
s32 HeadStackSegment_UpdateDeath(struct EwramData_EntityData *entity)
{
    const struct HeadStackDebrisVariant *info = &sUnk_081193DC[HEAD_STACK_TYPE(entity)];
    struct EwramData_EntityData *piece;
    s32 i;

    switch (entity->subState)
    {
    case 0:
        PlaySong(0x6B);
        for (i = info->lastDebrisFrame; i >= info->firstDebrisFrame; i--)
        {
            piece = EntityCreateInRange(0x49, 0xCF, (u32 *)HeadStackDebris_Update);
            if (piece != NULL)
            {
                const struct HeadStackDebrisVariant *t = &sUnk_081193DC[HEAD_STACK_TYPE(entity)];
                Entity_LoadGraphics(piece, t->graphics, t->palette, 0, t->spriteSheet);
                piece->animFrame = i;
                piece->posX.part16.integer = entity->posX.part16.integer;
                piece->posY.part16.integer = entity->posY.part16.integer;
                piece->velX.whole = RandomSigned(0x10) << 13;
                piece->velY.whole = (RandomSigned(0x10) << 12) - 0x30000;
                piece->accelY.whole = 0x2000;
                piece->spriteLayer = entity->spriteLayer;
                piece->flipX = entity->flipX;
                piece->scrollWithBg = 1;
            }
        }
        entity->timer = 8;
        entity->subState = 1;
        break;
    case 1:
        entity->hidden = 1;
        if (entity->timer == 0)
            return 1;
        entity->timer--;
        break;
    }
    return 0;
}
