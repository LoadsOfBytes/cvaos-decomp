/* IWRAM OAM builder (ARM, copied to gObjBuildOamArm by Obj_Init).
 * Walks the 32 per-layer sprite lists, applies layer priority, copies
 * the OAM entries out and scrolls all but layer 0 by the camera shake
 * offset. Built with the bundled agbcc_arm. */
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "structs/render_slots.h"

struct OamData
{
    /*0x00*/ u32 y:8;
    /*0x01*/ u32 affineMode:2;
             u32 objMode:2;
             u32 mosaic:1;
             u32 bpp:1;
             u32 shape:2;
    /*0x02*/ u32 x:9;
             u32 matrixNum:5;
             u32 size:2;
    /*0x04*/ u16 tileNum:10;
             u16 priority:2;
             u16 paletteNum:4;
    /*0x06*/ u16 affineParam;
};

struct ObjNode { struct OamData oam; s16 next; u16 pad; };

void Obj_BuildOamArm(struct ObjNode *objs, struct OamData *oam, struct ObjLayer *lists, struct ObjState *info)
{
    s32 count;
    s32 idx;
    s32 i;
    s32 dx, dy;
    u8 *pri;

    count = 0;
    pri = info->layerPriority;
    idx = lists[0].head;
    while (idx != -1)
    {
        objs[idx].oam.priority = pri[0];
        *oam = objs[idx].oam;
        oam++;
        count++;
        idx = objs[idx].next;
    }
    lists[0].tail = -1;
    lists[0].head = -1;

    dx = gEwramData->screenShakeOffsetX;
    dy = gEwramData->screenShakeOffsetY;
    for (i = 1; i < 0x20; i++)
    {
        idx = lists[i].head;
        while (idx != -1)
        {
            objs[idx].oam.priority = pri[i];
            *oam = objs[idx].oam;
            oam->x -= dx;
            oam->y -= dy;
            oam++;
            idx = objs[idx].next;
            count++;
        }
        lists[i].tail = -1;
        lists[i].head = -1;
    }
    info->oamCount = count;
}
