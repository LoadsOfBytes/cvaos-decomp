#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

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
extern void GladiatorPart_OnHit(struct EwramData_EntityData *entity);
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
