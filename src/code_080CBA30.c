/* Buer_CreateHead. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern struct EwramData_EntityData *Entity_CreateObject(u32 *updateFunc);
extern void BuerHead_Update(struct EwramData_EntityData *entity);
extern u8 sBaelBuerSpriteSheet[];

/**
 * @brief Create the linked frame-1 head sharing graphics and palette; store mutual slot indices and face it according to movement speed.
 */
void Buer_CreateHead(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child = Entity_CreateObject((u32 *)BuerHead_Update);

    if (child != NULL)
    {
        child->unk_4F4.part8.unk_4F5 = entity->unk_4F4.part8.unk_4F4;
        entity->unk_4F4.part8.unk_4F5 = child->unk_4F4.part8.unk_4F4;
        child->spriteLayer = entity->spriteLayer - 1;
        child->unk_510.part8.basePalette = entity->unk_510.part8.basePalette;
        child->flipX = entity->flipX;
        Sprite_Attach(child, sBaelBuerSpriteSheet, Sprite_GetTileSlot(entity), child->unk_510.part8.basePalette);
        child->animFrame = 1;
        child->scrollWithBg = 1;
        child->drawFunc = (u32 *)Sprite_DrawEntity;
        child->posX.whole = entity->posX.whole;
        child->posY.whole = entity->posY.whole;
        if ((s32)entity->unk_4FC.whole >= 0)
            child->flipX = 1;
        else
            child->flipX = 0;
    }
}


/* Bael/Alastor part behaviour 0x080CBAEC; only matches without -f2003-patch. */
extern s32 Buer_CheckCleanup(struct EwramData_EntityData *entity);
extern void Buer_MoveAndRotate(struct EwramData_EntityData *entity, s32 mode);
extern s32 Target_IsNearEntity(struct EwramData_EntityData *entity, u16 width, u16 height);
extern s32 Entity_GetTargetDirection(struct EwramData_EntityData *entity);
extern void Buer_UpdateCharge(struct EwramData_EntityData *entity);

#define ABS(x) ((x) < 0 ? -(x) : (x))

/**
 * @brief Move, choose a charge from target proximity and linked-head facing, or periodically reverse facing and reconsider movement.
 */
void Buer_UpdateActive(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *other = &gEwramData->entityData[entity->unk_4F4.part8.unk_4F5];
    // zero and timer: explicit temporaries for the original register use
    s32 zero;
    s32 timer;

    if (Buer_CheckCleanup(entity))
        return;
    switch (entity->subState)
    {
        case 0:
            Buer_MoveAndRotate(entity, 0);
            if (Target_IsNearEntity(entity, 0xC0, 0x30)
                && ((!other->flipX && (s8)Entity_GetTargetDirection(entity) == -1)
                    || (other->flipX && (s8)Entity_GetTargetDirection(entity) == 1)))
            {
                entity->subState = 1;
                entity->stage = 0;
            }
            timer = entity->timer + 1;
            zero = 0;
            entity->timer = timer;
            if ((u8)timer == 0xFF)
            {
                entity->timer = zero;
                if (!(RandomNumberGenerator() % 2))
                    entity->subState = 2;
                goto restart;
            }
            break;
        case 1:
            Buer_UpdateCharge(entity);
            break;
        case 2:
            switch (entity->stage)
            {
                case 0:
                    entity->timer = RandomNumberGenerator() % 32 + 32;
                    entity->stage++;
                case 1:
                    other->flipX = 1 - other->flipX;
                    entity->stage++;
                case 2:
                    if (Target_IsNearEntity(entity, 0x60, 0x30))
                    {
                        entity->subState = 1;
                        entity->stage = 0;
                    }
                    timer = entity->timer - 1;
                    zero = 0;
                    entity->timer = timer;
                    if ((u8)timer == 0xFF)
                    {
                        if (RandomNumberGenerator() % 2)
                            goto restart;
                        entity->subState = zero;
                        entity->stage = zero;
                        entity->timer = zero;
                        // unk_4FC is read as signed here
                        if (other->flipX)
                            *(s32 *)&entity->unk_4FC = ABS(*(s32 *)&entity->unk_4FC);
                        else
                            *(s32 *)&entity->unk_4FC = -ABS(*(s32 *)&entity->unk_4FC);
                    }
                    break;
            }
            break;
    }
    return;
restart:
    entity->stage = zero;
}
