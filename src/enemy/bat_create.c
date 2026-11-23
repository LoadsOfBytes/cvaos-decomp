/* Place a bat on the ceiling, unless time is frozen (EnemyBatCreate).
 * Needs -fno-cse-follow-jumps (Makefile). */
#include "core.h"
#include "graphics.h"
#include "structs/ewram.h"
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, u8 *, void *, u8, void *);
extern s32 GetEntityRoomXPositionInteger(struct EwramData_EntityData *);
extern s32 GetEntityRoomYPositionInteger(struct EwramData_EntityData *);
extern s32 Collision_GetPushDown(s32, s32);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void Entity_EnableBoxes(struct EwramData_EntityData *, void *, void *);
extern void Enemy_OnContact(struct EwramData_EntityData *);
extern void Bat_OnHit(struct EwramData_EntityData *);
extern u8 sBatGfx[], sBatPalette[], sBatSpriteSheet[];
static inline void SetState(struct EwramData_EntityData *entity, s8 value4F0, u8 state)
{
    entity->state = state;
    entity->stage = value4F0;
    entity->subState = 0;
}
void EnemyBatCreate(struct EwramData_EntityData *entity)
{
    u8 *palette = sBatPalette;
    u8 *sprite = sBatSpriteSheet;
    u8 *graphics = sBatGfx;
    s32 hit;
    s16 *y;
    s16 *py;
    s32 x;
    if (!Entity_LoadGraphics(entity, graphics, palette, 0, sprite))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    hit = *((u8 *)&gEwramData->gameState + 0x2E) & 0x40;
    if (hit != 0)
    {
        SetState(entity, 0, 4);
    }
    else
    {
        SetState(entity, 0, 1);
        y = &entity->posY.part16.integer;
        py = y;
        do
        {
            x = GetEntityRoomXPositionInteger(entity);
            hit = Collision_GetPushDown(x, GetEntityRoomYPositionInteger(entity));
            if (hit == 0)
                *py -= 8;
        } while (hit == 0);
        *y += hit;
        Entity_EnableBoxes(entity, Enemy_OnContact, Bat_OnHit);
    }
}
