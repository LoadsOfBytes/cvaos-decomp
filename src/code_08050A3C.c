#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_080211F0.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

struct PieceInfo { u8 graphics, palette, unk2, firstFrame, count; u8 pad[7]; };
struct PieceGraphics { void *graphics, *sprite, *palette; };
extern const struct PieceInfo sObject0EVariants[];
extern const struct PieceGraphics sObject0EGraphics[];
extern void Object0EPiece_Update(struct EwramData_EntityData *);
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, u8, void *);
extern void Sprite_GetPartCenterOffset(struct EwramData_EntityData *, s32, void *);
extern void Sprite_GetPartOriginOffset(struct EwramData_EntityData *, s32, void *);

/**
 * @brief Spawn the variant's count of falling pieces (Object0EPiece_Update) with
 * consecutive frames, loading their graphics; stop when no entity slot or
 * graphics are available.
 */
void Object0E_SpawnPieces(struct EwramData_EntityData *owner)
{
    const struct PieceInfo *info = &sObject0EVariants[owner->unk_514.part16.spawnParam0];
    const struct PieceGraphics *gfx = &sObject0EGraphics[info->graphics];
    s32 frame = info->firstFrame;
    s32 i;
    struct { s16 x, y; } pos;
    for (i = 0; i < info->count; i++, frame++)
    {
        struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, (u32 *)Object0EPiece_Update);
        if (!entity)
            goto done;
        if (!Entity_LoadGraphics(entity, gfx->graphics, gfx->sprite, info->palette, gfx->palette))
        {
            EntityDelete(entity);
            goto done;
        }
        entity->animFrame = frame;
        entity->spriteLayer = 0x14;
        entity->flipX = owner->flipX;
        Sprite_GetPartCenterOffset(owner, i, &pos);
        entity->posX.part16.integer = owner->posX.part16.integer + pos.x;
        entity->posY.part16.integer = owner->posY.part16.integer + pos.y;
        Sprite_GetPartOriginOffset(entity, 0, &pos);
        entity->posX.part16.integer += pos.x / 4;
        entity->posY.part16.integer -= pos.y / 4;
        entity->velX.whole = owner->velX.whole * ((RandomNumberGenerator() & 0xFFFF) + 0x10000);
        entity->velY.whole = -0x20000 - RandomSigned(0x20000);
        entity->accelY.whole = 0x2000;
        entity->scrollWithBg = 1;
    }
done:
    return;
}
