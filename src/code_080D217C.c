/* Five shared death fragments, their lifetime update, and a state helper. */
#include "code_08009A0.h"
#include "particle_effect.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s8 ObjPalette_FindOrLoadVariant(void *,s32);
extern s32 Entity_IsInRect(struct EwramData_EntityData *,s16,s16,u16,u16);
extern void FragmentSet_Update(struct EwramData_EntityData *);
extern u8 sMandragoraGfx[],sMandragoraPalette[],sMandragoraSpriteSheet[];
struct FragmentSpec { s8 x,y; u8 frame; s8 vx,vy; u8 timer; } __attribute__((packed));
extern struct FragmentSpec sFragmentSpecs[];
/**
 * @brief Unreferenced: spawn the five fragments of sFragmentSpecs
 * (FragmentSet_Update) from the entity, counting them in its unk_4F7.
 */
void Entity_SpawnFragmentSet(struct EwramData_EntityData *entity)
{
    s32 i;
    for(i=0;i<5;i++)
    {
        struct EwramData_EntityData *fragment=EntityCreateInRange(0x49,0xCF,(u32 *)FragmentSet_Update);
        struct FragmentSpec *spec;
        s32 graphics;
        s32 slot;
        s32 x;
        if(!fragment)
            break;
        spec=&sFragmentSpecs[i];
        FRAGMENT_SET_COUNT(entity)++;
        ENTITY_PARENT_INDEX(fragment)=ENTITY_INDEX(entity);
        fragment->spriteLayer=entity->spriteLayer;
        fragment->unk_510.part8.basePalette=entity->unk_510.part8.basePalette;
        fragment->flipX=entity->flipX;
        graphics=ObjGfx_LoadResource(sMandragoraGfx,0);
        slot=ObjPalette_FindOrLoadVariant(sMandragoraPalette,0);
        Sprite_Attach(fragment,sMandragoraSpriteSheet,graphics,slot);
        fragment->scrollWithBg=1;
        fragment->drawFunc=(u32 *)Sprite_DrawEntity;
        fragment->posX.whole=entity->posX.whole;
        fragment->posY.whole=entity->posY.whole;
        x=spec->x;
        fragment->posY.part16.integer+=spec->y;
        fragment->animFrame=spec->frame;
        fragment->velX.whole=spec->vx<<12;
        fragment->velY.whole=-(spec->vy<<12);
        fragment->timer=spec->timer;
        if(fragment->flipX)
        {
            fragment->velX.whole=-fragment->velX.whole;
            x=-x;
        }
        fragment->posX.part16.integer+=x;
    }
}
/**
 * @brief Fall with gravity 0x3000; delete the fragment (and decrement the owner
 * count) off screen, below y 170 or when its timer ends.
 */
void FragmentSet_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner=&gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    if(!Entity_IsInRect(entity,-128,-64,496,288) || --entity->timer==0xFF || (s16)entity->posY.part16.integer>170)
    {
        FRAGMENT_SET_COUNT(owner)--;
        entity->deleteRequested=1;
    }
    else
    {
        entity->posX.whole+=entity->velX.whole;
        entity->posY.whole+=entity->velY.whole;
        entity->velY.whole+=0x3000;
    }
}
/**
 * @brief Entry state (3): select combat state 2 and reset its counters.
 */
void Belmont_EnterCombat(struct EwramData_EntityData *entity)
{
    s32 zero=0;
    entity->state=2;
    entity->stage=zero;
    entity->subState=zero;
}

