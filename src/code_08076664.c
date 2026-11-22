/* Allocate a 100-halfword motion history and link ten body segments. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"
extern u8 WhiteDragon_LoadGraphics(struct EwramData_EntityData *);
extern void *Heap_Alloc(u16);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern struct EwramData_EntityData *Entity_CreateObject(u32 *);
extern void WhiteDragonSegment_Update(struct EwramData_EntityData *);
extern void WhiteDragonSegment_Draw(struct EwramData_EntityData *);
extern void WhiteDragon_DamagePlayerOnContact(void);
extern void WhiteDragon_OnHit(void);
extern u8 sQuezlcoatlWhiteDragonGfx[],sQuezlcoatlWhiteDragonSpriteSheet[];
static inline void StoreRate(s32 value,u16 *destination) { *destination=value; }
void EnemyWhiteDragonCreate(struct EwramData_EntityData *entity)
{
    struct BgLayerState *camera=&gEwramData->bgInfo[1];
    u32 box=0x0C10F8F8;
    struct EwramData_EntityData *previous;
    s32 index;
    u16 *angularSpeed;
    u8 *palette;
    u8 demo;
    if(!WhiteDragon_LoadGraphics(entity)) goto fail;
    entity->animFrame=0;
    entity->unk_504.whole=(s32)Heap_Alloc(50);
    if(!entity->unk_504.whole) goto fail;
    Enemy_ResetFromTemplate(entity);
    entity->scrollWithBg=0;
    entity->unk_4F8.part16.unk_4F8=entity->posX.part16.integer;
    entity->unk_4F8.part16.unk_4FA=entity->posY.part16.integer;
    entity->unk_4FC.part16.unk_4FC=camera->xPos.part16.integer;
    entity->unk_4FC.part16.unk_4FE[0]=camera->yPos.part16.integer;
    if(entity->unk_514.part16.spawnParam0)
        entity->flipX=1;
    else
        entity->flipX=0;
    if(entity->flipX)
        entity->velX.part16.integer=0;
    else
        entity->velX.part16.integer=(s16)0x8000;
    entity->velX.part16.fraction=0;
    entity->velY.part16.integer=0;
    entity->velY.part16.fraction=0;
    entity->unk_500.part16.unk_500=0;
    entity->unk_500.part16.unk_502=0;
    entity->unk_4F4.part16.unk_4F4=0;
    entity->unk_4F4.part16.unk_4F6=0;
    previous=entity;
    index=9;
    angularSpeed=(u16 *)&entity->velY.part16.fraction;
    palette=&entity->unk_510.part8.basePalette;
    do
    {
        struct EwramData_EntityData *part=Entity_CreateObject((u32 *)WhiteDragonSegment_Update);
        s32 graphics;
        if(!part) goto fail;
        part->spriteLayer=entity->spriteLayer+1;
        graphics=ObjGfx_Find(sQuezlcoatlWhiteDragonGfx);
        Sprite_Attach(part,sQuezlcoatlWhiteDragonSpriteSheet,graphics,*palette);
        part->unk_518.part8.enemyId=entity->unk_518.part8.enemyId;
        part->unk_510.part8.basePalette=*palette;
        part->scrollWithBg=0;
        part->drawFunc=(u32 *)WhiteDragonSegment_Draw;
        part->unk_4F8.unk_4F8_entity=entity;
        part->unk_4FC.unk_4FC_entity=previous;
        part->unk_500.part8.unk_500=index;
        Anim_StartIndexed(part,1,0,1);
        Anim_Update(part);
        previous=part;
        index--;
    } while(index>=0);
    demo=*(u8 *)((u8 *)gEwramData+0x8E) & 0x40;
    if(demo)
    {
        entity->state=4;
        StoreRate(0x400,angularSpeed);
        return;
    }
    goto normal;
fail:
    EntityDelete(entity);
    return;
normal:
    Hit_Enable(entity,8,WhiteDragon_DamagePlayerOnContact,(s32)WhiteDragon_OnHit);
    HitBox_SetBody(entity,&box);
    HitBox_SetAttack(entity,&box);
    entity->state=demo;
}
