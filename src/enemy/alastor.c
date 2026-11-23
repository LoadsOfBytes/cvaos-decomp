/*
 * Alastor: body, attacks, afterimages and linked parts.
 */
#include "core.h"
#include "graphics.h"
#include "structs/ewram.h"
#include "player_core.h"
#include "sound.h"
#include "gameplay.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "hitbox.h"
#include "math_anim.h"
#include "oam.h"
#include "obj_gfx.h"
#include "obj_palette.h"
#include "data/data_118B48.h"
#include "data/data_4F1444.h"

/* Alastor graphics, affine transform and callbacks. */
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *,u8 *,void *,u8,void *);
extern s32 Alastor_CreateLinkedPart(struct EwramData_EntityData *);
void Alastor_OnHit(struct EwramData_EntityData *entity,s32 kind,u16 *hit);
extern void Alastor_UpdateHitBoxes(struct EwramData_EntityData *);
extern void Alastor_UpdateAppear(struct EwramData_EntityData *);
static inline void SetAffine(struct EwramData_EntityData *e,s32 index) { e->affineIndex=index; }
void EnemyAlastorCreate(struct EwramData_EntityData *entity)
{
    void *palette=sAlastorPalette;
    void *sprite=sAlastorSpriteSheet;
    u8 *graphics=sAlastorGfx;
    u8 display;
    u16 affine;
    s32 layer;
    s32 zero;
    u8 *layerPtr;
    if (!Entity_LoadGraphics(entity,graphics,palette,0,sprite))
    {
        EntityDelete(entity);
        return;
    }
    Enemy_ResetFromTemplate(entity);
    layerPtr=&entity->spriteLayer;
    zero=0;
    layer=3;
    *layerPtr=layer;
    entity->animFrame=zero;
    display=((u8 *)&gEwramData->gameState)[0x2E] & 0x40;
    if (display)
    {
        entity->state=8;
        entity->stage=zero;
        entity->subState=zero;
        return;
    }
    affine=ObjAffine_Alloc(1);
    if ((s16)affine<0)
    {
        EntityDelete(entity);
        return;
    }
    SetAffine(entity,affine);
    entity->affine=1;
    ObjAffine_Set(0x10000,0x10000,0x10000,0,display,entity->affineIndex);
    if (!Alastor_CreateLinkedPart(entity))
        entity->deleteRequested=1;
    Hit_Enable(entity,8,Enemy_OnContactMask1,(s32)Alastor_OnHit);
    HitBox_SetBodyRaw(entity,0x1010F8F8);
    Alastor_UpdateHitBoxes(entity);
    entity->ext.alastor.attackSet=display;
    entity->ext.alastor.attackIndex=9;
    entity->ext.alastor.hitsBeforeReact=layer;
    Alastor_UpdateAppear(entity);
}

/* Alastor behavior dispatch and collision/animation refresh. */
extern void Alastor_UpdateBestiaryPreview(struct EwramData_EntityData *);
extern void Alastor_UpdateDeath(struct EwramData_EntityData *);
extern u8 Entity_IsTargetWithinRange(struct EwramData_EntityData *,s32);
extern void Hit_CheckWithPlayer(void *,struct EwramData_EntityData *);
extern void Alastor_UpdateApproach(struct EwramData_EntityData *);
extern void Alastor_UpdateAttackSelection(struct EwramData_EntityData *);
extern void Alastor_UpdateRushAttack(struct EwramData_EntityData *);
extern void Alastor_UpdatePatternAttack(struct EwramData_EntityData *);
extern void Alastor_UpdateAimedCharge(struct EwramData_EntityData *);
extern void Alastor_UpdateSpinningLeap(struct EwramData_EntityData *);
extern void Alastor_UpdateRecoil(struct EwramData_EntityData *);
void EnemyAlastorUpdate(struct EwramData_EntityData *entity)
{
    u8 buffer[16];
    s32 previous=entity->ext.alastor.angle;
    s32 facing=entity->flipX;
    if (entity->state==8)
    {
        Alastor_UpdateBestiaryPreview(entity);
        return;
    }
    if (entity->state==0xFF)
    {
        Alastor_UpdateDeath(entity);
        return;
    }
    Player_TrackEntity(entity);
    if (Enemy_IsStunned(entity))
    {
        if (!Entity_IsTargetWithinRange(entity,0x500000) && entity->state==0)
            return;
        if (!(entity->ext.alastor.flags & 0x20) && entity->state!=0)
        {
            Hit_CheckWithPlayer(buffer,entity);
            return;
        }
    }
    switch (entity->state)
    {
        case 0: Alastor_UpdateAppear(entity); break;
        case 1: Alastor_UpdateApproach(entity); break;
        case 2: Alastor_UpdateAttackSelection(entity); break;
        case 3: Alastor_UpdateRushAttack(entity); break;
        case 4: Alastor_UpdatePatternAttack(entity); break;
        case 5: Alastor_UpdateAimedCharge(entity); break;
        case 6: Alastor_UpdateSpinningLeap(entity); break;
        case 7: Alastor_UpdateRecoil(entity); break;
    }
    if (entity->ext.alastor.angle!=previous || entity->flipX!=facing)
        Alastor_UpdateHitBoxes(entity);
    if (!(entity->ext.alastor.flags & 0x20))
        Hit_CheckWithPlayer(buffer,entity);
}

/* Alastor appearance and blink sequence. */
extern void Entity_FaceTarget(struct EwramData_EntityData *);
static inline u32 Blink(u32 timer) { return ((u32)-(u8)(timer%3))>>31; }
/**
 * @brief Appearance state (0): hide the owner and linked part, wait for
 * proximity, blink them into view and hand off to approach state 1.
 */
void Alastor_UpdateAppear(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *part=&gEwramData->entityData[entity->ext.alastorPart.index];
    switch(entity->subState)
    {
        case 0:
            Entity_FaceTarget(entity);
            part->hidden=1;
            part->state=5;
            entity->hidden=1;
            entity->timer=0;
            entity->subState++;
        case 1:
            if (Entity_IsTargetWithinRange(entity,0x500000))
            {
                part->posX.part16.integer=entity->posX.part16.integer;
                part->posY.part16.integer=entity->posY.part16.integer;
                part->posX.part16.integer=entity->posX.part16.integer+10;
                part->posY.part16.integer=entity->posY.part16.integer+16;
                ObjAffine_Set(0x10000,0x10000,0x10000,entity->flipX,0,entity->affineIndex);
                entity->subState++;
            }
            break;
        case 2:
            entity->hidden=Blink(entity->timer);
            part->hidden=Blink(entity->timer);
            entity->objMode=1;
            if(entity->timer++>40)
            {
                entity->hidden=0;
                part->hidden=0;
                entity->objMode=0;
                entity->timer=0;
                entity->subState++;
            }
            break;
        case 3:
            if(entity->timer++>20)
            {
                entity->timer=0;
                entity->subState++;
            }
            break;
        case 4:
            part->hidden=Blink(entity->timer);
            if(entity->timer++>40)
            {
                part->hidden=1;
                part->state=0;
                part->stage=0;
                part->subState=0;
                entity->timer=0;
                entity->subState++;
            }
            break;
        case 5:
            if(entity->timer++>20)
            {
                entity->timer=0;
                entity->state=1;
                entity->stage=0;
                entity->subState=0;
            }
            break;
    }
}

/* Alastor approaches a point near the player and rocks its blade.
 * for the matching notes (s8 reachedX, pinned r0
 * temporary, separate += in both branches, u32 phase). */
extern s32 Entity_GetTargetDistanceX(struct EwramData_EntityData *);
extern u8 Angle_TurnTowards(s16 *,u16,u16);
#define DX(e) ((s16)Entity_GetTargetDistanceX(e))
#define DY(e) ((s16)Entity_GetTargetDistanceY(e))
#define ABS(x) ((x)<0?-(x):(x))
#define ANGLE(e) (e->ext.alastor.angle)
#define PHASE(e) (e->ext.alastor.phase)
#define FLAGS(e) (e->ext.alastor.flags)
static inline s32 Both(s32 x,s32 y) { return (x&y)==1; }
/**
 * @brief Approach state (1): move beside and above the target while
 * rocking the angle; enter attack selection state 2 on reaching both axes.
 * Preserve signed narrowing, register bindings and separate phase updates.
 */
void Alastor_UpdateApproach(struct EwramData_EntityData *entity)
{
    s8 reachedX;
    s32 reachedY;
    reachedX=0;
    reachedY=0;
    if (DX(entity)>0)
        FLAGS(entity)|=2;
    else
        FLAGS(entity)&=~2;
    Entity_FaceTarget(entity);
    switch(entity->subState)
    {
        case 0:
            entity->objMode=0;
            entity->velX.whole=0x8000;
            entity->velY.whole=0x8000;
            if (DX(entity)>0)
                entity->velX.whole=-entity->velX.whole;
            if (DY(entity)>0)
                entity->velY.whole=-entity->velY.whole;
            PHASE(entity)=0;
            entity->ext.alastor.turn=-100;
            entity->subState=2;
            break;
        case 1:
        case 2:
            if (DX(entity)>0)
            {
                if (Angle_TurnTowards(&ANGLE(entity),0x180,0x1000))
                    entity->subState++;
            }
            else
            {
                if (Angle_TurnTowards(&ANGLE(entity),0x180,0xF000))
                    entity->subState++;
            }
            ObjAffine_Set(ANGLE(entity),0x10000,0x10000,entity->flipX,0,entity->affineIndex);
            break;
        case 3:
            entity->velY.whole=0x8000;
            entity->velX.whole=0x8000;
            if (DX(entity)>0)
                entity->velX.whole=-ABS(entity->velX.whole);
            if (DY(entity)+32>0)
                entity->velY.whole=-ABS(entity->velY.whole);
            if (ABS(DX(entity))>72)
                entity->posX.whole+=entity->velX.whole;
            else
                reachedX=1;
            if (ABS(DY(entity)+32)>8)
                entity->posY.whole+=entity->velY.whole;
            else
                reachedY=1;
            {
                s32 both;
                do { both=reachedY&reachedX; } while (0); /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
            if (both==1)
            {
                entity->state=2;
                entity->stage=0;
                entity->subState=0;
            }
            }
            if ((DX(entity)>=0 && entity->velX.whole>0) || (DX(entity)<=0 && entity->velX.whole<0))
            {
                entity->subState=0;
                break;
            }
            ANGLE(entity)=Sin((u16)PHASE(entity))/32;
            if (DX(entity)>0)
                ANGLE(entity)+=0x1000;
            else
                ANGLE(entity)+=-0x1000;
            {
                u32 phase=(u16)PHASE(entity);
                PHASE(entity)=phase+0x200;
            }
            ObjAffine_Set(ANGLE(entity),0x10000,0x10000,entity->flipX,0,entity->affineIndex);
            if ((s16)entity->ext.alastor.waitTimer!=0)
                entity->ext.alastor.waitTimer--;
            else
                entity->ext.alastor.waitTimer=RandomNumberGenerator()%180+10;
            break;
    }
}

/* Alastor readies its blade and selects the next table-driven attack. */
#define SET(e) (e->ext.alastor.attackSet)
#define INDEX(e) (e->ext.alastor.attackIndex)
#define ATTACK(e) (sUnk_08528C2C[SET(e)][INDEX(e)])
#define STEP(e) (e->ext.alastor.turn)
/**
 * @brief Selection state (2): coordinate linked-part poses and rotate
 * toward preparatory angles, then choose states 3 through 6 from the
 * selected attack table and advance its index.
 */
void Alastor_UpdateAttackSelection(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *part=&gEwramData->entityData[entity->ext.alastorPart.index];
    s32 speed=0x20000;
    s32 target=0xC000;
    switch(entity->subState)
    {
        case 0:
            Entity_FaceTarget(entity);
            if(INDEX(entity)==9)
            {
                SET(entity)=RandomNumberGenerator() % 4;
                INDEX(entity)=0;
            }
            if(ATTACK(entity)==2)
            {
                entity->state=5;
                entity->stage=0;
                entity->subState=0;
                INDEX(entity)++;
                break;
            }
            if(!(entity->ext.alastor.flags & 2))
            {
                speed=-speed;
                target=0x10000-target;
            }
            entity->posX.whole+=speed;
            if(Angle_TurnTowards(&ANGLE(entity),(u16)ABS(STEP(entity)<<3),(u16)target))
            {
                part->state=2;
                part->stage=0;
                part->subState=0;
                entity->subState++;
            }
            goto transform;
        case 1:
            if(part->state==0)
                entity->subState++;
            break;
        case 2:
            target=0xAC00;
            if(!(entity->ext.alastor.flags & 2))
                target=0x5400;
            if(Angle_TurnTowards(&ANGLE(entity),(u16)ABS(STEP(entity)<<4),(u16)target))
            {
                if(ATTACK(entity)==0)
                    entity->subState+=2;
                else
                    entity->subState++;
            }
            goto transform;
        case 3:
            target=0x4000;
            if(!(entity->ext.alastor.flags & 2))
                target=0xC000;
            if(Angle_TurnTowards(&ANGLE(entity),(u16)ABS(STEP(entity)<<5),(u16)target))
                entity->subState++;
transform:
            ObjAffine_Set(ANGLE(entity),0x10000,0x10000,entity->flipX,0,entity->affineIndex);
            break;
        case 4:
        {
            s32 state=part->state;
            if(state!=0)
                break;
            if(INDEX(entity)==9)
            {
                SET(entity)=RandomNumberGenerator() % 4;
                INDEX(entity)=state;
            }
            switch(ATTACK(entity))
            {
                case 0: entity->state=3; entity->stage=state; entity->subState=state; break;
                case 1: entity->state=4; entity->stage=state; entity->subState=state; break;
                case 2: entity->state=5; entity->stage=state; entity->subState=state; break;
                case 3: entity->state=6; entity->stage=state; entity->subState=state; break;
            }
            INDEX(entity)++;
            break;
        }
    }
}

/* Rotate Alastor's collision rectangle with its angle and facing. */
/**
 * @brief Compute the offset and orientation-dependent rectangle from
 * angle/facing and write it to both body and attack hit boxes.
 */
void Alastor_UpdateHitBoxes(struct EwramData_EntityData *entity)
{
    s32 angle=entity->ext.alastor.angle;
    s32 horizontal=(angle+0x4000)&0xFFFF;
    s32 vertical=(angle+0xC000)&0xFFFF;
    s32 side=vertical;
    s8 width=48;
    s8 height=8;
    s8 x;
    u8 y;
    u32 box;
    if (!entity->flipX)
        horizontal+=0x8000;
    if (vertical>0xA000)
        side=vertical-0x8000;
    if ((u32)(side-0x2001)<=0x3FFE)
    {
        width=8;
        height=48;
    }
    x=FixedMultiply(Sin(horizontal+0x4000),0x2A0000)>>16;
    y=FixedMultiply(Sin(vertical),0x2A0000)>>16;
    if ((s8)x>0)
        x-=width;
    box=((u8)x | (y<<8)) + ((u8)width<<16) + ((u8)height<<24);
    HitBox_SetAttackRaw(entity,box);
    HitBox_SetBodyRaw(entity,box);
}

/* Alastor horizontal rush, braking turn, rising slash and recovery. */
extern s32 Alastor_IsTargetInUpperBox(struct EwramData_EntityData *,u16,u16);
extern void Afterimage_Create(struct EwramData_EntityData *,s32,s32,s32);
static inline u16 SubtractMargin(s32 distance,s32 margin) { return distance-margin; }
static inline u16 Clamp(u16 d) { register u32 r_ asm("r1"); /* FAKEMATCH: result in r1 */ r_ = d; asm("" :: "r"(d)); /* FAKEMATCH: narrowed value in its own register */ if((s16)r_<0) r_ = 0; return r_; }
/**
 * @brief Attack state (3): rush horizontally, brake and rotate; if the
 * target is in the upper test box, reverse into a rising attack with
 * afterimages, then recover to approach.
 */
void Alastor_UpdateRushAttack(struct EwramData_EntityData *entity)
{
    switch(entity->subState)
    {
        case 0:
        {
            s32 speed;
            s32 acceleration;
            s32 margin;
            s16 distance;
            entity->velX.whole=(speed=0x40000);
            entity->accelX.whole=(acceleration=-0x4000);
            entity->velY.whole=0xA000;
            if(!entity->flipX)
            {
                entity->velX.whole=-speed;
                entity->accelX.whole=-acceleration;
            }
            entity->subState++;
            margin=RandomNumberGenerator()%64+16;
            distance=Clamp(SubtractMargin(ABS(DX(entity)),margin));
            entity->timer=ABS(distance/(s16)entity->velX.part16.integer);
            PlaySong(0x86);
            break;
        }
        case 1:
            entity->posX.whole+=entity->velX.whole;
            entity->posY.whole+=entity->velY.whole;
            ObjAffine_Set(ANGLE(entity),0x10000,0x10000,entity->flipX,0,entity->affineIndex);
            if(--entity->timer==0xFF)
                entity->subState++;
            break;
        case 2:
        {
            if(entity->flipX)
                ANGLE(entity)=(u16)ANGLE(entity)-0x150;
            else
                ANGLE(entity)=0x150+(u16)ANGLE(entity);
            ObjAffine_Set(ANGLE(entity),0x10000,0x10000,entity->flipX,0,entity->affineIndex);
            entity->posX.whole+=entity->velX.whole;
            entity->velX.whole+=entity->accelX.whole;
            if(entity->velX.whole==0)
                entity->subState++;
            break;
        }
        case 3:
            switch(entity->stage)
            {
                case 0:
                {
                    s32 hit=Alastor_IsTargetInUpperBox(entity,128,256);
                    if(hit)
                    {
                        entity->stage++;
                        entity->flipX=1-entity->flipX;
                        ObjAffine_Set(ANGLE(entity),0x10000,0x10000,entity->flipX,0,entity->affineIndex);
                        entity->velY.whole=-0x500;
                        entity->accelY.whole=-0x5000;
                        entity->timer=40;
                        Afterimage_Create(entity,96,10,2);
                        Afterimage_Create(entity,96,6,2);
                        Afterimage_Create(entity,96,3,2);
                    }
                    else
                    {
                        entity->subState=0xFF;
                        entity->stage=hit;
                    }
                }
                case 1:
                    if(entity->flipX)
                        ANGLE(entity)=(u16)ANGLE(entity)+0x80;
                    else
                        ANGLE(entity)=(u16)ANGLE(entity)-0x80;
                    ObjAffine_Set(ANGLE(entity),0x10000,0x10000,entity->flipX,0,entity->affineIndex);
                    entity->posY.whole+=entity->velY.whole;
                    if(entity->timer==15)
                        PlaySong(0x86);
                    if(entity->timer<=14)
                        entity->velY.whole+=entity->accelY.whole;
                    if(--entity->timer==0xFF)
                    {
                        entity->accelY.whole=0x4000;
                        entity->stage++;
                    }
                    break;
                case 2:
                    entity->posY.whole+=entity->velY.whole;
                    entity->velY.whole+=entity->accelY.whole;
                    if(entity->velY.whole>0)
                    {
                        entity->subState=0xFF;
                        entity->stage=0;
                    }
                    break;
            }
            break;
        case 0xFF:
            if(entity->stage==0)
            {
                entity->timer=40;
                entity->stage++;
            }
            else if(--entity->timer==0xFF)
            {
                entity->timer=0;
                entity->state=1;
                entity->stage=0;
                entity->subState=0;
            }
            break;
    }
}

/* Alastor movement/rotation attack driven by timed segments. */
#undef INDEX
#define INDEX(e) (e->ext.alastorPattern.step)
/**
 * @brief Attack state (4): execute table-defined position, rotation and
 * duration segments, stop at the sentinel and recover to approach.
 */
void Alastor_UpdatePatternAttack(struct EwramData_EntityData *entity)
{
    s16 (*segments)[4]=sAlastorAttackSegments;
    s16 *segment=segments[INDEX(entity)];
    switch(entity->subState)
    {
        case 0:
            INDEX(entity)=0;
            segment=segments[0];
            entity->subState++;
            PlaySong(0x86);
        case 1:
        {
            s32 timer;
            s32 x;
            s32 facing;
            timer=(entity->timer=segment[3]);
            x=segment[0];
            facing=(s16)(entity->ext.alastor.flags & 2);
            if(facing)
                x=-x;
            x<<=16;
            entity->velX.whole=x/timer;
            entity->velY.whole=(segment[1]<<16)/timer;
            STEP(entity)=segment[2]/timer;
            if(!facing)
                STEP(entity)=-STEP(entity);
            entity->subState++;
        }
        case 2:
        {
            s32 zero;
            u16 step;
            entity->posX.whole+=entity->velX.whole;
            entity->posY.whole+=entity->velY.whole;
            ANGLE(entity)+=STEP(entity);
            ObjAffine_Set(ANGLE(entity),0x10000,0x10000,entity->flipX,(zero=0),entity->affineIndex);
            if(--entity->timer==0xFF)
            {
                INDEX(entity)++;
                if(segments[INDEX(entity)][0]==0x7FFF)
                    entity->subState=5;
                else
                    entity->subState=1;
                entity->timer=0;
            }
            break;
        }
        case 5:
            if(entity->timer++>40)
            {
                entity->timer=0;
                entity->state=1;
                entity->stage=0;
                entity->subState=0;
                INDEX(entity)=0;
            }
            break;
    }
}

/* Alastor wind-up and aimed charge. */
extern s32 GetEntityRoomXPositionWhole(struct EwramData_EntityData *);
extern s32 GetEntityRoomYPositionWhole(struct EwramData_EntityData *);
extern s32 Entity_GetTargetAngleAbove16(struct EwramData_EntityData *);
/**
 * @brief Attack state (5): spin during wind-up, aim velocity toward the
 * target, travel for the computed duration and return to approach.
 * Keep existing target reads, division and angle arithmetic.
 */
void Alastor_UpdateAimedCharge(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg=&gEwramData->bgInfo[1];
    s32 targetX=Target_GetX(entity)+bg->xPos.whole;
    s32 targetY=Target_GetY(entity)+bg->yPos.whole-0x200000;
    (void)ABS(targetX-GetEntityRoomXPositionWhole(entity));
    (void)ABS(targetY-GetEntityRoomYPositionWhole(entity));
    switch(entity->subState)
    {
        case 0:
            entity->accelY.whole=0;
            entity->accelX.whole=0;
            entity->velY.whole=0;
            entity->velX.whole=0;
            STEP(entity)=0x1000;
            if(!entity->flipX)
                STEP(entity)=-STEP(entity);
            entity->timer=0;
            entity->subState++;
        case 1:
            if(((u32)entity->timer % 16)==0)
                PlaySong(0x85);
            if(entity->timer++>64)
            {
                STEP(entity)=0;
                ANGLE(entity)=Entity_GetTargetAngleAbove16(entity)+0x4000;
                entity->velX.whole=FixedMultiply(Sin(ANGLE(entity)+0x10000),0x20000);
                entity->velY.whole=FixedMultiply(Sin(ANGLE(entity)+0xC000),0x20000);
                entity->timer=ABS((Target_GetX(entity)-entity->posX.whole)/entity->velX.whole);
                entity->subState++;
                PlaySong(0x86);
            }
            break;
        case 2:
            if(--entity->timer==0)
            {
                entity->subState=3;
                entity->timer=0;
            }
            break;
        case 3:
        case 4:
            entity->state=1;
            entity->stage=0;
            entity->subState=0;
            break;
    }
    entity->posX.whole+=entity->velX.whole;
    entity->posY.whole+=entity->velY.whole;
    entity->velX.whole+=entity->accelX.whole;
    entity->velY.whole+=entity->accelY.whole;
    ANGLE(entity)+=STEP(entity);
    ObjAffine_Set(ANGLE(entity),0x10000,0x10000,entity->flipX,0,entity->affineIndex);
}

/* Spawn a translucent copy of Alastor with the current frame/angle. */
extern void AlastorAfterimage_Update(struct EwramData_EntityData *);
/**
 * @brief Create a translucent sprite copy at the current frame and pose
 * with a timer and owner link. Allocate its own affine slot and return
 * the copy or NULL; preserve existing allocation and cleanup behavior.
 */
struct EwramData_EntityData *Alastor_CreateAfterimage(struct EwramData_EntityData *entity,u8 timer)
{
    s32 graphics=Sprite_GetTileSlot(entity);
    u8 *graphicsIndex=&entity->spriteSlot;
    struct SpriteSheet *source=gEwramData->spriteSlots[*graphicsIndex].sheet;
    struct SpriteSheet *sprite=source;
    struct EwramData_EntityData *copy=EntityCreateInRange(0x49,0xCF,(u32 *)AlastorAfterimage_Update);
    s32 affine;
    if(!copy)
        return NULL;
    if(!source) { EntityDelete(copy); return NULL; }
    affine=ObjAffine_Alloc(1);
    if(affine<0)
    {
        EntityDelete(copy);
        return NULL;
    }
    Sprite_Attach(copy,sprite,graphics,entity->palette);
    copy->animFrame=entity->animFrame;
    copy->spriteLayer=entity->spriteLayer+1;
    copy->posX.whole=entity->posX.whole;
    copy->posY.whole=entity->posY.whole;
    copy->scrollWithBg=1;
    copy->objMode=1;
    copy->flipX=entity->flipX;
    copy->drawFunc=(u32 *)Sprite_DrawEntity;
    copy->affine=entity->affine;
    if(copy->affine)
    {
        copy->affineIndex=affine;
        ObjAffine_Set(entity->ext.alastor.angle,0x10000,0x10000,copy->flipX,0,copy->affineIndex);
    }
    ENTITY_PARENT_INDEX(copy)=ENTITY_INDEX(entity);
    copy->timer=timer;
    return copy;
}

/* Alastor spinning leap, conditional dive, and recovery. */
extern s32 Alastor_IsTargetInLowerBox(struct EwramData_EntityData *,u16,u16);
extern s32 Entity_IsOffsetOutsideRoom(struct EwramData_EntityData *,s32,s32);
extern s32 Collision_GetPushUp(s32,s32);
static inline s32 RoomX(s32 pos) { struct BgLayerState *bg=&gEwramData->bgInfo[1]; return pos+(s16)bg->xPos.part16.integer; }
static inline s32 RoomY(s32 pos) { struct BgLayerState *bg=&gEwramData->bgInfo[1]; return pos+(s16)bg->yPos.part16.integer; }
static inline void Advance(struct EwramData_EntityData *e,s32 amount,u8 zero) { e->subState+=amount; e->stage=zero; }
/**
 * @brief Attack state (6): spin through a leap while spawning copies,
 * optionally dive when the target is in the lower test box, then recover.
 */
void Alastor_UpdateSpinningLeap(struct EwramData_EntityData *entity)
{
    switch(entity->subState)
    {
        case 0:
        {
            s32 speed;
            u16 step;
            entity->ext.alastorPattern.step=0;
            entity->subState++;
            entity->velX.whole=(speed=-0x40000);
            entity->velY.whole=-0x20000;
            entity->accelY.whole=0x4000;
            STEP(entity)=(step=0xF880);
            entity->timer=16;
            if(entity->flipX)
            {
                entity->velX.whole=-speed;
                STEP(entity)=-step;
            }
            PlaySong(0x1BB);
            PlaySong(0x86);
        }
        case 1:
        {
            s32 z;
            u16 zero;
            Alastor_CreateAfterimage(entity,5);
            entity->posX.whole+=entity->velX.whole;
            entity->posY.whole+=entity->velY.whole;
            entity->velY.whole+=entity->accelY.whole;
            {
                u16 a=ANGLE(entity)+STEP(entity);
                /* Assigning zero from z sets the two zero registers in the original order. */
                z=0; zero=z;
                ANGLE(entity)=a;
                ObjAffine_Set(ANGLE(entity),0x10000,0x10000,entity->flipX,z,entity->affineIndex);
            }
            if(--entity->timer==0xFF)
            {
                if(Alastor_IsTargetInLowerBox(entity,128,256))
                    Advance(entity,1,zero);
                else
                    Advance(entity,2,zero);

            }
            break;
        }
        case 2:
            switch(entity->stage)
            {
                case 0:
                    entity->velY.whole=0x500;
                    entity->accelY.whole=0x4000;
                    entity->timer=0;
                    entity->stage++;
                case 1:
                    entity->posY.whole+=entity->velY.whole;
                    if(entity->timer==10)
                        PlaySong(0x86);
                    if(entity->timer++>10)
                        entity->velY.whole+=entity->accelY.whole;
                    if(Entity_IsOffsetOutsideRoom(entity,0,0) || (s16)Collision_GetPushUp(RoomX((s16)entity->posX.part16.integer),RoomY((s16)entity->posY.part16.integer)+8)!=0)
                    {
                        entity->subState++;
                        entity->stage=0;
                    }
                    break;
            }
            break;
        case 3:
            if(entity->stage==0)
            {
                entity->timer=40;
                entity->stage++;
            }
            else if(--entity->timer==0xFF)
            {
                entity->timer=0;
                entity->state=1;
                entity->stage=0;
                entity->subState=0;
                entity->ext.alastorPattern.step=0;
            }
            break;
    }
}

/* Alastor recoil after its linked part is hit. */
static inline s32 Wave(s32 angle,s32 amplitude) { return (amplitude>>8)*(Sin(angle)>>8); }
static inline s32 Multiply(s32 a,s32 b) { return a*b; }
static inline s32 Scale(s32 a,s32 b) { return (a>>8)*(b>>8); }
/**
 * @brief Reaction state (7): move away while coordinating the linked
 * part reaction; when it rests, return to approach or death state 0xFF
 * according to HP. Mark flag 0x20 while defeated.
 */
void Alastor_UpdateRecoil(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *part=&gEwramData->entityData[entity->ext.alastorPart.index];
    switch(entity->subState)
    {
        case 0:
            entity->timer=0;
            PHASE(entity)=0;
            if(gEwramData->playerState.player->posX.whole<entity->posX.whole)
                FLAGS(entity)|=8;
            else
                FLAGS(entity)&=~8;
            entity->subState++;
        case 1:
        {
            s32 angle=0xD000;
            s32 speed=0x20000;
            s32 amplitude=0x18000;
            u16 phase=PHASE(entity);
            if(!(FLAGS(entity)&8))
                speed=-speed;
            entity->posX.whole+=Wave(angle+0x4000,speed);
            entity->posY.whole+=Scale(amplitude,Sin(angle))+Multiply((s16)phase,0xF00);
            PHASE(entity)++;
            if(part->state==4)
                entity->subState++;
            break;
        }
        case 2:
        {
            s32 state=part->state;
            if(state==0)
            {
                if((s16)entity->hp<=0)
                    entity->state=0xFF;
                else
                    entity->state=1;
                entity->stage=state;
                entity->subState=state;
            }
            break;
        }
    }
    if((s16)entity->hp<=0)
        FLAGS(entity)|=0x20;
}

/* Alastor death: smoke along the blade, then falling out of view. */
/**
 * @brief Death state (0xFF): emit effects along the blade and fall; free
 * the owner affine slot and request deletion of both owner and linked
 * part on the existing bounds/timer condition.
 */
void Alastor_UpdateDeath(struct EwramData_EntityData *entity)
{
    s32 state=entity->subState;
    switch(state)
    {
        case 0:
            PlaySong(0x6E);
            entity->palette=entity->basePalette;
            entity->subState++;
            entity->stage=state;
        case 1:
            switch(entity->stage)
            {
                case 0:
                    entity->timer=10;
                    entity->stage++;
                case 1:
                {
                    s32 angle=(entity->ext.alastor.angle+0xC000)&0xFFFF;
                    s32 radius=0x2A0000-entity->timer*0x54000;
                    s32 x=FixedMultiply(Sin(angle+0x4000),radius);
                    s32 y=FixedMultiply(Sin(angle),radius);
                    u8 *layer=&entity->spriteLayer;
                    s32 scale=0x100000;
                    s32 i;
                    for(i=2;i>=0;i--)
                        Effect_SpawnRandom(entity->posX.whole+x,entity->posY.whole+y+0x60000,scale,scale,*layer-1);
                    if(--entity->timer==0xFF)
                        entity->stage++;
                    break;
                }
                case 2:
                    entity->subState++;
                    entity->stage=0;
                    break;
            }
            break;
        case 2:
            if(entity->stage==0)
            {
                entity->velY.whole=0x8000;
                entity->accelY.whole=0x800;
                entity->timer=64;
                entity->stage++;
            }
            entity->posY.whole+=entity->velY.whole;
            entity->velY.whole+=entity->accelY.whole;
            if((s16)entity->posY.part16.integer>170 || entity->timer++>64)
            {
                ObjAffine_Free(entity->affineIndex);
                entity->deleteRequested=1;
                {
                    s32 index=entity->ext.alastorPart.index;
                    index[gEwramData->entityData].deleteRequested=1;
                }
            }
            break;
    }
}

/* Alastor damage, stun counter and linked-part state. */
extern s32 Hit_ResolveDamage(struct EwramData_EntityData *,s32,u16 *,s32);
extern void Entity_ShowDamage(struct EwramData_EntityData *,s32,s32);
#define PART(e) (&gEwramData->entityData[e->ext.alastorPart.index])
/**
 * @brief Resolve/display damage and apply hit flash. Count surviving hits;
 * on the reaction threshold or defeat, coordinate linked-part state 3
 * and enter recoil state 7. Drop rewards on defeat and reveal the owner.
 */
void Alastor_OnHit(struct EwramData_EntityData *entity,s32 kind,u16 *hit)
{
    s32 damage=Hit_ResolveDamage(entity,kind,hit,(s16)Player_CalculateAttackPower(hit[2]));
    {
        u16 hp=entity->hp;
        damage<<=16;
        damage>>=16;
        entity->hp=hp-damage;
    }
    Entity_ShowDamage(entity,damage,0);
    entity->hitFlashTimer=8;
    if (entity->ext.alastor.hitsBeforeReact && (s16)entity->hp>0)
        entity->ext.alastor.hitsBeforeReact--;
    else
    {
        struct EwramData_EntityData *part;
        if ((s16)entity->hp<=0)
        {
            Enemy_DropRewards(entity);
            PlaySong(0x1BE);
        }
        PART(entity)->state=3;
        PART(entity)->subState=PART(entity)->stage=0;
        entity->state=7;
        entity->stage=0;
        entity->subState=0;
        entity->ext.alastor.hitsBeforeReact=3;
    }
    entity->hidden=0;
}

/* Alastor linked part, sharing the owner's graphics. */
extern s8 ObjPalette_FindOrLoad(void *);
extern void AlastorPart_Update(struct EwramData_EntityData *);
/**
 * @brief Create a linked part using the owner graphics, store both slot
 * links and initialize its update. Return 1 on success, otherwise 0;
 * preserve the existing sprite-attachment failure cleanup behavior.
 */
s32 Alastor_CreateLinkedPart(struct EwramData_EntityData *entity)
{
    void *palette=sAlastorPalette;
    void *sprite=sAlastorSpriteSheet;
    u8 *graphics=sAlastorGfx;
    void (*update)(struct EwramData_EntityData *)=AlastorPart_Update;
    struct EwramData_EntityData *part=EntityCreateInRange(0x49,0xCF,(u32 *)update);
    if (part)
    {
        s32 graphic=ObjGfx_Find(graphics);
        s32 slot=ObjPalette_FindOrLoad(palette);
        if (Sprite_Attach(part,sprite,graphic,slot))
        {
            s32 index=ENTITY_INDEX(entity);
            s32 zero=0;
            ENTITY_PARENT_INDEX(part)=index;
            entity->ext.alastorPart.index=ENTITY_INDEX(part);
            part->objMode=1;
            part->drawFunc=(u32 *)Sprite_DrawEntity;
            part->scrollWithBg=1;
            part->spriteLayer=entity->spriteLayer+2;
            part->state=1;
            part->stage=zero;
            part->subState=zero;
            update(part);
            return 1;
        }
    }
    return 0;
}

/* Alastor linked part: following, attack pose, flash and rest. */
/**
 * @brief Follow the owner in appearance, preparation and reaction poses,
 * then blink and hide at rest. State transitions coordinate with the
 * owner attack-selection and recoil helpers. Exact visual identity unresolved.
 */
void AlastorPart_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner=&gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    if (Enemy_IsStunned(entity))
        return;
    switch (entity->state)
    {
        case 0:
            entity->hidden=1;
            break;
        case 1:
            entity->hidden=0;
            entity->animFrame=1;
            entity->posX.whole=owner->posX.whole;
            entity->posY.whole=owner->posY.whole;
            Entity_FaceTarget(entity);
            break;
        case 2:
            switch(entity->subState)
            {
                case 0:
                    entity->posX.whole=owner->posX.whole;
                    Entity_FaceTarget(entity);
                    entity->hidden=0;
                    entity->animFrame=1;
                    entity->timer=10;
                    entity->subState++;
                case 1:
                    entity->posX.whole=owner->posX.whole;
                    entity->posY.whole=owner->posY.whole;
                    if (entity->flipX)
                        entity->posX.whole=owner->posX.whole+0x60000;
                    else
                        entity->posX.whole=owner->posX.whole-0x60000;
                    entity->posY.whole=owner->posY.whole+0xA0000;
                    if (--entity->timer==0xFF)
                    {
                        entity->state=4;
                        entity->stage=0;
                        entity->subState=0;
                    }
                    break;
            }
            break;
        case 3:
            switch(entity->subState)
            {
                case 0:
                    entity->hidden=0;
                    entity->animFrame=2;
                    entity->timer=0;
                    entity->subState++;
                    if (gEwramData->playerState.player->posX.whole<owner->posX.whole)
                        entity->flipX=0;
                    else
                        entity->flipX=1;
                case 1:
                    if (!entity->flipX)
                        entity->posX.whole=owner->posX.whole-0xA0000;
                    else
                        entity->posX.whole=owner->posX.whole+0xA0000;
                    entity->posY.whole=owner->posY.whole-0x40000;
                    if (entity->timer++>39)
                    {
                        entity->state=4;
                        entity->stage=0;
                        entity->subState=0;
                    }
                    break;
            }
            break;
        case 4:
            switch(entity->subState)
            {
                case 0:
                    entity->timer=0;
                    entity->subState++;
                case 1:
                    entity->hidden=((u32)-(u8)((u32)entity->timer%3))>>31;
                    if (entity->timer++>30)
                    {
                        entity->state=0;
                        entity->stage=0;
                        entity->subState=0;
                        entity->timer=0;
                    }
                    break;
            }
            break;
        case 5:
            entity->posY.part16.integer=owner->posY.part16.integer+12;
            if (owner->flipX)
                entity->posX.part16.integer=owner->posX.part16.integer+14;
            else
                entity->posX.part16.integer=owner->posX.part16.integer-14;
            break;
    }
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Effect_FollowParentHitBoxCopy = Effect_FollowParentHitBox */

/**
 * @brief CED90 | Same as Effect_FollowParentHitBox.
 */
void Effect_FollowParentHitBoxCopy(struct EwramData_EntityData *entity)
{
    struct SpriteBox attackBox;
    struct SpriteBox bodyBox;
    struct EwramData_EntityData *parent = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];

    HitBox_GetBoxes(&attackBox, &bodyBox, parent);
    switch (entity->state)
    {
        case 0:
        {
            s32 gfx = ObjGfx_Find(sCommonObjectGfx);

            Sprite_Attach(entity, sCommonObjectSpriteSheet, gfx, 7);
            Anim_StartIndexed(entity, 0, 0, 0);
            entity->drawFunc = (u32 *)Sprite_DrawEntity;
            entity->state++;
        }
        case 1:
            entity->posX.part16.integer = parent->posX.part16.integer + attackBox.x;
            entity->posY.part16.integer = parent->posY.part16.integer + attackBox.y;
            break;
    }
}

/* Alastor bestiary preview, target-box tests and afterimage lifetime. */
extern s32 Entity_IsInRect(struct EwramData_EntityData *entity, s16 left, s16 top, u16 width, u16 height);

/**
 * @brief Bestiary state (8): move the displayed entity up 16 pixels once.
 */
void Alastor_UpdateBestiaryPreview(struct EwramData_EntityData *entity)
{
    if (entity->subState == 0)
    {
        entity->posY.part16.integer -= 0x10;
        entity->subState++;
    }
}

/**
 * @brief Test the current target in a supplied-size rectangle centered
 * horizontally on the owner and starting 40 pixels below it.
 */
s32 Alastor_IsTargetInLowerBox(struct EwramData_EntityData *entity, u16 width, u16 height)
{
    struct EwramData_EntityData *target = gEwramData->playerState.enemyTarget;
    u16 left = entity->posX.part16.integer - (width >> 1);
    u16 top = entity->posY.part16.integer + 0x28;

    return Entity_IsInRect(target, left, top, width, height);
}

/**
 * @brief Test the current target in a supplied-size rectangle centered
 * horizontally on the owner and ending at its Y coordinate.
 */
s32 Alastor_IsTargetInUpperBox(struct EwramData_EntityData *entity, u16 width, u16 height)
{
    struct EwramData_EntityData *target = gEwramData->playerState.enemyTarget;
    u16 left = entity->posX.part16.integer - (width >> 1);
    u16 top = entity->posY.part16.integer - height;

    return Entity_IsInRect(target, left, top, width, height);
}

/**
 * @brief On timer underflow or owner removal, free the affine slot and
 * delete the copied sprite.
 */
void AlastorAfterimage_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];

    if (--entity->timer == 0xFF || owner->updateFunc == NULL)
    {
        ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
    }
}
