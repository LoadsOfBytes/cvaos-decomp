#include "code_08009A0.h"
#include "code/code_08039340.h"
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, u8, void *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void Entity_EnableBoxes(struct EwramData_EntityData *, void *, void *);
extern void Enemy_OnContact(void);
extern void Ripper_OnHit(void);
extern void Entity_SnapToGround(struct EwramData_EntityData *);
extern u8 sFleamanRipperPalette[], sFleamanRipperSpriteSheet[], sFleamanRipperGfx[];

void EnemyRipperCreate(struct EwramData_EntityData *entity)
{
    void *palette = sFleamanRipperPalette;
    void *sprites = sFleamanRipperSpriteSheet;
    void *graphics = sFleamanRipperGfx;
    if (!Entity_LoadGraphics(entity, graphics, palette, 1, sprites))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 3;
        entity->stage = 0;
        entity->subState = 0;
        return;
    }
    entity->state = 1;
    entity->stage = 0;
    entity->subState = 0;
    if ((entity->unk_514.part16.spawnParam0 & entity->unk_514.part16.spawnParam1) == 0xFF)
        entity->unk_4F8.part8.unk_4F8 = 1;
    else entity->unk_4F8.part8.unk_4F8 = 0;
    Entity_EnableBoxes(entity, Enemy_OnContact, Ripper_OnHit);
    if ((u8)entity->unk_4F8.part8.unk_4F8 != 1) Entity_SnapToGround(entity);
}
