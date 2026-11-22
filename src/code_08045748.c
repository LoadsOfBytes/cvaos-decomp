#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

struct TrailTiming { u16 unused, timer; };
struct TrailSample { s16 x, y; u8 frame, flip; u8 pad[2]; };
extern void Heap_Free(void *);
extern s32 GetEntityRoomXPositionInteger(struct EwramData_EntityData *);
extern s32 GetEntityRoomYPositionInteger(struct EwramData_EntityData *);

/**
 * @brief Record owner position, frame and facing in 24 history entries and display the selected entry; free history and delete on expiration or invalid owner.
 */
void Afterimage_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = entity->unk_4FC.unk_4FC_entity;
    struct TrailSample *history = (struct TrailSample *)entity->unk_4F8.unk_4F8_entity;
    s32 i;
    struct BgLayerState *bg;
    if (!owner || !owner->updateFunc || ((entity->unk_504.whole & 0x20) && (s16)owner->unk_518.part16.hp <= 0))
        entity->state = 3;
    switch (entity->state)
    {
    case 0:
        if ((u16)((struct TrailTiming *)&entity->unk_500)->timer != 0)
            entity->state = 1;
        else
            entity->state = 2;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        break;
    case 1:
        if ((u16)((struct TrailTiming *)&entity->unk_500)->timer)
            ((struct TrailTiming *)&entity->unk_500)->timer--;
        else
            goto destroy;
        break;
    case 3:
    destroy:
        Heap_Free(entity->unk_4F8.unk_4F8_entity);
        EntityDelete(entity);
        return;
    }
    entity->hidden = owner->hidden;
    for (i = 23; i > 0; i--)
    {
        history[i].x = history[i - 1].x;
        history[i].y = history[i - 1].y;
        history[i].frame = history[i - 1].frame;
        history[i].flip = history[i - 1].flip;
    }
    history[0].x = GetEntityRoomXPositionInteger(owner);
    history[0].y = GetEntityRoomYPositionInteger(owner);
    history[0].frame = owner->animFrame;
    history[0].flip = owner->flipX;
    entity->timer++;
    if ((entity->unk_504.whole & 0x10) && (entity->timer & 3) == 0)
    {
        if (entity->subState == 0 || --entity->subState == 0)
        {
            entity->state = 3;
            return;
        }
    }
    if (entity->unk_504.whole & 4)
        entity->animFrame = owner->animFrame;
    else
        entity->animFrame = history[entity->subState].frame;
    entity->flipX = history[entity->subState].flip;
    if (entity->unk_504.whole & 0x80)
        entity->flipX ^= 1;
    {
        s32 offset = entity->subState * 8; struct TrailSample *sample; offset += (u32)history; sample = (struct TrailSample *)offset;
        bg = &gEwramData->bgInfo[1];
        entity->posX.part16.integer = sample->x - bg->xPos.part16.integer;
    }
    entity->posY.part16.integer = history[entity->subState].y - bg->yPos.part16.integer;
}
