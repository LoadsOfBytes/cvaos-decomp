/* Man Eater body segment: follows the head chain, swings, and breaks apart.
 * (flipX through an s8 byte pointer, one
 * variable for the divisor and the swing amplitude). */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern struct EwramData_EntityData *Entity_CreateObject(u32 *);
extern void Hit_CheckWithPlayerSharedHistory(void *, struct EwramData_EntityData *, struct EwramData_EntityData *);
extern void Enemy_OnContact(struct EwramData_EntityData *);
extern void ManEaterPart_OnHit(struct EwramData_EntityData *, struct EwramData_EntityData *, u16 *);
extern void ManEaterProjectile_Update(struct EwramData_EntityData *);
extern u8 sManEaterGfx[], sManEaterPalette[], sManEaterSpriteSheet[];

#define INDEX(e) ((e)->unk_4F8.part8.unk_4FB)
#define SWING(e) ((e)->unk_4FC.part16.unk_4FC)
#define SWING_SPEED(e) ((e)->unk_4FC.part16.unk_4FE[0])

void ManEaterSegment_Update(struct EwramData_EntityData *entity)
{
    u8 box[16];
    struct EwramData_EntityData *head = &gEwramData->entityData[(u8)entity->unk_4F8.part8.unk_4F8];
    struct EwramData_EntityData *prev = &gEwramData->entityData[entity->unk_4F8.part8.unk_4F9];
    struct EwramData_EntityData *next = &gEwramData->entityData[entity->unk_4F8.part8.unk_4FA];
    s32 amplitude;
    s32 angle;

    if ((head->unk_4F4.part8.unk_4F5 & 4) && head->subState < (INDEX(entity) + 1) * 4)
    {
        struct EwramData_EntityData *debris = Effect_SpawnShared(entity->posX.whole, entity->posY.whole, 13, 8);
        if (debris != NULL)
        {
            { s32 f = RandomNumberGenerator() % 2; s8 *p = (s8 *)debris + 0x58; s32 t = f << 6; *p = (*p & ~0x40) | t; }
            debris->flipY = RandomNumberGenerator() % 2;
        }
        PlaySong(0x70);
        goto remove;
    }
    if (entity->state <= 2 && (head->unk_4F4.part8.unk_4F5 & 1))
    {
        entity->unk_500.part8.unk_500 = entity->state;
        entity->state = 3;
        entity->subState = 0;
    }
    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, sManEaterGfx, sManEaterPalette, 0, sManEaterSpriteSheet))
        {
remove:
            EntityDelete(entity);
            return;
        }
        Anim_StartIndexed(entity, 4, 0, 1);
        {
            struct AnimScript *anim = entity->animScript;
            entity->animStep = (6 - INDEX(entity)) % anim->stepCount;
        }
        if ((entity->stage = INDEX(entity)) == 5)
        {
            entity->unk_500.part8.unk_501 = 1;
            Anim_StartIndexed(entity, 3, 0, 1);
        }
        entity->scrollWithBg = 1;
        entity->unk_504.whole = 0x60000;
        SWING_SPEED(entity) = 0x60;
        entity->unk_500.part16.unk_502 = INDEX(entity) * 8;
        if (SWING(entity) <= 0x2000)
            entity->state = 1;
        else
        {
            SWING(entity) = 0x4000 - SWING(entity);
            entity->state = 2;
        }
        entity->unk_518.part8.enemyId = head->unk_518.part8.enemyId;
        Enemy_ResetFromTemplate(entity);
        if (gEwramData->gameState.bossRushMode)
        {
            entity->attack += 0x14;
            entity->unk_518.part16.hp += 1000;
        }
        entity->spriteLayer = 0x11;
        Hit_Enable(entity, 8, Enemy_OnContact, (s32)ManEaterPart_OnHit);
        HitBox_SetBodyRaw(entity, 0x1010F8F8);
        HitBox_SetAttackRaw(entity, 0x1010F8F8);
        break;
    case 1:
        SWING(entity) += SWING_SPEED(entity);
        if (SWING(entity) > 0x2000)
        {
            SWING(entity) = 0x2000;
            entity->state = 2;
        }
        break;
    case 2:
        SWING(entity) -= SWING_SPEED(entity);
        if (SWING(entity) < -0x2000)
        {
            SWING(entity) = -0x2000;
            entity->state = 1;
        }
        break;
    case 3:
        switch (entity->subState)
        {
        case 0:
            entity->accelX.whole = head->posX.whole - entity->posX.whole;
            entity->accelY.whole = head->posY.whole - entity->posY.whole;
            entity->timer = 0;
            entity->subState++;
        case 1:
        {
            s32 x, y, nx;
            entity->posX.whole = x = head->posX.whole - entity->accelX.whole;
            entity->posY.whole = y = head->posY.whole - entity->accelY.whole;
            amplitude = INDEX(entity) * entity->timer / (7 - INDEX(entity)) + 0x60;
            nx = next->posX.whole;
            nx += (x - nx) * 0x60 / amplitude;
            {
                s32 ny = next->posY.whole;
                s32 q = (y - ny) * 0x60 / amplitude;
                entity->posX.whole = nx;
                entity->posY.whole = ny + q;
            }
            if (entity->timer < 0x30)
                entity->timer += 2;
            if (head->state == 3)
                entity->subState++;
            break;
        }
        case 2:
            entity->posX.whole += (head->posX.whole - entity->accelX.whole - entity->posX.whole) / 12;
            entity->posY.whole += (head->posY.whole - entity->accelY.whole - entity->posY.whole) / 12;
            if (head->state != 3)
                entity->state = entity->unk_500.part8.unk_500;
            break;
        }
        break;
    }
    if (entity->state <= 2)
    {
        if (INDEX(entity) != 0)
            amplitude = (s32)head->unk_504.whole / 256;
        else
            amplitude = 0;
        angle = (s16)(head->unk_4FC.part16.unk_4FC + (prev->unk_4FC.part16.unk_4FC + SWING(entity)));
        entity->posX.whole = prev->posX.whole + Sin(angle + 0x4000) / 256 * amplitude;
        entity->posY.whole = prev->posY.whole + Sin(angle) / 256 * amplitude;
        if (*(u8 *)&entity->unk_500.part8.unk_501 && (s16)head->unk_518.part16.hp <= 0xC7
            && entity->animStep <= 2 && entity->animTimer == 0 && (RandomNumberGenerator() & 7))
        {
            struct EwramData_EntityData *drop = Entity_CreateObject((u32 *)ManEaterProjectile_Update);
            if (drop != NULL)
            {
                drop->posX.whole = entity->posX.whole;
                drop->posY.whole = entity->posY.whole;
                drop->attack = entity->attack - (entity->attack >> 3);
            }
        }
    }
    entity->stage++;
    if (!(entity->stage & 7) && Entity_IsInRect(entity, -0x10, 0, 0x110, 0xB0))
        Hit_CheckWithPlayerSharedHistory(box, entity, head);
    Anim_Update(entity);
}
