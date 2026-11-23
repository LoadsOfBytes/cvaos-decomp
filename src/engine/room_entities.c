#include "room_entities.h"
#include "core.h"
#include "bg_and_collision.h"
#include "in_game.h"
#include "bg_effects.h"
#include "map_flags.h"
#include "music.h"
#include "soul_inventory.h"
#include "player_effects.h"
#include "gba.h"
#include "macros.h"
#include "syscalls.h"

#include "constants/main.h"

#include "data/data_0E0334.h"
#include "data/data_0E3464.h"

#include "structs/agb_sram.h"
#include "structs/ewram.h"
#include "structs/main.h"
#include "effects.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "math_anim.h"
#include "oam.h"
#include "obj_gfx.h"
#include "object_14.h"
#include "object_19_create.h"
#include "objects_00_07.h"
#include "objects_08_0f.h"
#include "objects_10_13.h"
#include "objects_15_18.h"
#include "objects_19_1c.h"
#include "objects_22_2a.h"
#include "objects_23_34.h"
#include "objects_35_37.h"
#include "palette.h"
#include "pickup.h"
#include "save_point.h"
#include "shop_update.h"
#include "soul_pickup.h"
#include "warp_room.h"
extern void HudMap_WriteTiles(s32, s32, u16, s32);

/* Local 0x24-byte template-table view: retain this TU's types and padding. */
struct EnemyTemplateView {
    u32 *createFunc;
    u32 *updateFunc; // TODO: function prototype
    u8 pad_8[0xC - 0x8];
    u16 hp;
    u8 pad_E[0x24 - 0xE];
};

extern struct EnemyTemplateView sEnemyData[];

void (*sObjectCreateFuncs[56])(struct EwramData_EntityData *) = {
    [0] = Object00Create,
    [1] = Object01Create,
    [2] = Object02Create,
    [3] = Object03Create,
    [4] = Object04Create,
    [5] = Object05Create,
    [6] = Object06Create,
    [7] = Object07Create,
    [8] = Object08Create,
    [9] = Object09Create,
    [10] = Object0ACreate,
    [11] = Object0BCreate,
    [12] = Object0CCreate,
    [13] = Object0DCreate,
    [14] = Object0ECreate,
    [15] = Object0FCreate,
    [16] = Object10Create,
    [17] = Object11Create,
    [18] = Object12Create,
    [19] = Object13Create,
    [20] = Object14Create,
    [21] = Object15Create,
    [22] = Object16Create,
    [23] = Object17Create,
    [24] = Object18Create,
    [25] = Object19Create,
    [26] = Object1ACreate,
    [27] = Object1BCreate,
    [28] = Object1CCreate,
    [29] = Object1DCreate,
    [30] = Object1ECreate,
    [31] = Object1FCreate,
    [32] = Object20Create,
    [33] = Object21Create,
    [34] = Object22Create,
    [35] = Object23Create,
    [36] = Object24Create,
    [37] = Object25Create,
    [38] = Object26Create,
    [39] = Object27Create,
    [40] = Object28Create,
    [41] = Object29Create,
    [42] = Object2ACreate,
    [43] = Object2BCreate,
    [44] = Object2CCreate,
    [45] = Object2DCreate,
    [46] = Object2ECreate,
    [47] = Object2FCreate,
    [48] = Object30Create,
    [49] = Object31Create,
    [50] = Object32Create,
    [51] = Object33Create,
    [52] = Object34Create,
    [53] = Object35Create,
    [54] = Object36Create,
    [55] = Object37Create,
};

void (*sObjectUpdateFuncs[56])(struct EwramData_EntityData *) = {
    [0] = Object00Update,
    [1] = Object01Update,
    [2] = Object02Update,
    [3] = Object03Update,
    [4] = Object04Update,
    [5] = Object05Update,
    [6] = Object06Update,
    [7] = Object07Update,
    [8] = Object08Update,
    [9] = Object09Update,
    [10] = Object0AUpdate,
    [11] = Object0BUpdate,
    [12] = Object0CUpdate,
    [13] = Object0DUpdate,
    [14] = Object0EUpdate,
    [15] = Object0FUpdate,
    [16] = Object10Update,
    [17] = Object11Update,
    [18] = Object12Update,
    [19] = Object13Update,
    [20] = Object14Update,
    [21] = Object15Update,
    [22] = Object16Update,
    [23] = Object17Update,
    [24] = Object18Update,
    [25] = Object19Update,
    [26] = Object1AUpdate,
    [27] = Object1BUpdate,
    [28] = Object1CUpdate,
    [29] = Object1DUpdate,
    [30] = Object1EUpdate,
    [31] = Object1FUpdate,
    [32] = Object20Update,
    [33] = Object21Update,
    [34] = Object22Update,
    [35] = Object23Update,
    [36] = Object24Update,
    [37] = Object25Update,
    [38] = Object26Update,
    [39] = Object27Update,
    [40] = Object28Update,
    [41] = Object29Update,
    [42] = Object2AUpdate,
    [43] = Object2BUpdate,
    [44] = Object2CUpdate,
    [45] = Object2DUpdate,
    [46] = Object2EUpdate,
    [47] = Object2FUpdate,
    [48] = Object30Update,
    [49] = Object31Update,
    [50] = Object32Update,
    [51] = Object33Update,
    [52] = Object34Update,
    [53] = Object35Update,
    [54] = Object36Update,
    [55] = Object37Update,
};

/**
 * @brief Spawn one room entity record unless its spawned bit (roomEntitySpawn.spawnedFlags)
 * is set or its type is 0: 1 enemy (sEnemyData), 2 object, 3 pickup
 * source, 4..6 conditional item/soul pickups (Soma only; 5 also needs
 * difficultyMode, 6 an unowned item and 100.0% soul completion). Mark the
 * record spawned and return 1 when an entity slot was available.
 */
s32 RoomEntity_Spawn(struct RoomEntityRecord *record)
{
    struct EwramData_EntityData *child;
    struct EwramData_EntityData *bonusItem;
    struct EwramData_EntityData *pickup;
    struct EwramData_EntityData *modeItem;
    struct EnemyTemplateView *enemy;
    struct BgLayerState *bg;
    s32 flip;
    s32 spawned;
    s32 itemType;
    s32 itemId;
    s32 x;
    s32 alreadySpawned;

    spawned = 0;
    alreadySpawned = gEwramData->gameState.roomEntitySpawn.spawnedFlags[record->spawnIndex >> 5] & (1 << (record->spawnIndex & 0x1F));
    if (alreadySpawned || (record->kind == 0))
    {
        return 0;
    }

    child = EntityCreateInRange(0x1B, 0x48, (u32 *) EntityUpdateNothing);
    if (child != 0)
    {
        x = record->x << 0x10;
        bg = &gEwramData->bgInfo[1];
        child->posX.whole = x - bg->xPos.whole;
        child->posY.whole = (record->y << 0x10) - bg->yPos.whole;
        flip = 0;
        if (child->posX.whole < gEwramData->playerState.player->posX.whole)
        {
            flip = 1;
        }
        child->flipX = flip;
        child->scrollWithBg = 1;
        child->spawnParam0 = record->spawnParam0;
        child->spawnParam1 = record->spawnParam1;
        child->spawnIndex = record->spawnIndex;
        child->enemyId = record->entityId;
        child->drawFunc = (u32*)Sprite_DrawEntity;
        switch (record->kind)
        {
            case 1:
                enemy = &sEnemyData[record->entityId];
                child->hp = enemy->hp;
                child->updateFunc = enemy->updateFunc;
                ((void (*)(struct EwramData_EntityData *))enemy->createFunc)(child);
                break;

            case 2:
                child->updateFunc = (u32*)sObjectUpdateFuncs[record->entityId];
                sObjectCreateFuncs[record->entityId](child);
                break;

            case 3:
                child->updateFunc = (u32*)PickupSource_Update;
                PickupSource_Init((struct EwramData_EntityData *) child);
                break;

            case 4:
                if (gEwramData->playerStatus.currentCharacter != 0)
                {
                    EntityDelete((struct EwramData_EntityData *) child);
                    break;
                }

                if (!(gEwramData->gameState.collectedPickupFlags[child->spawnParam0 >> 5] & (1 << (child->spawnParam0 & 0x1F))))
                {
                    if (child->enemyId < 5)
                    {
                        pickup = Pickup_CreateItem(child->posX.part16.integer, (u16)child->posY.part16.integer, child->enemyId, child->spawnParam1, 1);
                    }
                    else
                    {
                        pickup = SoulPickup_CreateContainer(child->posX.part16.integer, (u16)child->posY.part16.integer, child->enemyId - 5, child->spawnParam1 - 1);
                    }
                    if (pickup != 0)
                    {
                        pickup->spawnParam0 = child->spawnParam0;
                    }
                }
                EntityDelete((struct EwramData_EntityData *) child);
                break;

            case 5:
                if ((gEwramData->gameState.difficultyMode) && (gEwramData->playerStatus.currentCharacter == 0))
                {
                    if (!(gEwramData->gameState.collectedPickupFlags[child->spawnParam0 >> 5] & (1 << (child->spawnParam0 & 0x1F))))
                    {
                        if (child->enemyId < 5)
                        {
                            modeItem = Pickup_CreateItem(child->posX.part16.integer, (u16)child->posY.part16.integer, child->enemyId, child->spawnParam1, 1);
                        }
                        if (modeItem != 0)
                        {
                            modeItem->spawnParam0 = child->spawnParam0;
                        }
                    }
                }
                EntityDelete((struct EwramData_EntityData *) child);
                break;

            case 6:
                if (gEwramData->playerStatus.currentCharacter != 0)
                {
                    EntityDelete((struct EwramData_EntityData *) child);
                    break;
                }

                if (!(gEwramData->gameState.collectedPickupFlags[child->spawnParam0 >> 5] & (1 << (child->spawnParam0 & 0x1F))))
                {
                    itemType = child->enemyId;
                    itemId = child->spawnParam1;
                    if ((*ItemInventory_GetCountPtr((s32) itemType, (s32) itemId) == 0) && (SoulInventory_GetCompletionTenths() == 0x3E8))
                    {
                        bonusItem = Pickup_CreateItem(child->posX.part16.integer, (u16)child->posY.part16.integer, (s32) itemType, (s32) itemId, 1);
                        if (bonusItem != 0)
                        {
                            bonusItem->spawnParam0 = child->spawnParam0;
                        }
                    }
                }
                EntityDelete((struct EwramData_EntityData *) child);
                break;
        }
        spawned = 1;
    }

    gEwramData->gameState.roomEntitySpawn.spawnedFlags[record->spawnIndex >> 5] |= 1 << (record->spawnIndex & 0x1F);
    return spawned;
}

/**
 * @brief For an X-sorted list, spawn records within the camera X window
 * (-80..+320 px), store the scan index and spawn records flagged
 * always-active (flags & 0xF).
 */
void RoomEntity_SpawnVisibleByX(struct RoomEntitySpawnState *spawnState, struct RoomEntityRecord *records)
{
    s32 i;
    s16 minX;
    s16 maxX;

    i = 0;
    minX = gEwramData->bgInfo[1].xPos.part16.integer - 80;
    maxX = gEwramData->bgInfo[1].xPos.part16.integer + 320;
    while (TRUE)
    {
        if ((records[i].x >= (maxX)) || (records[i].kind == 0))
        {
            break;
        }
        if (records[i].x >= (minX) && records[i].x <= (maxX))
        {
            RoomEntity_Spawn(&records[i]);
        }
        i++;
    }
    spawnState->scanIndex = i;

    for (i = 0; records[i].x < 0x7FFF; i++)
    {
        if (records[i].flags & 0xF)
        {
            RoomEntity_Spawn(&records[i]);
        }
    }
}

/**
 * @brief For a Y-sorted list, spawn records within the camera Y window
 * (-80..+240 px), store the scan index and spawn records flagged
 * always-active (flags & 0xF).
 */
void RoomEntity_SpawnVisibleByY(struct RoomEntitySpawnState *spawnState, struct RoomEntityRecord *records)
{
    s32 i;
    s16 minY;
    s16 maxY;

    i = 0;
    minY = gEwramData->bgInfo[1].yPos.part16.integer - 80;
    maxY = gEwramData->bgInfo[1].yPos.part16.integer + 240;
    while (TRUE)
    {
        if ((records[i].y >= (maxY)) || (records[i].kind == 0))
        {
            break;
        }
        if (records[i].y >= (minY) && records[i].y <= (maxY))
        {
            RoomEntity_Spawn(&records[i]);
        }
        i++;
    }
    spawnState->scanIndex = i;

    for (i = 0; records[i].x < 0x7FFF; i++)
    {
        if (records[i].flags & 0xF)
        {
            RoomEntity_Spawn(&records[i]);
        }
    }
}
/**
 * @brief Install the room entity list, clear the spawned bits, preload enemy
 * resources and spawn the visible records along the room's longer axis
 * (X when width >= height), compensating the camera offset flag.
 */
void RoomEntity_InitList(struct RoomEntityRecord *records)
{
    struct RoomEntityRecord *list;
    struct RoomEntitySpawnState *spawnState;
    struct BgMapMetadata *metadata;
    struct BgLayerState *bgLayer;
    struct BgLayerState *bg;

    list = records;
    spawnState = &gEwramData->gameState.roomEntitySpawn;
    spawnState->records = (struct RoomEntityRecord *) records;

    if (records != NULL)
    {
        DMA_FILL_32(3, 0, &spawnState->spawnedFlags, sizeof(spawnState->spawnedFlags));
        bg = &gEwramData->bgInfo[1];
        metadata = bg->pBgMetadata;
        if (gEwramData->gameState.spawnCameraOffsetMode)
        {
            bg->yPos.part16.integer += 0x30;
        }
        RoomEntity_PreloadEnemies(records);
        DMA_FILL_32(3, 0, &spawnState->spawnedFlags, sizeof(spawnState->spawnedFlags));

        if (metadata->widthCells >= metadata->heightCells)
        {
            spawnState->scanAxis = 0;
            RoomEntity_SpawnVisibleByX(spawnState, (struct RoomEntityRecord *) records);
        }
        else
        {
            spawnState->scanAxis = 1;
            RoomEntity_SpawnVisibleByY(spawnState, list);
        }

        if (gEwramData->gameState.spawnCameraOffsetMode)
        {
            bgLayer = &gEwramData->bgInfo[1];
            bgLayer->yPos.part16.integer -= 0x30;
        }
    }
}

/**
 * @brief Per-frame spawner: advance the scan index along the list axis in
 * the camera scroll direction and spawn records entering the window.
 */
void RoomEntity_SpawnOnScroll(void)
{
    s32 spawnRight;
    s32 spawnLeft;
    s32 spawnBottom;
    s32 spawnTop;
    struct RoomEntitySpawnState *spawnState;
    struct RoomEntityRecord *records;
    s32 scrollX;
    s32 scrollY;
    s32 clamped;
    s32 index;
    s32 upIndex;
    struct BgLayerState *bg;

    spawnState = &gEwramData->gameState.roomEntitySpawn;
    records = spawnState->records;
    bg = &gEwramData->bgInfo[1];
    if (records == NULL)
    {
        return;
    }

    switch (spawnState->scanAxis)
    {
        case 0:
            scrollX = bg->xVel.whole;
            if (scrollX > 0)
            {
                index = spawnState->scanIndex;
                spawnRight = bg->xPos.whole + 0x1200000;
                while ((spawnRight - scrollX) > (records[index].x << 0x10))
                {
                    index++;
                }

                for (; spawnRight >= (records[index].x << 0x10); index++)
                {
                    RoomEntity_Spawn(&records[index]);
                }
                spawnState->scanIndex = index;
            }

            else if (scrollX < 0)
            {
                index = spawnState->scanIndex;
                spawnLeft = bg->xPos.whole - 0x300000;
                while ((((spawnLeft - scrollX) < (records[index].x << 0x10)) && (index > 0)))
                {
                    index--;
                }

                for (; (spawnLeft <= (records[index].x << 0x10)) && (records[index].x < 0x7FFF); index--)
                {
                    RoomEntity_Spawn(&records[index]);
                    if (index == 0)
                    {
                        break;
                    }
                }
                spawnState->scanIndex = index;
            }
            break;

        case 1:
            scrollY = bg->yVel.whole;
            if (scrollY > 0)
            {
                index = spawnState->scanIndex;
                spawnBottom = bg->yPos.whole + 0xD00000;
                while ((spawnBottom - scrollY) > (records[index].y << 0x10))
                {
                    index++;
                }

                for (; spawnBottom >= (records[index].y << 0x10); index++)
                {
                    RoomEntity_Spawn(&records[index]);
                }
                spawnState->scanIndex = index;
            }

            else if (scrollY < 0)
            {
                upIndex = spawnState->scanIndex;
                spawnTop = bg->yPos.whole - 0x300000;
                while (((s32) (spawnTop - scrollY) < (s32) (records[upIndex].y << 0x10)) && (upIndex > 0))
                {
                    upIndex--;
                }

                for (; (spawnTop <= (records[upIndex].y << 0x10)) && (records[upIndex].y < 0x7FFF); upIndex--)
                {
                    RoomEntity_Spawn(&records[upIndex]);
                    if (upIndex == 0)
                    {
                        break;
                    }
                }

                clamped = upIndex;
                if (clamped < 0)
                {
                    clamped = 0;
                }
                spawnState->scanIndex = clamped;
            }
            break;
    }
}

/**
 * @brief Spawn every enemy record, run each spawned entity once (deleting
 * those not marked active) and delete them all, then reset the heap.
 * This primes enemy resources before the real spawn pass.
 */
void RoomEntity_PreloadEnemies(struct RoomEntityRecord *records)
{
    s32 j;
    s32 i;
    struct EwramData_EntityData *entity;

    for (i = 0; records[i].x < 0x7FFF; i++)
    {
        if (records[i].kind == 1)
        {
            RoomEntity_Spawn(&records[i]);
        }
    }

    entity = &gEwramData->entityData[0x1B];
    for (j = 0; j < 0xB5; j++)
    {
        if (entity->updateFunc != NULL)
        {
            if (!(entity->roomActive))
            {
                if (entity->affineIndex || entity->affine)
                {
                    ObjAffine_Free(entity->affineIndex);
                }
                EntityDelete(&gEwramData->entityData[0x1B + j]);
            }
            else
            {
                entity->roomActive = 0;
                ((void (*)(struct EwramData_EntityData *)) entity->updateFunc)(entity);
                EntityDelete(&gEwramData->entityData[0x1B + j]);
            }
        }
        entity++;
    }
    Heap_Reset();
}

/**
 * @brief Load a room definition (following its variant chain to the active
 * entry): graphics, palettes, BG1..BG3 layers at the camera position,
 * map coordinates, collision blocks and music; mark the room visited
 * and install its entity list.
 */
void Room_Load(struct RoomDefinition *room, u16 cameraX, u16 cameraY)
{
    u16 *bgControl;
    s32 i;
    struct GameState *gameState;
    struct RoomDefinition *def;
    struct RoomBgLayerDefinition *layerDef;
    struct RoomResourceLoad *paletteLoad;
    struct RoomResourceLoad *graphicsLoad;

    gameState = &gEwramData->gameState;

    def = room;
    while (def->definitionChainMarker != 0xFFFF)
    {
        def = def->nextDefinition;
    }
    room = def;

    gUnk_03002CB0.dispCnt = room->displayControl;

    graphicsLoad = room->graphicsLoads;
    while (graphicsLoad->resource != 0)
    {
        Gfx_CopyResourceBlocks(graphicsLoad->resource, graphicsLoad->destinationIndex, graphicsLoad->sourceIndex, graphicsLoad->count);
        graphicsLoad += 1;
    }

    paletteLoad = room->paletteLoads;
    while (paletteLoad->resource != 0)
    {
        BgPalette_LoadRange((u8*)paletteLoad->resource, paletteLoad->sourceIndex, paletteLoad->count, paletteLoad->destinationIndex);
        paletteLoad += 1;
    }
    if (gEwramData->gameState.stateFlags & 0x800)
    {
        BgPalette_ApplyTintedGrayscale();
    }

    layerDef = room->bgLayers;
    bgControl = gDisplayRegisters.bgCnt;
    for (i = 1; i < 4; i++)
    {
        bgControl[i] = layerDef->control;
        gEwramData->bgInfo[i].scrollMode = layerDef->scrollMode;
        gEwramData->bgInfo[i].pixelWidth = layerDef->pixelWidth;
        gEwramData->bgInfo[i].pixelHeight = layerDef->pixelHeight;
        Obj_SetBgLayer(layerDef->layer, i);
        BgMap_Setup(i, (struct BgMapMetadata*)layerDef->dimensions, BgLayer_GetInitialScrollX(cameraX, layerDef->scrollMode, layerDef->dimensions->widthCells, layerDef->pixelWidth), BgLayer_GetInitialScrollY(cameraY, layerDef->scrollMode, layerDef->dimensions->heightCells, layerDef->pixelHeight));
        layerDef++;
    }

    Obj_SetBgLayer(0, 0);
    Bg_SyncPriorities();

    gameState->roomMapXPos = room->mapX;
    gameState->roomMapYPos = room->mapY;
    gameState->unk_8E_0 = room->unk_23_0;
    gEwramData->collisionBlocksActive = room->collisionBlocksEnabled;

    Collision_ClearAllBlocks();
    Music_UpdateRoom(gameState);
    Warp_ActivateCurrentRoom();
    gameState->currentRoomDefinition = room;
    RoomEntity_InitList(room->entityRecords);
    Room_OnLoaded(gameState);
}

/**
 * @brief Delete inactive entities in slots 2..0xCF, run active non-system
 * entities once before deleting them, free affine slots 8..31 and
 * unload object graphics without a lifetime.
 */
void Room_UnloadEntities(void)
{
    s32 i;
    struct EwramData_EntityData *entity;

    entity = &gEwramData->entityData[2];
    for (i = 2; i < 0xD0; i++)
    {
        if (entity->updateFunc != NULL)
        {
            if (!entity->roomActive)
            {
                if (entity->affineIndex || entity->affine)
                {
                    ObjAffine_Free(entity->affineIndex);
                }
                EntityDelete(&gEwramData->entityData[i]);
            }
            else
            {
                entity->roomActive = 0;
                if (i > 0x1A)
                {
                    ((void (*)(struct EwramData_EntityData *)) entity->updateFunc)(entity);
                    EntityDelete(entity);
                }
            }
        }
        entity++;
    }

    for (i = 8; i < 0x20; i++)
    {
        ObjAffine_Free(i);
    }

    for (i = 0; i < 0x10; i++)
    {
        if (gEwramData->objGfx[i].tileSlot != -1 && gEwramData->objGfx[i].lifetime == 0)
        {
            ObjGfx_Unload(gEwramData->objGfx[i].tileSlot);
        }
    }
}

struct AreaNameSuppressedRoom {
    s32 area;
    s32 room;
};

const struct AreaNameSuppressedRoom sAreaNameSuppressedRooms[13] = {
    [0] = {
        .area = 0,
        .room = 0
    },
    [1] = {
        .area = 0,
        .room = 26
    },
    [2] = {
        .area = 2,
        .room = 4
    },
    [3] = {
        .area = 2,
        .room = 8
    },
    [4] = {
        .area = 2,
        .room = 5
    },
    [5] = {
        .area = 4,
        .room = 6
    },
    [6] = {
        .area = 4,
        .room = 14
    },
    [7] = {
        .area = 5,
        .room = 6
    },
    [8] = {
        .area = 6,
        .room = 30
    },
    [9] = {
        .area = 7,
        .room = 7
    },
    [10] = {
        .area = 8,
        .room = 12
    },
    [11] = {
        .area = 11,
        .room = 11
    },
    [12] = {
        .area = 11,
        .room = 32
    },
};

static inline struct BgLayerState *GetBg1(void) { return &gEwramData->bgInfo[1]; }
/* TRUE if the room under the camera is one of the 13 (area, room) pairs of LIST. */
static inline s32 IsCurrentRoomListed(struct AreaNameSuppressedRoom *list)
{
    u8 x = GetBg1()->xPos.part8.integer1;
    u8 y = GetBg1()->yPos.part8.integer1;
    s32 area = GetAreaFromMapPosition(x, y);
    s32 room = GetRoomFromMapPosition(x, y);
    s32 i;

    for (i = 0; i < 13; i++)
    {
        if (area == list[i].area && room == list[i].room)
            return TRUE;
    }
    return FALSE;
}

/**
 * @brief Return the area index whose name banner should be shown for the room
 * under the camera (with four room-specific overrides), or -1 for
 * transition, save and warp rooms, 13 listed rooms and areas already
 * announced (shownAreaNameFlags bit).
 */
s32 AreaName_GetPendingArea(struct GameState *gameState)
{
    s32 area;
    s32 result;
    u8 y;
    u8 x;
    struct AreaNameSuppressedRoom list[13];

    x = GetBg1()->xPos.part8.integer1;
    y = GetBg1()->yPos.part8.integer1;

    if (CheckRoomTransition(x, y) != 0)
        return -1;
    if (GetSaveRoomFlagFromMapPosition(x, y) != 0 || GetWarpRoomFlagFromMapPosition(x, y) != 0)
        return -1;

    memcpy(list, sAreaNameSuppressedRooms, 0x68);
    if (IsCurrentRoomListed(list))
        return -1;

    area = GetAreaFromMapPosition(x, y);

    if (GetRoomFromMapPosition(x, y) == 13 && area == 7)
        area = 12;
    else if (GetRoomFromMapPosition(x, y) == 9 && area == 8)
        area = 13;
    else if (GetRoomFromMapPosition(x, y) == 10 && area == 8)
        area = 14;
    else if (GetRoomFromMapPosition(x, y) == 11 && area == 8)
        area = 15;
    
    if (!((gameState->shownAreaNameFlags >> area) & 1))
        result = area;
    else
        result = -1;
    return result;
}

/**
 * @brief When the banner H-blank effect is active, fill 16 BG0 scroll lines
 * with a sine wave whose amplitude follows the banner timer.
 */
void AreaName_UpdateWave(s32 subModeCounter)
{
    s16 *dst;
    s32 sine;
    s32 amplitude;
    s32 i;

    dst = (s16*)&gEwramData->hBlankEffect.hBlankBuffer[1 - gEwramData->hBlankEffect.currentBuffer][0x49];

    if (gEwramData->gameState.areaNameOwnsHBlank)
    {
        *dst++ = 0;
        for (i = 0; i < 0x10; i++)
        {
            if (subModeCounter >= (i * 3))
            {
                amplitude = (subModeCounter - i * 3) * 2;
            }
            else
            {
                amplitude = 0;
            }
            sine = Sin(subModeCounter * 0x400 - i * 0x1000);
            sine = Sin(subModeCounter * 0x400 - i * 0x1000);
            if (amplitude <= 0xC0)
            {
                *dst = (sine * (amplitude >> 1)) >> 0x10;
            }
            else
            {
                *dst = (u32)(sine * 3) >> 0xB;
            }
            dst++;
        }

        gEwramData->hBlankEffect.requestStart = 1;
    }
}

u8 *sAreaNameGfx[13] = {
    (u8*)0x08260328,
    (u8*)0x08260330,
    (u8*)0x08260338,
    (u8*)0x08260340,
    (u8*)0x08260348,
    (u8*)0x08260350,
    (u8*)0x08260358,
    (u8*)0x08260360,
    (u8*)0x08260378,
    (u8*)0x08260380,
    (u8*)0x08260370,
    (u8*)0x08260388,
    (u8*)0x08260368,
};

u8 *sAreaNameGfxLanguage3[13] = {
    (u8*)0x082738AC,
    (u8*)0x082738B4,
    (u8*)0x082738BC,
    (u8*)0x082738C4,
    (u8*)0x082738CC,
    (u8*)0x082738D4,
    (u8*)0x082738DC,
    (u8*)0x082738E4,
    (u8*)0x082738FC,
    (u8*)0x08273904,
    (u8*)0x082738F4,
    (u8*)0x0827390C,
    (u8*)0x082738EC,
};

u8 *sAreaNameGfxLanguage2[13] = {
    (u8*)0x08273914,
    (u8*)0x0827391C,
    (u8*)0x08273924,
    (u8*)0x0827392C,
    (u8*)0x08273934,
    (u8*)0x0827393C,
    (u8*)0x08273944,
    (u8*)0x0827394C,
    (u8*)0x08273964,
    (u8*)0x0827396C,
    (u8*)0x0827395C,
    (u8*)0x08273974,
    (u8*)0x08273954,
};

/**
 * @brief Open the area-name banner window, start its wave effect, load the
 * language-specific name graphics for the area and mark it announced.
 */
void AreaName_Show(struct GameState *gameState, s32 area)
{

    if (area >= 13)
    {
        return;
    }

    gUnk_03002CB0.dispCnt |= DCNT_WIN0;
    gDisplayRegisters.bldY = BLDALPHA_MAX_VALUE / 2;
    gDisplayRegisters.bldCnt &= ~BLDCNT_BG0_FIRST_TARGET_PIXEL;
    gDisplayRegisters.bldCnt |= BLDCNT_SCREEN_SECOND_TARGET;
    gDisplayRegisters.win0H = C_16_2_8(0, SCREEN_SIZE_X);
    gDisplayRegisters.winIn_L = WIN0_ALL;
    gDisplayRegisters.winOut_L = WIN0_ALL_NO_COLOR_EFFECT;
    gDisplayRegisters.win0V = C_16_2_8(70, 94);
    gDisplayRegisters.bldCnt |= BLDCNT_SCREEN_FIRST_TARGET | BLDCNT_BRIGHTNESS_DECREASE_EFFECT;
    gDisplayRegisters.bldCnt &= ~BLDCNT_BG0_FIRST_TARGET_PIXEL;

    if (!gEwramData->hBlankEffect.requestStart)
    {
        gameState->areaNameOwnsHBlank = 1;
        HBlankEffectSetup(240, (s32)(SCREEN_SIZE_Y * 0.6f), 2, REG_BG0HOFS);
        AreaName_UpdateWave(gameState->subModeCounter);
    }
    else
    {
        gameState->areaNameOwnsHBlank = 0;
    }

    BgPalette_LoadRange((u8 *)0x08266520, 0, 1, 0xC);
    switch (gEwramData->gameState.language)
    {
        case 2:
            Gfx_CopyResource(sAreaNameGfxLanguage2[area], VRAM_BASE + 0x6000, 0);
            break;

        case 3:
            Gfx_CopyResource(sAreaNameGfxLanguage3[area], VRAM_BASE + 0x6000, 0);
            break;

        default:
            Gfx_CopyResource(sAreaNameGfx[area], VRAM_BASE + 0x6000, 0);
            break;
    }

    HudMap_WriteTiles(0, 0xA, 0xC300U, 0x40);
    gameState->shownAreaNameFlags |= 1 << area;
}

/**
 * @brief Banner state machine while stateFlags bit 0x80 is set: pause, show,
 * wave in, hold, wave out with fade and hide; return 1 when finished.
 */
s32 AreaName_Update(struct GameState *gameState)
{
    if (!(gEwramData->gameState.stateFlags & 0x80))
    {
        return;
    }

    UpdateInGameTimer();
    if ((gEwramData->gameState.stateFlags & 0x101) == 1)
    {
        AreaName_Dismiss(gameState);
        return;
    }

    if (gameState->areaNameState == 0)
    {
        gEwramData->gameState.stateFlags |= 0x80;
        gUnk_03002CB0.dispCnt = gameState->currentRoomDefinition->displayControl;
        Game_SetMenusEnabled(0);
        gEwramData->paused = 1;
        gameState->subModeCounter = 0x3E;
        AreaName_Show(gameState, AreaName_GetPendingArea(gameState));
        gameState->areaNameState += 1;
    }

    else if (gameState->areaNameState == 1)
    {
        gameState->subModeCounter -= 2;
        AreaName_UpdateWave(gameState->subModeCounter);
        if (gameState->subModeCounter == 0)
        {
            gameState->subModeCounter = 0x3C;
            gameState->areaNameState += 1;
        }
    }

    else if (gameState->areaNameState == 2)
    {
        if (gameState->subModeCounter != 0)
        {
            gameState->subModeCounter -= 1;
        }
        else
        {
            gameState->areaNameState += 1;
        }
    }

    else if (gameState->areaNameState == 3)
    {
        gameState->subModeCounter += 1;
        AreaName_UpdateWave(gameState->subModeCounter);
        if (gameState->subModeCounter > 0x1F)
        {
            gDisplayRegisters.bldY = (0x60 - gameState->subModeCounter) >> 3;
        }
        if (gameState->subModeCounter > 0x5F)
        {
            AreaName_Hide(gameState);
            gEwramData->startPauseEnabled = 0;
            gEwramData->paused = 1;
            gameState->areaNameState = 4;
        }
    }

    else if (gameState->areaNameState == 4)
    {
        gEwramData->gameState.stateFlags &= ~0x80;
        gEwramData->startPauseEnabled = 1;
        gEwramData->paused = 0;
        gameState->areaNameState = 0;
        return 1;
    }

    return 0;
}

/**
 * @brief Clear the banner tiles and wave buffer, restore window, blend and
 * menu state and unpause.
 */
void AreaName_Hide(struct GameState *gameState)
{
    DMA_FILL_32(3, 0, 0x0600E280, 0x80);
    DMA_FILL_32(3, 0, &gEwramData->hBlankEffect.hBlankBuffer, sizeof(gEwramData->hBlankEffect.hBlankBuffer[0]));

    gUnk_03002CB0.dispCnt &= ~DCNT_WIN0;
    if (gameState->areaNameOwnsHBlank)
    {
        gEwramData->hBlankEffect.requestStop = 1;
    }
    Game_SetMenusEnabled(1);

    gameState->areaNameState = 0;
    gEwramData->paused = 0;
    gDisplayRegisters.bldCnt |= BLDCNT_SCREEN_SECOND_TARGET;
    gDisplayRegisters.bgOfs[0].hOfs = 0;
    gEwramData->bgControlMinWriteLevel = gEwramData->unusedWriteLevel = 0;
    gDisplayRegisters.bldCnt = gameState->currentRoomDefinition->blendControl;
    gDisplayRegisters.bldY = 0;
    Display_SetDefaultBlend();
    gDisplayRegisters.winIn_L = WIN0_ALL;
    gDisplayRegisters.winOut_L = WIN0_ALL;
}

static inline void GameState_ClearFlag80()
{
    gEwramData->gameState.stateFlags &= ~0x80;
}

static inline void GameState_SetFlag100000()
{
    gEwramData->gameState.stateFlags |= 0x100000;
}

/**
 * @brief Called when the player leaves the room bounds: dismiss the area
 * banner, enter in-game sub-mode 3, black out and pause, and clamp the
 * player velocity so the transition does not carry it back.
 */
void RoomTransition_Begin(struct GameState *gameState)
{
    if (gEwramData->gameState.stateFlags & 0x80)
    {
        AreaName_Hide(gameState);
        GameState_ClearFlag80();
    }
    gameState->subMode = 3;
    gameState->subModeStep = 0;

    gEwramData->screenShakeOffsetX = gEwramData->screenShakeOffsetY = 0;
    gDisplayRegisters.bldCnt = BLDCNT_SCREEN_FIRST_TARGET | BLDCNT_BRIGHTNESS_DECREASE_EFFECT;
    gUnk_03002CB0.dispCnt &= ~DCNT_WIN0;
    gDisplayRegisters.bldY = BLDY_MAX_VALUE;

    gEwramData->startPauseEnabled = 0;
    gEwramData->paused = 1;
    GameState_SetFlag100000();

    if (gEwramData->playerState.player->posY.part16.integer >= 0x31 && gEwramData->playerState.player->posY.part16.integer <= 0xCF)
    {
        if ((s16)gEwramData->playerState.player->posX.part16.integer < 0x78)
        {
            if (gEwramData->playerState.player->velX.whole > 0)
            {
                gEwramData->playerState.player->velX.whole = 0;
                gEwramData->playerState.player->accelX.whole = 0;
            }
        }

        else if (gEwramData->playerState.player->velX.whole < 0)
        {
            gEwramData->playerState.player->velX.whole = 0;
            gEwramData->playerState.player->accelX.whole = 0;
        }

        if (gEwramData->playerState.player->velY.whole < -0x50000)
        {
            gEwramData->playerState.player->velY.whole = -0x10000;
        }
    }
}

/**
 * @brief Find the room part whose cell matches the player position, set
 * the camera scroll needed to frame the player and load that room part.
 */
void Room_LoadAtPlayerPosition(struct GameState *gameState)
{
    s32 screenX;
    struct EwramData_EntityData *player;
    struct RoomDefinition *room;
    struct RoomTransitionPart *part;
    struct BgMapMetadata *metadata;
    s16 roomX;
    s16 roomY;
    s8 cellX;
    s8 cellY;
    struct BgLayerState *bg;
    struct BgLayerState *scrollBg;

    player = gEwramData->playerState.player;
    room = gameState->currentRoomDefinition;
    bg = &gEwramData->bgInfo[1];
    scrollBg = bg;
    metadata = bg->pBgMetadata;

    DMA_FILL_32(3, 0, 0x0600E800, 0x1800);
    roomX = GetEntityRoomXPositionInteger(player);
    roomY = GetEntityRoomYPositionInteger(player);
    cellX = roomX >> 8;
    if (metadata->widthCells == 1 && (s16)player->posX.part16.integer > 0xF0)
    {
        cellX = 1;
    }

    if (metadata->heightCells > 1)
    {
        if (roomY < 0x30)
        {
            cellY = (roomY - 0x30) >> 8;
        }
        else
        {
            if (roomY > ((metadata->heightCells << 8) - 0x30))
            {
                cellY = (roomY + 0x30) >> 8;
            }
            else
            {
                cellY = roomY >> 8;
            }
        }
    }
    else
    {
        cellY = (roomY - 0x30) >> 8;
        if (player->posY.part16.integer > 0xA0)
        {
            cellY += 1;
        }
    }

    part = room->roomParts;
    while (part->cellX != cellX || part->cellY != cellY)
    {
        part++;
    }

    if (roomX < 0)
    {
        roomX = roomX + 0xF0;
    }
    else
    {
        roomX = roomX & 0xFF;
        if (roomX > 0xF0)
        {
            roomX = roomX - 0xF0;
        }
    }
    screenX = roomX + part->entryXOffset;
    scrollBg->xVel.whole = ((s16)player->posX.part16.integer - (s16)screenX) << 0x10;

    if (roomY < 0x30)
    {
        roomY = roomY + 0x70;
    }
    else
    {
        roomY = (roomY - 0x30) & 0xFF;
        if (roomY > 0xA0)
        {
            roomY = roomY - 0xA0;
        }
    }
    scrollBg->yVel.whole = (player->posY.part16.integer - roomY) << 0x10;

    Entity_ScrollWithBg(1);
    VBlankIntrWait();
    Room_Load(part->destination, part->cameraX, part->cameraY);
}

/**
 * @brief Sub-mode 3 steps: clear the old room, load the room part at the
 * player position, scroll the camera into place, arm the area banner
 * for a new area, store the entry position and set two area flags;
 * return 1 when done.
 */
s32 RoomTransition_Update(struct GameState *gameState)
{
    s32 area;
    struct EwramData_EntityData *player;
    s32 i;
    u8 yOffset;
    u8 xOffset;
    struct BgLayerState *bgLayer;
    struct BgLayerState *bg;
    struct RoomDefinition *display;

    bg = &gEwramData->bgInfo[1];
    if (gameState->subModeStep == 0)
    {
        gEwramData->bgControlMinWriteLevel = gEwramData->unusedWriteLevel = 3;
        gDisplayRegisters.bldY = BLDY_MAX_VALUE;
        Room_UnloadEntities();

        for (i = 10; i < 14; i++)
        {
            PaletteAnim_Stop(i);
        }

        Room_ResetTransientState();
        ItemIconCache_ClearIds();
        gEwramData->hBlankEffect.requestStop = 1;
        gameState->subModeStep += 1;
    }

    else if (gameState->subModeStep == 1)
    {
        Room_LoadAtPlayerPosition(gameState);
        gUnk_03002CB0.dispCnt = 0;
        gameState->subModeStep += 1;
    }

    else if (gameState->subModeStep == 2)
    {
        player = gEwramData->playerState.player;
        Camera_ScrollWithPriority(0, ((s16)player->posX.part16.integer - gEwramData->playerState.cameraAnchorX) << 0x10, (player->posY.part16.integer - gEwramData->playerState.cameraAnchorY) << 0x10);

        if (bg->xVel.whole == 0 && bg->yVel.whole == 0)
        {
            if ((AreaName_GetPendingArea(gameState) >= 0) && (AreaName_GetPendingArea(gameState) <= 0xC))
            {
                gEwramData->gameState.stateFlags |= 0x80;
            }

            gUnk_03002CB0.dispCnt = gameState->currentRoomDefinition->displayControl;
            gEwramData->bgControlMinWriteLevel = gEwramData->unusedWriteLevel = 0;
            display = gameState->currentRoomDefinition;
            gDisplayRegisters.bldCnt = display->blendControl;
            Display_SetDefaultBlend();
            gDisplayRegisters.winIn_L = WIN0_ALL;
            gDisplayRegisters.winOut_L = WIN0_ALL;

            gEwramData->paused = 0;
            gameState->subModeStep += 1;
            Room_StoreEntryPosition(gameState);
            gEwramData->gameState.stateFlags &= 0xFFEFFFFF;

            bgLayer = &gEwramData->bgInfo[1];
            xOffset = bgLayer->xPos.part8.integer1;
            yOffset = bgLayer->yPos.part8.integer1;
            area = GetAreaFromMapPosition(xOffset, yOffset);
            if ((GetRoomFromMapPosition(xOffset, yOffset) == 5) && (area == 5))
            {
                GameFlag_Set((1 << 5) | 0x1D);
            }
            else if ((GetRoomFromMapPosition(xOffset, yOffset) == 1) && (area == 5))
            {
                GameFlag_Set((1 << 5) | 0x1C);
            }
        }
    }

    else if (gameState->subModeStep == 3)
    {
        gDisplayRegisters.bldY = 0;
        gEwramData->startPauseEnabled = 1;
        gameState->subModeStep = 0;
        return 1;
    }
    return 0;
}

/**
 * @brief Sub-mode 5 steps: fade out, clear the old room, load the room part
 * at the player position, center the camera on the player and fade
 * back in; player input is held. Return 1 when done.
 */
s32 RoomFadeTransition_Update(struct GameState *gameState)
{
    struct EwramData_EntityData *player;
    struct BgLayerState *bg;
    s32 i;

    player = gEwramData->playerState.player;
    bg = &gEwramData->bgInfo[1];
    SetPlayerInput(1, 0);

    if (gameState->subModeStep == 0)
    {
        gEwramData->bgControlMinWriteLevel = gEwramData->unusedWriteLevel = 3;
        if (gDisplayRegisters.bldY < BLDY_MAX_VALUE)
        {
            if (!(gameState->subModeCounter & 3))
            {
                gDisplayRegisters.bldY += 1;
            }
            gameState->subModeCounter += 1;
        }
        else
        {
            Room_UnloadEntities();
            for (i = 10; i < 14; i++)
            {
                PaletteAnim_Stop(i);
            }
            Room_ResetTransientState();
            ItemIconCache_ClearIds();
            gEwramData->hBlankEffect.requestStop = 1;
            gEwramData->playerState.player->hidden = 0;
            gameState->subModeStep += 1;
        }
    }

    else if (gameState->subModeStep == 1)
    {
        Room_LoadAtPlayerPosition(gameState);
        Room_StoreEntryPosition(gameState);
        gameState->subModeStep += 1;
    }

    else if (gameState->subModeStep == 2)
    {
        BgMap_ScrollWithMargin(1, ((s16)player->posX.part16.integer - 0x78) << 0x10, (player->posY.part16.integer - 0x70) << 0x10);
        Entity_ScrollWithBg(1);
        Bg_UpdateScrollModes();
        if ((bg->xVel.whole == 0) && (bg->yVel.whole == 0))
        {
            gameState->subModeStep += 1;
        }
    }

    else if (gameState->subModeStep == 3)
    {
        gameState->subModeCounter += 1;
        if (!(gameState->subModeCounter & 1))
        {
            gDisplayRegisters.bldY -= 1;
        }
        if (gDisplayRegisters.bldY == 0)
        {
            gameState->subModeStep = 0;
            gEwramData->bgControlMinWriteLevel = gEwramData->unusedWriteLevel = 0;
            return 1;
        }
    }

    return 0;
}

/**
 * @brief Store the current area and room, the player position and the
 * camera position (Y minus 0x30) in the gameState progress block.
 */
void Room_StoreEntryPosition(struct GameState *gameState)
{
    s16 roomX;
    s16 roomY;
    struct BgLayerState *bg;
    s32 cameraY;
    u16 *savedY;

    roomX = GetEntityRoomXPositionInteger(gEwramData->playerState.player);
    roomY = GetEntityRoomYPositionInteger(gEwramData->playerState.player);
    gameState->currentArea = GetAreaFromMapPosition(roomX, roomY);
    gameState->currentRoom = GetRoomFromMapPosition(roomX, roomY);

    gameState->savedPlayerX = gEwramData->playerState.player->posX.part16.integer;
    gameState->savedPlayerY = gEwramData->playerState.player->posY.part16.integer;

    bg = &gEwramData->bgInfo[1];
    gameState->savedCameraX = bg->xPos.part16.integer;

    // TODO: Fake?
    cameraY = bg->yPos.part16.integer;
    savedY = &gameState->savedCameraY;
    *savedY = cameraY - 0x30;
}

/**
 * @brief Copy every graphics block listed by the room definition.
 */
void Room_LoadGraphics(struct RoomDefinition *room)
{
    // TODO: Inline for Room_Load?
    struct RoomResourceLoad *load;

    load = room->graphicsLoads;
    while (load->resource != NULL)
    {
        Gfx_CopyResourceBlocks(load->resource, load->destinationIndex, load->sourceIndex, load->count);
        load += 1;
    }
}

/**
 * @brief Load every BG palette range listed by the room definition and apply
 * the tinted grayscale when stateFlags bit 0x800 is set.
 */
void Room_LoadPalettes(struct RoomDefinition *room)
{
    // TODO: Inline for Room_Load?
    struct RoomResourceLoad *load;

    load = room->paletteLoads;
    while (load->resource != NULL)
    {
        BgPalette_LoadRange((u8 *)load->resource, load->sourceIndex, load->count, load->destinationIndex);
        load += 1;
    }
    if (gEwramData->gameState.stateFlags & 0x800)
    {
        BgPalette_ApplyTintedGrayscale();
    }
}

/**
 * @brief If the banner is active, hide it and clear stateFlags bit 0x80.
 */
void AreaName_Dismiss(struct GameState *gameState)
{
    // TODO: Inline for RoomTransition_Begin?
    if (gEwramData->gameState.stateFlags & 0x80)
    {
        AreaName_Hide(gameState);
        // TODO: does this use GameState_ClearFlag80?
        gEwramData->gameState.stateFlags &= ~0x80;
    }
}

/**
 * @brief Enter in-game sub-mode 5 with a brightness fade.
 */
void RoomFadeTransition_Begin(struct GameState *gameState)
{
    // TODO: Inline for RoomTransition_Begin? But subMode is set to 3 there
    gameState->subMode = 5;
    gameState->subModeStep = 0;

    gEwramData->screenShakeOffsetX = gEwramData->screenShakeOffsetY = 0;
    gDisplayRegisters.bldCnt = BLDCNT_SCREEN_SECOND_TARGET | BLDCNT_BRIGHTNESS_INCREASE_EFFECT | BLDCNT_SCREEN_FIRST_TARGET;
    gameState->subModeCounter = 0;
}

/**
 * @brief Unload room entities, stop palette animations 10..13, clear
 * per-room effects and item icons and stop the H-blank effect.
 */
void Room_ClearForTransition(struct GameState *gameState)
{
    // TODO: Inline for RoomTransition_Update and RoomFadeTransition_Update?
    s32 i;

    Room_UnloadEntities();

    for (i = 10; i < 14; i++)
    {
        PaletteAnim_Stop(i);
    }

    Room_ResetTransientState();
    ItemIconCache_ClearIds();
    gEwramData->hBlankEffect.requestStop = 1;
}
// #include
