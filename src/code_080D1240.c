/* Belmont_UpdateJumpSlide: Belmont jump, landing and following attack/recovery sequence. */
#include "code/code_08039340.h"
#include "belmont_entity.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"
extern s32 Entity_MoveWithCollisionProfile(struct EwramData_EntityData *,s16 *,u16,s32);
extern s32 Entity_ClampXToRoom(struct EwramData_EntityData *,s32);
extern s16 sBelmontMoveProfile[];
/**
 * @brief Jump/slide state (10): jump and land, sequence the recovery
 * animations, then move with active damage until the countdown completes.
 * Retain the intentional fallthrough and pinned damage pointer.
 */
void Belmont_UpdateJumpSlide(struct EwramData_EntityData *entity)
{
    s32 state=entity->subState;
    s32 collision;
    switch(state)
    {
        case 0:
            switch(entity->stage)
            {
                case 0:
                    if(entity->flipX)
                        entity->velX.whole=-0x8000;
                    else
                        entity->velX.whole=0x8000;
                    entity->accelX.whole=state;
                    entity->velY.whole=-0x10000;
                    entity->accelY.whole=0x800;
                    entity->stage++;
                    PlaySong(0x125);
                    if(entity->animScriptIndex!=3)
                        Anim_StartIndexed(entity,3,1,0);
                    break;
                case 1:
                    collision=Entity_MoveWithCollisionProfile(entity,sBelmontMoveProfile,4,0);
                    if(collision & 3)
                    {
                        if(entity->flipX)
                            entity->accelX.whole=entity->velX.whole=state;
                        else
                            entity->accelX.whole=entity->velX.whole=state;
                    }
                    if(Entity_ClampXToRoom(entity,1))
                    {
                        if(entity->flipX)
                            entity->accelX.whole=entity->velX.whole=0;
                        else
                            entity->accelX.whole=entity->velX.whole=0;
                    }
                    if(entity->velY.whole >= -(entity->accelY.whole<<1))
                        entity->stage++;
                    break;
                case 2:
                    collision=Entity_MoveWithCollisionProfile(entity,sBelmontMoveProfile,4,0);
                    if(collision & 3)
                    {
                        if(entity->flipX)
                            entity->accelX.whole=entity->velX.whole=state;
                        else
                            entity->accelX.whole=entity->velX.whole=0;
                    }
                    if(Entity_ClampXToRoom(entity,1))
                    {
                        if(entity->flipX)
                            entity->accelX.whole=entity->velX.whole=0;
                        else
                            entity->accelX.whole=entity->velX.whole=0;
                    }
                    if(collision & 8)
                    {
                        entity->accelX.whole=0;
                        entity->velX.whole=0;
                        entity->accelY.whole=0;
                        entity->velY.whole=0;
                        entity->subState++;
                        entity->stage=0;
                    }
                    break;
            }
            entity->posX.whole+=entity->velX.whole;
            entity->posY.whole+=entity->velY.whole;
            entity->velX.whole+=entity->accelX.whole;
            entity->velY.whole+=entity->accelY.whole;
            break;
        case 1:
        {
            s32 zero;
            u8 *damage=&entity->attack;
            zero=0;
            *damage=zero;
            PlaySong(0x127);
            PlaySong(0x1E0);
            if(entity->animScriptIndex!=4)
                Anim_StartIndexed(entity,4,1,0);
            entity->subState++;
            entity->stage=zero;
        }
        /* The original intentionally continues through the following phases. */
        case 2:
            if(entity->animDone)
            {
                if(entity->animScriptIndex!=8)
                    Anim_StartIndexed(entity,8,1,0);
                entity->timer=48;
                entity->subState++;
                entity->stage=0;
                entity->velY.whole=0x8000;
                if(entity->flipX)
                {
                    entity->velX.whole=0x28000;
                    entity->accelX.whole=-0x900;
                }
                else
                {
                    entity->velX.whole=-0x28000;
                    entity->accelX.whole=0x900;
                }
            }
        case 3:
        {
            register u8 *damage asm("r4")=&entity->attack; /* FAKEMATCH: pinned register */
            s32 zero=0;
            *damage=75;
            collision=Entity_MoveWithCollisionProfile(entity,sBelmontMoveProfile,4,0);
            if(!(collision & 8))
                entity->posY.whole+=0x20000;
            if(collision & 3)
            {
                if(entity->flipX)
                    { entity->velX.whole=zero;
                            entity->accelX.whole=zero; }
                else
                    { entity->velX.whole=0;
                            entity->accelX.whole=0; }
            }
            if(Entity_ClampXToRoom(entity,1))
            {
                if(entity->flipX)
                    { entity->velX.whole=0;
                            entity->accelX.whole=0; }
                else
                    { entity->velX.whole=0;
                            entity->accelX.whole=0; }
            }
            if(--entity->timer==255)
            {
                *damage=entity->ext.belmont.attackOff;
                entity->state=2;
                entity->stage=0;
                entity->subState=0;
            }
            break;
        }
    }
}
