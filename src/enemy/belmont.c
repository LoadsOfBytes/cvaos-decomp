/*
 * Belmont (boss): scene, combat states, attacks, projectile waves, power particles, subweapons and fragments.
 */
#include "core.h"
#include "belmont_entity.h"
#include "in_game.h"
#include "graphics.h"
#include "structs/ewram.h"
#include "player_core.h"
#include "structs/entity_template.h"
#include "sound.h"
#include "music.h"
#include "map_flags.h"
#include "player_level.h"
#include "entity_util.h"
#include "effects.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "math_anim.h"
#include "oam.h"
#include "obj_gfx.h"
#include "obj_palette.h"
#include "shop_update.h"
#include "data/data_118B48.h"
#include "data/data_4F1444.h"

/* Belmont initialization distinguishes scene entry and combat entry. */
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *,u8 *,void *,u8,void *);
extern void Entity_EnableBoxes(struct EwramData_EntityData *,void *,s32);
extern void Enemy_OnContact(struct EwramData_EntityData *);
void Belmont_OnHit(struct EwramData_EntityData *entity,s32 kind,u16 *hit);
void EnemyBelmontCreate(struct EwramData_EntityData *entity)
{
    void *palette=sJuliusEntityPalette;
    void *sprite=sJuliusSpriteSheet;
    u8 *graphics=sBelmontGfx;
    s32 kind=entity->spawnParam0;
    u8 display;
    s32 zero;
    u8 *layer;
    if(kind==1)
    {
        if(!gEwramData->gameState.bestiaryPreview)
        {
            u16 completed;
            entity->ext.belmont.flags|=0x10;
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
    entity->ext.belmont.attack=entity->attack;
    entity->ext.belmont.attackOff=display;
    entity->attack=display;
    Entity_SnapToGround(entity);
    Entity_EnableBoxes(entity,Enemy_OnContact,(s32)Belmont_OnHit);
}

/* Spawn Belmont's scene object from a table and mark its event bit. */
struct BelmontSceneObject { s16 x,y; u8 event,pad5,kind,pad7; u16 param1,param2; };
extern struct BelmontSceneObject sBelmontSceneObjects[];
/**
 * @brief When scene flag 0x10 is set, create the indexed table-configured
 * object 0x20, link it to Belmont and set its event bit. Retain room-
 * relative positioning for entry 0 and owner positioning for later entries.
 */
void Belmont_CreateSceneObject(struct EwramData_EntityData *entity,s32 index)
{
    struct BelmontSceneObject *info=&sBelmontSceneObjects[index];
    if(entity->ext.belmont.flags & 0x10)
    {
        struct EwramData_EntityData *object=EntityCreateInRange(0x1B,0x48,(u32 *)EntityUpdateNothing);
        if(object)
        {
            s32 facing;
            entity->ext.belmontScene.objectIndex=ENTITY_INDEX(object);
            ENTITY_PARENT_INDEX(object)=ENTITY_INDEX(entity);
            if(index==0)
            {
                s32 x=info->x<<16;
                struct BgLayerState *bg=&gEwramData->bgInfo[1];
                object->posX.whole=x-bg->xPos.whole;
                object->posY.whole=(info->y<<16)-bg->yPos.whole;
            }
            else
            {
                object->posX.whole=entity->posX.whole;
                object->posY.whole=entity->posY.whole;
            }
            facing=0;
            if(object->posX.whole<gEwramData->playerState.player->posX.whole)
                facing=1;
            object->flipX=facing;
            object->scrollWithBg=1;
            object->spawnParam0=info->param1;
            object->spawnParam1=info->param2;
            object->spawnIndex=info->event;
            object->enemyId=info->kind;
            object->drawFunc=(u32 *)Sprite_DrawEntity;
            object->updateFunc=(u32 *)Object20Update;
            Object20Create(object);
            gEwramData->gameState.roomEntitySpawn.spawnedFlags[info->event>>5] |= 1 << (info->event & 31);
        }
    }
}

/* Belmont introduction waits, creates the scene object, then starts combat. */
/**
 * @brief Intro state (1): gate menus, supply player input, wait and create
 * scene entry 0, then reinitialize Belmont for combat when the scene
 * object is gone. Preserve the existing slot/pointer tests.
 */
void Belmont_UpdateIntro(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *object=&gEwramData->entityData[entity->ext.belmontScene.objectIndex];
    Game_SetMenusEnabled(0);
    if(entity->stage<=3)
        SetPlayerInput(1,0x20);
    switch(entity->stage)
    {
        case 0:
            entity->timer=56;
            entity->stage++;
            break;
        case 2:
            entity->hidden=1;
            Belmont_CreateSceneObject(entity,0);
            entity->stage++;
            entity->timer=32;
            break;
        case 1:
        case 3:
            if(--entity->timer==0xFF)
                entity->stage++;
            break;
        case 4:
            if(object==NULL)
            {
                entity->spawnParam0=0;
                EnemyBelmontCreate(entity);
            }
            else if(object->updateFunc==NULL || object->enemyId!=0x20)
            {
                entity->spawnParam0=0;
                EnemyBelmontCreate(entity);
            }
            break;
    }
}

/* Belmont scene setup and player position at the room boundary. */
#define PLAYER (gEwramData->playerState.player)
/**
 * @brief Scene state (13): load the display animation, create scene entry
 * 2 at the player boundary and hold the player there while the object
 * remains active.
 */
void Belmont_UpdateSceneBoundary(struct EwramData_EntityData *entity)
{
    void *palette=sJuliusEntityPalette;
    void *sprite=sJuliusSpriteSheet;
    u8 *graphics=sBelmontGfx;
    struct BgLayerState *bg=&gEwramData->bgInfo[1];
    u16 boundary=192-bg->xPos.part16.integer;
    {
        s32 state=entity->subState;
        if(state==1) goto active;
        if(state>1) return;
        if(state!=0) return;
            if(!Entity_LoadGraphics(entity,graphics,palette,0,sprite))
            {
                EntityDelete(entity);
                return;
            }
            if(entity->animScriptIndex!=4)
                Anim_StartIndexed(entity,4,1,0);
            Anim_Update(entity);
            entity->spriteLayer=17;
            entity->subState++;
active:
            if((s16)PLAYER->posX.part16.integer<(s16)boundary)
            {
                struct EwramData_EntityData *object;
                if(entity->stage==0)
                {
                    Belmont_CreateSceneObject(entity,2);
                    entity->stage++;
                }
                object=&gEwramData->entityData[entity->ext.belmontScene.objectIndex];
                if(object->updateFunc!=NULL)
                {
                    PLAYER->posX.part16.integer=boundary;
                    PLAYER->velX.whole=0;
                    PLAYER->accelX.whole=0;
                    PLAYER->flipX=1;
                }
            }
    }
}

/* Belmont dispatch, hit flashing, collision and active hitbox gating. */
extern void Belmont_UpdateBestiaryPreview(struct EwramData_EntityData *);
extern void Belmont_UpdateDefeat(struct EwramData_EntityData *);
extern void Belmont_UpdateDodge(struct EwramData_EntityData *);
extern void Belmont_UpdateHitReaction(struct EwramData_EntityData *);
extern void Belmont_EnterCombat(struct EwramData_EntityData *);
extern void Belmont_UpdateCombat(struct EwramData_EntityData *);
extern void Belmont_UpdateMeleeAttack(struct EwramData_EntityData *);
extern void Belmont_UpdateJumpAttack(struct EwramData_EntityData *);
extern void Belmont_UpdateWaveAttack(struct EwramData_EntityData *);
extern void Belmont_UpdateSubweaponAttack(struct EwramData_EntityData *);
extern void Belmont_UpdateJumpSlide(struct EwramData_EntityData *);
extern void Belmont_UpdatePoweredPhase(struct EwramData_EntityData *);
extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *);
#define FLASH(e) ((e)->ext.raw.unk_4F8.part8.unk_4F9)
void EnemyBelmontUpdate(struct EwramData_EntityData *entity)
{
    if(entity->state==1) { Belmont_UpdateIntro(entity); return; }
    if(entity->state==13) { Belmont_UpdateSceneBoundary(entity); return; }
    if(entity->state==12) { Belmont_UpdateBestiaryPreview(entity); return; }
    if(entity->state==0xF0) { Belmont_UpdateDefeat(entity); return; }
    if(entity->state==9) { Belmont_UpdateDodge(entity); return; }
    if(entity->state==11) { Belmont_UpdateHitReaction(entity); Entity_AnimateWithBoxes(entity); return; }
    if(!(entity->enemyFlags & 2))
    {
        if(Entity_IsInRect(entity,-16,0,272,176))
            Player_TrackEntity(entity);
        Enemy_UpdateHitFlash(entity);
    }
    if(FLASH(entity)!=0)
    {
        { u8 value=FLASH(entity) & 1; entity->hidden=value; }
        FLASH(entity)--;
    }
    else
        entity->hidden=0;
    switch(entity->state)
    {
        case 3: Belmont_EnterCombat(entity); break;
        case 2: Belmont_UpdateCombat(entity); break;
        case 7: Belmont_UpdateMeleeAttack(entity); break;
        case 4: Belmont_UpdateJumpAttack(entity); break;
        case 5: Belmont_UpdateWaveAttack(entity); break;
        case 6: Belmont_UpdateSubweaponAttack(entity); break;
        case 10: Belmont_UpdateJumpSlide(entity); break;
        case 8: Belmont_UpdatePoweredPhase(entity); break;
    }
    Entity_AnimateWithBoxes(entity);
    if(entity->attack==0)
        entity->hit.flags|=2;
    else
        entity->hit.flags&=0xFD;
    if(entity->state!=11 && FLASH(entity)==0)
        Enemy_CheckHitPlayer(entity);
}

/* Belmont_UpdateCombat. */
/* Belmont combat decisions: proximity, health phase, turns and attacks. */
extern s32 Entity_IsOffsetOutsideRoom(struct EwramData_EntityData *,s32,s32);
extern s32 Entity_IsTargetInCenteredBox(struct EwramData_EntityData *,s32,s32);
#define FLAGS(e) ((e)->ext.raw.unk_4F8.part8.unk_4F8)
#define COOLDOWN(e) ((e)->ext.raw.unk_504.part16.unk_506[0])
/**
 * @brief Combat state (2): update movement/cooldown and choose attacks,
 * dodges or the low-HP powered phase from proximity, player state,
 * room bounds and randomness. Preserve the existing health threshold tests.
 */
void Belmont_UpdateCombat(struct EwramData_EntityData *entity)
{
    s32 facing=entity->flipX;
    u32 work;
    s32 next;
    switch(entity->subState)
    {
        case 0:
        case 1:
            if((u16)COOLDOWN(entity)!=0)
                COOLDOWN(entity)--;
            if(entity->animScriptIndex!=1)
                Anim_StartIndexed(entity,1,1,1);
            if(!(Entity_TryWalk(entity,0x20000,0x200000,0x40000,0)&4))
                entity->posY.whole+=entity->velY.whole;
            if((s16)entity->hp < (u16)((u32)sEnemyData[entity->enemyId].hp/3))
            {
                if(Entity_IsOffsetOutsideRoom(entity,128,0) || Entity_IsOffsetOutsideRoom(entity,-128,0))
                {
                    if(Entity_IsOffsetOutsideRoom(entity,128,0))
                        entity->flipX=0;
                    else
                        entity->flipX=1;
                    entity->state=9;
                    entity->stage=0;
                    entity->subState=0;
                    return;
                }
                {
u8 mask=FLAGS(entity)&8; if(mask==0){ entity->state=8; entity->stage=mask; entity->subState=mask; FLAGS(entity)|=8;
                        gEwramData->gameState.stateFlags|=0x1000;
                        break;
                    }
                }
            }
            if(Entity_IsTargetInCenteredBox(entity,32,48))
            {
                work=RandomNumberGenerator()%12;
                if(work==0) { entity->state=9; entity->stage=work; entity->subState=work; return; }
                work=RandomNumberGenerator()%8;
                if(work==0) { entity->state=4; entity->stage=work; entity->subState=work; return; }
                work=RandomNumberGenerator()%4;
                if(work==0) { entity->state=10; entity->stage=work; entity->subState=work; return; }
            }
            if(entity->subState==1)
            {
                if(entity->flipX!=facing)
                {
                    entity->subState++;
                    entity->stage=0;
                    break;
                }
            }
            else
                entity->subState=1;
            { s32 nearby=Entity_IsTargetInCenteredBox(entity,128,64);
            if(nearby)
            {
                struct EwramData_EntityData *player=gEwramData->playerState.player;
                if(player->state==1 && player->flipX==entity->flipX && player->animStep==0)
                {
                    work=RandomNumberGenerator()%8;
                    if(work==0) { entity->state=9; entity->stage=work; entity->subState=work; return; }
                    work=RandomNumberGenerator()%6;
                    if(work==0) { entity->state=4; entity->stage=work; entity->subState=work; return; }
                }
                work=RandomNumberGenerator()%10;
                if(work==0) { entity->state=7; entity->stage=work; entity->subState=work; return; }
                work=RandomNumberGenerator()%50;
                if(work==0)
                {
                    entity->state=10;
                    entity->stage=work;
                    entity->subState=work;
                    return;
                }
            }
            else if((Player_GetFocusedActionFlags()&2) && RandomNumberGenerator()%60==20)
            {
                entity->state=4;
                entity->stage=nearby;
                entity->subState=5;
                break;
            }
            }
            if((FLAGS(entity)&8) && Entity_IsTargetInCenteredBox(entity,256,128) && RandomNumberGenerator()%30==0 && (u16)COOLDOWN(entity)==0)
            {
                COOLDOWN(entity)=RandomNumberGenerator()%4;
                entity->state=6;
                entity->stage=0;
                entity->subState=0;
            }
            break;
        case 2:
        {
            s32 roll;
            if(entity->stage==0)
            {
                entity->timer=10;
                entity->stage++;
            }
            Entity_TryWalk(entity,0x4000,0x10000,0x10000,0);
            roll=RandomNumberGenerator()&3;
            if(roll==0 && Entity_IsTargetInCenteredBox(entity,32,256) && (Player_GetFocusedActionFlags()&4))
            {
                entity->state=4;
                entity->stage=roll;
                entity->subState=roll;
                break;
            }
            if(--entity->timer==0xFF)
            {
                entity->subState--;
                entity->stage=0;
            }
            break;
        }
    }

}

/* Belmont melee strike: animation choice, sound cues and active hitbox. */
/**
 * @brief Melee state (7): select the strike animation, play event sounds
 * and gate contact damage by animation step, then return to combat.
 */
void Belmont_UpdateMeleeAttack(struct EwramData_EntityData *entity)
{
    switch(entity->subState)
    {
        case 0:
            if(Player_GetFocusedActionFlags() & 0x400)
            {
                if(entity->animScriptIndex!=6)
                    Anim_StartIndexed(entity,6,1,0);
            }
            else
            {
                if(entity->animScriptIndex!=5)
                    Anim_StartIndexed(entity,5,1,0);
            }
            entity->subState++;
            entity->stage=0;
        case 1:
        {
            u8 *frame=&entity->animStep;
            if(*frame==4)
            {
                u16 *songs=sBelmontMeleeSounds;
                PlaySong(songs[RandomNumberGenerator()%2]);
            }
            if(*frame==8)
                PlaySong(0xFC);
            if(*frame>7)
                entity->attack=entity->ext.belmont.attack;
            else
                entity->attack=0;
            if(entity->animDone)
            {
                entity->state=2;
                entity->stage=0;
                entity->subState=0;
                entity->attack=entity->ext.belmont.attackOff;
            }
            break;
        }
    }
}

/* Belmont leap and airborne attack selection, with landing recovery. */
extern s32 Entity_ClampXToRoom(struct EwramData_EntityData *,s32);
extern void Afterimage_Create(struct EwramData_EntityData *,s32,s32,s32);
extern struct EwramData_EntityData *Belmont_CreateMovementEffect(struct EwramData_EntityData *);
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
                entity->attack=entity->ext.belmont.attackOff;
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
                entity->attack=entity->ext.belmont.attack;
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
                entity->attack=entity->ext.belmont.attackOff;
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
                            entity->attack=entity->ext.belmont.attackOff;
                        }
                        break;
                }
            }
            break;
    }
}

/* Delayed wave projectile: rises, alternates two frames, then expires. */
/**
 * @brief Wait for the initial delay, rise with gravity and alternate
 * frames 7/8, then remove at the existing downward-velocity threshold.
 * Keep player hit checks throughout the state machine.
 */
void BelmontWaveProjectile_Update(struct EwramData_EntityData *entity)
{
    s32 state=entity->state;
    switch(state)
    {
        case 0:
            if(--entity->timer==0xFF)
            {
                entity->state++;
                entity->stage=state;
                entity->subState=state;
            }
            break;
        case 1:
            entity->hidden=0;
            entity->state++;
            entity->stage=0;
            entity->subState=0;
            entity->velY.whole=-0x1F000;
            entity->accelY.whole=0x1000;
            entity->animFrame=7;
            break;
        case 2:
            if((entity->timer++ & 1)!=0)
            {
                if(entity->animFrame==7)
                    entity->animFrame=8;
                else
                    entity->animFrame=7;
            }
            entity->posY.whole+=entity->velY.whole;
            entity->velY.whole+=entity->accelY.whole;
            if(entity->velY.whole==0x20000)
            {
                EntityDelete(entity);
                return;
            }
            break;
    }
    entity->hit.flags|=4;
    Enemy_CheckHitPlayer(entity);
}

/* Belmont projectile wave: nineteen delayed entities around the owner. */
#define ABS(x) ((x)<0?-(x):(x))
/**
 * @brief Create up to nineteen delayed wave projectiles at spaced offsets
 * from the owner, copying its attack damage. Stop on allocation or
 * graphics failure and preserve the asymmetric loop range -10 through 9.
 */
void Belmont_SpawnProjectileWave(struct EwramData_EntityData *entity)
{
    void *palette=sJuliusPalette;
    void *sprite=sBelmontSpriteSheet;
    u8 *graphics=sBelmontWeaponGfx;
    s32 i=-10;
    s32 offset=-0x8C0000;
    do
    {
        struct EwramData_EntityData *projectile;
        if(i!=0)
        {
        projectile=Entity_CreateObject(BelmontWaveProjectile_Update);
        if(!projectile)
            return;
        if(!Entity_LoadGraphics(projectile,graphics,palette,1,sprite))
        {
            EntityDelete(projectile);
            return;
        }
        projectile->posX.whole=entity->posX.whole+offset;
        projectile->posY.whole=entity->posY.whole+0x200000;
        projectile->timer=ABS(i);
        ENTITY_PARENT_INDEX(projectile)=ENTITY_INDEX(entity);
        projectile->spriteLayer=25;
        projectile->flipX=entity->flipX;
        projectile->scrollWithBg=1;
        projectile->animFrame=projectile->basePalette;
        projectile->hidden=1;
        Entity_EnableBoxes(projectile,Enemy_OnContact,0);
        projectile->hit.flags|=4;
        projectile->ext.belmont.attack=entity->ext.belmont.attack;
        projectile->attack=entity->ext.belmont.attack;
        }
        offset+=0xE0000;
        i++;
    } while(i<=9);
}

/* Belmont charged attack waits on animation and emits a projectile wave. */
/**
 * @brief Wave state (5): sequence charge/release animations and countdowns,
 * spawn the projectile wave, then recover to combat.
 */
void Belmont_UpdateWaveAttack(struct EwramData_EntityData *entity)
{
    s32 state=entity->subState;
    switch(state)
    {
        case 0:
            if(entity->stage==0)
            {
                Anim_StartIndexed(entity,7,1,0);
                entity->stage++;
            }
            if(entity->animFrame==30)
            {
                entity->subState++;
                entity->stage=state;
                entity->animFlags|=2;
            }
            break;
        case 1:
            if(entity->stage==0)
            {
                entity->timer=24;
                entity->stage++;
            }
            if(--entity->timer==0xFF)
            {
                entity->subState++;
                entity->stage=0;
            }
            break;
        case 2:
            if(entity->stage==0)
            {
                Anim_StartIndexed(entity,4,1,0);
                entity->stage++;
            }
            if(entity->animDone)
            {
                entity->subState++;
                entity->stage=0;
                Belmont_SpawnProjectileWave(entity);
            }
            break;
        case 3:
            if(entity->stage==0)
            {
                entity->timer=128;
                entity->stage++;
            }
            if(--entity->timer==0xFF)
            {
                entity->state=2;
                entity->stage=0;
                entity->subState=0;
            }
            break;
    }
}

/* Belmont subweapon attack: telegraph, timed effects, throw, recovery. */
extern struct EwramData_EntityData *Belmont_CreateSubweapon(struct EwramData_EntityData *,s32);
/**
 * @brief Subweapon state (6): select a variant from target position,
 * telegraph with timed effects and spawn at frame 32 before recovery.
 */
void Belmont_UpdateSubweaponAttack(struct EwramData_EntityData *entity)
{
    s32 state=entity->subState;
    switch(state)
    {
        case 0:
            Anim_StartIndexed(entity,7,1,0);
            if((s16)Entity_GetTargetDistanceY(entity)>16)
                entity->stage=state;
            else if(Entity_IsTargetInCenteredBox(entity,160,32))
                entity->stage=2;
            else
                entity->stage=1;
            entity->subState++;
            entity->timer=20;
            break;
        case 1:
            if(entity->animStep==4)
                entity->animFlags|=2;
            if(--entity->timer==255)
            {
                entity->animFlags&=0xFD;
                entity->subState++;
            }
            if((entity->timer & 3)==0)
            {
                s32 offset=0x80000;
                s32 variant=entity->stage;
                s32 kind=7;
                s32 position;
                u32 random;
                if(variant!=0)
                {
                    kind=9;
                    if(variant==1)
                        kind=6;
                }
                if(entity->flipX)
                    offset=-offset;
                random=RandomNumberGenerator();
                position=entity->posX.whole+offset;
                position+=random%0x60000;
                position-=0x30000;
                Effect_CreateFromTable(position,entity->posY.whole-0x200000,0,-0x2000,4,kind);
            }
            break;
        case 2:
            if(entity->animFrame==32)
            {
                switch(entity->stage)
                {
                    case 0: Belmont_CreateSubweapon(entity,0); break;
                    case 1: Belmont_CreateSubweapon(entity,1); break;
                    case 2: Belmont_CreateSubweapon(entity,2); break;
                }
                entity->subState++;
                entity->stage=0;
            }
            break;
        case 3:
            if(entity->animDone)
            {
                entity->state=2;
                entity->stage=0;
                entity->subState=0;
            }
            break;
    }
}

/* Particle movement toward its owner, retaining the original termination checks.
 * The owner register and cached X coordinate preserve agbcc's register use.
 * Matching does not establish the original source spelling. */

/**
 * @brief Step toward the owner using the configured signed pixel step.
 * Remove on the existing X or particle-relative Y test, or timer
 * underflow. Keep the pre-move Y comparison and cached coordinates.
 */
void BelmontPowerParticle_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner =
        &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    s8 step = entity->ext.belmontParticle.step;
    u16 initialYMinus32 = entity->posY.part16.integer - 32;

    if ((s16)owner->posX.part16.integer > (s16)entity->posX.part16.integer)
        entity->posX.part16.integer += step;
    else
        entity->posX.part16.integer -= step;
    if ((s16)owner->posY.part16.integer - 32 > (s16)entity->posY.part16.integer)
        entity->posY.part16.integer += step;
    else
        entity->posY.part16.integer -= step;
    {
        s32 ownX = (s16)entity->posX.part16.integer;
        s32 ownerX = (s16)owner->posX.part16.integer;

        /* The Y window uses this particle's position before movement,
         * minus 32, rather than the owner's Y coordinate. */
        if ((ownX < ownerX + 4 && ownX > ownerX - 4) ||
            ((s16)entity->posY.part16.integer < (s16)initialYMinus32 + 4 &&
             (s16)entity->posY.part16.integer > (s16)initialYMinus32 - 4))
            do { EntityDelete(entity); } while (0); /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
        else if (--entity->timer == 255)
            EntityDelete(entity);
    }
}

/* Randomly scattered visual effect linked back to Belmont. */
/**
 * @brief Create a randomized frame-64 particle linked to the owner,
 * with configured movement step 1 and initial countdown 32.
 */
struct EwramData_EntityData *Belmont_CreatePowerParticle(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *effect=EntityCreateInRange(0x49,0xCF,(u32 *)BelmontPowerParticle_Update);
    if(effect)
    {
        u32 random;
        s32 graphics=ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(effect,sCommonObjectSpriteSheet,graphics,6);
        effect->animFrame=64;
        ENTITY_PARENT_INDEX(effect)=ENTITY_INDEX(entity);
        effect->ext.belmontParticle.step=1;
        {
            u32 jitter=RandomNumberGenerator();
            s32 position=entity->posX.whole+0x800000;
            s32 offset=(jitter&0x3FFFFF)<<2;
            effect->posX.whole=position-offset;
        }
        {
            u32 jitter=RandomNumberGenerator();
            s32 position=entity->posY.whole+0x800000;
            s32 offset=(jitter&0x3FFFFF)<<2;
            effect->posY.whole=position-offset;
        }
        effect->spriteLayer=entity->spriteLayer-1;
        effect->scrollWithBg=1;
        effect->timer=32;
        effect->drawFunc=(u32 *)Sprite_DrawEntity;
    }
    return effect;
}

/* Belmont's powered sequence with its attached weapon and rising particles. */
extern s32 Belmont_CreateAttachedWeapon(struct EwramData_EntityData *);
/* This original call site consumes the sign-extended value in all of r0. */
extern s32 Collision_GetPushUp(s32,s32);
/**
 * @brief Powered state (8): create the attached weapon, sequence tracked
 * sounds and rising particles, then descend and clean up the weapon
 * on ground contact. Keep original parent tests and raw event calls.
 */
void Belmont_UpdatePoweredPhase(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *weapon=&gEwramData->entityData[entity->ext.belmontWeapon.index];
    s32 state=entity->subState;
    switch(state)
    {
        case 0:
            entity->ext.belmont.powerStep++;
            if(!Belmont_CreateAttachedWeapon(entity))
            {
                entity->state=2;
                entity->stage=state;
                entity->subState=state;
                return;
            }
            entity->accelX.whole=state;
            entity->velX.whole=state;
            entity->accelY.whole=state;
            entity->velY.whole=-0x2000;
            entity->subState++;
            entity->stage=state;
            weapon=&gEwramData->entityData[entity->ext.belmontWeapon.index];
            if(weapon->enemyId==entity->enemyId && weapon)
                Player_RegisterSolidObject(weapon,sBelmontWeaponSolidCallbacks);
            break;
        case 1:
            if(entity->stage==0)
            {
                Sound_PlayTracked(250);
                if(entity->animScriptIndex!=10)
                    Anim_StartIndexed(entity,10,1,1);
                entity->stage++;
                entity->timer=255;
            }
            else
            {
                state &= entity->timer;
                if(state)
                    Belmont_CreatePowerParticle(entity);
                if(--entity->timer==255)
                {
                    entity->velY.whole=0;
                    entity->subState++;
                    entity->stage=0;
                }
            }
            break;
        case 2:
        {
            s32 inner=entity->stage;
            if(inner==0)
            {
                if(weapon->animScriptIndex!=0)
                    Anim_StartIndexed(weapon,0,0,0);
                weapon->state++;
                weapon->stage=inner;
                weapon->subState=inner;
                Sound_StopTracked(250);
                Sound_PlayTracked(251);
                if(entity->animScriptIndex!=11)
                    Anim_StartIndexed(entity,11,1,1);
                entity->timer=255;
                entity->stage++;
            }
            else
            {
                if(entity->timer & 1)
                {
                    struct EwramData_EntityData *effect=Belmont_CreatePowerParticle(entity);
                    if(effect)
                        effect->ext.belmontParticle.step=state;
                }
                if(weapon->animDone && weapon->animScriptIndex!=1)
                    Anim_StartIndexed(weapon,1,0,1);
                if(--entity->timer==255)
                {
                    entity->subState++;
                    entity->stage=0;
                }
            }
            break;
        }
        case 3:
        {
            
            if(entity->stage==0)
            {
                entity->velY.whole=0x8000;
                entity->stage++;
                weapon->drawSkip=1;
            }
            else
            {
              s32 x=(s16)entity->posX.part16.integer;
              struct BgLayerState *camera=&gEwramData->bgInfo[1];
              x+=(s16)camera->xPos.part16.integer;
              if(Collision_GetPushUp(x,(s16)entity->posY.part16.integer+(s16)camera->yPos.part16.integer))
              {
                Sound_StopTracked(251);
                if(entity->animScriptIndex!=0)
                    Anim_StartIndexed(entity,0,1,1);
                entity->state=2;
                entity->stage=0;
                entity->subState=0;
                entity->ext.belmontWeapon.index=0;
                weapon->deleteRequested=1;
                Player_ClearSolidObject();
              }
            }
            break;
        }
    }
    entity->posX.whole+=entity->velX.whole;
    entity->posY.whole+=entity->velY.whole;
    entity->velX.whole+=entity->accelX.whole;
    entity->velY.whole+=entity->accelY.whole;
}

/* Belmont backward dodge with timed flashing and collision handling. */
extern void Entity_FaceTarget(struct EwramData_EntityData *);
/**
 * @brief Dodge state (9): create afterimages, move backward relative to
 * facing with timed flashing and terrain bounds handling, then return
 * to combat. Updates animation internally.
 */
void Belmont_UpdateDodge(struct EwramData_EntityData *entity)
{
    s32 state=entity->subState;
    switch(state)
    {
        case 0:
            entity->palette=entity->basePalette;
            entity->hitFlashTimer=state;
            entity->ext.belmont.flash=state;
            entity->ext.belmont.hits=state;
            if(entity->animScriptIndex!=3)
                Anim_StartIndexed(entity,3,1,1);
            Anim_Update(entity);
            entity->subState++;
            entity->stage=state;
            Afterimage_Create(entity,48,5,2);
            Afterimage_Create(entity,48,3,2);
            break;
        case 1:
            if(entity->animDone)
            {
                PlaySong(0x128);
                if(entity->flipX)
                {
                    entity->velX.whole=0x37000;
                    entity->accelX.whole=-0xB00;
                }
                else
                {
                    entity->velX.whole=-0x37000;
                    entity->accelX.whole=0xB00;
                }
                entity->subState++;
                entity->stage=0;
                entity->velY.whole=0x10000;
                entity->timer=80;
            }
            break;
        case 2:
        {
            s32 collision;
            Entity_FaceTarget(entity);
            { u8 value=entity->timer & 1; entity->hidden=value; }
            collision=Entity_MoveWithCollisionProfile(entity,sBelmontMoveProfile,4,0);
            if(!(collision & 8))
                entity->posY.whole+=0x10000;
            if(collision & 3)
            {
                if(entity->flipX)
                    entity->accelX.whole=entity->velX.whole=0;
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
            if(--entity->timer==255)
            {
                entity->state=2;
                entity->stage=0;
                entity->subState=0;
                entity->objMode=0;
                entity->timer=0;
                entity->hidden=0;
            }
            break;
        }
    }
    Anim_Update(entity);
}

/* Belmont_UpdateJumpSlide: Belmont jump, landing and following attack/recovery sequence. */
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

/* Short visual effect attached to Belmont's current position. */
extern void BelmontMovementEffect_Update(struct EwramData_EntityData *);
/**
 * @brief Create an animation-4 visual effect slightly above the owner
 * using shared effect graphics. Return it or NULL on entity allocation failure.
 */
struct EwramData_EntityData *Belmont_CreateMovementEffect(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *effect=EntityCreateInRange(0x49,0xCF,(u32 *)BelmontMovementEffect_Update);
    if(effect)
    {
        s32 graphics=ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(effect,sCommonObjectSpriteSheet,graphics,6);
        Anim_StartIndexed(effect,4,0,0);
        effect->posX.whole=entity->posX.whole;
        effect->posY.whole=entity->posY.whole-0x30000;
        effect->spriteLayer=entity->spriteLayer-1;
        effect->objMode=1;
        effect->scrollWithBg=1;
        effect->drawFunc=(u32 *)Sprite_DrawEntity;
    }
    return effect;
}

/* Belmont hit reaction: airborne knockback, landing, slide and recovery. */
/**
 * @brief Hit-reaction state (11): handle airborne knockback, landing,
 * sliding and recovery. Select defeat state 0xF0 when HP is exhausted
 * or return to combat/dodge according to the existing room tests.
 */
void Belmont_UpdateHitReaction(struct EwramData_EntityData *entity)
{
    s32 state=entity->subState;
    s32 collision;
    switch(state)
    {
        case 0:
        {
            s32 ground;
            entity->accelX.whole=state;
            entity->velX.whole=state;
            entity->accelY.whole=state;
            entity->velY.whole=state;
            ground=Entity_MoveWithCollisionProfile(entity,sBelmontMoveProfile,4,0) & 8;
            if(ground)
            {
                entity->timer=24;
                entity->subState=255;
                entity->stage=state;
            }
            else
            {
                s32 speed;
                s32 acceleration;
                speed=-0x18000;
                entity->velX.whole=speed;
                acceleration=-0x800;
                entity->accelX.whole=acceleration;
                if(gEwramData->playerState.player->posX.whole<entity->posX.whole)
                {
                    entity->velX.whole=-speed;
                    entity->accelX.whole=-acceleration;
                }
                entity->velY.whole=-0x10000;
                entity->accelY.whole=0x2000;
                entity->subState++;
                entity->stage=ground;
            }
            if(entity->animScriptIndex!=12)
                Anim_StartIndexed(entity,12,1,0);
            break;
        }
        case 1:
            collision=Entity_MoveWithCollisionProfile(entity,sBelmontMoveProfile,4,0);
            if(collision & 3)
            {
                if(entity->flipX)
                    entity->accelX.whole=entity->velX.whole=0;
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
                s32 speed;
                s32 acceleration;
                if(entity->animScriptIndex!=18)
                    Anim_StartIndexed(entity,18,1,0);
                speed=-0x18000;
                entity->velX.whole=speed;
                acceleration=0x800;
                entity->accelX.whole=acceleration;
                if(gEwramData->playerState.player->posX.whole<entity->posX.whole)
                {
                    entity->velX.whole=-speed;
                    entity->accelX.whole=-acceleration;
                }
                entity->velY.whole=0;
                entity->accelY.whole=0;
                entity->subState++;
                entity->stage=0;
            }
            break;
        case 2:
            if(entity->velX.whole!=0)
            {
                collision=Entity_MoveWithCollisionProfile(entity,sBelmontMoveProfile,4,0);
                if(collision & 3)
                {
                    if(entity->flipX)
                        entity->accelX.whole=entity->velX.whole=0;
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
                if((u8)(entity->timer++ % 7)==0)
                    Belmont_CreateMovementEffect(entity);
            }
            {
                s32 inner=entity->stage;
                switch(inner)
                {
                    case 0:
                        if(entity->animDone)
                        {
                            if((s16)entity->hp<=0)
                            {
                                entity->state=240;
                                entity->stage=inner;
                                entity->subState=inner;
                            }
                            else
                            {
                                if(entity->animScriptIndex!=14)
                                    Anim_StartIndexed(entity,14,1,0);
                                entity->stage++;
                            }
                        }
                        break;
                    case 1:
                        inner &= ((u8 *)entity)[0x59];
                        if(inner)
                        {
                            if(entity->animScriptIndex!=4)
                                Anim_StartIndexed(entity,4,1,0);
                            entity->timer=8;
                            entity->accelX.whole=0;
                            entity->velX.whole=0;
                            entity->accelY.whole=0;
                            entity->velY.whole=0;
                            entity->subState=255;
                            entity->stage=0;
                        }
                        break;
                }
            }
            break;
        case 255:
        {
            if(--entity->timer==255)
            {
                if((s16)entity->hp<=0)
                {
                    entity->state=240;
                    entity->stage=0;
                    entity->subState=0;
                    return;
                }
                if((Entity_IsOffsetOutsideRoom(entity,128,0) && !entity->flipX) ||
                        (Entity_IsOffsetOutsideRoom(entity,-128,0) && entity->flipX))
                {
                    entity->state=9;
                    entity->stage=0;
                    entity->subState=0;
                    return;
                }
                entity->state=2;
                entity->stage=0;
                entity->subState=0;
                return;
            }
            break;
        }
    }
}

/* Belmont defeat animation and linked scene completion. */
/**
 * @brief Defeat state (0xF0): play defeat animation, create scene entry 1
 * and update completion flags/target tracking when the scene object
 * finishes. Retain the existing idle final substate.
 */
void Belmont_UpdateDefeat(struct EwramData_EntityData *entity)
{
    switch(entity->subState)
    {
        case 0:
            if(entity->stage==0)
            {
                if(entity->animScriptIndex!=13)
                    Anim_StartIndexed(entity,13,1,0);
                entity->palette=entity->basePalette;
                entity->stage++;
            }
            if(entity->animDone)
            {
                entity->timer=24;
                entity->subState+=2;
                entity->stage=0;
            }
            Anim_Update(entity);
            break;
        case 1:
            if(entity->timer & 1)
            {
                s32 offset=-0x40000;
                if(entity->flipX)
                    offset=-offset;
                Effect_SpawnParticleAnim13(entity->posX.whole+offset,entity->posY.whole-0x160000,
                             0x8000,0x8000,entity->spriteLayer-1,2);
            }
            if(--entity->timer==255)
            {
                if(entity->animScriptIndex!=12)
                    Anim_StartIndexed(entity,12,1,0);
                entity->timer=24;
                entity->subState++;
                entity->stage=0;
            }
            break;
        case 2:
            if(--entity->timer==255)
            {
                entity->subState++;
                entity->stage=0;
            }
            break;
        case 3:
            if(entity->stage==0)
            {
                Belmont_CreateSceneObject(entity,1);
                entity->stage++;
            }
            else
            {
                struct EwramData_EntityData *object=&gEwramData->entityData[entity->ext.belmontScene.objectIndex];
                if(object->updateFunc==0)
                {
                    GameFlag_Set(0x29);
                    gEwramData->gameState.bossEventFlags|=4;
                    entity->subState++;
                    entity->stage=0;
                    gEwramData->gameState.stateFlags&=~2;
                    Player_SetSelectedTarget(0);
                    gEwramData->gameState.stateFlags|=0x1000;
                    gEwramData->gameState.gameFlagsTail[1]|=0x2000000;
                }
                else if(object->state==127)
                {
                    gEwramData->gameState.stateFlags|=0x400000;
                    gEwramData->gameState.bossEventFlags|=4;
                }
            }
            break;
        case 4:
            if(entity->stage==0)
                entity->stage++;
            break;
    }
}

/* Belmont hit response, defeat notification and repeated-hit stun. */
extern s32 Enemy_TakeDamage(struct EwramData_EntityData *,s32,u16 *);
/**
 * @brief Apply enemy damage, trigger the defeat reaction and reward drop
 * when reported, or count repeated hits toward a flashing stun reaction
 * while the powered flag is clear.
 */
void Belmont_OnHit(struct EwramData_EntityData *entity,s32 kind,u16 *hit)
{
    if(Enemy_TakeDamage(entity,kind,hit))
    {
        PlaySong(0x1E1);
        Music_SetCurrent(0x25);
        entity->state=11;
        entity->stage=0;
        entity->subState=0;
        if((s16)entity->hp<=0)
            Enemy_DropRewards(entity);
    }
    else
    {
        u8 mask=entity->ext.belmont.flags & 8;
        if(mask==0 && hit[0]==2)
        {
            if(entity->ext.belmont.hits>2)
            {
                entity->ext.belmont.flash=32;
                entity->ext.belmont.hits=mask;
                entity->state=11;
                entity->stage=mask;
                entity->subState=mask;
                entity->attack=entity->ext.belmont.attackOff;
                if(RandomNumberGenerator()%2==0)
                    PlaySong(0x1E3);
                else
                    PlaySong(0x1E2);
            }
            else
                entity->ext.belmont.hits++;
        }
    }
}

/* Belmont attached weapon entity and its independent graphics slot. */
extern void BelmontAttachedWeapon_Update(struct EwramData_EntityData *);
/**
 * @brief Create the powered-phase attached weapon with its own graphics
 * setup, owner slot links and stored damage 75. Start hidden/inert;
 * return 1 on success or 0 after entity/graphics failure cleanup.
 */
s32 Belmont_CreateAttachedWeapon(struct EwramData_EntityData *entity)
{
    void *palette=sJuliusPalette;
    void *sprite=sBelmontSpriteSheet;
    u8 *graphics=sBelmontWeaponGfx;
    struct EwramData_EntityData *weapon=Entity_CreateObject(BelmontAttachedWeapon_Update);
    if(!weapon) return 0;
    if(!Entity_LoadGraphics(weapon,graphics,palette,1,sprite))
    { EntityDelete(weapon); return 0; }
    {
            Anim_StartIndexed(weapon,0,0,1);
            Anim_Update(weapon);
            ENTITY_PARENT_INDEX(weapon)=ENTITY_INDEX(entity);
            entity->ext.belmontWeapon.index=ENTITY_INDEX(weapon);
            weapon->spriteLayer=entity->spriteLayer+1;
            weapon->flipX=entity->flipX;
            weapon->posX.whole=entity->posX.whole;
            weapon->posY.whole=entity->posY.whole;
            weapon->scrollWithBg=1;
            weapon->hidden=1;
            weapon->attack=75;
            weapon->enemyId=entity->enemyId;
            Entity_EnableBoxes(weapon,Enemy_OnContact,0);
            return 1;
    }
}

/* Belmont subweapon factory. All callers supply variants 0 through 3. */
extern void BelmontSubweapon0_Update(struct EwramData_EntityData *);
extern void BelmontSubweapon1_Update(struct EwramData_EntityData *);
extern void BelmontSubweapon2_Update(struct EwramData_EntityData *);
extern void BelmontSubweapon3_Update(struct EwramData_EntityData *);
/**
 * @brief Create subweapon variant 0 through 3 with its selected update
 * callback, graphics palette, owner link and spawn offset. Return the
 * entity or NULL. Preserve the existing valid-variant precondition.
 */
struct EwramData_EntityData *Belmont_CreateSubweapon(struct EwramData_EntityData *entity,s32 kind)
{
    void *palette=sJuliusPalette;
    void *sprite=sBelmontSpriteSheet;
    u8 *graphics=sBelmontWeaponGfx;
    s32 slot=1;
    void (*update)(struct EwramData_EntityData *);
    struct EwramData_EntityData *weapon;
    switch(kind)
    {
        case 0: update=BelmontSubweapon0_Update; slot=0; break;
        case 1: update=BelmontSubweapon1_Update; break;
        case 2: update=BelmontSubweapon2_Update; break;
        case 3: update=BelmontSubweapon3_Update; break;
    }
    weapon=Entity_CreateObject(update);
    if(!weapon)
        return NULL;
    if(!Entity_LoadGraphics(weapon,graphics,palette,slot,sprite))
    {
        EntityDelete(weapon);
        return NULL;
    }
    weapon->basePalette=kind;
    ENTITY_PARENT_INDEX(weapon)=ENTITY_INDEX(entity);
    weapon->spriteLayer=entity->spriteLayer-1;
    weapon->flipX=entity->flipX;
    weapon->posX.whole=entity->posX.whole;
    weapon->posY.whole=entity->posY.whole;
    weapon->scrollWithBg=1;
    weapon->animFrame=weapon->basePalette;
    weapon->hidden=1;
    if(kind==0 || kind==2)
    {
        s32 offset=-16;
        if(weapon->flipX)
            offset=16;
        weapon->posX.part16.integer+=offset;
        weapon->posY.part16.integer-=26;
    }
    else if(kind==1)
    {
        s32 offset=-19;
        if(weapon->flipX)
            offset=19;
        weapon->posX.part16.integer+=offset;
        weapon->posY.part16.integer-=6;
    }
    Entity_EnableBoxes(weapon,Enemy_OnContact,0);
    weapon->hit.flags|=4;
    weapon->attack=85;
    return weapon;
}

/* Belmont subweapon variant 0: rotating ballistic projectile. */
#define ANGLE(e) ((e)->hp)
#define STEP(e) (ENTITY_HALF_51A((e)))
/**
 * @brief Variant 0: allocate an affine slot, launch a rotating ballistic
 * projectile with afterimages and release the slot after leaving the screen.
 */
void BelmontSubweapon0_Update(struct EwramData_EntityData *entity)
{
    switch(entity->state)
    {
        case 0:
        {
            u16 affine=ObjAffine_Alloc(1);
            s32 speed;
            u16 step;
            if((s16)affine<0)
            {
                EntityDelete(entity);
                return;
            }
            entity->affineIndex=affine;
            entity->affine=1;
            STEP(entity)=(step=0xB00);
            if(!entity->flipX)
                STEP(entity)=-step;
            BELMONT_SUBWEAPON_SPEED(entity)=(void *)(u32)(speed=0x16000);
            entity->velX.whole=speed;
            if(!entity->flipX)
                entity->velX.whole=-speed;
            entity->velY.whole=-0x38000;
            entity->accelY.whole=0x2000;
            entity->hidden=0;
            entity->state++;
            Afterimage_Create(entity,48,5,2);
            Afterimage_Create(entity,48,3,2);
            PlaySong(0x1EC);
            PlaySong(0xC5);
            break;
        }
        case 1:
            EntityAccelerateAndMoveX(entity,0x80000);
            EntityAccelerateAndMoveY(entity,0x80000);
            break;
        case 2:
            ObjAffine_Free(entity->affineIndex);
            EntityDelete(entity);
            return;
    }
    ANGLE(entity)+=STEP(entity);
    ObjAffine_Set((s16)ANGLE(entity),0x10000,0x10000,entity->flipX,0,entity->affineIndex);
    if(EntityIsOutsideScreen(entity,4))
    {
        entity->state=2;
        entity->hidden=1;
    }
    Enemy_CheckHitPlayer(entity);
}

/* Belmont subweapon variant 1: accelerating rotating projectile with a trail. */
/**
 * @brief Variant 1: rotate and accelerate horizontally with a trail, then
 * release the affine slot and remove after leaving the screen.
 */
void BelmontSubweapon1_Update(struct EwramData_EntityData *entity)
{
    switch(entity->state)
    {
        case 0:
        {
            s32 affine=ObjAffine_Alloc(1);
            s32 speed;
            s32 acceleration;
            if(affine<0)
            {
                EntityDelete(entity);
                return;
            }
            entity->affineIndex=affine;
            entity->affine=1;
            entity->posY.part16.integer-=16;
            entity->velX.whole=(speed=0x34000);
            entity->accelX.whole=(acceleration=-0x1000);
            if(!entity->flipX)
            {
                entity->velX.whole=-speed;
                entity->accelX.whole=-acceleration;
            }
            entity->hidden=0;
            entity->state++;
            PlaySong(0x1E5);
        }
        case 1:
        {
            s32 speed;
            s32 acceleration;
            u16 step;
            entity->posX.whole+=entity->velX.whole;
            speed=entity->velX.whole;
            acceleration=entity->accelX.whole;
            if((acceleration<0 && speed>-0x80000) || (acceleration>0 && speed<0x80000))
                entity->velX.whole=speed+acceleration;
            if((entity->timer++ & 1)!=0)
            {
                s32 x=0x80000;
                s32 jitter;
                s32 y;
                struct EwramData_EntityData *trail;
                if(entity->velX.whole>0)
                    x=-x;
                jitter=((RandomNumberGenerator()%0x200000)>>1)-0x80000;
                y=entity->posY.whole-jitter;
                trail=Effect_CreateFromTable(entity->posX.whole+x,y,0,0,4,6);
                if(trail)
                    trail->spriteLayer=entity->spriteLayer+1;
            }
            STEP(entity)=(step=0x580);
            if(!entity->flipX)
                STEP(entity)=-step;
            ANGLE(entity)+=STEP(entity);
            ObjAffine_Set((s16)ANGLE(entity),0x10000,0x10000,entity->flipX,0,entity->affineIndex);
            entity->ext.subweaponUnused.sum+=entity->ext.subweaponUnused.step;
            if(EntityIsOutsideScreen(entity,4))
                entity->state=2;
            break;
        }
        case 2:
            ObjAffine_Free(entity->affineIndex);
            EntityDelete(entity);
            return;
    }
    Enemy_CheckHitPlayer(entity);
}

/* Belmont subweapon variants 2 and 3: ground impact and repeated flames. */
/**
 * @brief Variant 2: throw along terrain, stop horizontal motion at walls
 * and on ground impact repeatedly create variant 3 until the countdown
 * ends. Preserve lifetime and player-hit checks.
 */
void BelmontSubweapon2_Update(struct EwramData_EntityData *entity)
{
    switch(entity->state)
    {
        case 0:
        {
            s32 speed=0x24000;
            BELMONT_SUBWEAPON_SPEED(entity)=(void *)(u32)speed;
            entity->velX.whole=speed;
            if(!entity->flipX)
                entity->velX.whole=-speed;
            entity->velY.whole=-0x6000;
            entity->accelY.whole=0x4000;
            entity->hidden=0;
            entity->state++;
            break;
        }
        case 1:
        {
            s32 collision=Entity_Walk(entity,(s32)BELMONT_SUBWEAPON_SPEED(entity),sBelmontMoveProfile,0);
            if(collision & 3)
                BELMONT_SUBWEAPON_SPEED(entity)=NULL;
            if(collision & 8)
                entity->state=3;
            break;
        }
        case 2:
            EntityDelete(entity);
            return;
        case 3:
            if(entity->subState==0)
            {
                PlaySong(0x129);
                entity->hidden=1;
                entity->timer=60;
                entity->subState++;
            }
            if((u8)((u32)entity->timer%10)==0)
                Belmont_CreateSubweapon(entity,3);
            if(--entity->timer==0xFF)
                entity->state=2;
            break;
    }
    if(EntityIsOutsideScreen(entity,4))
    {
        entity->state=2;
        entity->hidden=1;
    }
    Enemy_CheckHitPlayer(entity);
}
/**
 * @brief Variant 3: animate and move along terrain, stopping at walls;
 * remove when animation completes or outside the screen, otherwise
 * update hit boxes and check player contact.
 */
void BelmontSubweapon3_Update(struct EwramData_EntityData *entity)
{
    if(entity->state==0)
    {
        Anim_StartIndexed(entity,2,0,0);
        BELMONT_SUBWEAPON_SPEED(entity)=(void *)0x12000;
        entity->hidden=0;
        entity->state++;
        PlaySong(0x91);
    }
    if(Entity_Walk(entity,(s32)BELMONT_SUBWEAPON_SPEED(entity),sBelmontMoveProfile,0) & 3)
        BELMONT_SUBWEAPON_SPEED(entity)=NULL;
    if(entity->animDone || EntityIsOutsideScreen(entity,4))
        EntityDelete(entity);
    else
    {
        Entity_AnimateWithBoxes(entity);
        entity->hit.flags|=4;
        Enemy_CheckHitPlayer(entity);
    }
}

/* Five shared death fragments, their lifetime update, and a state helper. */
extern s8 ObjPalette_FindOrLoadVariant(void *,s32);
extern void FragmentSet_Update(struct EwramData_EntityData *);
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
        entity->ext.fragmentSet.count++;
        ENTITY_PARENT_INDEX(fragment)=ENTITY_INDEX(entity);
        fragment->spriteLayer=entity->spriteLayer;
        fragment->basePalette=entity->basePalette;
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
        owner->ext.fragmentSet.count--;
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

/* Belmont_UpdateBestiaryPreview: variant of an already decompiled function, generated with
 * a cloning script (constants adjusted where needed), checked byte for
 * byte. */

/**
 * @brief Bestiary state (12): select animation 5 once, then update it.
 */
void Belmont_UpdateBestiaryPreview(struct EwramData_EntityData *entity)
{
    if (entity->subState == 0)
    {
        if (entity->animScriptIndex != 5)
            Anim_StartIndexed(entity, 5, 1, 1);
        entity->subState++;
    }
    Anim_Update(entity);
}

/* Attached weapon stays inert until its parent starts the active phase. */
/**
 * @brief Remain inert in state 0. Once the owner activates this entity,
 * show the weapon, animate with hit boxes and check player contact.
 */
void BelmontAttachedWeapon_Update(struct EwramData_EntityData *entity)
{
    if(entity->state!=0)
    {
        entity->hidden=0;
        Entity_AnimateWithBoxes(entity);
        Enemy_CheckHitPlayer(entity);
    }
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   BelmontMovementEffect_Update = ManticoreEffect_Update */

/**
 * @brief Update the effect animation or delete when it is done.
 */
void BelmontMovementEffect_Update(struct EwramData_EntityData *entity)
{
    if (entity->animDone)
        EntityDelete(entity);
    else
        Anim_Update(entity);
}

/* Unreferenced helper: signed horizontal retreat speed based on player speed. */
/**
 * @brief Carry slot of the solid table sBelmontWeaponSolidCallbacks that
 * Belmont_UpdatePoweredPhase registers for its weapon: signed horizontal
 * speed away from the player (at least 0x19000, from the player speed),
 * 0x10000 in animation 0.
 */
s32 BelmontWeaponSolid_GetCarryVelocity(struct EwramData_EntityData *entity)
{
    s32 speed;
    if(entity->animScriptIndex==0)
        speed=0x10000;
    else
    {
        s32 playerSpeed=gEwramData->playerState.player->velX.whole;
        if(playerSpeed<0)
            playerSpeed=-playerSpeed;
        speed=playerSpeed+0x1800;
        if(speed<0x19000)
            speed=0x19000;
    }
    if((s16)gEwramData->playerState.player->posX.part16.integer>(s16)entity->posX.part16.integer)
        speed=-speed;
    return speed;
}
