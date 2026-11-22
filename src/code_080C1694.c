/* Altair flight, carried-object release and screen exit. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
extern s32 Entity_GetTargetDistanceX(struct EwramData_EntityData *);

/**
 * @brief Flight state (1): carry and release the linked child through
 * animation events, then fly offscreen. Request child termination when
 * leaving before release; preserve the existing coordinate and lifetime tests.
 */
void Altair_UpdateFlight(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child = entity->unk_4F8.unk_4F8_entity;
    s32 distance;
    switch (entity->subState)
    {
    case 0:
        if (child != NULL)
        {
            Anim_StartIndexed(entity, 0, 0, 1);
            entity->unk_4FC.part8.unk_4FC = 0;
        }
        else
        {
            Anim_StartIndexed(entity, 1, 0, 1);
            entity->unk_4FC.part8.unk_4FC = 1;
        }
        entity->velX.whole = 0xA000;
        entity->velY.whole = 0;
        if (!entity->flipX) entity->velX.whole = -entity->velX.whole;
        entity->timer = 0x30;
        entity->subState++;
        /* fall through */
    case 1:
        entity->posX.whole += entity->velX.whole;
        if (child == NULL) break;
        distance = (s16)Entity_GetTargetDistanceX(entity);
        child->posX.whole = entity->posX.whole;
        child->posY.whole = entity->posY.whole + 0x130000;
        child->flipX = entity->flipX;
        if (!entity->flipX)
        {
            if (distance > 0x50)
            {
                if (!(RandomNumberGenerator() & 0x1F)) entity->unk_500.part8.unk_501 = 1;
            }
            else if (distance < -4) entity->unk_500.part8.unk_501 = 1;
        }
        else
        {
            if (distance < -0x50)
            {
                if (!(RandomNumberGenerator() & 0x1F)) entity->unk_500.part8.unk_501 = 1;
            }
            else if (distance > 4) entity->unk_500.part8.unk_501 = 1;
        }
        if ((u8)entity->unk_500.part8.unk_501 == 1 && (u16)(entity->posX.part16.integer - 0x21) <= 0xAE)
        {
            entity->subState++;
            entity->timer = 0;
        }
        break;
    case 2:
        Anim_StartIndexed(entity, 2, 0, 1);
        entity->unk_4FC.part8.unk_4FC = 2;
        entity->subState++;
        entity->timer = 0;
        /* fall through */
    case 3:
        {
            s32 x = entity->posX.whole;
            x += entity->velX.whole;
            entity->posX.whole = x;
            if (child != NULL)
            {
                child->posX.whole = x;
                child->posY.whole = entity->posY.whole + 0x110000;
                child->flipX = entity->flipX;
            }
        }
        if ((u8)entity->unk_4FC.part8.unk_4FC == 2 && entity->animFrame == 7 && entity->animTimer == 0)
        {
            if (child != NULL) child->subState++;
            entity->unk_500.part8.unk_500 = 0;
            entity->subState++;
            entity->timer = 0;
        }
        break;
    case 4:
        Anim_StartIndexed(entity, 1, 0, 1);
        entity->unk_4FC.part8.unk_4FC = 1;
        entity->subState++;
        entity->timer = 0;
        /* fall through */
    case 5:
        entity->posX.whole += entity->velX.whole;
        break;
    }
    {
        s16 *position = (s16 *)&entity->posX.part16.integer;
        s32 x = *position;
        s32 padding;
        position = &entity->velX.part16.integer;
        padding = *position;
        if (x < -padding - 0x80 || x > padding + 0x170)
        {
            struct EwramData_EntityData *attached;
            entity->deleteRequested = 1;
            attached = entity->unk_4F8.unk_4F8_entity;
            if (attached != NULL && entity->subState <= 2)
            {
                attached->state = 0xFF;
                attached->timer = 0;
            }
        }
    }
}
