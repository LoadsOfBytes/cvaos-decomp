/* Chaos second form: body creation (57BC). */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

struct Chaos2Data {
    s32 graphics;
    s32 palette;
    s32 palette2;
};
struct HitBox {
    s8 x;
    s8 y;
    u8 w;
    u8 h;
};

extern u8 sChaos2SpriteSheet[];
extern void *Heap_Alloc(u16);
extern struct EwramData_EntityData *Entity_CreateObject(u32 *);
extern s32 ObjAffine_Alloc(u32);
extern void ObjAffine_Set(s32, s32, s32, u16, u16, u16);
extern void Chaos2SnakeHead_Update(struct EwramData_EntityData *);
extern void Chaos2SnakeSegment_Update(struct EwramData_EntityData *);
extern void Chaos2SnakeSegment_EnableHitBoxes(struct EwramData_EntityData *);
extern void Chaos2SnakeHead_DamagePlayerOnContact(struct EwramData_EntityData *);

#define ANGLE(e) ((e)->unk_504.part16.unk_504)
#define INDEX(e) ((e)->unk_4FC.part8.unk_4FE)

/* Create the snake-like body at the core + (x, y): the head (Chaos2SnakeHead_Update)
 * with a 120-entry position history and 8 segments (Chaos2SnakeSegment_Update). */
void Chaos2_CreateSnake(struct EwramData_EntityData *core, s32 x, s32 y)
{
    u8 *trail;
    struct EwramData_EntityData *seg;
    s32 i;
    u16 slot;
    s32 graphics;
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    struct EwramData_EntityData *head = EntityCreateInRange(0x49, 0xCF, (u32 *)Chaos2SnakeHead_Update);
    struct HitBox box = { -8, -8, 16, 16 };
    struct Chaos2Data *data = (struct Chaos2Data *)core->unk_504.whole;

    if (head != NULL)
    {
        if ((head->unk_508.whole = (s32)Heap_Alloc(0xB4)) == 0)
        {
            EntityDelete(head);
            return;
        }
        head->posX.whole = core->posX.whole + x;
        head->posY.whole = core->posY.whole + y;
        head->spriteLayer = 0x1E;
        graphics = data->graphics;
        head->unk_510.part8.basePalette = data->palette2;
        Sprite_Attach(head, sChaos2SpriteSheet, graphics, head->unk_510.part8.basePalette);
        head->animFrame = 0xB;
        head->animMode = 2;
        head->unk_518.part8.enemyId = core->unk_518.part8.enemyId;
        head->unk_4F4.unk_4F4_entity = core;
        head->scrollWithBg = 0;
        head->attack = 0x55;
        head->unk_4FC.part8.unk_4FC = bg->xPos.part16.integer;
        head->unk_4FC.part8.unk_4FD = bg->yPos.part16.integer;
        head->unk_4F8.part8.unk_4FB = 0;
        head->unk_4FC.part8.unk_4FF = 4;
        Hit_Enable(head, 8, Chaos2SnakeHead_DamagePlayerOnContact, 0);
        HitBox_SetAttack(head, &box);
        head->hit.flags |= 4;
        head->hit.flags &= ~2;
        head->drawFunc = (u32 *)Sprite_DrawEntity;
        head->flipX = 0;
        ANGLE(head) = 0x4000;
        slot = ObjAffine_Alloc(1);
        if ((s16)slot < 0)
        {
            EntityDelete(head);
            return;
        }
        head->affineIndex = slot;
        head->affine = 1;
        ObjAffine_Set(0, 0x10000, 0x10000, head->flipX, head->flipY, head->affineIndex);
        trail = (u8 *)head->unk_508.whole;
        for (i = 119; i >= 0; i--)
        {
            trail[i * 6 + 0] = 0;
            trail[i * 6 + 1] = 0;
            trail[i * 6 + 2] = head->animFrame;
            trail[i * 6 + 3] = head->spriteLayer;
            trail[i * 6 + 4] = (u16)ANGLE(head) >> 8;
            trail[i * 6 + 5] = ANGLE(head);
        }
        i = 7;
        do
        {
            seg = Entity_CreateObject((u32 *)Chaos2SnakeSegment_Update);
            if (seg == NULL)
                goto fail;
            seg->spriteLayer = head->spriteLayer + 1;
            Sprite_Attach(seg, sChaos2SpriteSheet, data->graphics, head->unk_510.part8.basePalette);
            Chaos2SnakeSegment_EnableHitBoxes(seg);
            seg->unk_518.part8.enemyId = head->unk_518.part8.enemyId;
            seg->unk_510.part8.basePalette = head->unk_510.part8.basePalette;
            seg->scrollWithBg = 0;
            seg->spriteLayer = head->spriteLayer + 1;
            seg->drawFunc = (u32 *)Sprite_DrawEntity;
            seg->unk_4F4.unk_4F4_entity = head;
            INDEX(seg) = i;
            seg->timer = i;
            seg->attack = head->attack;
            seg->unk_4FC.part8.unk_4FC = head->unk_4FC.part8.unk_4FC;
            seg->unk_4FC.part8.unk_4FD = head->unk_4FC.part8.unk_4FD;
            seg->posX.whole = head->posX.whole;
            seg->posY.whole = head->posY.whole;
            seg->animFrame = 0xE;
            seg->animMode = 2;
            if (i == 7)
            {
                seg->animFrame = 5;
                slot = ObjAffine_Alloc(1);
                if ((s16)slot < 0)
                {
                    /* loop.c moves this block before the loop */
                    EntityDelete(seg);
                    return;
                }
                seg->affineIndex = slot;
                seg->affine = 1;
                ObjAffine_Set((u16)ANGLE(seg) - 0x8000, 0xC000, 0xC000, seg->flipX, seg->flipY, seg->affineIndex);
            }
            continue;
        fail:
            /* the two earlier failures are separate copies; jump2
               cross-jumps them into this block */
            EntityDelete(head);
            return;
        } while (--i >= 0);
    }
}
