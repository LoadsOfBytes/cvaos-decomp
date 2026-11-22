#include "code_08009A0.h"
#include "code/code_08039340.h"
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, u8, void *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *);
extern void Entity_SnapToGround(struct EwramData_EntityData *);
extern void Entity_EnableBoxes(struct EwramData_EntityData *, void *, void *);
extern void Hit_EnableContact(struct EwramData_EntityData *, s32, void *, void *);
extern void Enemy_OnContact(void);
extern void DiscArmor_OnHit(void);
extern void BomberRockArmor_OnHit(void);
extern u8 sDiscArmorGfx[], sDiscArmorPalette[], sDiscArmorSpriteSheet[];
extern u8 sBomberArmorRockArmorGfx[], sBomberArmorRockArmorPalette[], sBomberArmorSpriteSheet[], sRockArmorSpriteSheet[];

void EnemyDiscArmorCreate(struct EwramData_EntityData *entity)
{
    if (!Entity_LoadGraphics(entity, sDiscArmorGfx, sDiscArmorPalette, 0, sDiscArmorSpriteSheet))
    {
        EntityDelete(entity);
        return;
    }
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer = 0x12;
    Anim_StartIndexed(entity, 1, 0, 1);
    Anim_Update(entity);
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 3;
        return;
    }
    Entity_EnableBoxes(entity, Enemy_OnContact, DiscArmor_OnHit);
    Entity_UpdateBoxes(entity);
    Entity_SnapToGround(entity);
    entity->posY.part16.integer++;
    entity->scrollWithBg = 1;
    entity->state = 0;
}

