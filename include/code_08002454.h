#ifndef CODE_08002454_H
#define CODE_08002454_H

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

s32 GameModeDebugUpdate(void);
s32 GameModeDebugExitUpdate(void);
void KonamiCodeCheck(void);
s32 GameModeKonamiLogoUpdate(void);
s32 GameModeLicensedByNintendoUpdate(void);
s32 GameModeResetUpdate(void);
void TitleScreen_sub_08002F44(s32 arg0);
void TitleScreen_sub_08002FF8(void);
void TitleScreen_sub_08003080(void);
s32 GameModeTitleScreenUpdate(void);
void TitleScreenParticle_Update(struct EwramData_EntityData *entity);
void TitleScreenParticle_Init(struct EwramData_EntityData *entity);
void TitleScreenSpiralSprite_Update(struct EwramData_EntityData *entity);
void TitleScreenSpiralSprite_Init(struct EwramData_EntityData *entity);
void TitleScreen_CreateEffects(struct EwramData_EntityData *entity);
void TitleScreenFlash_Update(struct EwramData_EntityData *entity);
void TitleScreen_SpawnFlash(s32 skipBlend, s32 scale, u16 spin);
void TitleScreenLogo_Update(struct EwramData_EntityData *entity);
void TitleScreenLogo_Create(struct EwramData_EntityData *entity);
void TitleScreenMenu_Draw(struct EwramData_EntityData *entity);
void TitleScreenMenu_Update(struct EwramData_EntityData *entity);
void TitleScreenMenu_Create(void);
void TitleScreenFrame_Draw(struct EwramData_EntityData *entity);
void TitleScreenFrame_Create(void);
void TitleScreenMenu_StartSlide(struct EwramData_EntityData *entity);
void TitleScreen_FreeGraphics(struct EwramData_EntityData *entity);
void TitleScreenEffects_Update(struct EwramData_EntityData *entity);
void TitleScreenLogo_Draw(struct EwramData_EntityData *entity);
void BgEffect_Bg2InterlacedWave(void);
void MainMenu_SetupMessageWindow(s32 choice);
s32 MainMenu_HandleChoiceInput(struct EwramData_EntityData *entity);
void MainMenu_Close(struct EwramData_EntityData *entity);
s32 GameModeMainMenuUpdate(void);

#endif /* CODE_08002454_H */
