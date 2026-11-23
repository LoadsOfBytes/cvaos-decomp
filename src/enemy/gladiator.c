/*
 * Gladiator: body, arms, spinner, shots and fragments.
 */
#include "core.h"
#include "graphics.h"
#include "gameplay.h"
#include "entity_util.h"
#include "structs/ewram.h"
#include <string.h>
#include "sound.h"
#include "player_core.h"

extern s32 Gladiator_LoadGraphics(struct EwramData_EntityData *entity);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *entity);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *entity);
extern void Entity_SnapToGround(struct EwramData_EntityData *entity);
extern s32 Anim_GetMaxBoxCount(struct EwramData_EntityData *entity);
extern void Entity_EnableBoxes(struct EwramData_EntityData *entity, void *onHit, void *onHurt);
extern struct EwramData_EntityData *Entity_CreateObject(u32 *update);
extern s32 Hit_EnableContact(struct EwramData_EntityData *entity, s32 type, void *onHit, s32 onHurt);
extern void GladiatorBody_Update(struct EwramData_EntityData *entity);
extern void GladiatorArm_Update(struct EwramData_EntityData *entity);
extern void GladiatorSpinner_Update(struct EwramData_EntityData *entity);
extern void GladiatorPart_DamagePlayerOnContact(struct EwramData_EntityData *entity);
void GladiatorPart_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
extern void Gladiator_DamagePlayerOnContact(struct EwramData_EntityData *entity);
extern void Gladiator_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
extern u8 sGladiatorGfx[], sGladiatorSpriteSheet[];

/**
 * @brief A07FC | Gladiator create: animation 12 and three parts
 * Entity_CreateObject (GladiatorBody_Update / GladiatorArm_Update in front, GladiatorSpinner_Update behind
 * with its own palette and identity affine matrix) sharing graphics
 * sGladiatorSpriteSheet, palettes from unk_504 / unk_505 (written before the NULL
 * test, as in the original), offsets in unk_4F4 / unk_4F6; frozen time
 * gives state 2, otherwise box slot 8 (Hit_EnableContact when Anim_GetMaxBoxCount > 1)
 * and state 0. A failed allocation deletes it.
 */
void EnemyGladiatorCreate(struct EwramData_EntityData *entity)
{
    s32 i;
    struct EwramData_EntityData *part;
    u16 palette;
    u32 r;

    if ((u8)Gladiator_LoadGraphics(entity) == 0)
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 12, 0, 1);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    for (i = 0; i <= 2; i++)
    {
        part = NULL;
        switch (i)
        {
        case 0:
            part = Entity_CreateObject((u32 *)GladiatorBody_Update);
            part->basePalette = entity->ext.gladiator.palette;
            break;
        case 1:
            part = Entity_CreateObject((u32 *)GladiatorArm_Update);
            part->basePalette = entity->ext.gladiator.palette;
            break;
        case 2:
            part = Entity_CreateObject((u32 *)GladiatorSpinner_Update);
            part->basePalette = entity->ext.gladiator.palette1;
            break;
        }
        if (part != NULL)
        {
            if (i == 2)
                part->spriteLayer = entity->spriteLayer - 1;
            else
                part->spriteLayer = entity->spriteLayer + 1;
            Sprite_Attach(part, sGladiatorSpriteSheet, ObjGfx_Find(sGladiatorGfx), part->basePalette);
            part->enemyId = entity->enemyId;
            part->scrollWithBg = 0;
            part->drawFunc = (u32 *)Sprite_DrawEntity;
            part->attack = entity->attack;
            ENTITY_OWNER(part) = entity;
            part->ext.gladiatorSpinner.index = i;
            part->posX.whole = entity->posX.whole;
            part->posY.whole = entity->posY.whole;
            switch (i)
            {
            case 0:
                Anim_StartIndexed(part, 4, 0, 1);
                part->ext.gladiatorSpinner.offsetX = 0x24;
                part->ext.gladiatorSpinner.offsetY = -0x11;
                break;
            case 1:
                Anim_StartIndexed(part, 0, 0, 1);
                part->ext.gladiatorSpinner.offsetX = -0x38;
                part->ext.gladiatorSpinner.offsetY = -0x11;
                r = RandomNumberGenerator() & 0x3F;
                part->ext.gladiatorSpinner.angle = r;
                break;
            case 2:
                Anim_StartIndexed(part, 0x10, 0, 1);
                part->ext.gladiatorSpinner.offsetX = 0x3C;
                part->ext.gladiatorSpinner.offsetY = -0xF;
                if (gEwramData->gameState.bestiaryPreview)
                    part->ext.gladiatorSpinner.spin = 0x800;
                break;
            }
            Anim_Update(part);
            if ((u32)i <= 1 && !gEwramData->gameState.bestiaryPreview)
                Entity_EnableBoxes(part, GladiatorPart_DamagePlayerOnContact, GladiatorPart_OnHit);
            if (i == 2)
            {
                palette = ObjAffine_Alloc(1);
                if ((s16)palette < 0)
                {
                    EntityDelete(entity);
                    EntityDelete(part);
                    return;
                }
                part->affineIndex = palette;
                part->affine = 1;
                ObjAffine_Set(0, 0x10000, 0x10000, part->flipX, part->flipY, part->affineIndex);
            }
            continue;
        }
        EntityDelete(entity);
        return;
    }
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 2;
        return;
    }
    if (Anim_GetMaxBoxCount(entity) > 1)
        Hit_EnableContact(entity, 8, Gladiator_DamagePlayerOnContact, (s32)Gladiator_OnHit);
    else
        Hit_Enable(entity, 8, Gladiator_DamagePlayerOnContact, (s32)Gladiator_OnHit);
    Entity_UpdateBoxes(entity);
    Entity_SnapToGround(entity);
    entity->state = 0;
}

extern s32 Entity_MoveWithCollisionAlt(struct EwramData_EntityData *, s32, s32, s32);
extern s32 Entity_GetHorizontalRoomEdge(struct EwramData_EntityData *, s32);
extern void Effect_SpawnParticleWithDamping(s32 x, s32 y, s32 velX, s32 velY, s32 layer, s32 variant, s32 damping, s32 flags, s32 palette);
extern void EntityDampVelocityX(struct EwramData_EntityData *, s32);
extern const s16 sGladiatorOffsetsA[12];
extern const s16 sGladiatorOffsetsB[12];

/**
 * @brief A0A8C | Charging part: state 0 accelerates (accelX = +-0x1000)
 * along the floor until it hits a wall on the facing side (Entity_GetHorizontalRoomEdge),
 * with sound 0x7D every 32 frames, then crashes (animation 0xD, sound
 * 0x127); state 1 brakes and throws debris (Effect_SpawnParticleWithDamping) every 4th frame
 * from the s16 offsets sGladiatorOffsetsA / sGladiatorOffsetsB (indexed by the
 * animation frame) until the animation ends, then turns around.
 */
void Gladiator_UpdateCharge(struct EwramData_EntityData *entity)
{
    s16 offsA[12];
    s16 offsB[12];
    s32 state;
    s32 hit;
    s32 speed;
    s32 x;
    s32 vx;

    memcpy(offsA, sGladiatorOffsetsA, sizeof(offsA));
    memcpy(offsB, sGladiatorOffsetsB, sizeof(offsB));
    state = entity->subState;
    switch (state)
    {
    case 0:
        if (entity->flipX)
            entity->accelX.whole = 0x1000;
        else
            entity->accelX.whole = -0x1000;
        Entity_MoveWithCollisionAlt(entity, 0x20000, 0, 0x30000);
        hit = 0;
        if (entity->flipX)
        {
            if (Entity_GetHorizontalRoomEdge(entity, 0x18) == 2)
                hit = 1;
        }
        else if (Entity_GetHorizontalRoomEdge(entity, 0x18) == 1)
        {
            hit = 1;
        }
        if (!(entity->timer & 0x1F) && !EntityIsOutsideScreen(entity, 6))
            PlaySong(0x7D);
        entity->timer++;
        if (!hit)
            break;
        entity->subState++;
        Anim_StartIndexed(entity, 0xD, 0, 0);
        entity->timer = 0;
        if (!EntityIsOutsideScreen(entity, 6))
            PlaySong(0x127);
        break;
    case 1:
        EntityDampVelocityX(entity, 10);
        speed = 0x20000;
        Entity_MoveWithCollisionAlt(entity, speed, 0, 0x30000);
        if (entity->animDone)
        {
            entity->subState = 0;
            entity->timer = 0;
            Anim_StartIndexed(entity, 0xC, 0, 1);
            entity->flipX ^= 1;
            if (entity->flipX)
            {
                x = -0x300000;
                vx = -(RandomNumberGenerator() & 0x7FFF) - 0x20000;
            }
            else
            {
                x = 0x300000;
                vx = (RandomNumberGenerator() & 0x7FFF) + speed;
            }
            Effect_SpawnParticleWithDamping(entity->posX.whole + x, entity->posY.whole, vx, -0x8000 - (RandomNumberGenerator() & 0x7FFF), entity->spriteLayer - 2, 0, 0x10, 0xF, 6);
            break;
        }
        if (!(entity->timer++ & 3))
        {
            if (entity->flipX)
            {
                x = -offsA[entity->animStep] << 16;
                vx = (RandomNumberGenerator() & 0x7FFF) + speed;
            }
            else
            {
                x = offsA[entity->animStep] << 16;
                vx = -(RandomNumberGenerator() & 0x7FFF) - 0x20000;
            }
            Effect_SpawnParticleWithDamping(entity->posX.whole + x, entity->posY.whole, vx, -0x8000 - (RandomNumberGenerator() & 0x7FFF), entity->spriteLayer - 2, 0, 0x10, 0xF, 6);
        }
        if ((entity->timer & 3) == 2)
        {
            if (entity->flipX)
            {
                x = -offsB[entity->animStep] << 16;
                vx = (RandomNumberGenerator() & 0x7FFF) + 0x20000;
            }
            else
            {
                x = offsB[entity->animStep] << 16;
                vx = -0x20000 - (RandomNumberGenerator() & 0x7FFF);
            }
            Effect_SpawnParticleWithDamping(entity->posX.whole + x, entity->posY.whole, vx, -0x8000 - (RandomNumberGenerator() & 0x7FFF), entity->spriteLayer - 2, 0, 0x10, 0xF, 6);
        }
        break;
    }
}

/* Gladiator_OnDamaged. */
extern void Link_MarkEnemyDamaged(void);
extern void Enemy_DropRewards(struct EwramData_EntityData *entity);

/**
 * @brief A0D7C | Hit callback: flash (hitFlashTimer = 8), subtract damage from
 * the HP in hp and Link_MarkEnemyDamaged; at 0 HP play song 0x71, release the
 * hit box (Enemy_DropRewards) and enter the death state 1 (timer 0x3C, palette
 * from basePalette, gravity 0x1000).
 */
void Gladiator_OnDamaged(struct EwramData_EntityData *entity, s32 unused1, u16 *unused2, s32 damage)
{
    entity->hitFlashTimer = 8;
    entity->hp -= damage;
    Link_MarkEnemyDamaged();
    if ((s16)entity->hp <= 0)
    {
        PlaySong(0x71);
        Enemy_DropRewards(entity);
        entity->ext.gladiator.flinch = 1;
        entity->palette = entity->basePalette;
        entity->timer = 0x3C;
        entity->state = 1;
        entity->subState = 0;
        entity->stage = 0;
        entity->accelX.whole = 0;
        entity->accelY.whole = 0x1000;
        entity->velY.whole = 0;
    }
}

extern void Hit_CheckWithPlayerSharedHistory(void *res, struct EwramData_EntityData *entity, struct EwramData_EntityData *owner);
extern void Gladiator_SpawnFragments(struct EwramData_EntityData *entity, s32 x, s32 y);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *);
extern const s16 sUnk_08119174[4];
extern const s16 sGladiatorBodyDx[12];

/**
 * @brief A0DEC | Gladiator body part (owner type 0x5D in enemyId): follows
 * the owner at offset unk_4F4 / unk_4F6 with its facing, palette (4 while
 * the owner flashes) and hidden; during the owner's animation 0xC the
 * vertical offset comes from sUnk_08119174 per frame. When the owner dies
 * (state 1) it bursts into fragments (Gladiator_SpawnFragments) at the owner + the
 * sGladiatorBodyDx offset. Otherwise animation 4 / 0x11 and, in animation 0xC,
 * a hit box against the owner (Hit_CheckWithPlayerSharedHistory). State 1: blink and expire.
 */
void GladiatorBody_Update(struct EwramData_EntityData *entity)
{
    u8 box[0x10];
    s16 dy[4];
    s16 dx[12];
    struct EwramData_EntityData *owner = ENTITY_OWNER(entity);

    memcpy(dy, sUnk_08119174, sizeof(dy));
    memcpy(dx, sGladiatorBodyDx, sizeof(dx));
    if (entity->state == 0)
    {
        s32 x;
        s32 y;
        if (owner == NULL || owner->updateFunc == NULL || owner->enemyId != 0x5D)
            goto delete;
        entity->hidden = owner->hidden;
        if (owner->palette == 4)
            entity->palette = 4;
        else
            entity->palette = entity->basePalette;
        entity->flipX = owner->flipX;
        entity->posX.part16.integer = owner->posX.part16.integer;
        entity->posY.part16.integer = owner->posY.part16.integer;
        if (entity->flipX)
            entity->posX.part16.integer = entity->posX.part16.integer - entity->ext.gladiatorPart.offsetX;
        else
            entity->posX.part16.integer += entity->ext.gladiatorPart.offsetX;
        entity->posY.part16.integer += entity->ext.gladiatorPart.offsetY;
        if (owner->animScriptIndex == 0xC)
        {
            if (owner->animTimer)
            {
                entity->posY.part16.integer = entity->posY.part16.integer + dy[owner->animStep];
                entity->ext.gladiatorPart.bobY = dy[owner->animStep];
            }
            else
            {
                entity->posY.part16.integer = entity->posY.part16.integer + entity->ext.gladiatorPart.bobY;
            }
        }
        if (owner->state == 1)
        {
            if (owner->animScriptIndex == 0xC)
                x = entity->posX.whole;
            else if (owner->flipX)
                x = owner->posX.whole - (dx[owner->animStep] << 16);
            else
                x = (dx[owner->animStep] << 16) + owner->posX.whole;
            y = entity->posY.whole;
            entity->velX.whole = owner->velX.whole;
            Gladiator_SpawnFragments(entity, x, y);
            goto delete;
        }
        if (owner->animScriptIndex == 0xC)
        {
            if (entity->animScriptIndex != 4)
                Anim_StartIndexed(entity, 4, 0, 1);
        }
        else
        {
            if (entity->animScriptIndex != 0x11)
                Anim_StartIndexed(entity, 0x11, 0, 0);
            entity->posX.part16.integer = owner->posX.part16.integer;
            entity->posY.part16.integer = owner->posY.part16.integer;
        }
        if (!gEwramData->gameState.bestiaryPreview)
            Entity_AnimateWithBoxes(entity);
        else
            Anim_Update(entity);
        if (owner->animScriptIndex == 0xC && !gEwramData->gameState.bestiaryPreview)
            Hit_CheckWithPlayerSharedHistory(box, entity, owner);
        return;
    delete:
        EntityDelete(entity);
        return;
    }
    if (entity->subState == 0)
    {
        entity->subState++;
        entity->scrollWithBg = 1;
    }
    Anim_Update(entity);
    if ((s8)--entity->timer <= 0)
        EntityDelete(entity);
}

extern void GladiatorArm_FireShot(struct EwramData_EntityData *entity, s32 dx, s32 dy, s32 vx, s32 vy);
extern const s16 sGladiatorArmDx[12];

/**
 * @brief A1060 | Gladiator arm part: like GladiatorBody_Update but facing away from
 * the owner and with the burst offsets in sGladiatorArmDx. During the owner's
 * animation 0xC it idles with random pauses (unk_500, animation 3 or 1) and
 * throws shots via GladiatorArm_FireShot (8 px ahead, 0x15 px up, song 0x89 when on
 * screen) on frame 3 of animation 1 and frames 3 / 9 / 15 of animation 3;
 * otherwise it is hidden (hidden) in animation 0.
 */
void GladiatorArm_Update(struct EwramData_EntityData *entity)
{
    u8 box[0x10];
    s16 dy[4];
    s16 dx[12];
    struct EwramData_EntityData *owner = ENTITY_OWNER(entity);

    memcpy(dy, sUnk_08119174, sizeof(dy));
    memcpy(dx, sGladiatorArmDx, sizeof(dx));
    if (entity->state == 0)
    {
        s32 x;
        s32 y;
        if (owner == NULL || owner->updateFunc == NULL || owner->enemyId != 0x5D)
            goto delete;
        if (owner->palette == 4)
            entity->palette = 4;
        else
            entity->palette = entity->basePalette;
        entity->flipX = owner->flipX ^ 1;
        entity->posX.part16.integer = owner->posX.part16.integer;
        entity->posY.part16.integer = owner->posY.part16.integer;
        if (entity->flipX)
            entity->posX.part16.integer = entity->posX.part16.integer - entity->ext.gladiatorPart.offsetX;
        else
            entity->posX.part16.integer += entity->ext.gladiatorPart.offsetX;
        entity->posY.part16.integer += entity->ext.gladiatorPart.offsetY;
        if (owner->animScriptIndex == 0xC)
        {
            if (owner->animTimer)
            {
                entity->posY.part16.integer = entity->posY.part16.integer + dy[owner->animStep];
                entity->ext.gladiatorPart.bobY = dy[owner->animStep];
            }
            else
            {
                entity->posY.part16.integer = entity->posY.part16.integer + entity->ext.gladiatorPart.bobY;
            }
        }
        if (owner->state == 1)
        {
            if (owner->animScriptIndex == 0xC)
                x = entity->posX.whole;
            else if (owner->flipX)
                x = owner->posX.whole - (dx[owner->animStep] << 16);
            else
                x = (dx[owner->animStep] << 16) + owner->posX.whole;
            y = entity->posY.whole;
            entity->velX.whole = owner->velX.whole;
            Gladiator_SpawnFragments(entity, x, y);
            goto delete;
        }
        if (!gEwramData->gameState.bestiaryPreview)
        {
            if (owner->animScriptIndex == 0xC)
            {
                entity->hidden = 0;
                if (entity->animScriptIndex == 0)
                {
                    if (entity->ext.gladiatorArm.timer-- <= 1)
                    {
                        entity->ext.gladiatorArm.timer = (u32)RandomNumberGenerator() % 64;
                        if ((RandomNumberGenerator() & 0x7F) < 0x20)
                            Anim_StartIndexed(entity, 3, 0, 0);
                        else
                            Anim_StartIndexed(entity, 1, 0, 0);
                    }
                }
                else if (entity->animScriptIndex == 1)
                {
                    if (entity->animDone)
                        Anim_StartIndexed(entity, 0, 0, 1);
                    if (*(u16 *)&entity->animStep == 3)
                    {
                        GladiatorArm_FireShot(entity, entity->flipX ? 0x80000 : -0x80000, -0x150000, 0, 0);
                        if (!EntityIsOutsideScreen(entity, 6))
                            PlaySong(0x89);
                    }
                }
                else if (entity->animScriptIndex == 3)
                {
                    if (entity->animDone)
                        Anim_StartIndexed(entity, 0, 0, 1);
                    if (entity->animTimer == 0 && (entity->animStep == 3 || entity->animStep == 9 || entity->animStep == 0xF))
                    {
                        GladiatorArm_FireShot(entity, entity->flipX ? 0x80000 : -0x80000, -0x150000, 0, 0);
                        if (!EntityIsOutsideScreen(entity, 6))
                            PlaySong(0x89);
                    }
                }
            }
            else
            {
                entity->hidden = 1;
                if (entity->animScriptIndex != 0)
                    Anim_StartIndexed(entity, 0, 0, 1);
            }
        }
        if (!gEwramData->gameState.bestiaryPreview)
            Entity_AnimateWithBoxes(entity);
        else
            Anim_Update(entity);
        if (owner->animScriptIndex == 0xC && !gEwramData->gameState.bestiaryPreview)
            Hit_CheckWithPlayerSharedHistory(box, entity, owner);
        return;
    delete:
        EntityDelete(entity);
        return;
    }
    if (entity->subState == 0)
    {
        entity->subState++;
        entity->scrollWithBg = 1;
    }
    Anim_Update(entity);
    if ((s8)--entity->timer <= 0)
        EntityDelete(entity);
}

extern s32 Entity_GetVisibilityPattern(s32 step);
extern void EntityDampVelocityX(struct EwramData_EntityData *entity, s32 factor);

/**
 * @brief A13D0 | Gladiator spinning part: follows the owner like
 * GladiatorBody_Update, spins its affine matrix by unk_502 (accelerating by 0x18 up
 * to 0x800), visible only during the owner's animation 0xC. When the owner
 * dies it falls (velY -0xC000, gravity 0x1000) still spinning, blinks for
 * 0x40 frames and releases its affine slot.
 */
void GladiatorSpinner_Update(struct EwramData_EntityData *entity)
{
    u8 unused[0x10];
    struct EwramData_EntityData *owner = ENTITY_OWNER(entity);

    if (entity->state == 0)
    {
        if (owner == NULL || owner->updateFunc == NULL || owner->enemyId != 0x5D)
            goto delete;
        if (owner->palette == 4)
            entity->palette = 4;
        else
            entity->palette = entity->basePalette;
        entity->flipX = owner->flipX;
        entity->posX.part16.integer = owner->posX.part16.integer;
        entity->posY.part16.integer = owner->posY.part16.integer;
        if (entity->flipX)
            entity->posX.part16.integer = entity->posX.part16.integer - entity->ext.gladiatorSpinner.offsetX;
        else
            entity->posX.part16.integer += entity->ext.gladiatorSpinner.offsetX;
        entity->posY.part16.integer += entity->ext.gladiatorSpinner.offsetY;
        Anim_Update(entity);
        if (entity->flipX)
            entity->ext.gladiatorSpinner.angle += entity->ext.gladiatorSpinner.spin;
        else
            entity->ext.gladiatorSpinner.angle = entity->ext.gladiatorSpinner.angle - entity->ext.gladiatorSpinner.spin;
        if ((u16)entity->ext.gladiatorSpinner.spin < 0x800)
            entity->ext.gladiatorSpinner.spin += 0x18;
        ObjAffine_Set((u16)entity->ext.gladiatorSpinner.angle, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
        if (owner->animScriptIndex == 0xC)
        {
            entity->hidden = 0;
        }
        else
        {
            entity->hidden = 1;
            entity->ext.gladiatorSpinner.spin = 0;
        }
        if (owner->state == 1)
        {
            entity->state = 1;
            entity->subState = 0;
            entity->timer = 0x40;
            entity->velX.whole = owner->velX.whole;
            entity->velY.whole = -0xC000;
            entity->accelX.whole = 0;
            entity->accelY.whole = 0x1000;
            if (entity->hidden)
            {
                ObjAffine_Free(entity->affineIndex);
            delete:
                EntityDelete(entity);
            }
        }
        return;
    }
    EntityDampVelocityX(entity, 0x20);
    EntityAccelerateAndMove(entity, 0x50000);
    if (entity->flipX)
        entity->ext.gladiatorSpinner.angle += entity->ext.gladiatorSpinner.spin;
    else
        entity->ext.gladiatorSpinner.angle = entity->ext.gladiatorSpinner.angle - entity->ext.gladiatorSpinner.spin;
    ObjAffine_Set((u16)entity->ext.gladiatorSpinner.angle, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
    entity->hidden = Entity_GetVisibilityPattern(entity->timer);
    if ((s8)--entity->timer <= 0)
    {
        ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
    }
}

extern void GladiatorShot_Update(struct EwramData_EntityData *entity);
extern void GladiatorShot_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
extern void GladiatorShot_CreateTip(struct EwramData_EntityData *entity);

/**
 * @brief A15CC | Gladiator shot: entity GladiatorShot_Update at (dx, dy) from the
 * owner with velocity (vx, vy), graphics sGladiatorSpriteSheet / sGladiatorGfx
 * with the owner's palette, animation 0xE, box slot 8 (GladiatorShot_OnHit) with
 * box 0x0404FEFE (boxes off), layer one below, owner in unk_4F8, the
 * owner's facing in unk_4FD and its attack; own palette with blending and
 * an affine matrix pointing along the facing (unk_502 0 / 0x8000); then
 * GladiatorShot_CreateTip. Deleted when the graphics or palette allocation fails.
 */
void GladiatorArm_FireShot(struct EwramData_EntityData *entity, s32 dx, s32 dy, s32 vx, s32 vy)
{
    u32 box = 0x0404FEFE;
    struct EwramData_EntityData *shot = EntityCreateInRange(0x1B, 0x48, (u32 *)GladiatorShot_Update);
    s32 tiles;
    u16 palette;

    if (shot == NULL)
        return;
    tiles = ObjGfx_Find(sGladiatorGfx);
    if (tiles < 0)
        goto delete;
    Sprite_Attach(shot, sGladiatorSpriteSheet, tiles, entity->basePalette);
    Anim_StartIndexed(shot, 0xE, 0, 0);
    Anim_Update(entity);
    shot->drawFunc = (u32 *)Sprite_DrawEntity;
    Hit_Enable(shot, 8, NULL, (s32)GladiatorShot_OnHit);
    HitBox_SetBody(shot, &box);
    shot->hit.flags &= 0xFB;
    shot->hit.flags |= 2;
    shot->scrollWithBg = 1;
    shot->spriteLayer = entity->spriteLayer - 1;
    shot->basePalette = entity->basePalette;
    shot->enemyId = entity->enemyId;
    ENTITY_OWNER(shot) = entity;
    shot->posX.whole = entity->posX.whole + dx;
    shot->posY.whole = entity->posY.whole + dy;
    shot->velX.whole = vx;
    shot->velY.whole = vy;
    shot->ext.gladiatorShot.unusedDir = entity->flipX;
    shot->attack = entity->attack;
    if (entity->flipX)
        shot->ext.gladiatorShot.angle = 0;
    else
        shot->ext.gladiatorShot.angle = 0x8000;
    palette = ObjAffine_Alloc(1);
    if ((s16)palette < 0)
    {
    delete:
        EntityDelete(shot);
        return;
    }
    shot->affineIndex = palette;
    shot->affine = 1;
    ObjAffine_Set((u16)shot->ext.gladiatorShot.angle - 0x8000, 0x10000, 0x10000, shot->flipX, shot->flipY, shot->affineIndex);
    GladiatorShot_CreateTip(shot);
}

extern void EntitySetVelocityFromAngle(struct EwramData_EntityData *, s32, s32);
extern void EntityMove(struct EwramData_EntityData *entity);
extern s32 Tile_Classify(s16 x, s16 y);
extern s32 Hit_CheckWithPlayer(void *res, struct EwramData_EntityData *b);

#define FIXED_TO_INT(v) ((v) < 0 ? -(-(v) >> 16) : (v) >> 16)

/**
 * @brief A1740 | Homing projectile: turn the angle unk_502 by 0x100 per frame
 * towards 0x4000 (straight down), fly at 0x30000; it leaves a debris state
 * (10) when the probe 8 px ahead hits a wall, and expires after 0xF0 frames.
 * Drawn rotated by the angle (affine slot affineIndex).
 */
void GladiatorShot_Update(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    s32 x, y;
    u32 d;

    switch (entity->state)
    {
    case 0:
        if ((u16)entity->ext.gladiatorShot.angle > 0x4000)
            entity->ext.gladiatorShot.angle -= 0x100;
        else if ((u16)entity->ext.gladiatorShot.angle < 0x4000)
            entity->ext.gladiatorShot.angle += 0x100;
        EntitySetVelocityFromAngle(entity, (u16)entity->ext.gladiatorShot.angle, 0x30000);
        EntityMove(entity);
        x = FixedMultiply(Sin((u16)entity->ext.gladiatorShot.angle + 0x4000), 0x80000);
        y = FixedMultiply(Sin((u16)entity->ext.gladiatorShot.angle), 0x80000);
        x = FIXED_TO_INT(x + entity->posX.whole + bg->xPos.whole);
        y = FIXED_TO_INT(y + entity->posY.whole + bg->yPos.whole);
        if (Tile_Classify(x, y) != 1)
        {
            entity->state = 10;
            entity->timer = 0x78;
        }
        if (entity->timer++ > 0xF0)
        {
            ObjAffine_Free(entity->affineIndex);
            EntityDelete(entity);
            return;
        }
        Anim_Update(entity);
        Hit_CheckWithPlayer(collision, entity);
        break;
    case 1:
        ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
        return;
    case 10:
        entity->timer--;
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
        if (entity->timer == 0)
        {
            ObjAffine_Free(entity->affineIndex);
            EntityDelete(entity);
            return;
        }
        Anim_Update(entity);
        Hit_CheckWithPlayer(collision, entity);
        break;
    }
    d = -0x8000;
    ObjAffine_Set((u16)entity->ext.gladiatorShot.angle + d, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
}

/* GladiatorShot_CreateTip: spawn the follower; GladiatorShotTip_Update: follow a live owner
 * (type byte 0x5D) at an eight-pixel angular offset. */
extern s32 Hit_CheckWithPlayer(void *res, struct EwramData_EntityData *b);
extern void GladiatorShotTip_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void GladiatorShotTip_OnHit(struct EwramData_EntityData *, s32, u16 *);
void GladiatorShotTip_Update(struct EwramData_EntityData *entity);

/* Create the follower of owner: graphics 081CC0AC / 08229104, animation 15,
 * hit callbacks GladiatorShotTip_DamagePlayerOnContact / GladiatorShotTip_OnHit, box 0x0806FAFD (bytes x -3,
 * y -6, w 6, h 8; not an address), one priority level above the owner and
 * placed eight pixels from it along the owner's angle (unk_502). The
 * original clears unk_53D bit 1 through a byte AND with a -3 kept in r5
 * since the prologue, hence the explicit mask local. */
void GladiatorShot_CreateTip(struct EwramData_EntityData *owner)
{
    s32 mask = -3;
    u32 box = 0x0806FAFD;
    struct EwramData_EntityData *child = EntityCreateInRange(0x1B, 0x48, (u32 *)GladiatorShotTip_Update);
    s32 graphics;
    s32 x, y;

    if (child == NULL)
        return;
    graphics = ObjGfx_Find(sGladiatorGfx);
    if (graphics < 0)
    {
        EntityDelete(child);
        return;
    }
    Sprite_Attach(child, sGladiatorSpriteSheet, graphics, owner->basePalette);
    Anim_StartIndexed(child, 0xF, 0, 1);
    Anim_Update(owner);
    child->drawFunc = (u32 *)Sprite_DrawEntity;
    Hit_Enable(child, 10, GladiatorShotTip_DamagePlayerOnContact, (s32)GladiatorShotTip_OnHit);
    HitBox_SetAttack(child, &box);
    HitBox_SetBody(child, &box);
    child->hit.flags &= ~4;
    child->hit.flags &= ~2;
    mask &= *((u8 *)&child->accelY + 5); /* unk_53D: clear scrollWithBg */
    *((u8 *)&child->accelY + 5) = mask;
    child->spriteLayer = owner->spriteLayer - 1;
    child->basePalette = owner->basePalette;
    ENTITY_OWNER(child) = owner;
    child->attack = owner->attack;
    x = FixedMultiply(Sin((u16)owner->ext.gladiatorShot.angle + 0x4000), 0x80000);
    y = FixedMultiply(Sin((u16)owner->ext.gladiatorShot.angle), 0x80000);
    child->posX.whole = x + owner->posX.whole;
    child->posY.whole = y + owner->posY.whole;
}

void GladiatorShotTip_Update(struct EwramData_EntityData *entity)
{
    u32 buffer[4];
    struct EwramData_EntityData *owner = ENTITY_OWNER(entity);
    if (owner != NULL && owner->updateFunc != NULL
        && owner->enemyId == 0x5D && entity->state == 0)
    {
        s32 x, y;
        entity->hidden = owner->hidden;
        Anim_Update(entity);
        Hit_CheckWithPlayer(buffer, entity);
        x = FixedMultiply(Sin((u16)owner->ext.gladiatorShot.angle + 0x4000), 0x80000);
        y = FixedMultiply(Sin((u16)owner->ext.gladiatorShot.angle), 0x80000);
        entity->posX.whole = x + owner->posX.whole;
        entity->posY.whole = y + owner->posY.whole;
    }
    else EntityDelete(entity);
}

/* Spawn seven fragments with the owner's palette and horizontal motion. */
extern void GladiatorFragment_Update(struct EwramData_EntityData *);
/**
 * @brief Burst into seven fragments at (x, y) with the velocity of entity plus a
 * random part (GladiatorFragment_Update).
 */
void Gladiator_SpawnFragments(struct EwramData_EntityData *entity, s32 x, s32 y)
{
    s32 i;
    for (i = 0; i <= 6; i++)
    {
        struct EwramData_EntityData *effect = EntityCreateInRange(0x49, 0xCF, (u32 *)GladiatorFragment_Update);
        if (effect != NULL)
        {
            effect->posX.whole = x;
            effect->posY.whole = y;
            effect->velX.whole = entity->velX.whole + (RandomNumberGenerator() & 0x1FFFF) - 0x10000;
            effect->velY.whole = -(RandomNumberGenerator() & 0xFFFF) - 0x4000;
            effect->accelY.whole = 0x1000;
            effect->spriteLayer = entity->spriteLayer;
            Sprite_Attach(effect, sGladiatorSpriteSheet, ObjGfx_Find(sGladiatorGfx), entity->basePalette);
            Anim_StartIndexed(effect, (u16)(i + 5), 0, 0);
            effect->flipX = entity->flipX;
            effect->scrollWithBg = 1;
            effect->drawFunc = (u32 *)Sprite_DrawEntity;
            effect->timer = (RandomNumberGenerator() % 4) + 0x28;
        }
    }
}

/* GladiatorShot_OnHit. */
/**
 * @brief A1B3C | Hit callback: Player_CalculateAttackPower(hit[2]) and state 1.
 */
void GladiatorShot_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    Player_CalculateAttackPower(hit[2]);
    entity->state = 1;
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   GladiatorShotTip_DamagePlayerOnContact = GreatArmorShockwave_DamagePlayerOnContact */

/**
 * @brief A1B50 | Same as GreatArmorShockwave_DamagePlayerOnContact.
 */
void GladiatorShotTip_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 2;
    contactDamage.unused = 0;
    contactDamage.reaction = 0;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/* Helpers 0x080A1B80..0x080A1BEB. */
/**
 * @brief A1B80 | Hit callback: Player_CalculateAttackPower(hit[2]) and put the
 * parent (unk_4F8_entity) into state 1.
 */
void GladiatorShotTip_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    Player_CalculateAttackPower(hit[2]);
    ENTITY_OWNER(entity)->state = 1;
}

/**
 * @brief A1B98 | Slow down and fall; blink (bit 5 of 0x53C) during the
 * last 0x28 frames of the timer timer and delete when it runs out.
 */
void GladiatorFragment_Update(struct EwramData_EntityData *entity)
{
    EntityDampVelocityX(entity, 0x20);
    EntityAccelerateAndMove(entity, 0x80000);
    if (entity->timer <= 0x28)
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);

    Anim_Update(entity);
    if ((s8)--entity->timer <= 0)
        EntityDelete(entity);
}

/* Load the shared fragment graphics (081CC0AC / 08229104) with the two
 * palette variants of 0820AF98 into unk_504 / unk_505, the second one also
 * as the palette (basePalette); 0 when the graphics are not loaded. */
extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);
extern u8 sGladiatorGfx[], sGladiatorSpriteSheet[], sGladiatorPalette[];

s32 Gladiator_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 graphics = ObjGfx_LoadResource(sGladiatorGfx, 0);
    if (graphics < 0)
        return 0;
    entity->ext.gladiator.palette = ObjPalette_FindOrLoadVariant(sGladiatorPalette, 0);
    entity->basePalette = entity->ext.gladiator.palette1 = ObjPalette_FindOrLoadVariant(sGladiatorPalette, 1);
    Sprite_Attach(entity, sGladiatorSpriteSheet, graphics, entity->basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Gladiator_DamagePlayerOnContact = Object28_OnPlayerContact
 *   Gladiator_OnHit = Manticore_OnHit
 *   GladiatorPart_DamagePlayerOnContact = Object28_OnPlayerContact */

extern u16 Player_CalculateAttackPower(u16 attackCategory);
extern s16 Hit_ResolveDamage(struct EwramData_EntityData *entity, s32 other, u16 *hit, s32 inputDamage);
extern void Entity_ShowDamage(struct EwramData_EntityData *owner, s16 value, s32 kind);

/**
 * @brief A1C50 | Same as Object28_OnPlayerContact.
 */
void Gladiator_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 0;
    contactDamage.unused = 0;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief A1C80 | Same as Manticore_OnHit.
 */
void Gladiator_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    s16 damage = Hit_ResolveDamage(entity, attacker, hit, (s16)Player_CalculateAttackPower(hit[2]));

    Entity_ShowDamage(entity, damage, 0);
    Gladiator_OnDamaged(entity, attacker, hit, damage);
}

/**
 * @brief A1CC8 | Same as Object28_OnPlayerContact.
 */
void GladiatorPart_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 0;
    contactDamage.unused = 0;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   GladiatorPart_OnHit = ManticoreTail_OnHit */

extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *);
extern void Enemy_UpdateFragmentDeath(struct EwramData_EntityData *);
extern s32 Entity_IsTimeStopped(struct EwramData_EntityData *);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *);
void Gladiator_UpdateDeath(struct EwramData_EntityData *entity);

/**
 * @brief A1CF8 | Same as ManticoreTail_OnHit.
 */
void GladiatorPart_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    struct EwramData_EntityData *gladiator = ENTITY_OWNER(entity);
    s32 oldX = gladiator->posX.whole;
    s32 oldY = gladiator->posY.whole;
    s16 damage;
    s16 attackPower;

    gladiator->posX.whole = entity->posX.whole;
    gladiator->posY.whole = entity->posY.whole;
    attackPower = Player_CalculateAttackPower(hit[2]);
    damage = Hit_ResolveDamage(ENTITY_OWNER(entity), attacker, hit, attackPower);
    gladiator->posX.whole = oldX;
    gladiator->posY.whole = oldY;
    Entity_ShowDamage(entity, damage, 0);
    Gladiator_OnDamaged(ENTITY_OWNER(entity), attacker, hit, damage);
}

/* Gladiator: dispatch the AI (0), the death (1) and a frozen state (2). */
void EnemyGladiatorUpdate(struct EwramData_EntityData *entity)
{
    u8 collision[48];

    if (entity->state != 1)
    {
        if (Entity_IsInRect(entity, -16, 0, 272, 176)) Player_TrackEntity(entity);
        if (entity->state != 1) Enemy_UpdateHitFlash(entity);
    }
    if (Entity_IsTimeStopped(entity) && entity->state != 1)
    {
        Hit_CheckWithPlayer(collision, entity);
        return;
    }
    switch (entity->state)
    {
    case 0:
        Gladiator_UpdateCharge(entity);
        break;
    case 1:
        Gladiator_UpdateDeath(entity);
        break;
    case 2:
        Anim_Update(entity);
        return;
    }
    if (entity->ext.gladiator.flinch == 0) Entity_AnimateWithBoxes(entity);
    Enemy_CheckHitPlayer(entity);
}

/* Gladiator death: slide to a stop and fall, sound once. */
void Gladiator_UpdateDeath(struct EwramData_EntityData *entity)
{
    if (entity->subState == 0)
    {
        EntityDampVelocityX(entity, 0x18);
        EntityAccelerateAndMove(entity, 0x50000);
        Enemy_UpdateFragmentDeath(entity);
        if (entity->stage == 0)
        {
            entity->stage++;
            PlaySong(0x6B);
        }
    }
}
