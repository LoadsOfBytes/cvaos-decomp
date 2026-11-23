/* Cutscene setup with two helper entities (EventScene09_Init). The do/while (0)
 * around Anim_Update(entity) gives entity 32 refs so it gets r4. */
#include "code_08009A0.h"
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
    entity->ext.event09.actor5 = Entity_CreateFromObjectTable(5);
    if (!entity->ext.event09.actor5)
        goto delete;
    Anim_StartIndexed(entity->ext.event09.actor5, 5, 0, 0);
    Anim_Update(entity->ext.event09.actor5);
    entity->ext.event09.actor5->posX.part16.integer = entity->posX.part16.integer + 0x13;
    entity->ext.event09.actor5->posY.part16.integer = entity->posY.part16.integer;
    entity->ext.event09.actor5->velX.whole = -0x18000;
    entity->ext.event09.actor5->flipX = 1 - entity->flipX;
    entity->ext.event09.actor5->spriteLayer = entity->spriteLayer + 1;
    slot = ObjAffine_Alloc(1);
    if (slot < 0)
        goto delete;
    entity->ext.event09.scaleX = 0x10000;
    entity->ext.event09.scaleY = 0x10000;
    entity->ext.event09.actor5->affineIndex = slot;
    entity->ext.event09.actor5->affine = 1;
    ObjAffine_Set(0x10000, entity->ext.event09.scaleX, entity->ext.event09.scaleY, 0, flag, entity->ext.event09.actor5->affineIndex);
    entity->ext.event09.actor1 = Entity_CreateFromObjectTable(1);
    if (!entity->ext.event09.actor1)
    {
        EntityDelete(entity->ext.event09.actor5);
    delete:
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity->ext.event09.actor1, 0, 0, 1);
    Anim_Update(entity->ext.event09.actor1);
    { struct EwramData_EntityData *c = entity->ext.event09.actor1; struct BgLayerState *b = &gEwramData->bgInfo[1]; c->posX.part16.integer = 0xFF - b->xPos.part16.integer; }
    entity->ext.event09.actor1->posY.part16.integer = entity->posY.part16.integer;
    entity->ext.event09.actor1->velX.whole = -0x6000;
    entity->ext.event09.actor1->flipX = 0;
    gEwramData->gameState.stateFlags |= 1;
}
