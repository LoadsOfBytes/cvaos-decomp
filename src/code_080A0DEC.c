#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern void Hit_CheckWithPlayerSharedHistory(void *res, struct EwramData_EntityData *entity, struct EwramData_EntityData *owner);
extern void Gladiator_SpawnFragments(struct EwramData_EntityData *entity, s32 x, s32 y);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *);
extern const s16 sUnk_08119174[4];
extern const s16 sGladiatorBodyDx[12];

/**
 * @brief A0DEC | Gladiator body part (owner type 0x5D in unk_51A): follows
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
    struct EwramData_EntityData *owner = entity->unk_4F8.unk_4F8_entity;

    memcpy(dy, sUnk_08119174, sizeof(dy));
    memcpy(dx, sGladiatorBodyDx, sizeof(dx));
    if (entity->state == 0)
    {
        s32 x;
        s32 y;
        if (owner == NULL || owner->updateFunc == NULL || owner->unk_518.part8.enemyId != 0x5D)
            goto delete;
        entity->hidden = owner->hidden;
        if (owner->palette == 4)
            entity->palette = 4;
        else
            entity->palette = entity->unk_510.part8.basePalette;
        entity->flipX = owner->flipX;
        entity->posX.part16.integer = owner->posX.part16.integer;
        entity->posY.part16.integer = owner->posY.part16.integer;
        if (entity->flipX)
            entity->posX.part16.integer = entity->posX.part16.integer - entity->unk_4F4.part16.unk_4F4;
        else
            entity->posX.part16.integer += entity->unk_4F4.part16.unk_4F4;
        entity->posY.part16.integer += entity->unk_4F4.part16.unk_4F6;
        if (owner->animScriptIndex == 0xC)
        {
            if (owner->animTimer)
            {
                entity->posY.part16.integer = entity->posY.part16.integer + dy[owner->animStep];
                entity->unk_4FC.part16.unk_4FE[0] = dy[owner->animStep];
            }
            else
            {
                entity->posY.part16.integer = entity->posY.part16.integer + entity->unk_4FC.part16.unk_4FE[0];
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
