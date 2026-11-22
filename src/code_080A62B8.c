/* Chaos second form: body segment update (62B8). */
#include "code_08009A0.h"
#include "chaos_entity.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

/* The head's position history (unk_508): 120 entries of 6 bytes
 * (dx, dy, animation, priority, angle high, angle low). */
#define TRAIL_DX 0
#define TRAIL_DY 1
#define TRAIL_ANIM 2
#define TRAIL_PRIORITY 3
#define TRAIL_ANGLE_HI 4
#define TRAIL_ANGLE_LO 5

extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void EntityAccelerateAndMove(struct EwramData_EntityData *, s32);
extern s32 Entity_GetVisibilityPattern(s32);
extern void ObjAffine_Set(s32, s32, s32, u16, u16, u16);
extern void ObjAffine_Free(s32);

#define INDEX(e) ((s8)CHAOS_SNAKE_INDEX((e)))
#define TRAIL(t, e, f) ((t)[INDEX(e) * 48 + 48 + (f)])

/* Segment unk_4FE of the body: follow the head's trail 8 entries per
 * segment, compensating camera movement; the last segment (7) also takes
 * the rotation. When the head breaks (state 10) fall and blink out. */
void Chaos2SnakeSegment_Update(struct EwramData_EntityData *seg)
{
    u8 collision[16];
    struct EwramData_EntityData *head = CHAOS_LINK(seg);
    struct BgLayerState *bg = &gEwramData->bgInfo[1];

    if (seg->state == 0)
    {
        u8 *trail;

        if (head != NULL && head->updateFunc != NULL
            && head->unk_518.part8.enemyId == seg->unk_518.part8.enemyId)
        {
        trail = (u8 *)CHAOS_SNAKE_TRAIL(head);
        seg->hidden = head->hidden;
        seg->palette = head->palette;
        seg->timer++;
        seg->velX.part16.integer = (s8)TRAIL(trail, seg, TRAIL_DX)
            + ((u8)CHAOS_SNAKE_CAMERA_X(seg) - bg->xPos.part16.integer);
        seg->velY.part16.integer = (s8)TRAIL(trail, seg, TRAIL_DY)
            + (CHAOS_SNAKE_CAMERA_Y(seg) - bg->yPos.part16.integer);
        seg->posX.part16.integer += seg->velX.part16.integer;
        seg->posY.part16.integer += seg->velY.part16.integer;
        if ((seg->spriteLayer = TRAIL(trail, seg, TRAIL_PRIORITY) + 1) != 0x12)
        {
            seg->hit.flags |= 2;
            seg->hit.flags |= 4;
        }
        else
        {
            seg->hit.flags &= ~2;
            seg->hit.flags &= ~4;
        }
        CHAOS_SNAKE_CAMERA_X(seg) = bg->xPos.part16.integer;
        CHAOS_SNAKE_CAMERA_Y(seg) = bg->yPos.part16.integer;
        if (INDEX(seg) != 7)
        {
            s8 dx = (s8)TRAIL(trail, seg, TRAIL_DX);

            if (dx > 0)
                seg->flipX = 1;
            else if (dx < 0)
                seg->flipX = 0;
        }
        if (INDEX(seg) == 7)
        {
            ObjAffine_Set((((s8)TRAIL(trail, seg, TRAIL_ANGLE_HI) << 8) & 0xFF00) | TRAIL(trail, seg, TRAIL_ANGLE_LO),
                         0xC000, 0xC000, seg->flipX, seg->flipY, seg->affineIndex);
            {
                s8 anim = TRAIL(trail, seg, TRAIL_ANIM);

                if (anim == 5)
                    seg->animFrame = 5;
                else if (anim == 6)
                    seg->animFrame = 10;
                else if (anim == 10)
                    seg->animFrame = 6;
                else if (anim == 11)
                    seg->animFrame = 6;
            }
        }
        else
        {
            s8 anim = TRAIL(trail, seg, TRAIL_ANIM);

            if (anim == 5)
                seg->animFrame = 14;
            else if (anim == 11)
                seg->animFrame = 15;
        }
        if ((seg->timer & 7) == 0)
            Hit_CheckWithPlayer(collision, seg);
        if (head->state == 10)
        {
            seg->state = 1;
            seg->subState = 0;
            seg->accelX.whole = 0;
            seg->accelY.whole = 0x2000;
            seg->timer = RandomNumberGenerator() % 4 + 60;
        }
        }
        else
            EntityDelete(seg);
        return;
    }
    if (seg->subState == 0)
    {
        seg->subState++;
        seg->scrollWithBg = 1;
    }
    EntityAccelerateAndMove(seg, 0x70000);
    seg->hidden = Entity_GetVisibilityPattern(seg->timer);
    if ((s8)--seg->timer <= 0)
    {
        seg->deleteRequested = 1;
        if (INDEX(seg) == 7)
            ObjAffine_Free(seg->affineIndex);
    }
}
