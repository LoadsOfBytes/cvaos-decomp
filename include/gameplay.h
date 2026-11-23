#ifndef GAMEPLAY_H
#define GAMEPLAY_H

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern u8* Text_GetLocalizedEntry(s32 index);

extern s32 HitBox_GetBoxes(struct SpriteBox *attack, struct SpriteBox *body, struct EwramData_EntityData *entity); // param types?

extern void Hit_UpdateTimers(struct EwramData_EntityData *entity);
extern void HitBox_ClearAll(void);
extern s32 Hit_Enable(struct EwramData_EntityData *entity, s32 type, void *onHit, s32 onHurt);
extern void Hit_Disable(struct EwramData_EntityData *entity);
extern s32 HitBox_SetAttack(struct EwramData_EntityData *entity, void *box);
extern s32 HitBox_SetAttackRaw(struct EwramData_EntityData *entity, s32 box); // TODO: is param_1 a pointer or value?
extern s32 HitBox_SetBodyRaw(struct EwramData_EntityData *entity, s32 box);
extern s32 HitBox_SetBody(struct EwramData_EntityData *entity, void *box);
extern void Hit_SetTimerOverride(s32 index, s32 value);
extern struct SpriteBox* Hit_GetEntityBoxes(struct EwramData_EntityData *entity);
extern void Player_ShowDamage(s32 value, u8 kind);
extern void HudHPNumber_Create(void);
extern void Sprite_DrawPartsArm(void); // TODO: arm code, figure out types
extern void Sprite_DrawPartsAffineArm(void); // TODO: arm code, figure out types
extern struct EwramData_EntityData* Pickup_CreateItem(s32 x, s32 y, s32 type, s32 id, s32 hidden);
extern struct EwramData_EntityData* SoulPickup_CreateContainer(s32 x, s32 y, s32 variant, s32 index);
extern void Player_AddGold(s32 amount);
extern u8* ItemInventory_GetCountPtr(s32 type, s32 index);
extern struct EwramData_EntityData* Effect_CreateFromTable(s32 x, s32 y, s32 ax, s32 ay, s32 index, u8 palette); // TODO: verify param types
extern struct EwramData_EntityData* Effect_SpawnShared(s32 x, s32 y, s32 script, u32 palette);
extern struct EwramData_EntityData* Effect_CreateSharedFrame(s32 x, s32 y, s32 frame, u32 palette);
extern void PickupSource_Update(struct EwramData_EntityData* entity);
extern void PickupSource_Init(struct EwramData_EntityData* entity);
extern void Player_ClearEquipmentAndSoulLists(void);
extern void MenuText_PrintWithInsertedEntry(s32 id, s32 arg);
extern void MenuText_Print(s32 index);
// extern void HudMap_WriteTiles(s32 param_0, s32 param_1, u16 param_2, s32 param_3);
extern s32 HudMap_PrintNumberAttr(s32 x, s32 y, s32 value, s32 digits, s32 attr, s32 zeroPad);
extern void MenuText_ClearAndPrint(s32 index);
extern void PauseMenu_Enter(struct GameState *gameState);
extern s32 PauseMenu_Update(struct GameState *gameState);
extern s32 CursorSprite_Create(struct EwramData_EntityData *entity, u16 palette);
extern void Player_RefreshEquipmentStats(void);
extern void Player_EquipItem(s32 category, s32 item);
extern void ButtonConfig_ResetDefaults(void);
extern void Object00Create(struct EwramData_EntityData *entity);
extern void Object00Update(struct EwramData_EntityData *entity);
extern void Object02Create(struct EwramData_EntityData *entity);
extern void Object02Update(struct EwramData_EntityData *entity);
extern void Object03Create(struct EwramData_EntityData *entity);
extern void Object03Update(struct EwramData_EntityData *entity);
extern void Object04Create(struct EwramData_EntityData *entity);
extern void Object04Update(struct EwramData_EntityData *entity);
extern void Object06Create(struct EwramData_EntityData *entity);
extern void Object06Update(struct EwramData_EntityData *entity);
extern void Object05Create(struct EwramData_EntityData *entity);
extern void Object05Update(struct EwramData_EntityData *entity);
extern void Object07Create(struct EwramData_EntityData *entity);
extern void Object07Update(struct EwramData_EntityData *entity);
extern void Warp_ActivateCurrentRoom(void);

struct WarpRoomEntry {
    s16 mapX;
    s16 mapY;
    void *roomDefinition;
};
extern struct WarpRoomEntry *Warp_GetRoomEntry(u8 index);

extern s32 Warp_FindRoomIndex(u16 x, u16 y); // return s32 or s8?
extern void Object08Create(struct EwramData_EntityData *entity);
extern void Object08Update(struct EwramData_EntityData *entity);
extern void Object09Create(struct EwramData_EntityData *entity);
extern void Object09Update(struct EwramData_EntityData *entity);
extern void Object0ACreate(struct EwramData_EntityData *entity);
extern void Object0AUpdate(struct EwramData_EntityData *entity);
extern void Object0BCreate(struct EwramData_EntityData *entity);
extern void Object0BUpdate(struct EwramData_EntityData *entity);
extern void Object0CUpdate(struct EwramData_EntityData *entity);
extern void Object0CCreate(struct EwramData_EntityData *entity);
extern void Object0DCreate(struct EwramData_EntityData *entity);
extern void Object0DUpdate(struct EwramData_EntityData *entity);

#endif /* GAMEPLAY_H */
