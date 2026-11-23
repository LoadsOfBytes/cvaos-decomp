/*
 * Mandragora: body, linked and terminal parts, death parts.
 */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "code_080D73B8.h"
#include "code/code_08060B98.h"

/* Mandragora graphics, room setup and linked part. */
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

/* Mandragora linked part mirrors combat properties and shares graphics. */
extern struct EwramData_EntityData *Entity_CreateObject(void *);
extern void MandragoraPart_Update(struct EwramData_EntityData *);
extern void MandragoraPart_OnHit(struct EwramData_EntityData *);
extern s32 Mandragora_CreateTerminalPart(struct EwramData_EntityData *);
extern u8 sMandragoraSpriteSheet[];
/**
 * @brief Create the intermediate part with copied combat properties and
 * shared graphics, link both slot indices and create its terminal part.
 * Return 1 on success; delete the intermediate part and return 0 on failure.
 */
s32 Mandragora_CreateLinkedPart(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *part=Entity_CreateObject(MandragoraPart_Update);
    if(part)
    {
        void *sprite;
        s32 graphics;
        ENTITY_PARENT_INDEX(part)=ENTITY_INDEX(entity);
        entity->ext.mandragora.linkIndex=ENTITY_INDEX(part);
        part->spriteLayer=entity->spriteLayer-1;
        part->basePalette=entity->basePalette;
        part->flipX=entity->flipX;
        part->posX.whole=entity->posX.whole;
        part->posY.whole=entity->posY.whole;
        part->scrollWithBg=1;
        part->spawnIndex=entity->spawnIndex;
        part->enemyId=entity->enemyId;
        part->hp=entity->hp;
        part->mp=entity->mp;
        part->attack=entity->attack;
        part->defense=entity->defense;
        part->drawFunc=(u32 *)Sprite_DrawEntity;
        sprite=sMandragoraSpriteSheet;
        graphics=Sprite_GetTileSlot(entity);
        Sprite_Attach(part,sprite,graphics,part->basePalette);
        Anim_StartIndexed(part,3,0,1);
        Anim_Update(part);
        Entity_EnableBoxes(part,Enemy_OnContact,(s32)MandragoraPart_OnHit);
        if(Mandragora_CreateTerminalPart(part))
            return 1;
        EntityDelete(part);
    }
    return 0;
}

/* Mandragora intermediate part coordinates its owner and terminal part. */
extern void Enemy_DropRewards(struct EwramData_EntityData *);
extern void Effect_SpawnRandom(s32,s32,s32,s32,s32);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *);
#define FLAGS(e) (e->ext.mandragora.flags)
/**
 * @brief Coordinate the root owner and terminal part through activation,
 * scream timing and recovery. A defeated intermediate part drops owner
 * rewards and marks owner flags; exact visual identity remains unresolved.
 */
void MandragoraPart_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner=&gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    struct EwramData_EntityData *tip=&gEwramData->entityData[entity->ext.mandragora.linkIndex];
    switch(entity->state)
    {
        case 0:
            if(entity->subState==0)
            {
                entity->posX.whole=owner->posX.whole;
                entity->posY.whole=owner->posY.whole;
                entity->subState++;
                entity->stage=0;
                entity->hidden=1;
            }
            break;
        case 1:
            if(entity->subState==0)
            {
                FLAGS(owner)|=2;
                entity->hp=owner->hp;
                if(entity->animScriptIndex!=3)
                    Anim_StartIndexed(entity,3,0,1);
                entity->state++;
                entity->stage=0;
                entity->subState=0;
                entity->hidden=0;
                PlaySong(0x83);
                goto activate;
            }
            goto countdown;
        case 2:
activate:
            if(entity->subState==0)
            {
                tip->state=1;
                tip->stage=0;
                tip->subState=0;
                entity->subState++;
                entity->stage=0;
                entity->timer=32;
            }
countdown:
            if(--entity->timer==0xFF)
            {
                entity->state=0xF1;
                entity->stage=0;
                entity->subState=0;
            }
            break;
        case 0xF0:
            PlaySong(0x70);
            Enemy_DropRewards(owner);
            entity->state=0xF1;
            entity->stage=0;
            entity->subState=0;
            FLAGS(owner)|=0x20;
        case 0xF1:
            if(entity->subState==0)
            {
                if(FLAGS(owner)&0x20)
                {
                    tip->state=0;
                    tip->stage=0;
                    tip->subState=0;
                }
                FLAGS(owner)|=8;
                entity->drawSkip=1;
                entity->subState++;
                entity->stage=0;
            }
            if((u8)((u32)entity->timer++%7)==0 && (FLAGS(owner)&0x20))
            {
                s32 x=-0x80000;
                if(entity->flipX)
                    x=-x;
                Effect_SpawnRandom(entity->posX.whole+x,entity->posY.whole-0x140000,0x100000,0x100000,entity->spriteLayer+1);
            }
            if(entity->animDone)
            {
                entity->state=0;
                entity->stage=0;
                entity->subState=0;
            }
            break;
    }
    if(entity->state!=0)
    {
        Anim_Update(entity);
        if(entity->state<=0xEF)
            Enemy_CheckHitPlayer(entity);
    }
}

/* Mandragora linked part mirrors combat properties and shares graphics. */
extern void MandragoraTerminalPart_Update(struct EwramData_EntityData *);
extern void Hit_Enable(struct EwramData_EntityData *,s32,void *,s32);
extern void HitBox_SetAttackRaw(struct EwramData_EntityData *,s32);
extern s32 sMandragoraAttackBox[];
/**
 * @brief Create the terminal part with shared graphics and copied combat
 * properties, store both slot links and configure its attack box.
 * Return 1 on success or 0 on entity allocation failure.
 */
s32 Mandragora_CreateTerminalPart(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *part=Entity_CreateObject(MandragoraTerminalPart_Update);
    if(!part)
        return 0;
    {
        void *sprite;
        s32 graphics;
        ENTITY_PARENT_INDEX(part)=ENTITY_INDEX(entity);
        entity->ext.mandragora.linkIndex=ENTITY_INDEX(part);
        part->spriteLayer=entity->spriteLayer+1;
        part->basePalette=entity->basePalette;
        part->flipX=entity->flipX;
        part->posX.whole=entity->posX.whole;
        part->posY.whole=entity->posY.whole;
        part->hp=entity->hp;
        part->mp=entity->mp;
        part->attack=entity->attack;
        part->defense=entity->defense;
        part->scrollWithBg=1;
        part->drawFunc=(u32 *)Sprite_DrawEntity;
        part->roomActive=1;
        sprite=sMandragoraSpriteSheet;
        graphics=Sprite_GetTileSlot(entity);
        Sprite_Attach(part,sprite,graphics,part->basePalette);
        Anim_StartIndexed(part,4,0,1);
        Anim_Update(part);
        Hit_Enable(part,8,Enemy_OnContact,0);
        HitBox_SetAttackRaw(part,sMandragoraAttackBox[0]);
        part->objMode=1;
        return 1;
    }
}

/* MandragoraTerminalPart_Update. */
/* Mandragora terminal part controls the scream's palette flashes. */
extern s32 sMandragoraTerminalPartAttackBox[];
static inline s32 Tick(struct EwramData_EntityData *e) { s32 timer=e->timer; if(timer) e->timer--; return timer; }
/**
 * @brief Sequence the scream animation, BG/OBJ palette flashes, expanded
 * attack box and root owner flag 0x10. Reset when the intermediate
 * part finishes; preserve existing fade and parent-validity handling.
 */
void MandragoraTerminalPart_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *part=&gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    struct EwramData_EntityData *owner=&gEwramData->entityData[ENTITY_PARENT_INDEX(part)];
    if(!entity->roomActive)
    {
        BgPalette_StopColorFade();
        ObjPalette_StopColorFade();
        return;
    }
    if(part->updateFunc==NULL || owner->updateFunc==NULL)
        return;
    switch(entity->state)
    {
        case 0:
        {
            s32 state=entity->subState;
            if(state==0)
            {
                BgPalette_StopColorFade();
                ObjPalette_StopColorFade();
                entity->posX.whole=part->posX.whole;
                entity->posY.whole=part->posY.whole;
                entity->subState++;
                entity->stage=state;
                entity->hidden=1;
                FLAGS(owner)&=0xEF;
            }
            break;
        }
        case 1:
        {
            s32 state=entity->subState;
            if(state==0)
            {
                entity->hidden=0;
                if(entity->animScriptIndex!=4)
                    Anim_StartIndexed(entity,4,0,0);
                entity->subState++;
                entity->stage=state;
            }
            if(entity->animDone)
            {
                BgPalette_StartColorFade(31,31,31,0x10000,0,4);
                ObjPalette_StartColorFade(31,31,31,0x10000,0,4);
                entity->timer=4;
                entity->state++;
                entity->stage=0;
                entity->subState=0;
            }
            break;
        }
        case 2:
        {
            s32 timer; s16 t = entity->timer; timer=t; if(timer) entity->timer=t-1; else
            {
                BgPalette_StartColorFade(31,31,31,0,0x10000,32);
                ObjPalette_StartColorFade(31,31,31,0,0x10000,32);
                entity->state++;
                entity->stage=timer;
                entity->subState=timer;
            }
            break;
        }
        case 3:
        {
            s32 state=entity->subState;
            if(state==0)
            {
                if(entity->animScriptIndex!=5)
                    Anim_StartIndexed(entity,5,0,1);
                HitBox_SetAttackRaw(entity,sMandragoraTerminalPartAttackBox[0]);
                entity->subState++;
                entity->stage=state;
                FLAGS(owner)|=0x10;
            }
            if(part->state>0xEF)
            {
                entity->state=0xF0;
                entity->stage=0;
                entity->subState=0;
            }
            break;
        }
        case 0xF0:
            if(entity->subState==0)
            {
                if(part->state==0)
                {
                    entity->drawSkip=2;
                    entity->timer=16;
                    entity->subState++;
                    entity->stage=0;
                }
                else
                    entity->drawSkip=1;
            }
            else if(--entity->timer==0xFF)
            {
                entity->state=0;
                entity->stage=0;
                entity->subState=0;
            }
            break;
    }
    if(entity->state!=0)
    {
        Anim_Update(entity);
        if(entity->state!=0xF0)
            Enemy_CheckHitPlayer(entity);
    }
}

/* Mandragora proximity trigger, scream animation and linked-part timing. */
extern s32 Target_IsNearEntity(struct EwramData_EntityData *,s32,s32);
/**
 * @brief Waiting state (1): play animation 0, then select scream state 2
 * after the target enters the proximity box.
 */
void Mandragora_UpdateWait(struct EwramData_EntityData *entity)
{
    s32 state=entity->subState;
    switch(state)
    {
        case 0:
            if(entity->animScriptIndex!=0)
                Anim_StartIndexed(entity,0,0,1);
            entity->subState=1;
            entity->stage=state;
        case 1:
            if(Target_IsNearEntity(entity,256,128))
            {
                entity->subState=2;
                entity->stage=0;
            }
            break;
        case 2:
            entity->state=state;
            entity->stage=0;
            entity->subState=0;
            break;
    }
}
/**
 * @brief Scream state (2): sequence root animations and sound, activate
 * the intermediate part and wait for its timing/owner flags before
 * entering death state 0xF0.
 */
void Mandragora_UpdateScream(struct EwramData_EntityData *entity)
{
    struct EwramData *ram=gEwramData;
    struct EwramData_EntityData *part=&ram->entityData[entity->ext.mandragora.linkIndex];
    switch(entity->subState)
    {
        case 0:
            switch(entity->stage)
            {
                case 0:
                    entity->timer=80;
                    if(entity->animScriptIndex!=1)
                        Anim_StartIndexed(entity,1,0,0);
                    entity->stage++;
                case 1:
                    if(entity->timer==1)
                        PlaySong(0x119);
                    if(entity->timer!=0)
                        entity->timer--;
                    if(entity->animDone)
                    {
                        entity->timer=0;
                        entity->subState=1;
                        entity->stage=0;
                    }
                    break;
            }
            break;
        case 1:
        {
            s32 state=entity->stage;
            switch(state)
            {
                case 0:
                    if(entity->animScriptIndex!=2)
                        Anim_StartIndexed(entity,2,0,0);
                    part->state++;
                    part->stage=state;
                    part->subState=state;
                    entity->stage++;
                case 1:
                    if(entity->animDone)
                    {
                        entity->animFlags|=2;
                        entity->animFrame=10;
                        entity->stage++;
                    }
                    break;
                case 2:
                    if(part->state==2 && part->timer<=7)
                        entity->animFrame=10;
                    if(entity->ext.mandragora.flags & 0x18)
                    {
                        entity->state=0xF0;
                        entity->stage=0;
                        entity->subState=0;
                    }
                    break;
            }
            break;
        }
    }
}

/* Mandragora death waits for its linked part and pending effects. */
extern void Mandragora_SpawnDeathParts(struct EwramData_EntityData *);
extern void Mandragora_RequestLinkedDeletion(struct EwramData_EntityData *);
/**
 * @brief Death state (0xF0): hide, spawn five parts and wait for their live
 * count plus linked-part/palette state before requesting removal
 * of all three linked entities.
 */
void Mandragora_UpdateDeath(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *part=&gEwramData->entityData[entity->ext.mandragora.linkIndex];
    switch(entity->subState)
    {
        case 0:
            entity->hidden=1;
            entity->palette=entity->basePalette;
            Mandragora_SpawnDeathParts(entity);
            entity->subState++;
            if(FLAGS(entity)&2)
                goto wait_part;
            PlaySong(0x6B);
        case 1:
            if(!(FLAGS(entity)&2))
            {
                if(entity->ext.mandragoraDeath.parts==0)
                    Mandragora_RequestLinkedDeletion(entity);
            }
            else
            {
wait_part:
                if(part->hidden && !(FLAGS(entity)&0x10) && entity->ext.mandragoraDeath.parts==0)
                    Mandragora_RequestLinkedDeletion(entity);
            }
            break;
    }
}

/* Mandragora 0x080CF804..0x080CFAAF. Mandragora_SpawnDeathParts / MandragoraDeathPart_Update / Mandragora_UpdateBestiaryPreview are machine-identical
 * copies of the Needles / Blue Crow functions (cloning script). */

extern s32 Enemy_PreUpdate(struct EwramData_EntityData *entity);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *entity);
extern s32 Enemy_TakeDamage(struct EwramData_EntityData *entity);

/* Same layout as sNeedlesDeathParts in code_080CB6BC.c. */
enum {
    NEEDLE_OFFSET_X,
    NEEDLE_OFFSET_Y,
    NEEDLE_UNK_2,
    NEEDLE_VEL_X,
    NEEDLE_VEL_Y,
    NEEDLE_TIMER
};

void MandragoraDeathPart_Update(struct EwramData_EntityData *entity);
extern const s8 sMandragoraDeathParts[5][6];

/**
 * @brief Spawn up to five table-configured sprite parts with mirrored
 * velocity and offsets; count their lifetimes in the root owner field.
 */
void Mandragora_SpawnDeathParts(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child;
    const s8 *needle;
    s32 offsetX;
    s32 i;

    for (i = 0; i < 5; i++)
    {
        child = EntityCreateInRange(0x49, 0xCF, (u32 *)MandragoraDeathPart_Update);
        if (child == NULL)
            continue;
        needle = sMandragoraDeathParts[i];
        entity->ext.mandragoraDeath.parts++;
        ENTITY_PARENT_INDEX(child) = ENTITY_INDEX(entity);
        child->spriteLayer = entity->spriteLayer;
        child->basePalette = entity->basePalette;
        child->flipX = entity->flipX;
        Sprite_Attach(child, sMandragoraSpriteSheet, Sprite_GetTileSlot(entity), child->basePalette);
        child->scrollWithBg = 1;
        child->drawFunc = (u32 *)Sprite_DrawEntity;
        child->posX.whole = entity->posX.whole;
        child->posY.whole = entity->posY.whole;
        offsetX = needle[NEEDLE_OFFSET_X];
        child->posY.part16.integer += needle[NEEDLE_OFFSET_Y];
        child->animFrame = needle[NEEDLE_UNK_2];
        child->velX.whole = needle[NEEDLE_VEL_X] << 12;
        child->velY.whole = -(needle[NEEDLE_VEL_Y] << 12);
        child->timer = needle[NEEDLE_TIMER];
        if (child->flipX)
        {
            child->velX.whole = -child->velX.whole;
            offsetX = -offsetX;
        }
        child->posX.part16.integer += offsetX;
    }
}

/**
 * @brief Move under gravity; decrement the root live-part count and
 * request deletion on bounds, bottom-Y or timer-underflow completion.
 */
void MandragoraDeathPart_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];

    if (!Entity_IsInRect(entity, -0x80, -0x40, 0x1F0, 0x120) || (u8)--entity->timer == 0xFF
        || entity->posY.part16.integer > 0xAA)
    {
        parent->ext.mandragoraDeath.parts--;
        entity->deleteRequested = 1;
    }
    else
    {
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
        entity->velY.whole += 0x3000;
    }
}

void Mandragora_UpdateBestiaryPreview(struct EwramData_EntityData *entity);

/**
 * @brief CF99C | Mandragora update: state 3 -> Mandragora_UpdateBestiaryPreview, state 0xF0 ->
 * Mandragora_UpdateDeath; otherwise (unless Enemy_PreUpdate intervenes while animation
 * is not 2) run state 1 / 2 and animate.
 */
void EnemyMandragoraUpdate(struct EwramData_EntityData *entity)
{
    if (entity->state == 3)
    {
        Mandragora_UpdateBestiaryPreview(entity);
    }
    else if (entity->state == 0xF0)
    {
        Mandragora_UpdateDeath(entity);
    }
    else if (entity->animScriptIndex == 2 || !Enemy_PreUpdate(entity))
    {
        switch (entity->state)
        {
            case 1:
                Mandragora_UpdateWait(entity);
                break;
            case 2:
                Mandragora_UpdateScream(entity);
                break;
        }
        Anim_Update(entity);
        Enemy_CheckHitPlayer(entity);
    }
}

/**
 * @brief Apply enemy damage to the intermediate part; on defeat enter
 * its state 0xF0 with substate counters reset.
 */
void MandragoraPart_OnHit(struct EwramData_EntityData *entity)
{
    if (Enemy_TakeDamage(entity))
    {
        entity->state = 0xF0;
        entity->stage = 0;
        entity->subState = 0;
    }
}

/**
 * @brief Bestiary state (3): select animation 0 once, then update it.
 */
void Mandragora_UpdateBestiaryPreview(struct EwramData_EntityData *entity)
{
    if (entity->subState == 0)
    {
        if (entity->animScriptIndex != 0)
            Anim_StartIndexed(entity, 0, 0, 1);
        entity->subState++;
    }
    Anim_Update(entity);
}

/**
 * @brief Apply enemy damage to the root owner; on defeat enter death
 * state 0xF0 with substate counters reset.
 */
void Mandragora_OnHit(struct EwramData_EntityData *entity)
{
    if (Enemy_TakeDamage(entity))
    {
        entity->state = 0xF0;
        entity->stage = 0;
        entity->subState = 0;
    }
}

/**
 * @brief Request deletion of the root owner, intermediate part and
 * terminal part through their stored slot links.
 */
void Mandragora_RequestLinkedDeletion(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *linked = &gEwramData->entityData[entity->ext.mandragora.linkIndex];
    struct EwramData_EntityData *linked2 = &gEwramData->entityData[linked->ext.mandragora.linkIndex];

    linked2->deleteRequested = 1;
    linked->deleteRequested = 1;
    entity->deleteRequested = 1;
}
