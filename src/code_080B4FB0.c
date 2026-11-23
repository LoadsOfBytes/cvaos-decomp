#include "structs/ewram.h"
#include "balore_entity.h"

extern struct EwramData_EntityData *Entity_CreateObject(u32 *);
extern void BaloreHand_Update(struct EwramData_EntityData *);
extern void BaloreHandB_Update(struct EwramData_EntityData *);
extern void BaloreHandHitBox_Update(struct EwramData_EntityData *);
extern void BaloreHandBHitBox_Update(struct EwramData_EntityData *);

static inline s32 Sub16(s16 a, u16 b) { return (s16)a - b; }

/* Create a hand and its associated part at a room position. */
void Balore_CreateHand(struct EwramData_EntityData *owner, u16 x, u16 y, u8 flip, s32 kind)
{
    struct EwramData_EntityData *hand, *part;
    struct BgLayerState *bg;
    if (kind == 0) part = Entity_CreateObject((u32 *)BaloreHand_Update);
    else part = Entity_CreateObject((u32 *)BaloreHandB_Update);
    if (part != NULL)
    {
        part->ext.balorePart.body = owner;
        part->ext.baloreHand.armLength = 0;
        part->ext.baloreHand.homeX = x;
        part->ext.baloreHand.homeY = y;
        bg = &gEwramData->bgInfo[1];
        part->posX.part16.integer = Sub16(x, bg->xPos.part16.integer);
        part->posY.part16.integer = Sub16(y, bg->yPos.part16.integer);
        part->flipX = flip;
        part->scrollWithBg = 1;
        hand = part;
        if (kind == 0)
        {
            part->hidden = 1;
            part = Entity_CreateObject((u32 *)BaloreHandHitBox_Update);
        }
        else part = Entity_CreateObject((u32 *)BaloreHandBHitBox_Update);
        if (part != NULL)
        {
            part->ext.balorePart.body = owner;
            part->ext.baloreHitBox.hand = hand;
            part->scrollWithBg = 1;
            if (kind == 0) part->hidden = 1;
        }
    }
}




