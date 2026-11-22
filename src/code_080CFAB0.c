/* Belmont initialization distinguishes scene entry and combat entry. */
#include "code_08009A0.h"
#include "belmont_entity.h"
#include "code_0800B700.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void Player_SetSelectedTarget(struct EwramData_EntityData *);
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *,u8 *,void *,u8,void *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void Entity_SnapToGround(struct EwramData_EntityData *);
extern void Entity_EnableBoxes(struct EwramData_EntityData *,void *,s32);
extern void Enemy_OnContact(struct EwramData_EntityData *);
extern void Belmont_OnHit(struct EwramData_EntityData *);
extern u8 sJuliusEntityPalette[],sJuliusSpriteSheet[],sBelmontGfx[];
void EnemyBelmontCreate(struct EwramData_EntityData *entity)
{
    void *palette=sJuliusEntityPalette;
    void *sprite=sJuliusSpriteSheet;
    u8 *graphics=sBelmontGfx;
    s32 kind=entity->unk_514.part16.spawnParam0;
    u8 display;
    s32 zero;
    u8 *layer;
    if(kind==1)
    {
        if(!gEwramData->gameState.bestiaryPreview)
        {
            u16 completed;
            BELMONT_FLAGS(entity)|=0x10;
            completed=gEwramData->gameState.bossEventFlags & 4;
            if(completed)
            {
                EntityDelete(entity);
                return;
            }
            Game_SetMenusEnabled(0);
            entity->state=kind;
            entity->stage=completed;
            entity->subState=completed;
            return;
        }
    }
    else if(!gEwramData->gameState.bestiaryPreview)
    {
        Game_SetMenusEnabled(1);
        Player_SetSelectedTarget(entity);
        gEwramData->gameState.stateFlags|=2;
    }
    if(!Entity_LoadGraphics(entity,graphics,palette,0,sprite))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity,0,1,1);
    Anim_Update(entity);
    layer=&entity->spriteLayer;
    zero=0;
    *layer=17;
    entity->enemyFlags|=4;
    entity->flipX=zero;
    display=((u8 *)&gEwramData->gameState)[0x2E]&0x40;
    if(display)
    {
        entity->state=12;
        entity->stage=zero;
        entity->subState=zero;
        return;
    }
    Enemy_ResetFromTemplate(entity);
    entity->state=3;
    entity->stage=display;
    entity->subState=display;
    BELMONT_ATTACK(entity)=entity->attack;
    BELMONT_ATTACK_OFF(entity)=display;
    entity->attack=display;
    Entity_SnapToGround(entity);
    Entity_EnableBoxes(entity,Enemy_OnContact,(s32)Belmont_OnHit);
}

