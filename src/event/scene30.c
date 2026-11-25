/*
 * Event scene 30 (transformation): fragments, sparks, emitters, orbit rings, center effect and halo (its update follows in a separately compiled file).
 */
#include "scene30.h"
#include "core.h"
#include "event_scene30.h"
#include "graphics.h"
#include "structs/ewram.h"
#include "structs/main.h"
#include "gba.h"
#include "entity_util.h"
#include <string.h>
#include "math_anim.h"
#include "oam.h"
#include "obj_gfx.h"
#include "palette.h"
#include "scene30_transformation.h"
#include "scenes_c.h"
#include "data/data_118B48.h"
#include "data/data_0E5364.h"
#include "data/data_4F1444.h"
#include "entity_common.h"

struct OrbitPoint { u16 angle; u8 radiusX,radiusY; u16 scale; u16 padding; };
extern struct OrbitPoint sEventSceneEndingOrbitPoints[];

/* Set up, play and fade the background transformation scene. */
/**
 * @brief Event scene 30 sequence: set up the BG layers and graphics, run
 * EventSceneEnding_UpdateTransformation with brightness fade-in, wait, fade
 * to white and clear the maps; return 1 when done.
 */
s32 EventSceneEnding_PlayTransformation(struct EwramData_EntityData *entity)
{
    s32 done=0;
    switch(entity->subState)
    {
    case 0:
        if(entity->stage==0){entity->timer=4;entity->stage++;}
        gUnk_03002CB0.dispCnt=0;
        gDisplayRegisters.bgOfs[0].hOfs=0;gDisplayRegisters.bgOfs[0].vOfs=0;
        gDisplayRegisters.bgOfs[1].hOfs=0;gDisplayRegisters.bgOfs[1].vOfs=0;
        gDisplayRegisters.bgOfs[2].hOfs=0;gDisplayRegisters.bgOfs[2].vOfs=0;
        gDisplayRegisters.bldCnt=255;gDisplayRegisters.bldY=16;{u16 alpha=0x1000;gDisplayRegisters.bldAlpha=alpha;}
        gEwramData->gameModeCounter=0;gEwramData->gameModeTimer=0;
        if(--entity->timer==255){BgPalette_StopColorFade();ObjPalette_StopColorFade();entity->subState=1;}
        break;
    case 1:
        gUnk_03002CB0.dispCnt=0x1E01;
        gDisplayRegisters.bgCnt[1]=0x1D09;gDisplayRegisters.bgCnt[2]=0x5E80;
        Obj_SetBgLayer(22,1);Obj_SetBgLayer(6,2);Bg_SyncPriorities();
        Gfx_CopyResource(sEventSceneEndingTransformationGfx,(void *)VRAM_BASE,0);
        Gfx_CopyResource(sUnk_08119898,(void *)(VRAM_BASE + 0x8000),0);
        BgPalette_LoadRange(sEventSceneEndingTransformationPalette,0,16,0);
        BgMap_Setup(1,&sEventSceneEndingBgMap,0,48);BgMap_Setup(2,&sEventSceneEndingTransformationBg2Map,0,0);
        entity->subState=2;
        break;
    case 2:
        EventSceneEnding_UpdateTransformation(entity);EventSceneEnding_UploadFrame();
        gDisplayRegisters.bldY=16-(gEwramData->gameModeTimer>>1);
        if(gDisplayRegisters.bldY)
        {if(entity->timer%8==0)gEwramData->gameModeTimer++;}
        else {gDisplayRegisters.bldCnt=0x3F40;gEwramData->gameModeTimer=0;entity->subState=3;}
        entity->timer++;
        break;
    case 3:
        EventSceneEnding_UpdateTransformation(entity);EventSceneEnding_UploadFrame();
        if(entity->ext.event30.phase==10)
        {if(gEwramData->gameModeTimer++>50){gEwramData->gameModeTimer=0;entity->subState=4;}}
        break;
    case 4:
        EventSceneEnding_UpdateTransformation(entity);EventSceneEnding_UploadFrame();
        if(gEwramData->gameModeTimer++>128)
        {
            BgPalette_StopColorFade();ObjPalette_StopColorFade();
            BgPalette_StartColorFade(31,31,31,0x10000,0,64);ObjPalette_StartColorFade(31,31,31,0x10000,0,64);
            gEwramData->gameModeTimer=0;entity->subState=5;
        }
        break;
    case 5:
        EventSceneEnding_UpdateTransformation(entity);EventSceneEnding_UploadFrame();
        if(++gEwramData->gameModeTimer>64)
        {
            DMA_FILL_32(3,0,(void *)(VRAM_BASE + 0xE800),0x800);
            DMA_FILL_32(3,0,(void *)(VRAM_BASE + 0xF000),0x800);
            DMA_FILL_32(3,0,(void *)(VRAM_BASE + 0xF800),0x800);
            entity->subState=6;
        }
        break;
    case 6:
        done=1;
        break;
    }
    gEwramData->gameModeCounter++;
    return done;
}

/* Allocate a shared effect, optionally reserving an affine sprite slot. */
/**
 * @brief Create an effect sprite entity of event scene 30 (graphics
 * sEventSceneEndingGfx, palette 14), optionally with an affine slot at scale
 * 0x12000; return NULL on failure.
 */
struct EwramData_EntityData *EventSceneEnding_CreateEffectSprite(s32 affine)
{
    struct EwramData_EntityData *entity=EntityCreateInRange(0x1B,0x48,EntityUpdateNothing);
    if(entity)
    {
        s32 slot=ObjGfx_LoadResource(sEventSceneEndingGfx,0);
        if(slot<0)
        {
            EntityDelete(entity);
            return 0;
        }
        entity->basePalette=14;
        Sprite_Attach(entity,sEventSceneEndingSpriteSheet,slot,14);
        entity->drawFunc=Sprite_DrawEntity;
        if(affine)
        {
            entity->ext.event30Effect.scale=0x12000;
            slot=ObjAffine_Alloc(1);
            if(slot<0)
            {
                EntityDelete(entity);return 0;
            }
            entity->affineIndex=slot;entity->affine=1;
            ObjAffine_Set(0x10000,entity->ext.event30Effect.scale,entity->ext.event30Effect.scale,0,0,entity->affineIndex);
        }
        entity->spriteLayer=17;
    }
    return entity;
}

/* Delayed fragments spiral away, alternating their direction of rotation. */
static inline void flashAt(s16 x,s16 y)
{
    struct EwramData_EntityData *effect=EntityCreateInRange(0x1B,0x48,EventSceneEndingFlash_Update);
    if(effect)
    {
        if(!Entity_LoadGraphics(effect,sEventSceneEndingGfx,sEventSceneEndingPalette,0,sEventSceneEndingSpriteSheet))EntityDelete(effect);
        else
        {
            effect->posX.part16.integer=x;effect->posY.part16.integer=y;effect->spriteLayer=0;
            Anim_StartIndexed(effect,1,0,0);Anim_Update(effect);
        }
    }
}
/**
 * @brief After its delay, flash in and move along its random angle with a
 * trail, then fade out.
 */
void EventSceneEndingOrbitFragment_Update(struct EwramData_EntityData *entity)
{
    s32 stage=entity->state;
    switch(stage)
    {
    case 0:
        if(--entity->timer==255)
        {
            entity->timer=stage;EVENT30_EFFECT_ANGLE(entity)=0x10000;entity->state++;
            entity->stage=stage;entity->subState=stage;entity->hidden=0;
            flashAt(entity->posX.part16.integer,entity->posY.part16.integer);
        }
        break;
    case 1:
    {
        u32 angle=(u16)((u16)entity->ext.event30Fragment.phase<<11);
        entity->velX.whole=(Sin(angle+0x4000)>>8)<<10;
        entity->velY.whole=(Sin(angle)>>8)<<10;
        entity->posX.whole+=entity->velX.whole;entity->posY.whole+=entity->velY.whole;
        if(++entity->timer>32)entity->spriteLayer=17;else entity->spriteLayer=3;
        if(entity->timer%2==0)
        {
            if(entity->ext.event30Fragment.dir)
            {entity->ext.event30Fragment.phase++;EVENT30_EFFECT_ANGLE(entity)+=0x200;}
            else {entity->ext.event30Fragment.phase--;EVENT30_EFFECT_ANGLE(entity)-=0x200;}
        }
        ObjAffine_Set(EVENT30_EFFECT_ANGLE(entity),0x12000,0x12000,0,0,entity->affineIndex);
        if(entity->timer>63){ObjAffine_Free(entity->affineIndex);entity->deleteRequested=1;}
        break;
    }
    }
}

/* Place nine timed affine fragments from the owner's saved origin. */
/**
 * @brief Spawn the nine delayed fragments of sEventSceneEndingTimedFragments
 * (EventSceneEndingOrbitFragment_Update) around the owner position.
 */
void EventSceneEnding_SpawnOrbitFragments(struct EwramData_EntityData *owner)
{
    struct TimedFragment *item=sEventSceneEndingTimedFragments;
    s32 index;
    for(index=0;index<9;index++,item++)
    {
        struct EwramData_EntityData *entity=EventSceneEnding_CreateEffectSprite(1);
        if(entity)
        {
            entity->updateFunc=EventSceneEndingOrbitFragment_Update;
            entity->posX.part16.integer=owner->ext.event30.centerX+item->x;
            entity->posY.part16.integer=owner->ext.event30.centerY+item->y+32;
            entity->timer=item->delay;
            entity->animFrame=item->frame;
            entity->ext.event30Fragment.phase=RandomNumberGenerator();
            entity->ext.event30Fragment.dir=index&1;
            entity->ext.event30Fragment.index=index;
            entity->hidden=1;
        }
    }
}

/* Falling, orbiting, delayed release, then shrinking flight. */
/**
 * @brief Fall, then bob on a sine until the owner sets flag 2, then leave.
 */
void EventSceneEndingFloatingFragment_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner=&gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    s32 stage=entity->state;
    s32 state=(u8)stage;
    if(state==0)
    {
        entity->posY.whole+=entity->velY.whole;
        entity->velY.whole+=entity->accelY.whole;
        if(--entity->timer==255)
        {
            entity->state=stage+1;entity->stage=state;entity->subState=state;
            entity->accelY.whole=state;entity->velY.whole=state;
        }
    }
    else if(state==1)
    {
        entity->velY.whole=Sin((u16)entity->ext.event30Fragment.phase+0x4000)/4;
        entity->ext.event30Fragment.phase+=0x400;
        entity->posY.whole+=entity->velY.whole;
        if(owner->attack&2)
        {
            entity->state++;entity->stage=0;entity->subState=0;
            entity->timer=RandomNumberGenerator()%32;
        }
    }
    else if(state==2)
    {
        if(--entity->timer==255)
        {entity->timer=18;entity->state=stage+1;entity->stage=0;entity->subState=0;}
    }
    else if(state==3)
    {
        if(entity->stage==0)entity->stage++;
        if(!Entity_StepTowardPoint(entity,0x780000,0x500000,4))
        {ObjAffine_Free(entity->affineIndex);entity->deleteRequested=1;return;}
        if(entity->timer)
        {entity->ext.event30Effect.scale-=0x1000;entity->timer--;}
    }
    if(entity->state==1)
    {
        entity->velX.whole=Sin(*(u16 *)((u8 *)entity+0x3E)+0x4000)*2;
        entity->velY.whole=Sin(*(u16 *)((u8 *)entity+0x3E))*2;
        *(u16 *)((u8 *)entity+0x3E)+=0x155;
    }
    entity->posX.whole+=entity->velX.whole;
    entity->posY.whole+=entity->velY.whole;
    entity->velY.whole+=entity->accelY.whole;
    ObjAffine_Set(EVENT30_EFFECT_ANGLE(entity),entity->ext.event30Effect.scale,entity->ext.event30Effect.scale,0,0,entity->affineIndex);
    EVENT30_EFFECT_ANGLE(entity)+=0x200;
}

/* Create a falling affine particle linked to the parent's entity slot. */
/**
 * @brief Spawn a falling then floating fragment
 * (EventSceneEndingFloatingFragment_Update) at (x, y) with a random frame.
 */
void EventSceneEnding_SpawnFloatingFragment(struct EwramData_EntityData *owner,s16 x,s16 y,s32 alternate)
{
    struct EwramData_EntityData *entity=EventSceneEnding_CreateEffectSprite(1);
    if(entity)
    {
        u8 zero;
        entity->updateFunc=EventSceneEndingFloatingFragment_Update;
        ENTITY_PARENT_INDEX(entity)=ENTITY_INDEX(owner);
        zero=0;
        entity->ext.event30Effect.scale=0x12000;
        entity->posX.part16.integer=x;entity->posY.part16.integer=y;
        entity->velX.whole=zero;entity->velY.whole=0x4000;
        entity->accelX.whole=zero;entity->accelY.whole=0x800;
        entity->spriteLayer=17;entity->timer=20;entity->group=1;
        if(!alternate)entity->animFrame=RandomNumberGenerator()%5+9;
        else entity->animFrame=RandomNumberGenerator()%7+2;
        /* Replace the packed render flags at entity + 0x3E. */
        {u16 flags=0x8000; *(u16 *)((u8 *)entity+0x3E)=flags;}
    }
}

/* Allocate a directional fragment with selectable frame families. */
/**
 * @brief Spawn a drifting fragment (EventSceneEndingEffect_UpdateBallistic) at (x,
 * y), mirrored when reverse is set.
 */
struct EwramData_EntityData *EventSceneEnding_SpawnDriftFragment(s16 x,s16 y,s32 reverse,s32 alternate)
{
    struct EwramData_EntityData *effect=EventSceneEnding_CreateEffectSprite(0);
    struct FragmentFrames frames=sEventSceneEndingFragmentFrames;
    if(effect)
    {
        effect->updateFunc=EventSceneEndingEffect_UpdateBallistic;
        effect->posX.part16.integer=x;
        effect->posY.part16.integer=y;
        effect->velX.whole=0x30000;
        effect->velY.whole=-0x8000;
        effect->accelX.whole=0x800;
        effect->accelY.whole=-0x100;
        if(reverse)
        {
            effect->velX.whole=-effect->velX.whole;
            effect->velY.whole=-effect->velY.whole;
            effect->accelX.whole=-effect->accelX.whole;
            effect->accelY.whole=-effect->accelY.whole;
        }
        effect->timer=64;
        effect->spriteLayer=3;
        if(!alternate)effect->animFrame=frames.frame[RandomNumberGenerator()%3];
        else effect->animFrame=RandomNumberGenerator()%7+2;
        effect->group=128;
    }
    return effect;
}

/* Delayed accelerated flight; retire outside the scene's bounds. */
/**
 * @brief After its delay, show the spark and move it with acceleration; delete
 * outside the box.
 */
void EventSceneEndingSpark_Update(struct EwramData_EntityData *entity)
{
    if(entity->state==0)
    {
        if(--entity->timer==255){entity->hidden=0;entity->state++;}
    }
    else if(entity->state==1)
    {
        entity->posX.whole+=entity->velX.whole;
        entity->posY.whole+=entity->velY.whole;
        entity->velX.whole+=entity->accelX.whole;
        entity->velY.whole+=entity->accelY.whole;
        if(!Entity_IsInRect(entity,-128,-64,496,288))entity->deleteRequested=1;
    }
}

/* Initial flash followed by three delayed, randomly varied fragments. */
static inline void flashAt2(s16 x,s16 y)
{
    struct EwramData_EntityData *effect=EntityCreateInRange(0x1B,0x48,EventSceneEndingFlash_Update);
    if(effect)
    {
        if(!Entity_LoadGraphics(effect,sEventSceneEndingGfx,sEventSceneEndingPalette,0,sEventSceneEndingSpriteSheet))EntityDelete(effect);
        else
        {
            effect->posX.part16.integer=x;effect->posY.part16.integer=y;effect->spriteLayer=0;
            Anim_StartIndexed(effect,0,0,0);Anim_Update(effect);
        }
    }
}
/**
 * @brief Flash at (x, y) and spawn three delayed sparks around it.
 */
void EventSceneEnding_SpawnSparkBurst(s16 x,s16 y)
{
    struct BurstFrames frames=sEventSceneEndingBurstFrames;
    s32 count;
    s32 mask;
    flashAt2(x,y);
    mask=7;
    for(count=2;count>=0;count--)
    {
        struct EwramData_EntityData *effect=EventSceneEnding_CreateEffectSprite(0);
        if(effect)
        {
            effect->updateFunc=EventSceneEndingSpark_Update;
            effect->posX.part16.integer=(u16)((u16)x+4)-(RandomNumberGenerator()&mask);
            effect->posY.part16.integer=(u16)y+4-(RandomNumberGenerator()&mask);
            effect->velY.whole=0x14000-(RandomNumberGenerator()&0x7FFF);
            effect->accelY.whole=(RandomNumberGenerator()&0x7FF)+0x800;
            effect->velY.whole/=8;
            effect->accelY.whole/=8;
            effect->spriteLayer=3;
            effect->animFrame=frames.frame[RandomNumberGenerator()%3];
            effect->group=128;effect->hidden=1;
            effect->timer=RandomNumberGenerator()%20+32;
        }
    }
}

/* Initial flash followed by three delayed, randomly varied fragments. */
static inline void flashAt3(s16 x,s16 y,s32 animation)
{
    struct EwramData_EntityData *effect=EntityCreateInRange(0x1B,0x48,EventSceneEndingFlash_Update);
    if(effect)
    {
        if(!Entity_LoadGraphics(effect,sEventSceneEndingGfx,sEventSceneEndingPalette,0,sEventSceneEndingSpriteSheet))EntityDelete(effect);
        else
        {
            effect->posX.part16.integer=x;effect->posY.part16.integer=y;effect->spriteLayer=0;
            Anim_StartIndexed(effect,animation,0,0);Anim_Update(effect);
        }
    }
}
/**
 * @brief Flash at (x, y) with a random animation and spawn two delayed sparks
 * around it.
 */
void EventSceneEnding_SpawnSparkBurstSmall(s16 x,s16 y)
{
    struct BurstFrames frames=sEventSceneEndingBurstFrames;
    struct BurstAnimations animations=sEventSceneEndingBurstAnimations;
    s32 count;
    flashAt3(x,y,animations.frame[RandomNumberGenerator()%5]);
    for(count=1;count>=0;count--)
    {
        struct EwramData_EntityData *effect=EventSceneEnding_CreateEffectSprite(0);
        if(effect)
        {
            effect->updateFunc=EventSceneEndingSpark_Update;
            effect->posX.part16.integer=(u16)((u16)x+4)-(RandomNumberGenerator()%8);
            effect->posY.part16.integer=(u16)y+4-(RandomNumberGenerator()%8);
            effect->velY.whole=0x14000-(RandomNumberGenerator()&0x7FFF);
            effect->accelY.whole=(RandomNumberGenerator()&0x7FF)+0x800;
            effect->velY.whole/=8;
            effect->accelY.whole/=8;
            effect->spriteLayer=3;
            effect->animFrame=frames.frame[RandomNumberGenerator()%3];
            effect->group=128;effect->hidden=1;
            effect->timer=RandomNumberGenerator()%8;
        }
    }
}

/* Play fifteen timed burst positions; the state selects the burst variant. */
static inline void flashAt4(s16 x,s16 y,s32 animation)
{
    struct EwramData_EntityData *effect=EntityCreateInRange(0x1B,0x48,EventSceneEndingFlash_Update);
    if(effect)
    {
        if(!Entity_LoadGraphics(effect,sEventSceneEndingGfx,sEventSceneEndingPalette,0,sEventSceneEndingSpriteSheet))EntityDelete(effect);
        else
        {
            effect->posX.part16.integer=x;effect->posY.part16.integer=y;effect->spriteLayer=0;
            Anim_StartIndexed(effect,animation,0,0);Anim_Update(effect);
        }
    }
}
/**
 * @brief Emitter callback 0 of EventSceneEnding_UpdateTransformation: after the
 * step delays of sEventSceneEndingBurstPositions spawn spark bursts at the listed
 * positions.
 */
void EventSceneEndingEmitterA_Update(struct EwramData_EntityData *entity)
{
    struct BurstPosition positions[15];
    s32 stage;
    memcpy(positions,sEventSceneEndingBurstPositions,sizeof(positions));
    stage=entity->state;
    if((u8)stage==0)
    {
        entity->timer=positions[entity->stage].delay;
        entity->state=stage+1;
    }
    else if((u8)stage==1)
    {
        if(--entity->timer==255)entity->state=stage+1;
    }
    else
    {
        s32 parity=1;
        parity&=stage;
        if(parity)
        {
            EventSceneEnding_SpawnSparkBurst((s16)(positions[entity->stage].x+entity->posX.part16.integer),
                (s16)(positions[entity->stage].y+entity->posY.part16.integer));
        }
        else
        {
            flashAt4((s16)(positions[entity->stage].x+entity->posX.part16.integer),
                (s16)(positions[entity->stage].y+entity->posY.part16.integer),RandomNumberGenerator()%4);
        }
        entity->state=0;entity->stage++;
    }
    if(entity->stage==15)entity->deleteRequested=1;
}

/* Sequence ten timed effects with a small independent positional jitter. */
/**
 * @brief Emitter callback 1: step through sEventSceneEndingEmitterBSteps spawning small spark
 * bursts.
 */
void EventSceneEndingEmitterB_Update(struct EwramData_EntityData *entity)
{
    struct EffectStep steps[10];
    s32 stage;
    memcpy(steps,sEventSceneEndingEmitterBSteps,sizeof(steps));
    stage=entity->state;
    if((u8)stage==0)
    {
        entity->timer=steps[entity->stage].delay;
        entity->state=stage+1;
    }
    else if((u8)stage==1)
    {
        if(--entity->timer==255) entity->state=stage+1;
    }
    else
    {
        u32 random=RandomNumberGenerator();
        s32 x,y,mask;
        struct EffectStep *step;
        u16 *coordinate;
        step=&steps[entity->stage];
        coordinate=&entity->posX.part16.integer;
        x=*coordinate;
        x+=step->x;
        mask=3;
        random=(random&mask)-2;
        x-=random;
        x=(u32)x<<16;
        x>>=16;
        random=RandomNumberGenerator();
        step=&steps[entity->stage];
        coordinate=(u16 *)&entity->posY.part16.integer;
        y=step->y+*coordinate;
        random=(random&mask)-2;
        y=(s16)(y-random);
        EventSceneEnding_SpawnSparkBurstSmall(x,y);
        entity->state=0;
        entity->stage++;
    }
    if(entity->stage==10)entity->deleteRequested=1;
}

/* Five fixed effect positions, each activated after its own delay. */
/**
 * @brief Emitter callback 2: step through sEventSceneEndingEmitterCSequence spawning spark bursts.
 */
void EventSceneEndingEmitterC_Update(struct EwramData_EntityData *entity)
{
    struct EffectSequence sequence=sEventSceneEndingEmitterCSequence;
    s32 stage=entity->state;
    if((u8)stage==0)
    {
        entity->timer=sequence.steps[entity->stage].delay;
        entity->state=stage+1;
    }
    else if((u8)stage==1)
    {
        if(--entity->timer==255) entity->state=stage+1;
    }
    else
    {
        struct EffectStep *step=&sequence.steps[entity->stage];
        EventSceneEnding_SpawnSparkBurst(step->x+entity->posX.part16.integer,step->y+(u16)entity->posY.part16.integer);
        entity->state=0;
        entity->stage++;
    }
    if(entity->stage==5)entity->deleteRequested=1;
}

/* Sequence ten timed effects with a small independent positional jitter. */
/**
 * @brief Emitter callback 3: step through sEventSceneEndingEmitterDSteps spawning small spark
 * bursts.
 */
void EventSceneEndingEmitterD_Update(struct EwramData_EntityData *entity)
{
    struct EffectStep steps[10];
    s32 stage;
    memcpy(steps,sEventSceneEndingEmitterDSteps,sizeof(steps));
    stage=entity->state;
    if((u8)stage==0)
    {
        entity->timer=steps[entity->stage].delay;
        entity->state=stage+1;
    }
    else if((u8)stage==1)
    {
        if(--entity->timer==255) entity->state=stage+1;
    }
    else
    {
        u32 random=RandomNumberGenerator();
        s32 x,y,mask;
        struct EffectStep *step;
        u16 *coordinate;
        step=&steps[entity->stage];
        coordinate=&entity->posX.part16.integer;
        x=*coordinate;
        x+=step->x;
        mask=3;
        random=(random&mask)-2;
        x-=random;
        x=(u32)x<<16;
        x>>=16;
        random=RandomNumberGenerator();
        step=&steps[entity->stage];
        coordinate=(u16 *)&entity->posY.part16.integer;
        y=step->y+*coordinate;
        random=(random&mask)-2;
        y=(s16)(y-random);
        EventSceneEnding_SpawnSparkBurstSmall(x,y);
        entity->state=0;
        entity->stage++;
    }
    if(entity->stage==10)entity->deleteRequested=1;
}

/* Timed sequence of five animated effect entities. */
/**
 * @brief Emitter callback 4: step through sEventSceneEndingEmitterESequence spawning flashes.
 */
void EventSceneEndingEmitterE_Update(struct EwramData_EntityData *entity)
{
    struct EffectSequence sequence=sEventSceneEndingEmitterESequence;
    s32 stage=entity->state;
    if((u8)stage==0)
    {
        entity->timer=sequence.steps[entity->stage].delay;
        entity->state=stage+1;
    }
    else if((u8)stage==1)
    {
        if(--entity->timer==255) entity->state=stage+1;
    }
    else
    {
        struct EffectStep *step=&sequence.steps[entity->stage];
        s32 x=(s16)(step->x+entity->posX.part16.integer);
        s32 y=(s16)(step->y+(u16)entity->posY.part16.integer);
        u32 animation=RandomNumberGenerator()&3;
        struct EwramData_EntityData *effect=EntityCreateInRange(0x1B,0x48,EventSceneEndingFlash_Update);
        if(effect)
        {
            if(!Entity_LoadGraphics(effect,sEventSceneEndingGfx,sEventSceneEndingPalette,0,sEventSceneEndingSpriteSheet))EntityDelete(effect);
            else
            {
                effect->posX.part16.integer=x;
                effect->posY.part16.integer=y;
                effect->spriteLayer=0;
                Anim_StartIndexed(effect,animation,0,0);
                Anim_Update(effect);
            }
        }
        entity->state=0;
        entity->stage++;
    }
    if(entity->stage==5)entity->deleteRequested=1;
}

/* Emit four accelerating fragments at four-frame intervals. */
/**
 * @brief After its delay, spawn spinning pieces
 * (EventSceneEndingEffect_UpdateSpinning) with random frames.
 */
void EventSceneEndingPieceEmitter_Update(struct EwramData_EntityData *entity)
{
    u8 frames[4];
    u32 variant;
    memcpy(frames,sEventSceneEndingPieceEmitterFrames,4);
    variant=RandomNumberGenerator()%3;
    switch(entity->state)
    {
    case 0:
        if(entity->timer==0)entity->state=1;
        else entity->timer--;
        break;
    case 1:
    {
        struct EwramData_EntityData *effect=EventSceneEnding_CreateEffectSprite(1);
        if(effect)
        {
            s32 zero,xSpeed,ySpeed;
            effect->updateFunc=EventSceneEndingEffect_UpdateSpinning;
            effect->posX.part16.integer=(u16)(entity->posX.part16.integer+8)-(RandomNumberGenerator()%16);
            zero=0;
            effect->posY.part16.integer=(u16)entity->posY.part16.integer+8-(RandomNumberGenerator()%16);
            xSpeed=0x18000; ySpeed=-0x4000;
            effect->accelX.whole=0x800;
            effect->accelY.whole=-0x100;
            effect->velX.whole=(variant+1)*xSpeed;
            effect->velY.whole=(variant+1)*ySpeed;
            effect->spriteLayer=3;
            effect->animFrame=frames[entity->stage];
            effect->ext.event30Effect.spin=0x400;
            effect->ext.event30Effect.scale=0x10000;
            entity->stage++;
            entity->state=zero;
            entity->timer=4;
        }
        break;
    }
    }
    if(entity->stage==4)entity->deleteRequested=1;
}

/* Expanding orbit, parent-triggered release, drift, then affine fade-out. */
/**
 * @brief Place a ring point on its ellipse around its centre and follow the
 * owner phases.
 */
void EventSceneEndingOrbitRing_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner=&gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    s32 stage=entity->state;
    if((u8)stage==0)
    {
        u32 angle=(u16)entity->ext.event30Ring.angle;
        s32 quarter=0x4000;
        Sin(angle+quarter);
        Sin((u16)entity->ext.event30Ring.angle);
        entity->posX.part16.integer=entity->ext.event30Ring.centerX;
        entity->posY.part16.integer=entity->ext.event30Ring.centerY;
        entity->posX.part16.integer+=(Sin((u16)entity->ext.event30Ring.angle+quarter)*entity->ext.event30Ring.radiusX)>>13;
        entity->posY.part16.integer+=(Sin((u16)entity->ext.event30Ring.angle)*entity->ext.event30Ring.radiusY)>>13;
        entity->velX.whole=Sin((u16)entity->ext.event30Ring.angle+quarter)>>1;
        entity->velY.whole=Sin((u16)entity->ext.event30Ring.angle)>>1;
        entity->posX.whole+=entity->ext.event30Ring.spread*entity->velX.whole;
        entity->posY.whole+=entity->ext.event30Ring.spread*entity->velY.whole;
        if(entity->timer--%2==0)entity->ext.event30Ring.spread++;
        entity->ext.event30Ring.angle+=32;
        if(owner->attack&2)
        {
            u32 index;
            entity->velX.whole=Sin((u16)entity->ext.event30Ring.angle+quarter)>>2;
            entity->velY.whole=Sin((u16)entity->ext.event30Ring.angle)>>2;
            entity->state++;
            index=entity->ext.event30Ring.index;
            entity->timer=(6-(index>>2))*index;
        }
    }
    else if((u8)stage==1)
    {
        if(--entity->timer==255) { entity->state=stage+1;entity->timer=0; }
        else { entity->posX.whole+=entity->velX.whole;entity->posY.whole+=entity->velY.whole; }
    }
    else
    {
        if(Entity_StepTowardPoint(entity,0x780000,0x500000,3))
        { if(entity->ext.event30Effect.scale)entity->ext.event30Effect.scale-=0x1000; }
        else
        { ObjAffine_Free(entity->affineIndex);entity->deleteRequested=1; }
    }
    ObjAffine_Set(EVENT30_EFFECT_ANGLE(entity),entity->ext.event30Effect.scale,entity->ext.event30Effect.scale,0,0,entity->affineIndex);
}

/* Scatter twenty expanding orbit particles around a saved origin. */
/**
 * @brief Spawn the twenty ring points of sEventSceneEndingOrbitPoints
 * (EventSceneEndingOrbitRing_Update) around (x, y).
 */
void EventSceneEnding_SpawnOrbitRing(struct EwramData_EntityData *owner,u16 x,u16 y)
{
    struct OrbitPoint points[20];
    struct OrbitPoint *point;
    s32 index;
    memcpy(points,sEventSceneEndingOrbitPoints,sizeof(points));
    index=0;point=points;
    do
    {
        struct EwramData_EntityData *effect=EventSceneEnding_CreateEffectSprite(1);
        if(effect)
        {
            u32 random,radius;
            u8 *radiusX,*radiusY;
            Sin(point->angle+0x4000);
            Sin(point->angle);
            effect->updateFunc=EventSceneEndingOrbitRing_Update;
            ENTITY_PARENT_INDEX(effect)=ENTITY_INDEX(owner);
            effect->ext.event30Ring.index=index;
            effect->ext.event30Ring.centerX=x;
            effect->ext.event30Ring.centerY=y;
            radius=point->radiusX;
            radiusX=&effect->ext.event30Ring.radiusX;
            *radiusX=radius;
            radius=point->radiusY>>1;
            radiusY=&effect->ext.event30Ring.radiusY;
            *radiusY=radius;
            random=(RandomNumberGenerator()&0xFFF)+640;
            effect->ext.event30Ring.angle=point->angle-random;
            EVENT30_EFFECT_ANGLE(effect)=0;
            effect->ext.event30Ring.size=1;
            effect->posX.part16.integer=effect->ext.event30Ring.centerX;
            effect->posY.part16.integer=effect->ext.event30Ring.centerY;
            effect->posX.part16.integer+=(Sin((u16)effect->ext.event30Ring.angle+0x4000)* *radiusX)>>13;
            effect->posY.part16.integer+=(Sin((u16)effect->ext.event30Ring.angle)* *radiusY)>>13;
            effect->ext.event30Ring.spread=0;
            effect->velX.whole=Sin((u16)effect->ext.event30Ring.angle+0x4000)>>8;
            effect->velY.whole=Sin((u16)effect->ext.event30Ring.angle)>>8;
            effect->ext.event30Effect.scale=0x10000;
            effect->ext.event30Ring.size=point->scale*2;
            effect->timer=index-128-index/5;
            effect->spriteLayer=17;
            effect->animFrame=RandomNumberGenerator()%3+10;
        }
        point++;index++;
    }while(index<=19);
}

/* Emit four accelerating fragments at four-frame intervals. */
/**
 * @brief Unreferenced variant of EventSceneEndingPieceEmitter_Update using
 * EventSceneEndingEffect_UpdateSpinningKeepSlot.
 */
void EventSceneEndingPieceEmitterAlt_Update(struct EwramData_EntityData *entity)
{
    u8 frames[5];
    u32 variant;
    memcpy(frames,sEventSceneEndingPieceEmitterAltFrames,5);
    variant=RandomNumberGenerator()%3;
    switch(entity->state)
    {
    case 0:
        if(entity->timer==0)entity->state=1;
        else entity->timer--;
        break;
    case 1:
    {
        struct EwramData_EntityData *effect=EventSceneEnding_CreateEffectSprite(1);
        if(effect)
        {
            s32 zero,xSpeed,ySpeed,step;
            effect->updateFunc=EventSceneEndingEffect_UpdateSpinningKeepSlot;
            effect->posX.part16.integer=(u16)(entity->posX.part16.integer+8)-(RandomNumberGenerator()%16);
            zero=0;
            effect->posY.part16.integer=(u16)entity->posY.part16.integer+8-(RandomNumberGenerator()%16);
            xSpeed=0x18000; ySpeed=-0x4000;
            effect->accelX.whole=0x800;
            effect->accelY.whole=-0x100;
            step=variant+1;
            effect->velX.whole=step*xSpeed;
            effect->velY.whole=step*ySpeed;
            effect->spriteLayer=17;
            effect->animFrame=frames[entity->stage];
            effect->ext.event30Effect.spin=0x200;
            effect->ext.event30Effect.scale=step*0x9000;
            entity->stage++;
            entity->state=zero;
            entity->timer=16;
        }
        break;
    }
    }
    if(entity->stage==5)entity->deleteRequested=1;
}

/* Allocate a directional fragment with selectable frame families. */
/**
 * @brief Unreferenced: as EventSceneEnding_SpawnDriftFragment with a faster
 * horizontal start velocity.
 */
struct EwramData_EntityData *EventSceneEnding_SpawnFastDriftFragment(u16 x, u16 y, s32 reverse, s32 alternate)
{
    struct EwramData_EntityData *effect = EventSceneEnding_CreateEffectSprite(0);
    struct FragmentFrames frames = sEventSceneEndingFragmentFrames;
    if (effect)
    {
        u32 zero;
        effect->updateFunc = EventSceneEndingEffect_UpdateBallistic;
        { u16 *px = &effect->posX.part16.integer; zero = 0; *px = x; }
        effect->posY.part16.integer = y;
        effect->velX.whole = 0x60000;
        effect->velY.whole = zero;
        effect->accelX.whole = 0x800;
        effect->accelY.whole = -0x100;
        if (reverse)
        {
            effect->velX.whole = -effect->velX.whole;
            effect->accelX.whole = -effect->accelX.whole;
            effect->accelY.whole = -effect->accelY.whole;
        }
        asm("" :: "r"(zero)); /* FAKEMATCH: keeps r3 live, so reload picks r5 */
        effect->timer = 64;
        effect->spriteLayer = 3;
        if (!alternate)
            effect->animFrame = frames.frame[RandomNumberGenerator() % 3];
        else
            effect->animFrame = RandomNumberGenerator() % 7 + 2;
        effect->group = 128;
    }
    return effect;
}

/* Parent-linked affine effect: attachment, release, sparks and departure. */
/**
 * @brief Central effect: follow the owner timer, spawn spark bursts at set
 * times and scale with the owner radius.
 */
void EventSceneEndingCenterEffect_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner=&gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    s32 offsets[21];
    s32 stage;
    u32 radius;
    s32 scale;
    memcpy(offsets,sEventSceneEndingCenterEffectOffsets,sizeof(offsets));
    stage=entity->state;
    switch(stage)
    {
    case 0:
        if(owner->ext.event30.timer==263)
            EventSceneEnding_SpawnSparkBurst(entity->posX.part16.integer,(s16)(entity->posY.part16.integer+16));
        if(owner->ext.event30.timer==255)
        {
            entity->timer=stage;
            entity->state+=2;
            EVENT30_EFFECT_ANGLE(entity)=-0x800;
            entity->velX.whole=0x600;
            entity->velY.whole=stage;
            entity->accelX.whole=24;
            entity->accelY.whole=-8;
            break;
        }
        if(entity->timer<30)entity->timer++;
        entity->posX.part16.integer=(*(s8 *)&owner->ext.event30.jitterX-24)+owner->ext.event30.centerX;
        entity->posY.part16.integer=(owner->ext.event30.centerY-72)+(s8)EVENT30_JITTER_Y(owner)+(u8)(entity->timer/10);
        break;
    case 1:
        if(entity->timer<20)entity->timer++;
        if(owner->ext.event30.timer==160)
        {entity->timer=0;entity->state++;break;}
        { s32 dx=*(s8 *)&owner->ext.event30.jitterX; dx-=24; entity->posX.part16.integer=dx+owner->ext.event30.centerX+offsets[entity->timer]; }
        entity->posY.part16.integer=(owner->ext.event30.centerY-72)+(s8)EVENT30_JITTER_Y(owner)+(u8)(entity->timer/10);
        break;
    case 2:
        entity->posX.whole+=entity->velX.whole;
        entity->posY.whole+=entity->velY.whole;
        entity->velX.whole+=entity->accelX.whole;
        entity->velY.whole+=entity->accelY.whole;
        { u32 spin=0xFFF0; entity->ext.event30Effect.spin=spin-1;
        EVENT30_EFFECT_ANGLE(entity)+=(s16)spin; }
        if(owner->ext.event30.timer==0)
        {entity->timer=0;entity->state++;}
        else entity->timer--;
        break;
    case 3:
        if((entity->timer&1)==0 && entity->timer<40 && entity->timer>24)
        {
            s32 x=(s16)((entity->posX.part16.integer+4)-(RandomNumberGenerator()&7));
            s32 y=(s16)((entity->posY.part16.integer+4)-(RandomNumberGenerator()&7));
            EventSceneEnding_SpawnDriftFragment(x,y,0,0);
        }
        entity->posX.whole+=entity->velX.whole;
        entity->posY.whole+=entity->velY.whole;
        entity->velX.whole+=entity->accelX.whole;
        entity->velY.whole+=entity->accelY.whole;
        if((s8)entity->timer>=0)entity->timer++;
        if(entity->timer==72)
        {
            entity->velX.whole=0x10000;
            entity->velY.whole=-0x1200;
            entity->accelX.whole=0x2000;
            entity->accelY.whole=-0x200;
        }
        EVENT30_EFFECT_ANGLE(entity)+=entity->ext.event30Effect.spin;
        if(entity->ext.event30Effect.spin> -576)entity->ext.event30Effect.spin--;
        if((s16)entity->posX.part16.integer>271)
        {ObjAffine_Free(entity->affineIndex);entity->deleteRequested=1;}
        break;
    }
    radius=(u16)owner->ext.event30.scale;
    scale=0x100;
    scale-=radius;
    scale<<=8;
    scale+=0x10000;
    entity->ext.event30Effect.scale=scale;
    ObjAffine_Set(EVENT30_EFFECT_ANGLE(entity),scale,scale,0,0,entity->affineIndex);
}

/* Allocate an affine effect attached to an owner's entity slot. */
/**
 * @brief Create the central effect sprite (EventSceneEndingCenterEffect_Update)
 * attached to the owner.
 */
void EventSceneEnding_CreateCenterEffect(struct EwramData_EntityData *owner)
{
    EntityFunc callback=EventSceneEndingCenterEffect_Update;
    struct EwramData_EntityData *effect=EntityCreateInRange(0x1B,0x48,callback);
    u8 zero;
    if(effect)
    {
        s32 slot=ObjGfx_LoadResource(sEventSceneEndingGfx,0);
        if(slot<0)
        {
            EntityDelete(effect);
            return;
        }
        effect->basePalette=14;
        zero=0;
        Sprite_Attach(effect,sEventSceneEndingSpriteSheet,slot,14);
        effect->drawFunc=Sprite_DrawEntity;
        ENTITY_PARENT_INDEX(effect)=ENTITY_INDEX(owner);
        effect->ext.event30Effect.scale=0x1A000;
        slot=ObjAffine_Alloc(1);
        if(slot<0)
        {
            EntityDelete(effect);
            return;
        }
        effect->affineIndex=slot;
        effect->affine=1;
        ObjAffine_Set(0x10000,effect->ext.event30Effect.scale,effect->ext.event30Effect.scale,0,zero,effect->affineIndex);
        effect->updateFunc=callback;
        effect->timer=32;
        effect->ext.raw.unk_4FC.part16.unk_4FC=zero;
        effect->posX.part16.integer=130;
        effect->posY.part16.integer=28;
        effect->velX.whole=-0x800;
        effect->velY.whole=0x800;
        effect->accelX.whole=-0x100;
        effect->accelY.whole=0x100;
        effect->ext.event30Effect.spin=(s16)0xFE00;
        effect->spriteLayer=3;
        effect->animFrame=zero;
    }
}

/* Follow the parent, release, accelerate and retire the affine effect. */
#define ATTACH_X(e,p) ((e)->posX.part16.integer=(*(s8 *)&p->ext.event30.jitterX-48)+p->ext.event30.centerX)
#define ATTACH_Y(e,p) ((e)->posY.part16.integer=(s16)((s8)EVENT30_JITTER_Y((p))+8)+p->ext.event30.centerY)
/**
 * @brief Halo: follow the owner, switch frame and scale with the owner radius.
 */
void EventSceneEndingHalo_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner=&gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    s32 stage=entity->state;
    s32 scale;
    u32 radius;
    switch(stage)
    {
    case 0:
        if(owner->ext.event30.timer==160)entity->animFrame=1;
        if(owner->ext.event30.timer==0) { entity->timer=stage;entity->state++; }
        else entity->timer--;
        ATTACH_X(entity,owner); ATTACH_Y(entity,owner);
        radius=(u16)owner->ext.event30.scale;
        scale=0x100;
        scale-=radius;
        scale<<=8;
        scale+=0x10000;
        entity->ext.event30Effect.scale=scale;
        ObjAffine_Set(EVENT30_EFFECT_ANGLE(entity),scale,scale,0,0,entity->affineIndex);
        break;
    case 1:
        ATTACH_X(entity,owner); ATTACH_Y(entity,owner);
        if(entity->ext.event30Effect.scale!=0x12000)entity->ext.event30Effect.scale-=0x400;
        if(++entity->timer==81)
        { entity->animFrame=stage;entity->state++;entity->stage=0;entity->subState=0; }
        radius=(u16)owner->ext.event30.scale;
        scale=0x100;
        scale-=radius;
        scale<<=8;
        scale+=0x10000;
        entity->ext.event30Effect.scale=scale;
        ObjAffine_Set(EVENT30_EFFECT_ANGLE(entity),scale,scale,0,0,entity->affineIndex);
        break;
    case 2:
        entity->posX.whole+=entity->velX.whole;
        entity->posY.whole+=entity->velY.whole;
        entity->velX.whole+=entity->accelX.whole;
        entity->velY.whole+=entity->accelY.whole;
        radius=(u16)owner->ext.event30.scale;
        scale=0x100;
        scale-=radius;
        scale<<=8;
        scale+=0x10000;
        entity->ext.event30Effect.scale=scale;
        ObjAffine_Set(EVENT30_EFFECT_ANGLE(entity),scale,scale,0,0,entity->affineIndex);
        if(entity->timer==104)
        {
            entity->velX.whole=-0x40000;entity->velY.whole=0x10000;
            entity->accelX.whole=-0x10000;entity->accelY.whole=0x800;
            entity->ext.event30Effect.spin=0x400;
        }
        else entity->timer++;
        EVENT30_EFFECT_ANGLE(entity)+=entity->ext.event30Effect.spin;
        if((s16)entity->posX.part16.integer< -32)
        { ObjAffine_Free(entity->affineIndex);entity->deleteRequested=1; }
        break;
    }
}

/* Allocate an affine effect attached to an owner's entity slot. */
/**
 * @brief Create the halo sprite (EventSceneEndingHalo_Update) attached to the
 * owner.
 */
void EventSceneEnding_CreateHalo(struct EwramData_EntityData *owner)
{
    struct EwramData_EntityData *effect=EntityCreateInRange(0x1B,0x48,EventSceneEndingHalo_Update);
    u8 zero;
    if(effect)
    {
        s32 slot=ObjGfx_LoadResource(sEventSceneEndingGfx,0);
        if(slot<0)
        {
            EntityDelete(effect);
            return;
        }
        effect->basePalette=14;
        zero=0;
        Sprite_Attach(effect,sEventSceneEndingSpriteSheet,slot,14);
        effect->drawFunc=Sprite_DrawEntity;
        ENTITY_PARENT_INDEX(effect)=ENTITY_INDEX(owner);
        effect->ext.event30Effect.scale=0x1A000;
        slot=ObjAffine_Alloc(1);
        if(slot<0)
        {
            EntityDelete(effect);
            return;
        }
        effect->affineIndex=slot;
        effect->affine=1;
        ObjAffine_Set(0x10000,effect->ext.event30Effect.scale,effect->ext.event30Effect.scale,0,zero,effect->affineIndex);
        effect->ext.event30Fragment.phase=zero;
        effect->posX.part16.integer=(*(s8 *)&owner->ext.event30.jitterX-48)+owner->ext.event30.centerX;
        effect->posY.part16.integer=(s16)((s8)EVENT30_JITTER_Y(owner)+8)+owner->ext.event30.centerY;
        effect->velX.whole=-1000;
        effect->velY.whole=0x1000;
        effect->accelX.whole=-0x800;
        effect->accelY.whole=0x600;
        EVENT30_EFFECT_ANGLE(effect)=zero;
        effect->ext.event30Effect.spin=(s16)0xFF56;
        effect->spriteLayer=3;
        effect->animFrame=24;
    }
}

/* Progress through six timed palette transitions. */
/**
 * @brief Advance the six-step BG/OBJ palette transition of sEventSceneEndingPaletteTransitions by its
 * durations.
 */
void EventSceneEnding_StepPaletteTransition(struct EwramData_EntityData *entity)
{
    u8 *index=&EVENT30_PALETTE_STEP(entity);
    struct PaletteTransition *step=&sEventSceneEndingPaletteTransitions[*index];
    struct PaletteTransition *table=sEventSceneEndingPaletteTransitions;
    if(*index<=5 && (s16)EVENT30_PALETTE_TIMER(entity)++>step->duration)
    {
        s32 zero=0;
        EVENT30_PALETTE_TIMER(entity)=zero;
        BgPalette_LoadRange(step->data,step->count,1,0);
        ObjPalette_LoadRange(step->data,step->count,1,14);
        if(++*index<=5)
        {
            s32 scale;
            void *data;
            s32 count;
            step=&table[*index];
            data=step->data;
            count=step->count;
            scale=0x10000;
            BgPalette_StartBlend(0,data,count,scale,zero,step->duration);
            ObjPalette_StartBlend(entity->basePalette,step->data,step->count,scale,zero,step->duration);
        }
    }
}
