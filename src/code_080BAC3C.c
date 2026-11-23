/* Man Eater creation: affine slot, body segments.
 * (plain literal zeros and entity->flipX as the argument fix the allocation). */
#include "code_08009A0.h"
#include "code_08013960.h"
#include "code_08033CAC.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/entity_render_flags.h"
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void EntityDampVelocityX(struct EwramData_EntityData *, s32);
extern void Effect_SpawnRandom(s32, s32, s32, s32, s32);
extern struct EwramData_EntityData *Entity_CreateObject(void *);
extern u8 sManEaterGfx[], sManEaterPalette[], sManEaterSpriteSheet[];
extern const u8 sUnk_085288A8[][4];

extern void ManEaterAttachedPart_Update(struct EwramData_EntityData *);
void EnemyManEaterCreate(struct EwramData_EntityData *entity)
{
    u16 affine;
    s32 index;
    struct EwramData_EntityData *child;
    if (entity->unk_514.part16.spawnParam0 != 0 && gEwramData->gameState.bossRushMode == 0 && (gEwramData->gameState.bossEventFlags & 0x2000)) goto remove;
    if (!Entity_LoadGraphics(entity, sManEaterGfx, sManEaterPalette, 0, sManEaterSpriteSheet)) goto remove;
    affine = ObjAffine_Alloc(1);
    if ((s16)affine < 0)
    {
remove:
        EntityDelete(entity);
        return;
    }
    entity->affineIndex = affine;
    entity->affine = 1;
    entity->animFrame = 0;
    entity->flipX = 0;
    ObjAffine_Set(entity->ext.manEaterBody.angle, 0x10000, 0x10000, entity->flipX, 0, entity->affineIndex);
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer = 0x12;
    entity->ext.manEaterBody.headCount = 3;
    for (index = 0; index <= 2; index++)
    {
        child = Entity_CreateObject(ManEaterAttachedPart_Update);
        if (child != NULL)
        {
            child->ext.manEater.ownerIndex = ENTITY_INDEX(entity);
            child->ext.manEater.orbitSlot = index;
            child->scrollWithBg = 1;
        }
    }
    if (gEwramData->gameState.bestiaryPreview) entity->state = 4;
    else if (entity->unk_514.part16.spawnParam0 != 0)
    {
        Music_SetCurrent(0x10);
        Player_SetSelectedTarget(entity);
    }
}
