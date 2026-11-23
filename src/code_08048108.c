/* Soul type selector entity. */
#include "gba.h"
#include "menu_page.h"
#include "structs/ewram.h"
#include "code/code_08039340.h"
#include "structs/entity_render_flags.h"

/* Overlay of the entity fields used by the soul type selector. */
struct SoulTypeIcons
{
    u8 pad_0[0xA];
    u8 state;          /* 0x0A */
    u8 pad_B[2];
    u8 step;           /* 0x0D */
    u8 pad_E[2];
    s8 sel;            /* 0x10 */
    u8 pad_11[3];
    s32 acc[3];        /* 0x14 */
    s32 dir;           /* 0x20 */
    u8 pad_24[4];
    u16 matrix[3];     /* 0x28 */
    s16 scale;         /* 0x2E */
    u8 pad_30[8];
    u8 unk_38;         /* 0x38 */
    u8 unk_39;         /* 0x39 */
    u8 pad_3A[6];
    s32 posX;          /* 0x40 */
    s32 posY;          /* 0x44 */
    u8 pad_48[0x10];
    /* Match the canonical entity slot type without a separate store overlay. */
    u16 slot:5;        /* 0x58 */
    u8 slotRest:3;
    u8 flags59_0:3;    /* 0x59 */
    u8 hidden:1;
    u8 affine:1;
    u8 flags59_5:3;
    u8 flags5A_0:3;    /* 0x5A */
    u8 palette:4;
    u8 flags5A_7:1;
    u8 pad_5B[0x65 - 0x5B];
    u8 owned;          /* 0x65 */
};

extern u8 sMenuSpriteSheet[];

/**
 * @brief 48108 | Soul type selector (three rotating icons for the red,
 * blue and yellow soul): set up, rotate by one position per 16 frames in
 * state 3 (direction from dir), mark the icon of an equipped type.
 */
void SoulTypeSelector_Update(struct SoulTypeIcons *e)
{
    struct EwramData *ew = gEwramData;
    struct PlayerStatus *stats = &ew->playerStatus;
    s32 found;
    s32 found2;

    switch (e->state)
    {
    case 0:
        Sprite_Attach((struct EwramData_EntityData *)e, sMenuSpriteSheet, MENU_GFX_SLOT(({ struct EwramData_EntityData *s_ = &ew->entityData[18]; s_; })), 0xD);
        Anim_StartIndexed((struct EwramData_EntityData *)e, 0, 0, 1);
        e->matrix[0] = ObjAffine_Alloc(2);
        e->matrix[1] = ObjAffine_Alloc(2);
        e->matrix[2] = ObjAffine_Alloc(2);
        e->owned = 0;
        e->sel = 0;
    case 1:
        e->state = 2;
        e->step = 0;
        e->acc[0] = 0;
        e->acc[1] = 0;
        e->acc[2] = 0;
        e->dir = 0x8000;
        e->unk_39 = 0;
    case 2:
    case 3:
        if (e->state == 3)
        {
            if (e->dir >= 0)
            {
                e->scale += 0x400;
                if (e->scale > 0x3FFF)
                    e->scale = 0x4000;
            }
            else
                e->scale -= 0x400;
        }

        switch (e->sel)
        {
        case 0:
            e->palette = 0xA;
            break;
        case 1:
            e->palette = 0xB;
            break;
        case 2:
            e->palette = 0xC;
            break;
        }
        if (e->state == 3)
        {
            e->slot = e->matrix[0];
            e->affine = 1;
            if (e->dir >= 0)
                ObjAffine_Set(0, 0x10000 - e->scale * 2, 0x10000 - e->scale * 2, 0, 0, e->matrix[0]);
            else
                ObjAffine_Set(0, e->scale + 0x10000, e->scale + 0x10000, 0, 0, e->matrix[0]);
            e->acc[0] += e->dir;
        }
        else
        {
            e->affine = 0;
            e->acc[0] = 0;
        }
        e->posX = 0x150000;
        e->posY = 0x200000;
        if (e->dir >= 0)
        {
            s32 kx = 0xA000;
            s32 ky = 0x1C000;

            e->posX = e->step * kx + 0x150000;
            e->posY = e->step * ky + 0x200000;
        }
        else
        {
            s32 kx = 0xA000;

            e->posX = 0x150000 - e->step * kx;
            e->posY = (e->step << 16) + 0x200000;
        }
        found = 0;
        switch (e->sel)
        {
        case 0:
            if (stats->equippedRedSoul)
                found = 1;
            break;
        case 1:
            if (stats->equippedBlueSoul)
                found = 1;
            break;
        case 2:
            if (stats->equippedYellowSoul)
                found = 1;
            break;
        }
        if (found)
        {
            e->owned = 1;
            Anim_Update((struct EwramData_EntityData *)e);
            Sprite_Draw((struct EwramData_EntityData *)e);
        }
        if (!found)
        {
            e->owned = 0;
            Sprite_Draw((struct EwramData_EntityData *)e);
        }

        switch (e->sel)
        {
        case 2:
            e->palette = 0xA;
            break;
        case 0:
            e->palette = 0xB;
            break;
        case 1:
            e->palette = 0xC;
            break;
        }
        e->slot = e->matrix[1];
        e->affine = 1;
        if (e->state == 3)
        {
            ObjAffine_Set(0, e->scale + 0xC000, e->scale + 0xC000, 0, 0, e->matrix[1]);
            e->acc[1] += e->dir;
        }
        else
        {
            ObjAffine_Set(0, 0xC000, 0xC000, 0, 0, e->matrix[1]);
            e->acc[1] = 0;
        }
        e->posX = 0xB0000;
        e->posY = 0x300000;
        if (e->dir >= 0)
        {
            s32 kx = 0xA000;

            e->posX = e->step * kx + 0xB0000;
            e->posY = 0x300000 - (e->step << 16);
        }
        else
        {
            s32 kx = 0x14000;
            s32 ky = 0xC000;

            e->posX = e->step * kx + 0xB0000;
            e->posY = e->step * ky + 0x300000;
        }
        found2 = 0;
        switch (e->sel)
        {
        case 2:
            if (stats->equippedRedSoul)
                found2 = 1;
            break;
        case 0:
            if (stats->equippedBlueSoul)
                found2 = 1;
            break;
        case 1:
            if (stats->equippedYellowSoul)
                found2 = 1;
            break;
        }
        if (found2)
        {
            e->owned = 1;
            Sprite_Draw((struct EwramData_EntityData *)e);
        }
        if (!found2)
        {
            e->owned = 0;
            Sprite_Draw((struct EwramData_EntityData *)e);
        }

        switch (e->sel)
        {
        case 1:
            e->palette = 0xA;
            break;
        case 2:
            e->palette = 0xB;
            break;
        case 0:
            e->palette = 0xC;
            break;
        }
        e->slot = e->matrix[2];
        e->affine = 1;
        if (e->state == 3)
        {
            if (e->dir >= 0)
                ObjAffine_Set(0, e->scale + 0x8000, e->scale + 0x8000, 0, 0, e->matrix[2]);
            else
                ObjAffine_Set(0, 0x8000 - e->scale * 2, 0x8000 - e->scale * 2, 0, 0, e->matrix[2]);
            e->acc[2] += e->dir;
        }
        else
        {
            ObjAffine_Set(0, 0x8000, 0x8000, 0, 0, e->matrix[2]);
            e->acc[2] = 0;
        }
        {
            s32 x0, y0;
            s32 kx, ky;

            x0 = 0x1F0000;
            e->posX = x0;
            y0 = 0x3C0000;
            e->posY = y0;
            if (e->dir >= 0)
            {
                kx = 0x14000;
                ky = 0xC000;
            }
            else
            {
                kx = 0xA000;
                ky = 0x1C000;
            }
            e->posX = x0 - e->step * kx;
            e->posY = y0 - e->step * ky;
        }
        found2 = 0;
        switch (e->sel)
        {
        case 1:
            if (stats->equippedRedSoul)
                found2 = 1;
            break;
        case 2:
            if (stats->equippedBlueSoul)
                found2 = 1;
            break;
        case 0:
            if (stats->equippedYellowSoul)
                found2 = 1;
            break;
        }
        if (found2)
        {
            e->owned = 1;
            Sprite_Draw((struct EwramData_EntityData *)e);
        }
        if (!found2)
        {
            e->owned = 0;
            Sprite_Draw((struct EwramData_EntityData *)e);
        }

        if (e->state == 3)
        {
            if (++e->step > 15)
            {
                e->scale = 0;
                e->state--;
                e->step = 0;
                if (e->unk_38 == 0)
                {
                    if (e->dir >= 0)
                    {
                        if (++e->sel > 2)
                            e->sel = 0;
                    }
                    else
                    {
                        if (--e->sel < 0)
                            e->sel = 2;
                    }
                }
                else
                    e->unk_38 = 0;
                e->acc[0] = 0;
                e->acc[1] = 0;
                e->acc[2] = 0;
            }
        }
        break;
    case 4:
        if (e->unk_38 == 0)
        {
            e->state = 3;
            if (e->dir >= 0)
            {
                if (++e->sel > 2)
                    e->sel = 0;
            }
            else
            {
                if (--e->sel < 0)
                    e->sel = 2;
            }
        }
        else
        {
            e->state -= 2; /* 4 -> 2 */
            e->unk_38 = 0;
        }
        e->step = 0;
        e->scale = 0;
        e->acc[0] = 0;
        e->acc[1] = 0;
        e->acc[2] = 0;
        break;
    case 0xFF:
        ObjAffine_Free((s16)e->matrix[0]);
        ObjAffine_Free((s16)e->matrix[1]);
        ObjAffine_Free((s16)e->matrix[2]);
        e->hidden = 1;
        break;
    }
}
