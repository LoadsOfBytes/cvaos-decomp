/* Tsuchinoko: create an arcing projectile sharing the parent's graphics. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void TsuchinokoShot_Update(struct EwramData_EntityData *);
extern s32 TsuchinokoShot_LoadGraphics(struct EwramData_EntityData *, struct EwramData_EntityData *);
extern void TsuchinokoShot_EnableHitBoxes(struct EwramData_EntityData *);
/**
 * @brief Spit a falling shot from parent (TsuchinokoShot_Update, attribute 0x40
 * on contact).
 */
void Tsuchinoko_SpitShot(struct EwramData_EntityData *parent)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x1B, 0x48, (u32 *)TsuchinokoShot_Update);
    if (child != NULL)
    {
        if ((u8)TsuchinokoShot_LoadGraphics(parent, child) == 0)
            EntityDelete(child);
        child->scrollWithBg = 1;
        Anim_StartIndexed(child, 4, 0, 1);
        Anim_Update(child);
        TsuchinokoShot_EnableHitBoxes(child);
        child->spriteLayer = parent->spriteLayer + 1;
        ENTITY_CREATOR(child) = parent;
        child->flipX = parent->flipX;
        if (parent->flipX)
            child->posX.part16.integer = parent->posX.part16.integer;
        else
            child->posX.part16.integer = parent->posX.part16.integer;
        child->posY.part16.integer = parent->posY.part16.integer - 26;
        if (parent->flipX)
            child->velX.whole = 0x30000;
        else
            child->velX.whole = -0x30000;
        child->velY.whole = -0x10000;
        child->accelX.whole = 0;
        child->accelY.whole = 0x1800;
        child->spawnIndex = parent->spawnIndex;
        child->basePalette = parent->basePalette;
        child->attack = parent->attack;
    }
}
