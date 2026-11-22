/* Belmont leap and airborne attack selection, with landing recovery. */
#include "code_08009A0.h"
#include "belmont_entity.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s32 Entity_MoveWithCollisionProfile(struct EwramData_EntityData *,s16 *,u16,s32);
extern s32 Entity_ClampXToRoom(struct EwramData_EntityData *,s32);
extern s32 Entity_IsTargetInCenteredBox(struct EwramData_EntityData *,s32,s32);
extern void Afterimage_Create(struct EwramData_EntityData *,s32,s32,s32);
extern struct EwramData_EntityData *Belmont_CreateMovementEffect(struct EwramData_EntityData *);
extern s16 sBelmontMoveProfile[];
/* Retain the facing test present in the original collision response. */
#define STOP_HORIZONTAL(e) { \
    if((e)->flipX) \
        (e)->accelX.whole=(e)->velX.whole=0; \
    else \
        (e)->accelX.whole=(e)->velX.whole=0; \
}
/**
 * @brief Jump state (4): leap, choose airborne attacks and afterimages,
 * resolve terrain and recover on landing. Preserve the duplicated
 * facing branches and existing effect allocation assumptions.
 */
void Belmont_UpdateJumpAttack(struct EwramData_EntityData *entity)
{
    s32 collision;
    switch(entity->subState)
    {
        case 0:
        {
            s32 zero;
            s32 speed;
            PlaySong(0x125);
            zero=0;
            entity->accelX.whole=zero;
            entity->accelY.whole=zero;
            entity->velY.whole=zero;
            speed=0x10000;
            entity->velX.whole=speed;
            if(entity->flipX)
                entity->velX.whole=-speed;
            entity->accelY.whole=0x1800;
            entity->velY.whole=entity->accelY.whole * -35;
            entity->subState++;
            entity->stage=zero;
            if(entity->animScriptIndex!=3)
                Anim_StartIndexed(entity,3,1,0);
            break;
        }
        case 1:
            collision=Entity_MoveWithCollisionProfile(entity,sBelmontMoveProfile,4,0);
            if(collision & 3)
                STOP_HORIZONTAL(entity);
            if(Entity_ClampXToRoom(entity,1))
                STOP_HORIZONTAL(entity);
            if(entity->velY.whole >= -(entity->accelY.whole<<1))
            {
                entity->subState++;
                entity->stage=0;
            }
            break;
        case 2:
            if(entity->stage==0)
            {
                if(entity->animScriptIndex!=16)
                    Anim_StartIndexed(entity,16,1,0);
                Anim_Update(entity);
                entity->timer=4;
                entity->stage++;
            }
            else
            {
                Entity_MoveWithCollisionProfile(entity,sBelmontMoveProfile,4,0);
                if(--entity->timer==255)
                {
                    u32 roll=RandomNumberGenerator() & 1;
                    if(roll==0)
                    {
                        s32 speed=-0x30000;
                        entity->velX.whole=speed;
                        entity->velY.whole=0x20000;
                        entity->accelX.whole=-0x400;
                        entity->accelY.whole=0x400;
                        if(entity->flipX)
                        {
                            entity->velX.whole=-speed;
                            entity->accelX.whole=0x400;
                        }
                        entity->attack=80;
                        if(entity->animScriptIndex!=9)
                            Anim_StartIndexed(entity,9,1,0);
                        PlaySong(0x1EC);
                        Anim_Update(entity);
                        Afterimage_Create(entity,28,5,2);
                        Afterimage_Create(entity,28,3,2);
                    }
                    else
                    {
                        if(entity->animScriptIndex!=17)
                            Anim_StartIndexed(entity,17,1,0);
                        Anim_Update(entity);
                    }
                    entity->subState++;
                    entity->stage=0;
                }
            }
            break;
        case 3:
            if(entity->stage==0)
                entity->stage++;
            collision=Entity_MoveWithCollisionProfile(entity,sBelmontMoveProfile,4,0);
            if(collision & 3)
                STOP_HORIZONTAL(entity);
            if(Entity_ClampXToRoom(entity,1))
                STOP_HORIZONTAL(entity);
            if(collision & 8)
            {
                entity->accelX.whole=0;
                entity->velX.whole=0;
                entity->accelY.whole=0;
                entity->velY.whole=0;
                entity->subState++;
                entity->stage=0;
                entity->attack=BELMONT_ATTACK_OFF(entity);
            }
            break;
        case 4:
            switch(entity->stage)
            {
                case 0:
                    PlaySong(0x126);
                    entity->stage++;
                case 1:
                    if(entity->animScriptIndex!=4)
                        Anim_StartIndexed(entity,4,1,0);
                    entity->stage++;
                    entity->timer=16;
                case 2:
                    if((u8)(entity->timer % 5)==0)
                    {
                        struct EwramData_EntityData *effect=Belmont_CreateMovementEffect(entity);
                        u32 random=RandomNumberGenerator();
                        u16 *x=&effect->posX.part16.integer;
                        s32 jitter=(random & 15)-8;
                        *x-=jitter;
                        effect->posY.part16.integer+=2;
                    }
                    if(--entity->timer==255)
                    {
                        entity->state=2;
                        entity->stage=0;
                        entity->subState=0;
                    }
                    break;
            }
            break;
        case 5:
        {
            s32 zero=entity->stage;
            if(zero==0)
            {
                s32 speed;
                PlaySong(0x125);
                entity->accelX.whole=zero;
                entity->accelY.whole=zero;
                entity->velY.whole=zero;
                speed=-0x10000;
                entity->velX.whole=speed;
                if(entity->flipX)
                    entity->velX.whole=-speed;
                entity->accelY.whole=0x1800;
                entity->velY.whole=entity->accelY.whole * -35;
                if(entity->animScriptIndex!=3)
                    Anim_StartIndexed(entity,3,1,0);
                entity->stage++;
            }
            collision=Entity_MoveWithCollisionProfile(entity,sBelmontMoveProfile,4,0);
            if(collision & 3)
                STOP_HORIZONTAL(entity);
            if(Entity_ClampXToRoom(entity,1))
                STOP_HORIZONTAL(entity);
            if(entity->velY.whole >= -(entity->accelY.whole<<1))
            {
                entity->subState=2;
                entity->stage=0;
            }
            if(Entity_IsTargetInCenteredBox(entity,128,64))
            {
                entity->subState=6;
                entity->stage=0;
                entity->attack=BELMONT_ATTACK(entity);
            }
            break;
        }
        case 6:
            collision=Entity_MoveWithCollisionProfile(entity,sBelmontMoveProfile,4,0);
            if(collision & 3)
                STOP_HORIZONTAL(entity);
            if(Entity_ClampXToRoom(entity,1))
                STOP_HORIZONTAL(entity);
            if(collision & 8)
            {
                entity->accelX.whole=0;
                entity->velX.whole=0;
                entity->accelY.whole=0;
                entity->velY.whole=0;
                entity->subState=4;
                entity->stage=0;
                entity->attack=BELMONT_ATTACK_OFF(entity);
            }
            else
            {
                switch(entity->stage)
                {
                    case 0:
                        if(entity->animScriptIndex!=5)
                            Anim_StartIndexed(entity,5,1,0);
                        entity->stage++;
                        break;
                    case 1:
                        if(entity->animStep==8)
                        {
                            PlaySong(0xFC);
                            if(RandomNumberGenerator()%3==0)
                                PlaySong(0x1DF);
                        }
                        if(entity->animDone)
                        {
                            if(entity->velY.whole>=(s32)((u32)(-entity->accelY.whole)<<1) && entity->velY.whole<=entity->accelY.whole*2)
                            {
                                entity->subState=2;
                                entity->stage=0;
                            }
                            else
                            {
                                if(entity->animScriptIndex!=17)
                                    Anim_StartIndexed(entity,17,1,0);
                                entity->subState=3;
                                entity->stage=0;
                            }
                            entity->attack=BELMONT_ATTACK_OFF(entity);
                        }
                        break;
                }
            }
            break;
    }
}
