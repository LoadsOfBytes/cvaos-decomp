#include "code_08009A0.h"
#include "balore_entity.h"
#include "code_0800B700.h"
#include "code_08013960.h"
#include "code/code_08039340.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"
extern s32 Entity_DistanceToPoint(struct EwramData_EntityData *, s32, s32);
extern void Balore_CreateHand(struct EwramData_EntityData *, u16, u16, u8, s32);
extern void SharedObjPalette_ResetSlots(void);
extern void ObjGfx_Unload(s32 tileSlot);
extern void EnemyBaloreUpdate(struct EwramData_EntityData *entity);
extern void EnemyBaloreCreate(struct EwramData_EntityData *entity);
extern void BaloreIntroPart_Update(struct EwramData_EntityData *entity);
extern struct EwramData_EntityData *Entity_CreateObject(void *);

/**
 * @brief B6880 | Balore entrance cutscene: waits until the player is near
 * (Entity_DistanceToPoint distance <= 0xBFFFFF: takes the input, stateFlags bit 0, song
 * 0x1010), then closer (<= 0x3FFFFF, Music_SetCurrent(0x10)); after a 0xA0-frame
 * rumble (song 0xFF on frame 0x50) it spawns BaloreIntroPart_Update, waits, runs
 * Balore_CreateHand and a few timers, then releases the input, creates the real
 * Balore (EnemyBaloreUpdate / EnemyBaloreCreate) at its position and
 * deletes itself. While the input is taken the player walks towards it in
 * state 2 and is frozen (0x80) after state 5.
 */
void BaloreIntro_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *e;
    s32 input;

    switch (entity->state)
    {
    case 0:
        entity->timer = 0x80;
        entity->state = 1;
        break;
    case 1:
    {
        struct EwramData_EntityData *player = gEwramData->playerState.player;
        if (Entity_DistanceToPoint(entity, player->posX.whole, player->posY.whole) > 0xBFFFFF)
            break;
        entity->state = 2;
        PlaySong(0x1010);
        gEwramData->gameState.stateFlags |= 1;
        Game_SetMenusEnabled(0);
        break;
    }
    case 2:
    {
        struct EwramData_EntityData *player = gEwramData->playerState.player;
        if (Entity_DistanceToPoint(entity, player->posX.whole, player->posY.whole) > 0x3FFFFF)
            break;
        entity->state = 3;
        Music_SetCurrent(0x10);
        break;
    }
    case 3:
        entity->state = 4;
        entity->timer = 0xA0;
        PlaySong(0x100);
        break;
    case 4:
        if (entity->timer)
        {
            if (entity->timer == 0x50)
                PlaySong(0xFF);
            entity->timer--;
            break;
        }
        e = EntityCreateInRange(0x49, 0xCF, (u32 *)BaloreIntroPart_Update);
        if (e)
        {
            e->ext.balorePiece.owner = entity;
            e->posX.whole = entity->posX.whole;
            e->posY.whole = entity->posY.whole;
            e->scrollWithBg = 1;
        }
        entity->timer = 0x80;
        entity->state = 5;
    case 5:
        if (entity->timer)
        {
            entity->timer--;
            break;
        }
        entity->state = 6;
        break;
    case 6:
        Balore_CreateHand(entity, 0x40, 0x90, 1, 1);
        entity->state = 7;
        entity->timer = 0x70;
        break;
    case 7:
        if (entity->timer)
        {
            entity->timer--;
            break;
        }
        entity->state = 8;
        entity->timer = 0x30;
        break;
    case 8:
        if (entity->timer)
        {
            entity->timer--;
            break;
        }
        entity->state = 9;
        entity->timer = 0;
        break;
    case 9:
        if (entity->timer)
        {
            entity->timer--;
            break;
        }
        entity->state = 10;
        break;
    case 10:
        entity->hidden = 1;
        entity->state = 11;
        break;
    case 11:
        gEwramData->gameState.stateFlags &= ~1;
        Game_SetMenusEnabled(1);
        SharedObjPalette_ResetSlots();
        ObjGfx_Unload((s8)entity->ext.balore.gfx);
        e = Entity_CreateObject(EnemyBaloreUpdate);
        if (e)
        {
            e->enemyId = entity->enemyId;
            e->spawnIndex = entity->spawnIndex;
            e->scrollWithBg = 1;
            e->posX.whole = entity->posX.whole;
            e->posY.whole = entity->posY.whole;
            EnemyBaloreCreate(e);
        }
        EntityDelete(entity);
        return;
    }
    Anim_Update(entity);
    if (gEwramData->gameState.stateFlags & 1)
    {
        input = 0;
        if (entity->state == 2)
        {
            s32 px = gEwramData->playerState.player->posX.whole;
            input = entity->posX.whole > px ? 0x10 : 0x20;
        }
        else if (entity->state > 5)
            input = 0x80;
        SetPlayerInput(2, input);
    }
}
