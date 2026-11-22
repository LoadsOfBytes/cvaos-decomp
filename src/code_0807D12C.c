#include "code_08009A0.h"
#include "giant_ghost_entity.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern s32 GiantGhost_LoadGraphics(struct EwramData_EntityData *entity);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *entity);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *entity);
extern s32 Anim_GetMaxBoxCount(struct EwramData_EntityData *entity);
extern void GiantGhost_ResetAnimation(struct EwramData_EntityData *entity);
extern struct EwramData_EntityData *Entity_CreateObject(u32 *update);
extern void GiantGhostAura_Update(struct EwramData_EntityData *entity);
extern void GiantGhostHandA_Update(struct EwramData_EntityData *entity);
extern void GiantGhostHandB_Update(struct EwramData_EntityData *entity);
extern void GiantGhost_DamagePlayerOnContact(struct EwramData_EntityData *entity);
extern void GiantGhost_OnHit(struct EwramData_EntityData *entity, s32 other, u16 *hit);
extern s32 Hit_EnableContact(struct EwramData_EntityData *entity, s32 type, void *onHit, s32 onHurt);
extern u8 sGiantGhostGfx[], sGiantGhostSpriteSheet[];

/**
 * @brief 7D12C | Giant Ghost create: three parts Entity_CreateObject
 * (GiantGhostAura_Update / GiantGhostHandA_Update / GiantGhostHandB_Update, animations 6 / 8 / 7)
 * sharing the graphics sGiantGhostSpriteSheet and the palette, the first one in
 * front with its own palette (ObjAffine_Alloc) and blending; frozen time
 * keeps the parts 32 px left and 40 px up and gives state 3; otherwise box
 * slot 8 (Hit_EnableContact when Anim_GetMaxBoxCount > 1), state 0 and GiantGhost_ResetAnimation.
 * Any failed allocation deletes it.
 */
void EnemyGiantGhostCreate(struct EwramData_EntityData *entity)
{
    s32 i;
    struct EwramData_EntityData *part;
    u16 palette;

    if ((u8)GiantGhost_LoadGraphics(entity) == 0)
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    for (i = 0; i <= 2; i++)
    {
        part = NULL;
        switch (i)
        {
        case 0:
            part = Entity_CreateObject((u32 *)GiantGhostAura_Update);
            break;
        case 1:
            part = Entity_CreateObject((u32 *)GiantGhostHandA_Update);
            break;
        case 2:
            part = Entity_CreateObject((u32 *)GiantGhostHandB_Update);
            break;
        }
        if (part != NULL)
        {
        if (i == 0)
            part->spriteLayer = entity->spriteLayer + 1;
        else
            part->spriteLayer = entity->spriteLayer - 1;
        Sprite_Attach(part, sGiantGhostSpriteSheet, ObjGfx_Find(sGiantGhostGfx), entity->unk_510.part8.basePalette);
        part->unk_518.part8.enemyId = entity->unk_518.part8.enemyId;
        part->unk_510.part8.basePalette = entity->unk_510.part8.basePalette;
        part->scrollWithBg = 0;
        part->drawFunc = (u32 *)Sprite_DrawEntity;
        ENTITY_OWNER(part) = entity;
        GIANT_GHOST_HAND_INDEX(part) = i;
        part->posX.whole = entity->posX.whole;
        part->posY.whole = entity->posY.whole;
        switch (i)
        {
        case 0:
            Anim_StartIndexed(part, 6, 0, 1);
            break;
        case 1:
            Anim_StartIndexed(part, 8, 0, 1);
            break;
        case 2:
            Anim_StartIndexed(part, 7, 0, 1);
            break;
        }
        Anim_Update(part);
        if (i == 0)
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
            part->doubleSize = 1;
            part->objMode = 1;
        }
        if (gEwramData->gameState.bestiaryPreview)
        {
            GIANT_GHOST_HAND_POINT_X(part) = part->posX.whole - 0x200000;
            GIANT_GHOST_HAND_POINT_Y(part) = part->posY.whole - 0x280000;
            GIANT_GHOST_HAND_SWAY_DIR(part) = 0;
        }
        continue;
        }
        EntityDelete(entity);
        return;
    }
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 3;
        return;
    }
    if (Anim_GetMaxBoxCount(entity) > 1)
        Hit_EnableContact(entity, 8, GiantGhost_DamagePlayerOnContact, (s32)GiantGhost_OnHit);
    else
        Hit_Enable(entity, 8, GiantGhost_DamagePlayerOnContact, (s32)GiantGhost_OnHit);
    Entity_UpdateBoxes(entity);
    entity->state = 0;
    GiantGhost_ResetAnimation(entity);
}
