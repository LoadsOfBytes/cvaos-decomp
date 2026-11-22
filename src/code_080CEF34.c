/* Mandragora graphics, room setup and linked part. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *,u8 *,void *,u8,void *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void Entity_EnableBoxes(struct EwramData_EntityData *,void *,s32);
extern void Enemy_OnContact(struct EwramData_EntityData *);
extern void Mandragora_OnHit(struct EwramData_EntityData *);
extern void Entity_SnapToGround(struct EwramData_EntityData *);
extern s32 Mandragora_CreateLinkedPart(struct EwramData_EntityData *);
extern u8 sMandragoraPalette[],sMandragoraSpriteSheet[],sMandragoraGfx[];
void EnemyMandragoraCreate(struct EwramData_EntityData *entity)
{
    void *palette=sMandragoraPalette;
    void *sprite=sMandragoraSpriteSheet;
    u8 *graphics=sMandragoraGfx;
    u8 display;
    if(!Entity_LoadGraphics(entity,graphics,palette,0,sprite))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity,0,0,1);
    Anim_Update(entity);
    display=((u8 *)&gEwramData->gameState)[0x2E]&0x40;
    if(display)
    {
        entity->state=3;
        entity->stage=0;
        entity->subState=0;
        return;
    }
    entity->spriteLayer=17;
    Enemy_ResetFromTemplate(entity);
    entity->state=1;
    entity->stage=display;
    entity->subState=display;
    Entity_EnableBoxes(entity,Enemy_OnContact,(s32)Mandragora_OnHit);
    Entity_SnapToGround(entity);
    if(!Mandragora_CreateLinkedPart(entity))
    {
        EntityDelete(entity);
        return;
    }
    entity->enemyFlags|=1;
}
