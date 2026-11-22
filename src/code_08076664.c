/* Allocate a 100-halfword motion history and link ten body segments. */
#include "code_08009A0.h"
#include "white_dragon_entity.h"
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
    WHITE_DRAGON_HISTORY(entity)=(s32)Heap_Alloc(50);
    if(!WHITE_DRAGON_HISTORY(entity)) goto fail;
    Enemy_ResetFromTemplate(entity);
    entity->scrollWithBg=0;
    WHITE_DRAGON_OFFSET_X(entity)=entity->posX.part16.integer;
    WHITE_DRAGON_OFFSET_Y(entity)=entity->posY.part16.integer;
    WHITE_DRAGON_ANCHOR_X(entity)=camera->xPos.part16.integer;
    WHITE_DRAGON_ANCHOR_Y(entity)=camera->yPos.part16.integer;
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
    WHITE_DRAGON_SWAY(entity)=0;
    WHITE_DRAGON_SWAY_PHASE(entity)=0;
    WHITE_DRAGON_MOVE_TIMER(entity)=0;
    WHITE_DRAGON_SHOT_TIMER(entity)=0;
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
        ENTITY_OWNER(part)=entity;
        WHITE_DRAGON_SEGMENT_PREV(part)=previous;
        WHITE_DRAGON_SEGMENT_INDEX(part)=index;
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
