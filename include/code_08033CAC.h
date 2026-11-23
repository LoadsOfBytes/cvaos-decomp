#ifndef CODE_08033CAC_H
#define CODE_08033CAC_H

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

s32 Player_ApplyEarnedLevels(struct PlayerStatus *status);
void Player_InitBaseLevelStats(void);
s32 Player_GetExperienceForLevel(s32 level);
s32 Player_GetExperienceToNextLevel(struct PlayerStatus *status);
void Player_SetLevel(s32 level);
void Player_ApplyJuliusStatGrowth(void);
void LinkPartner_Create(void);
void LinkPartner_Update(struct EwramData_EntityData *entity);
void Link_ExchangePlayerState(void);

struct HitBoxDef {
    s8 x;
    s8 y;
    u8 width;
    u8 height;
};
void HitBox_SetHitRect(struct EwramData_EntityData *entity, struct HitBoxDef *box);

void Link_ApplyPartnerHit(void);
void Link_UpdatePlayerSync(void);
void Player_SetSelectedTarget(struct EwramData_EntityData *entity);
void Link_MarkEnemyDamaged(void);
void Link_UnusedStub(void);
void LinkPartnerMarker_Update(struct EwramData_EntityData *entity);
void Julius_UpdateSubweaponIcon(struct EwramData_EntityData *entity);
void Julius_HandleSubweaponInput(struct EwramData_EntityData *entity);
void JuliusCross_Update(struct EwramData_EntityData *entity);
void JuliusHolyWaterFlame_Update(struct EwramData_EntityData *entity);
void JuliusHolyWater_Update(struct EwramData_EntityData *entity);
void JuliusAxeProjectile_Update(struct EwramData_EntityData *entity);
void JuliusAxe_Update(struct EwramData_EntityData *entity);
void JuliusGrandCross_Update(struct EwramData_EntityData *entity);

// TODO: figure out struct
struct Unk_080E1ED8 {
    u8 pad_0[0x10 - 0x0];
};
extern struct Unk_080E1ED8* Julius_GetSubweaponInfo(u8 subweapon);

void JuliusOverlay_AttachSprite(struct EwramData_EntityData *entity, s32 altPalette);
void Whip_UpdateChainHitSegment(struct EwramData_EntityData *entity);
void Whip_ConstrainChainNode(struct WhipChainNode *node, struct WhipChainNode *prev, s16 index);
void Whip_UpdateChain(struct EwramData_EntityData *entity);
void Whip_DrawChain(struct EwramData_EntityData *entity);
struct EwramData_EntityData* Whip_CreateChain(s32 x, s32 y, u8 *resource, struct EwramData_EntityData *owner);
struct EwramData_EntityData* Julius_CreateOverlaySprite(void);
void Entity_DeleteWithChild(struct EwramData_EntityData *entity);
void Whip_ActivateChain(struct EwramData_EntityData *entity);
void Whip_DeactivateChain(struct EwramData_EntityData *entity);
void Whip_LayChainStraight(struct EwramData_EntityData *entity, s16 flip);
void Whip_SwingChainRight(struct EwramData_EntityData *entity);
void Whip_SwingChainLeft(struct EwramData_EntityData *entity);
void Whip_SwingChainDown(struct EwramData_EntityData *entity);
void Whip_SwingChainUp(struct EwramData_EntityData *entity);
void JuliusOverlay_Update(struct EwramData_EntityData *entity);
s16 DebugPlatform_FloorCorrection(struct EwramData_EntityData *entity, s32 x, s32 y);
void DebugPlatform_Update(struct EwramData_EntityData *entity);

#endif /* CODE_08033CAC_H */