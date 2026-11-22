#include "code_08009A0.h"
#include "room_object_entity.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
extern void HitBox_SetBodyRaw(struct EwramData_EntityData *, u32);
extern u16 sObject1BColors[];
extern u8 sObject1BBgPalette[];

void Object1BUpdate(struct EwramData_EntityData *entity)
{
    s32 index;
    u16 color;
    u16 *colors;
    Player_QueueEntity(entity, 0);
    switch (entity->state)
    {
    case 0:
        if (gEwramData->playerState.standingPlatform == entity)
        {
            entity->subState ^= 1;
            HitBox_SetBodyRaw(entity, 0x0C10F4F8);
            Anim_StartIndexed(entity, entity->subState + 2, 0, 0);
            PlaySong(0x133);
            entity->state = 1;
        }
        break;
    case 1:
        if (gEwramData->gameState.conveyorSpeed < 0x14000) gEwramData->gameState.conveyorSpeed += 0x1000;
        break;
    }
    OBJECT1B_SCROLL(entity) += gEwramData->gameState.conveyorSpeed;
    colors = sObject1BColors;
    index = ((OBJECT1B_SCROLL(entity) / 8) >> 16) % 3;
    if (index < 0) index = -index;
    color = colors[index];
    if ((u8)OBJECT1B_COLOR(entity) != color)
    {
        OBJECT1B_COLOR(entity) = color;
        BgPalette_LoadRange(sObject1BBgPalette, color, 1, 8);
    }
    Anim_Update(entity);
}

/* Object 0x1A has no create or update behavior. */
void Object1ACreate(struct EwramData_EntityData *entity)
{
}

void Object1AUpdate(struct EwramData_EntityData *entity)
{
}
