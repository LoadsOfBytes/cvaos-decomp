#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern void LegionCorpse_Update(struct EwramData_EntityData *entity);
extern u8 sLegionSpriteSheet[], sLegionAnimScript1[], sLegionAnimScript2[];

/**
 * @brief 9DF7C | Create a Legion shell part LegionCorpse_Update at room position
 * (x, y) (screen position minus bgInfo[1], x also kept in unk_4F8 /
 * unk_4FC): index in unk_504, palette slot 3 below the owner, graphics
 * sLegionSpriteSheet with the owner's tiles (first word of its state block) and
 * palette (owner unk_504), animation sLegionAnimScript1, random phase animTimer
 * and facing (velX 0x4000 away), velY 0x40000, boxes off; owner in
 * unk_4F4; index > 9 starts in state 1.5 with animation sLegionAnimScript2.
 * Returns the part or NULL.
 */
struct EwramData_EntityData *Legion_CreateCorpse(struct EwramData_EntityData *entity, s32 x, s32 y, s32 index)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)LegionCorpse_Update);
    u32 *data = (u32 *)entity->ext.legion.data.value;
    u32 tiles, r;

    if (child != NULL)
    {
    child->posX.whole = x - bg->xPos.whole;
    child->posY.whole = y - bg->yPos.whole;
    child->ext.legionCorpse.targetX = x;
    child->ext.legionCorpse.targetY = x;
    child->ext.legionCorpse.frame = index;
    child->spriteLayer = entity->spriteLayer - 3;
    tiles = *data;
    child->basePalette = entity->ext.legion.animPalette;
    Sprite_Attach(child, sLegionSpriteSheet, tiles, child->basePalette);
    Anim_Start(child, (struct AnimScript *)sLegionAnimScript1, 2, 1);
    Anim_Update(child);
    r = RandomNumberGenerator() & 7;
    child->animTimer = r;
    child->enemyId = entity->enemyId;
    child->ext.legionPart.owner = entity;
    child->scrollWithBg = 1;
    r = RandomNumberGenerator() & 1;
    child->flipX = r;
    if (child->flipX)
        child->velX.whole = -0x4000;
    else
        child->velX.whole = 0x4000;
    child->velY.whole = 0x40000;
    child->accelY.whole = 0x2800;
    child->hit.flags |= 4;
    child->hit.flags |= 2;
    child->drawFunc = (u32 *)Sprite_DrawEntity;
    if (index > 9)
    {
        child->state = 1;
        child->subState = 5;
        Anim_Start(child, (struct AnimScript *)sLegionAnimScript2, 2, 0);
    }
    }
    else
        return NULL;
    return child;
}
