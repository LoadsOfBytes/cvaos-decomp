#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern void Hit_CheckWithPlayerSharedHistory(void *res, struct EwramData_EntityData *entity, struct EwramData_EntityData *owner);
extern void Gladiator_SpawnFragments(struct EwramData_EntityData *entity, s32 x, s32 y);
extern void GladiatorArm_FireShot(struct EwramData_EntityData *entity, s32 dx, s32 dy, s32 vx, s32 vy);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *);
extern const s16 sUnk_08119174[4];
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
