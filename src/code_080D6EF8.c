/* Create a scene object and update the corresponding persistent event bit. */
#include "code_08009A0.h"
#include "code_08013960.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"
extern u8 sCommonObjectGfx[];
extern u8 sCommonObjectSpriteSheet[];
extern void BossRecoveryPickup_Update(struct EwramData_EntityData *);
extern void BossRecoveryPickup_Draw(struct EwramData_EntityData *);
/**
 * @brief Update the existing boss-event/music flags and create a hidden
 * delayed recovery pickup with affine scaling and a custom draw
 * callback. Retain mode-specific event storage and failure paths.
 */
void BossRecoveryPickup_Spawn(u16 x,u16 y,s32 event,s32 delay,s32 special)
{
    struct EwramData_EntityData *entity;
    if(gEwramData->playerStatus.currentCharacter)
        Music_SubmitCommandOnce(0x1040);
    if(gEwramData->gameState.bossRushMode)
        gEwramData->gameState.bossEventFlags|=1<<event;
    else if(!special && !gEwramData->playerStatus.currentCharacter)
    {
        gEwramData->gameState.bossEventFlags|=1<<event;
        gEwramData->gameState.stateFlags&=~2;
    }
    entity=EntityCreateInRange(0x49,0xCF,(u32 *)BossRecoveryPickup_Update);
    if(entity)
    {
        u16 sprite=ObjAffine_Alloc(1);
        if((s16)sprite<0)
        {
            EntityDelete(entity);
            return;
        }
        entity->affineIndex=sprite;
        entity->affine=1;
        entity->doubleSize=1;
        {
            s32 graphics=ObjGfx_Find(sCommonObjectGfx);
            Sprite_Attach(entity,sCommonObjectSpriteSheet,graphics,7);
        }
        entity->drawFunc=(u32 *)BossRecoveryPickup_Draw;
        entity->animFrame=0x7C;
        entity->spriteLayer=10;
        entity->scrollWithBg=1;
        entity->posX.part16.integer=x;
        entity->posY.part16.integer=y;
        entity->accelY.whole=0x3000;
        entity->timer=delay;
        Hit_Enable(entity,11,0,0);
        HitBox_SetAttackRaw(entity,0x1010F8F8);
        entity->hit.flags|=2;
        entity->hit.flags|=4;
        entity->hidden=1;
        if(!gEwramData->gameState.bossRushMode && gEwramData->playerStatus.currentCharacter)
            entity->spawnParam0=event;
    }
}
