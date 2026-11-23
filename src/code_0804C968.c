/* BestiaryMenu_UpdateDetailScreen, BestiaryMenu_SpawnEnemyPreview. */
#include "gba.h"
#include "menu_page.h"
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "macros.h"
#include "structs/ewram.h"
#include "structs/entity_template.h"

/* Language dependent graphics (English/other, French, German). */
extern u8 sLocalizedWindowGfx[];
extern u8 sLocalizedWindowGfxLanguage2[];
extern u8 sLocalizedWindowGfxLanguage3[];
extern struct BgMapMetadata sBestiaryDetailBgMap;
extern void HudMap_ClearAll(void);
extern void HudMap_WriteTiles(s32 x, s32 y, u16 tile, s32 count);
extern void BestiaryMenu_ReleaseResources(void);
extern void BestiaryMenu_DrawEntry(struct EwramData_EntityData *entity);
extern struct EwramData_EntityData *Entity_CreateObject(u32 *updateFunc);
extern void Heap_Reset(void);
extern void Hit_Disable(struct EwramData_EntityData *entity);
s32 BestiaryMenu_SpawnEnemyPreview(struct EwramData_EntityData *menu);

/**
 * @brief 4C968 | Update of the bestiary detail screen for entity: state 0
 * loads the language dependent graphics, clears the map, draws the header
 * rows and sets mode 3; then every frame BestiaryMenu_ReleaseResources, the object preview
 * (BestiaryMenu_SpawnEnemyPreview) and the list (BestiaryMenu_DrawEntry); leaving mode 3 switches to
 * mode 7 with timer = 1.
 */
void BestiaryMenu_UpdateDetailScreen(struct EwramData_EntityData *entity)
{
    u8 *set;

    switch (entity->stage)
    {
        case 0:
            switch (gEwramData->gameState.language)
            {
                case 2:
                    set = sLocalizedWindowGfxLanguage2;
                    break;
                case 3:
                    set = sLocalizedWindowGfxLanguage3;
                    break;
                default:
                    set = sLocalizedWindowGfx;
                    break;
            }
            Gfx_CopyResource(set, VRAM_BASE + 0xA000, 0);
            BgMap_Setup(1, &sBestiaryDetailBgMap, 0, 0);
            HudMap_ClearAll();
            HudMap_WriteTiles(0x12, 8, 0x1320, 10);
            HudMap_WriteTiles(0x12, 9, 0x1330, 10);
            Text_SetForegroundColor(1);
            entity->subState = 3;
            entity->stage++;
        case 1:
            BestiaryMenu_ReleaseResources();
            BestiaryMenu_SpawnEnemyPreview(entity);
            BestiaryMenu_DrawEntry(entity);
            if (entity->subState != 3)
            {
                entity->subState = 7;
                entity->timer = 1;
            }
            break;
    }
}

/**
 * @brief 4CA18 | Spawn the object of template unk_4F9 (of menu) at
 * (0x40, 0x58) plus the template offset: reset the cursor entityData[18],
 * allocate an entity with EntityUpdateNothing, copy hp, updateFunc and
 * the template index, then call the template's createFunc. Non-void in
 * the original (returns through r1) but no value is returned.
 */
s32 BestiaryMenu_SpawnEnemyPreview(struct EwramData_EntityData *menu)
{
    u8 index = menu->ext.menuPage.cursor;
    struct EntityTemplate *template = &sEnemyData[index];
    struct EwramData_EntityData *entity;

    Heap_Reset();
    /* The original adds the slot offset to the loaded gEwramData in place. */
    entity = (struct EwramData_EntityData *)gEwramData;
    entity = (struct EwramData_EntityData *)((u32)entity + OFFSET_OF(struct EwramData, entityData[18]));
    Sprite_Free(entity, 1);
    Hit_Disable(entity);
    entity = Entity_CreateObject((u32 *)EntityUpdateNothing);
    EntityGetSlot(entity);
    if (entity != NULL)
    {
        entity->unk_518.part16.hp = template->hp;
        entity->updateFunc = template->updateFunc;
        entity->unk_518.part8.enemyId = index;
        entity->posX.part16.integer = 0x40;
        entity->posY.part16.integer = 0x58;
        entity->posX.part16.integer += template->previewOffsetX;
        entity->posY.part16.integer += template->previewOffsetY;
        template->createFunc(entity);
    }
}
