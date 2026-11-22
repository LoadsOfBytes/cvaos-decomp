/* Spawn a fragment at owner + (x, y): graphics 081C15F4 / 0820ED60
 * (palette 7, animation 3, priority 20), hurt box 0x100CF8FA (bytes x -6,
 * y -8, w 12, h 16; not an address), hit callback MimicTrigger_OnHit; it keeps the
 * owner (unk_500) and its type byte. */
#include "code_08009A0.h"
#include "mimic_entity.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"
extern void MimicTrigger_Update(struct EwramData_EntityData *);
extern void MimicTrigger_OnHit(struct EwramData_EntityData *, s32, u16 *);
extern void PickupSource_Draw(struct EwramData_EntityData *);
extern u8 sCommonObjectGfx[], sCommonObjectSpriteSheet[];

/**
 * @brief Spawn the decoy at owner + (x, y): an effect sprite (animation 3) drawn
 * like a pickup source (PickupSource_Draw); hitting it wakes the
 * Mimic (MimicTrigger_OnHit).
 */
void Mimic_SpawnTrigger(struct EwramData_EntityData *owner, s32 x, s32 y)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)MimicTrigger_Update);
    u32 box = 0x100CF8FA;
    s32 graphics;

    if (child == NULL)
        return;
    child->posX.whole = owner->posX.whole + x;
    child->posY.whole = owner->posY.whole + y;
    child->spriteLayer = 0x14;
    graphics = ObjGfx_Find(sCommonObjectGfx);
    Sprite_Attach(child, sCommonObjectSpriteSheet, graphics, 7);
    Anim_StartIndexed(child, 3, 0, 1);
    Anim_Update(child);
    Hit_Enable(child, 8, NULL, (s32)MimicTrigger_OnHit);
    HitBox_SetBody(child, &box);
    child->hit.flags &= ~4;
    child->hit.flags |= 2;
    MIMIC_TRIGGER_OWNER(child) = owner;
    child->unk_518.part8.enemyId = owner->unk_518.part8.enemyId;
    child->scrollWithBg = 1;
    child->drawFunc = (u32 *)PickupSource_Draw;
}
