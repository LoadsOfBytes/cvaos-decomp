/* Sprite object (OAM) manager: object buffer, 32 layers, BG-relative
 * priorities, affine sets, OAM build and transfer (see the overview in
 * structs/render_slots.h). Extracted from the previously unlinked upstream
 * src/engine/obj_gfx.c, 0x0803DB6C..0x0803E81F; Obj_BuildOam matches. */
#include "core.h"
#include "structs/render_slots.h"
#include "gba.h"
#include "syscalls.h"
#include "math_anim.h"
#include "oam.h"

// ARM routine copied to IWRAM (gObjBuildOamArm); only its address is used here.
extern void Obj_BuildOamArm(void);

/**
 * @brief 3DB6C | Clear the object buffer, the shadow OAM and all layers
 *
 * Shadow OAM entries are parked at x = 0x1E0 (off screen).
 */
void Obj_ResetBuffers(void)
{
    s32 i;

    DMA_FILL_32(3, 0, &gOamBuffer, 0x800);
    DMA_FILL_32(3, 0, &gObjEntries, 0xC00);

    for (i = 0; i < 0x100; i++)
    {
        gOamBuffer[i].x = 0x1E0;
        gObjEntries[i].next = -1;
        gObjEntries[i].index = i;
    }

    for (i = 0; i < 0x20; i++)
    {
        gObjLayers[i].head = -1;
        gObjLayers[i].tail = -1;
    }

    gObjState.objCount = 0;
    gObjState.layeredCount = 0;
}

/**
 * @brief 3DC18 | Set the layer positions of BG0-BG3 and derive the priorities
 *
 * Each BG's priority is its rank among the four layer values (ties count as
 * in front). Object layer N then gets the priority of the first BG whose layer
 * value is not below N, so sprites interleave with the backgrounds.
 *
 * @param bg0 Layer value of BG0 (0-31)
 * @param bg1 Layer value of BG1
 * @param bg2 Layer value of BG2
 * @param bg3 Layer value of BG3
 */
void Obj_SetBgLayers(s32 bg0, s32 bg1, s32 bg2, s32 bg3)
{
    s32 bg;
    s32 i;
    s32 priority;
    s32 rank0;
    s32 rank1;
    s32 rank2;
    s32 rank3;
    u8 layer0;
    u8 layer1;
    u8 layer2;
    u8 layer3;
    s32 threshold[0x5];

    gObjState.bgLayer[0] = bg0;
    gObjState.bgLayer[1] = bg1;
    gObjState.bgLayer[2] = bg2;
    gObjState.bgLayer[3] = bg3;

    layer0 = gObjState.bgLayer[0];
    layer1 = gObjState.bgLayer[1];
    layer2 = gObjState.bgLayer[2];
    layer3 = gObjState.bgLayer[3];

    rank0 = 0;
    if (layer0 >= layer1)
    {
        rank0 = 1;
    }
    if (layer0 >= layer2)
    {
        rank0 += 1;
    }
    if (layer0 >= layer3)
    {
        rank0 += 1;
    }
    gObjState.bgPriority[0] = rank0;

    rank1 = 0;
    if (layer1 >= layer0)
    {
        rank1 = 1;
    }
    if (layer1 >= layer2)
    {
        rank1 += 1;
    }
    if (layer1 >= layer3)
    {
        rank1 += 1;
    }
    gObjState.bgPriority[1] = rank1;

    rank2 = 0;
    if (layer2 >= layer0)
    {
        rank2 = 1;
    }
    if (layer2 >= layer1)
    {
        rank2 += 1;
    }
    if (layer2 >= layer3)
    {
        rank2 += 1;
    }
    gObjState.bgPriority[2] = rank2;

    rank3 = 0;
    if (layer3 >= layer0)
    {
        rank3 = 1;
    }
    if (layer3 >= layer1)
    {
        rank3 += 1;
    }
    if (layer3 >= layer2)
    {
        rank3 += 1;
    }
    gObjState.bgPriority[3] = rank3;

    threshold[4] = 0x1F;
    for (i = 3; i >= 0; i--)
    {
        threshold[i] = threshold[i + 1];

        for (bg = 0; bg < 4; bg++)
        {
            if (gObjState.bgPriority[bg] == i)
            {
                threshold[i] = gObjState.bgLayer[bg];
            }
        }
    }

    for (i = 0, priority = 0; i < 0x20; i++)
    {
        for (; i > threshold[priority] && priority < 3; priority++)
        {}
        gObjState.layerPriority[i] = priority;
    }
}

/**
 * @brief 3DD14 | Set the layer position of one BG (see Obj_SetBgLayers)
 *
 * @param layer Layer value (0-31)
 * @param bg BG number (0-3)
 */
void Obj_SetBgLayer(s32 layer, s16 bg_)
{
    s32 j;
    s32 i;
    s32 priority;
    s32 rank0;
    s32 rank1;
    s32 rank2;
    s32 rank3;
    u8 layer0;
    u8 layer1;
    u8 layer2;
    u8 layer3;
    s32 threshold[0x5];

    gObjState.bgLayer[bg_] = layer;

    layer0 = gObjState.bgLayer[0];
    layer1 = gObjState.bgLayer[1];
    layer2 = gObjState.bgLayer[2];
    layer3 = gObjState.bgLayer[3];

    rank0 = 0;
    if (layer0 >= layer1)
    {
        rank0 = 1;
    }
    if (layer0 >= layer2)
    {
        rank0 += 1;
    }
    if (layer0 >= layer3)
    {
        rank0 += 1;
    }
    gObjState.bgPriority[0] = rank0;

    rank1 = 0;
    if (layer1 >= layer0)
    {
        rank1 = 1;
    }
    if (layer1 >= layer2)
    {
        rank1 += 1;
    }
    if (layer1 >= layer3)
    {
        rank1 += 1;
    }
    gObjState.bgPriority[1] = rank1;

    rank2 = 0;
    if (layer2 >= layer0)
    {
        rank2 = 1;
    }
    if (layer2 >= layer1)
    {
        rank2 += 1;
    }
    if (layer2 >= layer3)
    {
        rank2 += 1;
    }
    gObjState.bgPriority[2] = rank2;

    rank3 = 0;
    if (layer3 >= layer0)
    {
        rank3 = 1;
    }
    if (layer3 >= layer1)
    {
        rank3 += 1;
    }
    if (layer3 >= layer2)
    {
        rank3 += 1;
    }
    gObjState.bgPriority[3] = rank3;

    threshold[4] = 0x1F;
    for (i = 3; i >= 0; i--)
    {
        threshold[i] = threshold[i + 1];

        for (j = 0; j < 4; j++)
        {
            if (gObjState.bgPriority[j] == i)
            {
                threshold[i] = gObjState.bgLayer[j];
            }
        }
    }

    for (i = 0, priority = 0; i < 0x20; i++)
    {
        for (; i > threshold[priority] && priority < 3; priority++)
        {}
        gObjState.layerPriority[i] = priority;
    }
}

/* OAM entry and render-list node views of gOamBuffer / gObjEntries. */
struct OamData
{
    u32 y:8;
    u32 affineMode:2;
    u32 objMode:2;
    u32 mosaic:1;
    u32 bpp:1;
    u32 shape:2;
    u32 x:9;
    u32 matrixNum:5;
    u32 size:2;
    u16 tileNum:10;
    u16 priority:2;
    u16 paletteNum:4;
    u16 affineParam;
};
struct ObjNode { struct OamData oam; s16 next; u16 pad; };

#define OBJS ((struct ObjNode *)gObjEntries)
/**
 * @brief 3DE10 | Copy the render lists (32 priorities) into the OAM buffer,
 * applying the list priority and the camera offset (lists 1..31), and reset
 * the lists; store the number of objects.
 */
void Obj_BuildOam(void)
{
    s32 count;
    s32 idx;
    s32 i;
    struct ObjNode *objs;
    s32 dx, dy;
    struct OamData *oam;
    u8 *pri = gObjState.layerPriority;

    oam = (struct OamData *)gOamBuffer;
    do { count = 0; } while (0); /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
    idx = gObjLayers[0].head;
    while (idx != -1)
    {
        OBJS[idx].oam.priority = pri[0];
        *oam = OBJS[idx].oam;
        oam++;
        idx = OBJS[idx].next;
        count++;
    }
    gObjLayers[0].tail = -1;
    gObjLayers[0].head = -1;

    dx = gEwramData->screenShakeOffsetX;
    dy = gEwramData->screenShakeOffsetY;
    for (i = 1; i < 0x20; i++)
    {
        objs = OBJS;
        idx = gObjLayers[i].head;
        while (idx != -1)
        {
            objs[idx].oam.priority = pri[i];
            *oam = OBJS[idx].oam;
            oam->x -= dx;
            oam->y -= dy;
            oam++;
            idx = OBJS[idx].next;
            count++;
        }
        gObjLayers[i].tail = -1;
        gObjLayers[i].head = -1;
    }
    gObjState.oamCount = count;
}

/**
 * @brief 3DF70 | Build the shadow OAM (ARM routine) and insert the affine parameters
 *
 * The affine parameters go to the window that Obj_TransferToOam will upload
 * (shifted by layeredCount - 128 in the flicker phase).
 */
void Obj_BuildOamFast(void)
{
    s32 i;
    s32 start;
    struct OamShadow *oam;
    struct ObjAffine *affine;
    ObjBuildOamArmFunc build;

    start = 0;
    build = (void*)gObjBuildOamArm;
    build(gObjEntries, gOamBuffer, gObjLayers, &gObjState);
    if ((gObjState.layeredCount > 0x80) && (gObjState.flags & 2))
    {
        start = gObjState.layeredCount - 0x80;
    }
    oam = &gOamBuffer[start];
    affine = gObjAffine;

    for (i = 0; i < 0x20; i++)
    {
        oam->affineParam = affine->pa;
        oam++;
        oam->affineParam = affine->pb;
        oam++;
        oam->affineParam = affine->pc;
        oam++;
        oam->affineParam = affine->pd;
        oam++;
        affine++;
    }
}

/**
 * @brief 3DFE8 | Set the OAM parameters of an affine set directly (identity transform)
 *
 * @param pa OAM parameter A (8.8)
 * @param pb OAM parameter B
 * @param pc OAM parameter C
 * @param pd OAM parameter D
 * @param index Affine set (0-31)
 */
void ObjAffine_SetParams(u16 pa, u16 pb, u16 pc, u16 pd, u16 index)
{
    if (index < 0x20)
    {
        gObjAffine[index].angle = 0;
        gObjAffine[index].scaleX = 0x10000;
        gObjAffine[index].scaleY = 0x10000;
        gObjAffine[index].flipX = 0;
        gObjAffine[index].flipY = 0;
        Matrix_SetIdentity(&gObjAffine[index].matrix);
        gObjAffine[index].pa = pa;
        gObjAffine[index].pb = pb;
        gObjAffine[index].pc = pc;
        gObjAffine[index].pd = pd;
    }
}

/**
 * @brief 3E058 | Set an affine set from rotation, scale and flips
 *
 * Stores the request and the forward matrix, then writes the inverse
 * rotation/scale as OAM parameters (scales up to 0x400 count as huge).
 *
 * @param angle Angle (0x10000 = full turn)
 * @param scaleX Horizontal scale (16.16)
 * @param scaleY Vertical scale (16.16)
 * @param flipX Nonzero: mirror horizontally
 * @param flipY Nonzero: mirror vertically
 * @param index Affine set (0-31)
 */
void ObjAffine_Set(s32 angle, s32 scaleX, s32 scaleY, u16 flipX, u16 flipY, u16 index)
{
    s32 sin;
    s32 cos;
    s32 invScaleX;
    s32 invScaleY;

    sin = Sin(angle);
    cos = Sin(angle + 0x4000);
    if (index >= 0x20)
    {
        return;
    }

    gObjAffine[index].angle = angle;
    gObjAffine[index].scaleX = scaleX;
    gObjAffine[index].scaleY = scaleY;
    gObjAffine[index].flipX = flipX;
    gObjAffine[index].flipY = flipY;
    Matrix_SetRotationScale(&gObjAffine[index].matrix, angle, scaleX, scaleY);

    if (scaleX > 0x400)
    {
        invScaleX = FixedReciprocal(scaleX);
    }
    else
    {
        invScaleX = 0x7FFFFF;
    }

    if (scaleY > 0x400)
    {
        invScaleY = FixedReciprocal(scaleY);
    }
    else
    {
        invScaleY = 0x7FFFFF;
    }

    gObjAffine[index].pa = (FixedMultiply(cos, invScaleX) < 0) ? (-(-FixedMultiply(cos, invScaleX) >> 8)) : (FixedMultiply(cos, invScaleX) >> 8);

    gObjAffine[index].pb = (FixedMultiply(sin, invScaleX) < 0) ? (-(-FixedMultiply(sin, invScaleX) >> 8)) : (FixedMultiply(sin, invScaleX) >> 8);

    gObjAffine[index].pc = (FixedMultiply(-sin, invScaleY) < 0) ? (-(-FixedMultiply(-sin, invScaleY) >> 8)) : (FixedMultiply(-sin, invScaleY) >> 8);

    gObjAffine[index].pd = (FixedMultiply(cos, invScaleY) < 0) ? (-(-FixedMultiply(cos, invScaleY) >> 8)) : (FixedMultiply(cos, invScaleY) >> 8);

    if (gObjAffine[index].flipX != 0)
    {
        gObjAffine[index].pa = -gObjAffine[index].pa;
        gObjAffine[index].pb = -gObjAffine[index].pb;
    }
    if (gObjAffine[index].flipY != 0)
    {
        gObjAffine[index].pc = -gObjAffine[index].pc;
        gObjAffine[index].pd = -gObjAffine[index].pd;
    }
}

/**
 * @brief 3E238 | Like ObjAffine_Set, with the inverse scales reduced by 0x800 (1/32)
 *
 * @param angle Angle (0x10000 = full turn)
 * @param scaleX Horizontal scale (16.16)
 * @param scaleY Vertical scale (16.16)
 * @param flipX Nonzero: mirror horizontally
 * @param flipY Nonzero: mirror vertically
 * @param index Affine set (0-31)
 */
void ObjAffine_SetBiased(s32 angle, s32 scaleX, s32 scaleY, u16 flipX, u16 flipY, u16 index)
{
    s32 cos;
    s32 sin;
    s32 invScaleX;
    s32 invScaleY;

    sin = Sin(angle);
    cos = Sin(angle + 0x4000);
    if (index >= 0x20)
    {
        return;
    }

    gObjAffine[index].angle = angle;
    gObjAffine[index].scaleX = scaleX;
    gObjAffine[index].scaleY = scaleY;
    gObjAffine[index].flipX = flipX;
    gObjAffine[index].flipY = flipY;
    Matrix_SetRotationScale(&gObjAffine[index].matrix, angle, scaleX, scaleY);

    if (scaleX > 0x400)
    {
        invScaleX = FixedReciprocal(scaleX);
    }
    else
    {
        invScaleX = 0x7FFFFF;
    }

    if (scaleY > 0x400)
    {
        invScaleY = FixedReciprocal(scaleY);
    }
    else
    {
        invScaleY = 0x7FFFFF;
    }

    gObjAffine[index].pa = (FixedMultiply(cos, invScaleX - 0x800) < 0) ? (-((-FixedMultiply(cos, invScaleX - 0x800)) >> 8)) : (FixedMultiply(cos, invScaleX - 0x800) >> 8);

    gObjAffine[index].pb = (FixedMultiply(sin, invScaleX - 0x800) < 0) ? (-((-FixedMultiply(sin, invScaleX - 0x800)) >> 8)) : (FixedMultiply(sin, invScaleX - 0x800) >> 8);

    gObjAffine[index].pc = (FixedMultiply(-sin, invScaleY - 0x800) < 0) ? (-((-FixedMultiply(-sin, invScaleY - 0x800)) >> 8)) : (FixedMultiply(-sin, invScaleY - 0x800) >> 8);

    gObjAffine[index].pd = (FixedMultiply(cos, invScaleY - 0x800) < 0) ? (-((-FixedMultiply(cos, invScaleY - 0x800)) >> 8)) : (FixedMultiply(cos, invScaleY - 0x800) >> 8);

    if (gObjAffine[index].flipX != 0)
    {
        gObjAffine[index].pa = -gObjAffine[index].pa;
        gObjAffine[index].pb = -gObjAffine[index].pb;
    }
    if (gObjAffine[index].flipY != 0)
    {
        gObjAffine[index].pc = -gObjAffine[index].pc;
        gObjAffine[index].pd = -gObjAffine[index].pd;
    }
}

/**
 * @brief 3E438 | Initialise the object manager
 *
 * Resets buffers and affine sets, enables the OAM transfer, puts all BGs on
 * layer 31, splits the affine sets 8/24 and copies Obj_BuildOamArm to IWRAM.
 */
void Obj_Init(void)
{
    gObjState.flags = 0;
    Obj_ResetBuffers();
    ObjAffine_ResetAll();
    Obj_EnableTransfer();
    Obj_SetBgLayers(0x1F, 0x1F, 0x1F, 0x1F);
    ObjAffine_InitGroups(8, 0x18);
    CPU_COPY_32(Obj_BuildOamArm, gObjBuildOamArm, 0x200);
}

/**
 * @brief 3E480 | Clear all 32 affine sets (identity matrices)
 */
void ObjAffine_ResetAll(void)
{
    s32 i;

    DMA_FILL_32(3, 0, &gObjAffine, 0x500);

    for (i = 0; i < 0x20; i++)
    {
        Matrix_SetIdentity(&gObjAffine[i].matrix);
    }
}

/**
 * @brief 3E4C0 | Allocate one object for this frame
 *
 * @return struct ObjEntry* Object, or NULL if all 256 are used
 */
struct ObjEntry* Obj_Alloc(void)
{
    if (gObjState.objCount < 0x100)
    {
        return &gObjEntries[gObjState.objCount++];
    }
    return NULL;
}

/**
 * @brief 3E4F0 | Allocate consecutive objects for this frame
 *
 * @param count Number of objects
 * @return struct ObjEntry* First object, or NULL if not enough are left
 */
struct ObjEntry* Obj_AllocMany(u32 count)
{
    struct ObjEntry *first;

    if ((gObjState.objCount + count) <= 0x100)
    {
        first = &gObjEntries[gObjState.objCount];
        gObjState.objCount += count;
        return first;
    }
    return NULL;
}

/**
 * @brief 3E528 | Append an object to a layer's list
 *
 * @param obj Object from Obj_Alloc/Obj_AllocMany
 * @param layer Layer (0-31); other values are ignored
 */
void Obj_AddToLayer(struct ObjEntry *obj, u32 layer)
{
    s32 tail;

    if (layer < 0x20)
    {
        tail = gObjLayers[layer].tail;
        if (tail != -1)
        {
            gObjEntries[tail].next = obj->index;
            gObjLayers[layer].tail = obj->index;
        }
        else
        {
            gObjLayers[layer].tail = obj->index;
            gObjLayers[layer].head = obj->index;
        }
        gObjState.layeredCount += 1;
    }
}

/**
 * @brief 3E57C | Get the priority derived for a BG by Obj_SetBgLayers
 *
 * @param bg BG number (0-3)
 * @return s32 Priority (0-3)
 */
s32 Obj_GetBgPriority(s16 bg)
{
    return gObjState.bgPriority[bg];
}

/**
 * @brief 3E594 | Copy the shadow OAM to OAM (VBlank)
 *
 * With more than 128 linked objects every other frame uploads the window
 * starting at layeredCount - 128, so all objects are shown alternately.
 */
void Obj_TransferToOam(void)
{
    s32 start;
    struct OamShadow *src;

    if (gObjState.flags & 1)
    {
        start = 0;
        if ((gObjState.layeredCount > 0x80) && (gObjState.flags & 2))
        {
            start = gObjState.layeredCount - 0x80;
        }

        src = &gOamBuffer[start];
        DMA_COPY_32(3, src, OAM_BASE, OAM_SIZE);
    }

    gObjState.flags ^= 2;
}

/**
 * @brief 3E5F4 | Enable Obj_TransferToOam
 */
void Obj_EnableTransfer(void)
{
    gObjState.flags |= 1;
}

/**
 * @brief 3E604 | Disable Obj_TransferToOam
 */
void Obj_DisableTransfer(void)
{
    gObjState.flags &= ~1;
}

/**
 * @brief 3E618 | Define the affine allocation groups and free all sets
 *
 * Group 0 = [0, split0), group 1 = [split0, split0 + split1), group 2 =
 * [split1, 32) (see ObjAffine_Alloc). Also clears the affine backup.
 *
 * @param split0 Size of group 0
 * @param split1 Size of group 1 / start of group 2
 */
void ObjAffine_InitGroups(s32 split0, s32 split1)
{
    gObjState.affineSplit0 = split0;
    gObjState.affineSplit1 = split1;
    gObjState.affineUsedMask = 0;
    DMA_FILL_32(3, 0, &gEwramData->objAffineBackup, 0x508);
}

/**
 * @brief 3E654 | Allocate the first free affine set of a group (bit in gObjState.affineUsedMask)
 * 
 * @param group Group (0: [0, affineSplit0), 1: [affineSplit0, affineSplit0 + affineSplit1), 2: [affineSplit1, 32))
 * @return s32 Affine set index, -1 if the group is full
 */
s32 ObjAffine_Alloc(u32 group)
{
    s32 first;
    s32 bit;
    s32 i;
    s32 end;

    switch (group)
    {
        case 0:
            first = 0;
            end = gObjState.affineSplit0;
            break;
        case 1:
            first = gObjState.affineSplit0;
            end = gObjState.affineSplit1 + first;
            break;
        case 2:
            first = gObjState.affineSplit1;
            end = 0x20;
            break;
    }

    for (i = first; i < end; i++)
    {
        struct ObjState *slots = &gObjState;

        bit = 1 << i;
        if (!(slots->affineUsedMask & bit))
        {
            slots->affineUsedMask |= bit;
            return i;
        }
    }
    return -1;
}

/**
 * @brief 3E6BC | Free an affine set
 *
 * @param index Affine set (0-31)
 */
void ObjAffine_Free(s32 index)
{
    gObjState.affineUsedMask &= ~(1U << index);
}

/**
 * @brief 3E6D0 | Get the angle of an affine set
 *
 * @param index Affine set (0-31)
 * @return s32 Angle
 */
s32 ObjAffine_GetAngle(s32 index)
{
    return gObjAffine[index].angle;
}

/**
 * @brief 3E6E4 | Get the horizontal scale of an affine set
 *
 * @param index Affine set (0-31)
 * @return s32 Scale (16.16)
 */
s32 ObjAffine_GetScaleX(s32 index)
{
    return gObjAffine[index].scaleX;
}

/**
 * @brief 3E6F8 | Get the vertical scale of an affine set
 *
 * @param index Affine set (0-31)
 * @return s32 Scale (16.16)
 */
s32 ObjAffine_GetScaleY(s32 index)
{
    return gObjAffine[index].scaleY;
}

/**
 * @brief 3E70C | Get the horizontal flip of an affine set
 *
 * @param index Affine set (0-31)
 * @return s32 Flip flag
 */
s32 ObjAffine_GetFlipX(s32 index)
{
    return gObjAffine[index].flipX;
}

/**
 * @brief 3E720 | Get the vertical flip of an affine set
 *
 * @param index Affine set (0-31)
 * @return s32 Flip flag
 */
s32 ObjAffine_GetFlipY(s32 index)
{
    return gObjAffine[index].flipY;
}

/**
 * @brief 3E734 | Copy the forward matrix of an affine set
 *
 * @param dst Destination matrix
 * @param index Affine set (0-31)
 */
void ObjAffine_GetMatrix(struct Matrix2 *dst, s32 index)
{
    Matrix_Copy(dst, &gObjAffine[index].matrix);
}

/**
 * @brief 3E750 | Number of entries written by the last OAM build
 *
 * @return s32 Entry count
 */
s32 Obj_GetOamCount(void)
{
    return gObjState.oamCount;
}

/**
 * @brief 3E75C | Back up the affine sets and their allocation mask, then free all
 *
 * @return 1 when saved, 0 when a backup was already held.
 */
s32 ObjAffine_SaveState(void)
{
    if (gEwramData->objAffineBackup.saved == 0)
    {
        gEwramData->objAffineBackup.usedMask = gObjState.affineUsedMask;
        gObjState.affineUsedMask = 0;
        DMA_COPY_32(3, &gObjAffine, &gEwramData->objAffineBackup.data, 0x500);
        gEwramData->objAffineBackup.saved = 1;
        return 1;
    }

    return 0;
}

/**
 * @brief 3E7C0 | Restore the affine sets backed up by ObjAffine_SaveState
 *
 * @return 1 when restored, 0 when no backup was held.
 */
s32 ObjAffine_RestoreState(void)
{
    if (gEwramData->objAffineBackup.saved != 0)
    {
        gObjState.affineUsedMask = gEwramData->objAffineBackup.usedMask;
        DMA_COPY_32(3, &gEwramData->objAffineBackup.data, &gObjAffine, 0x500);
        gEwramData->objAffineBackup.saved = 0;
        return 1;
    }

    return 0;
}
