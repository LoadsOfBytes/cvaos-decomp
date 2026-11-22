/* Chaos second form: palette sequence (4CE4) and part creation (4D64). */
#include "code_08009A0.h"
#include "chaos_entity.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

/* allocation of 10 words in the core's unk_504 (second form) */
struct Chaos2Data {
    s32 graphics;
    s32 palette;
    s32 palette2;
    s32 timer;
    s32 count;
    s32 summoned;
    s32 paletteStep;      /* 0x18: palette sequence step */
    s32 paletteFrameTime; /* 0x1C */
    s32 paletteSequenceDone;      /* 0x20 */
    u8 *paletteSequence;  /* 0x24: count, then (colour, frames) pairs */
};


struct HitBox {
    s8 x;
    s8 y;
    u8 w;
    u8 h;
};
extern u8 sChaos2SpriteSheet[];
extern void Chaos2Part_Update(struct EwramData_EntityData *);
extern void Chaos2Part_OnHit(struct EwramData_EntityData *);
extern void Chaos2Part_DamagePlayerOnContact(struct EwramData_EntityData *);

/* Step the palette sequence: hold each colour for its frame count and
 * load it into the core's palette slot when it changes. */
void Chaos2_StepPaletteSequence(struct EwramData_EntityData *core)
{
    struct Chaos2Data *data = (struct Chaos2Data *)CHAOS_DATA(core);
    u8 *sequence = data->paletteSequence;
    u8 palette, colour, slot;

    if (data->paletteSequenceDone != 0)
        return;
    if (++data->paletteFrameTime >= sequence[data->paletteStep * 2 + 2])
    {
        data->paletteFrameTime = 0;
        if (++data->paletteStep >= sequence[0])
        {
            data->paletteSequenceDone = 1;
            return;
        }
    }
    palette = core->unk_510.part8.basePalette;
    colour = sequence[data->paletteStep * 2 + 1];
    slot = palette - 10;
    if (gEwramData->gameState.sharedObjPaletteVariants[slot] != colour)
    {
        gEwramData->gameState.sharedObjPaletteVariants[slot] = colour;
        ObjPalette_LoadRange((u8 *)gEwramData->gameState.sharedObjPaletteSources[slot], colour, 1, palette);
    }
}

/* Create a 3000 HP part (graphics sChaos2SpriteSheet, random start frame,
 * box 0x1010F8F8, hit by Chaos2Part_DamagePlayerOnContact / Chaos2Part_OnHit) at (x, y), mirrored
 * by bits 0/1 of mirror. */
void Chaos2_CreatePart(struct EwramData_EntityData *core, s32 x, s32 y, s32 mirror)
{
    struct EwramData_EntityData *part = EntityCreateInRange(0x49, 0xCF, (u32 *)Chaos2Part_Update);
    struct HitBox box = { -8, -8, 16, 16 };
    struct Chaos2Data *data = (struct Chaos2Data *)CHAOS_DATA(core);
    if (part != NULL)
    {
        s32 graphics;

        part->posX.whole = x;
        part->posY.whole = y;
        part->spriteLayer = 0x14;
        graphics = data->graphics;
        part->unk_510.part8.basePalette = data->palette;
        Sprite_Attach(part, sChaos2SpriteSheet, graphics, part->unk_510.part8.basePalette);
        Anim_StartIndexed(part, 0, 2, 1);
        part->animMode = 2;
        part->animTimer = RandomNumberGenerator() % 6;
        part->unk_518.part8.enemyId = core->unk_518.part8.enemyId;
        CHAOS_LINK(part) = core;
        part->scrollWithBg = 0;
        part->attack = 0x55;
        part->unk_518.part16.hp = 3000;
        part->enemyFlags |= 1;
        Hit_Enable(part, 8, Chaos2Part_DamagePlayerOnContact, (s32)Chaos2Part_OnHit);
        HitBox_SetBody(part, &box);
        HitBox_SetAttack(part, &box);
        part->hit.flags &= ~4;
        part->hit.flags &= ~2;
        if (mirror & 1)
            part->flipX = 1;
        if (mirror & 2)
            part->flipY = 1;
        part->drawFunc = (u32 *)Sprite_DrawEntity;
    }
}
