/* Graham projectile: sprite, collision, direction and optional homing. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"
extern void GrahamSceneProjectile_Update(struct EwramData_EntityData *);
extern void GrahamSceneProjectile_OnContact(void);
extern s32 Entity_AngleToPoint(struct EwramData_EntityData *,s32,s32);
extern u8 sCommonObjectGfx[],sCommonObjectSpriteSheet[];
/**
 * @brief Create an animated scene projectile at pixel coordinates with
 * randomized initial angle, optional homing/contact callback and
 * mode-dependent lifetime. Return the entity or NULL.
 */
struct EwramData_EntityData *Graham_CreateSceneProjectile(s32 xInput,s32 yInput,s32 homing)
{
    u16 x=xInput;
    u16 y=yInput;
    struct EwramData_EntityData *effect=EntityCreateInRange(0x49,0xCF,(u32 *)GrahamSceneProjectile_Update);
    if(effect)
    {
        s32 zero;
        s32 graphics;
        s32 targetX,targetY;
        u8 *kind;
        u8 *layer=&effect->spriteLayer;
        zero=0;
        *layer=zero+3;
        graphics=ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(effect,sCommonObjectSpriteSheet,graphics,5);
        Anim_StartIndexed(effect,6,zero,1);
        Anim_Update(effect);
        effect->stage=1;
        effect->drawFunc=(u32 *)Sprite_DrawEntity;
        if(homing)
        {
            Hit_Enable(effect,11,GrahamSceneProjectile_OnContact,0);
            effect->hit.flags|=4;
            HitBox_SetAttackRaw(effect,0x0C0CFAFA);
        }
        else
        {
            effect->hit.flags|=2;
            effect->hit.flags|=4;
        }
        {
        s32 turn;
        u16 *positionX=&effect->posX.part16.integer;
        turn=0;
        *positionX=x;
        effect->posY.part16.integer=y;
        effect->scrollWithBg=1;
        effect->group=64;
        kind=&effect->unk_508.part8.unk_508;
        *kind=homing;
        targetX=gEwramData->playerState.player->posX.whole;
        targetY=gEwramData->playerState.player->posY.whole;
        targetY-=((RandomNumberGenerator()&0x7F)<<12);
        targetY-=0x80000;
        effect->unk_4FC.whole=Entity_AngleToPoint(effect,targetX,targetY);
        effect->unk_4FC.whole=(effect->unk_4FC.whole+(RandomNumberGenerator()&0xFFFF))&0xFFFF;
        effect->unk_500.whole=turn;
        effect->unk_504.whole=0x30000;
        if(*kind)
            effect->timer=200;
        else
            effect->timer=16;
        }
    }
    return effect;
}
