/* Cutscene setup with two helper entities (EventScene09_Init). The do/while (0)
 * around Anim_Update(entity) gives entity 32 refs so it gets r4. */
#include "code_08009A0.h"
#include "event_scene09.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s32 GameFlag_GetMask(s32);
extern s32 Entity_SetupFromObjectTable(struct EwramData_EntityData *, s32);
extern struct EwramData_EntityData *Entity_CreateFromObjectTable(s32);
extern s32 ObjAffine_Alloc(u32 group);
extern void ObjAffine_Set(s32, s32, s32, u16, u16, u16);

/**
 * @brief Init of event scene variant 9 (Object 0x20 unk_514 = 9,
 * sEventSceneCreateFuncs[9]).
 */
void EventScene09_Init(struct EwramData_EntityData *self)
{
    struct EwramData_EntityData *entity = self;
    s32 flag;
    struct EwramData_EntityData *player;
    struct BgLayerState *bg;
    s32 slot;

    flag = GameFlag_GetMask(0x26);
    if (flag || (player = gEwramData->playerState.player, (s16)player->posX.part16.integer <= 0x7F))
    {
        entity->deleteRequested = 1;
        return;
    }
    player->flipX = 1;
    if (!Entity_SetupFromObjectTable(entity, 2))
        goto delete;
    Anim_StartIndexed(entity, 3, 0, 1);
    do { Anim_Update(entity); } while (0); /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
    bg = &gEwramData->bgInfo[1];
    entity->posX.part16.integer = 0x48 - bg->xPos.part16.integer;
    entity->posY.part16.integer = 0xA0 - bg->yPos.part16.integer;
    entity->flipX = 1;
    entity->spriteLayer = gEwramData->playerState.player->spriteLayer - 1;
    EVENT09_ACTOR5(entity) = Entity_CreateFromObjectTable(5);
    if (!EVENT09_ACTOR5(entity))
        goto delete;
    Anim_StartIndexed(EVENT09_ACTOR5(entity), 5, 0, 0);
    Anim_Update(EVENT09_ACTOR5(entity));
    EVENT09_ACTOR5(entity)->posX.part16.integer = entity->posX.part16.integer + 0x13;
    EVENT09_ACTOR5(entity)->posY.part16.integer = entity->posY.part16.integer;
    EVENT09_ACTOR5(entity)->velX.whole = -0x18000;
    EVENT09_ACTOR5(entity)->flipX = 1 - entity->flipX;
    EVENT09_ACTOR5(entity)->spriteLayer = entity->spriteLayer + 1;
    slot = ObjAffine_Alloc(1);
    if (slot < 0)
        goto delete;
    EVENT09_SCALE_X(entity) = 0x10000;
    EVENT09_SCALE_Y(entity) = 0x10000;
    EVENT09_ACTOR5(entity)->affineIndex = slot;
    EVENT09_ACTOR5(entity)->affine = 1;
    ObjAffine_Set(0x10000, EVENT09_SCALE_X(entity), EVENT09_SCALE_Y(entity), 0, flag, EVENT09_ACTOR5(entity)->affineIndex);
    EVENT09_ACTOR1(entity) = Entity_CreateFromObjectTable(1);
    if (!EVENT09_ACTOR1(entity))
    {
        EntityDelete(EVENT09_ACTOR5(entity));
    delete:
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(EVENT09_ACTOR1(entity), 0, 0, 1);
    Anim_Update(EVENT09_ACTOR1(entity));
    { struct EwramData_EntityData *c = EVENT09_ACTOR1(entity); struct BgLayerState *b = &gEwramData->bgInfo[1]; c->posX.part16.integer = 0xFF - b->xPos.part16.integer; }
    EVENT09_ACTOR1(entity)->posY.part16.integer = entity->posY.part16.integer;
    EVENT09_ACTOR1(entity)->velX.whole = -0x6000;
    EVENT09_ACTOR1(entity)->flipX = 0;
    gEwramData->gameState.stateFlags |= 1;
}
