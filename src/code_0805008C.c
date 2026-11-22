#include "code_08009A0.h"
#include "code_08001194.h"
#include "code_08011DD0.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

extern s32 Hit_CheckWithPlayer(void *res, struct EwramData_EntityData *b);
extern void Debris_SpawnBurst(struct EwramData_EntityData *entity, s32 dy);
extern void BreakableBlock_SpawnDust(struct EwramData_EntityData *entity);
extern void BreakableBlock_ClearCollision(struct EwramData_EntityData *entity);
extern u8 sCommonObjectGfx[];

/**
 * @brief 5008C | Breakable object (unk_516 = flag index): while
 * Player_TestSoulStateFlags holds it sheds dust Effect_SpawnShared every 16th frame; when hit
 * (contact bits 0x1A) a debris object BreakableBlock_SpawnDust appears, each hit costs
 * one of unk_518 (song 0xD3); the last one breaks it (Debris_SpawnBurst, song
 * 0xAB, BreakableBlock_ClearCollision), sets its flag in gameFlagsTail, updates the map
 * (GameFlag_FindRoomCoordinate / GameFlag_Set) and finishes it.
 */
void Object08Update(struct EwramData_EntityData *entity)
{
    u16 hit[8];
    struct EwramData_EntityData *child;
    s32 r;
    u16 x, y;
    struct EwramData_EntityData *dust;

    if (entity->unk_514.part16.spawnParam1 == 0)
        return;
    if (Player_TestSoulStateFlags(0x1000000))
    {
        entity->timer++;
        if ((entity->timer & 0xF) == 0)
        {
            dust = Effect_SpawnShared(entity->posX.whole, entity->posY.whole, 12, 6);
            if (dust != NULL)
                dust->spriteLayer = entity->spriteLayer - 1;
        }
    }
    if (entity->state != 0)
        return;
    Hit_CheckWithPlayer(hit, entity);
    if (!(hit[0] & 0x1A))
        return;
    ObjGfx_Find(sCommonObjectGfx);
    child = EntityCreateInRange(0x49, 0xCF, (u32 *)BreakableBlock_SpawnDust);
    if (child != NULL)
    {
        child->unk_4F8.part8.unk_4F8 = entity->unk_4F8.part8.unk_4F8;
        child->spriteLayer = entity->spriteLayer - 1;
        child->posX.part16.integer = entity->posX.part16.integer;
        child->posY.part16.integer = entity->posY.part16.integer;
        child->scrollWithBg = 1;
    }
    if ((s16)entity->unk_518.part16.hp != 0)
    {
        PlaySong(0xD3);
        entity->unk_518.part16.hp--;
        return;
    }
    Debris_SpawnBurst(entity, 0);
    PlaySong(0xAB);
    BreakableBlock_ClearCollision(entity);
    gEwramData->gameState.gameFlagsTail[entity->unk_514.part16.spawnParam1 >> 5] |= 1 << (entity->unk_514.part16.spawnParam1 & 0x1F);
    x = GetEntityRoomXPositionInteger(entity);
    y = GetEntityRoomYPositionInteger(entity);
    r = GameFlag_FindRoomCoordinate((s16)x, (s16)y, (u8)entity->unk_4F8.part8.unk_4F8);
    if (r >= 0)
        GameFlag_Set(r);
    entity->deleteRequested = 1;
}
