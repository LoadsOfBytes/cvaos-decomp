/* Select attack damage and knockback, then use the current frame's hitbox. */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"
extern void AGBAssert(const char *,int,const char *,int);
extern const char sAssertFileLtInline[],sAssertExprOpdId[];
static inline struct SpriteSheet *GetSprite(s16 slot)
{
    if(!(slot>=0 && slot<240)) AGBAssert(sAssertFileLtInline,94,sAssertExprOpdId,1);
    return gEwramData->spriteSlots[slot].sheet;
}
void Werejaguar_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct EntityHit *attack=&entity->hit;
    struct PlayerContactDamage hit;
    u8 *power;
    s32 kind;
    hit.attack=entity->attack;
    power=&entity->attack;
    hit.attributes=0; hit.unused=0; hit.reaction=0;
    hit.sourceX=entity->posX.whole; hit.sourceY=entity->posY.whole;
    attack->type=8;
    kind=(s8)entity->ext.werejaguar.contactKind;
    switch(kind)
    {
        case 0: hit.reaction=kind; Player_ApplyContactDamage(&hit); break;
        case 1:
            attack->type=9;
            hit.reaction=0; hit.attack=*power*3/4;
            Player_ApplyContactDamage(&hit);
            break;
        case 2:
        {
            s32 push;
            hit.reaction=3; hit.attack=*power;
            Player_ApplyContactDamage(&hit);
            push=-0x30000;
            if(gEwramData->playerState.player->posX.whole>entity->posX.whole) push=-push;
            Player_SetKnockbackVelocity(push,-0x30000);
            break;
        }
        case 3:
        {
            s32 push;
            hit.attack=*power+*power/4; hit.reaction=kind;
            Player_ApplyContactDamage(&hit);
            push=-0x30000;
            if(gEwramData->playerState.player->posX.whole>entity->posX.whole) push=-push;
            Player_SetKnockbackVelocity(push,-0x60000);
            break;
        }
    }
    entity->ext.werejaguar.contactHit=1;
    HitBox_SetAttack(entity,GetSprite(entity->spriteSlot)->frames[entity->animFrame].boxes);
}
