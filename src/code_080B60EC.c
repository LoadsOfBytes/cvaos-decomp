#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

void PlaySong(u32 song);
extern const u8 sUnk_085287D6[4];

/**
 * @brief B60EC | Boss part next to BaloreIntroPiece_Update: appear with the parent
 * graphics, flash its palette slot (sUnk_085287D6) while counting down, then
 * jump up, fly sideways and delete itself after the animation ends; deleted
 * at once when the parent enters state 10.
 */
void BaloreIntroPart_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->unk_4FC.unk_4FC_entity;

    if (parent->state == 10)
        entity->state = 5;
    switch (entity->state)
    {
    case 0:
        Sprite_Attach(entity, (void *)0x08244D4C, parent->unk_4F8.bytes[3], parent->unk_510.part8.basePalette);
        Anim_StartIndexed(entity, 3, 2, 0);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 0x1A;
        entity->state = 1;
        entity->timer = 0x10;
    case 1:
        if (entity->timer != 0)
        {
            entity->timer--;
            goto cycle;
        }
        Anim_StartIndexed(entity, 4, 2, 0);
        entity->state = 2;
        entity->timer = 0x30;
        PlaySong(0x102);
        break;
    case 2:
        if (entity->timer == 0)
            goto start_fall;
        if (--entity->timer <= 0x20)
            break;
    cycle:
        {
            u8 palette = parent->unk_510.part8.basePalette;
            u8 value = sUnk_085287D6[(entity->timer >> 1) & 3];
            u8 slot = palette - 10;
            u8 *slots = gEwramData->gameState.sharedObjPaletteVariants;

            if (slots[slot] != value)
            {
                slots[slot] = value;
                ObjPalette_LoadRange((u8 *)gEwramData->gameState.sharedObjPaletteSources[slot], value, 1, palette);
            }
        }
        break;
    start_fall:
        Anim_StartIndexed(entity, 0xE, 2, 1);
        entity->state = 3;
        entity->timer = 0;
        entity->velY.whole = -0x1F000;
        entity->accelY.whole = 0xC00;
        PlaySong(0x101);
        break;
    case 3:
        EntityAccelerateAndMoveY(entity, 0x80000);
        if (entity->timer <= 0x4F)
        {
            entity->timer++;
            break;
        }
        entity->velY.whole = 0;
        entity->accelY.whole = 0;
        Anim_StartIndexed(entity, 6, 2, 0);
        entity->state = 4;
        entity->velX.whole = 0x60000;
        entity->accelX.whole = -0x8000;
        break;
    case 4:
        if (entity->velX.whole > 0x20000)
            EntityAccelerateAndMoveX(entity, 0x80000);
        if (!(entity->animFlags & 4))
            break;
    case 5:
        EntityDelete(entity);
        return;
    }
    Anim_Update(entity);
}
