#include "code_08009A0.h"
#include "balore_entity.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern u8 sBaloreSpriteSheet2[];
extern void BaloreDebris_Update(struct EwramData_EntityData *);
#define TO_PIXELS(x) ((x) < 0 ? -(-(x) >> 16) : (x) >> 16)

/* Second Balore hand family's attached part, spawning effects at animation 8. */
void BaloreHandBHitBox_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *body = BALORE_BODY(entity);
    struct EwramData_EntityData *hand = BALORE_HAND(entity);
    struct EwramData_EntityData *child;
    u16 matrix;
    s32 x, y;
    if (hand->hidden || body->state == 10)
        entity->state = 4;
    switch (entity->state)
    {
    case 0:
        matrix = ObjAffine_Alloc(1);
        if ((s16)matrix < 0) goto delete;
        entity->affineIndex = matrix;
        entity->affine = 1;
        entity->spriteLayer = hand->spriteLayer;
        Sprite_Attach(entity, sBaloreSpriteSheet2, (s8)BALORE_GFX(body), hand->unk_510.part8.basePalette);
        Anim_StartIndexed(entity, 10, 2, 0);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->state = 1;
        break;
    case 1:
        if (entity->animScriptIndex == 8)
        {
            entity->state = 2;
            entity->timer = 16;
        }
        break;
    case 2:
        if (entity->timer != 0)
        {
            if ((entity->timer & 3) == 0)
            {
                struct EwramData_EntityData *spark;
                x = (s16)(0xFFF8 + entity->posX.part16.integer + (RandomNumberGenerator() & 7));
                y = (s16)(entity->posY.part16.integer - 22);
                spark = EntityCreateInRange(0x49, 0xCF, (u32 *)BaloreDebris_Update);
                if (spark != NULL)
                {
                    BALORE_BODY(spark) = body;
                    spark->subState = 12;
                    spark->posX.part16.integer = x;
                    spark->posY.part16.integer = y;
                    spark->spriteLayer = 0x1C;
                    spark->scrollWithBg = 1;
                    spark->spriteLayer = 0x14;
                }
            }
            entity->timer--;
        }
        else
        {
            x = (s16)entity->posX.part16.integer;
            y = (s16)(entity->posY.part16.integer + 24);
            child = EntityCreateInRange(0x49, 0xCF, (u32 *)BaloreDebris_Update);
            if (child != NULL)
            {
                BALORE_BODY(child) = body;
                child->subState = 11;
                child->posX.part16.integer = x;
                child->posY.part16.integer = y;
                child->spriteLayer = 0x1C;
                child->scrollWithBg = 1;
            }
            entity->state = 3;
        }
        break;
    case 3:
        break;
    case 4:
        if (entity->affine) ObjAffine_Free(entity->affineIndex);
    delete:
        EntityDelete(entity);
        return;
    }
    entity->palette = hand->palette;
    if (hand->flipX)
        x = BALORE_ARM_ANGLE(hand);
    else
        x = BALORE_ARM_ANGLE(hand) + 0x8000;
    y = TO_PIXELS(BALORE_ARM_LENGTH(hand));
    entity->posX.whole = hand->posX.whole + y * Sin(x + 0x4000);
    entity->posY.whole = hand->posY.whole + y * Sin(x);
    ObjAffine_Set(BALORE_ARM_ANGLE(hand), 0x10000, 0x10000, hand->flipX, 0, entity->affineIndex);
    if (entity->animScriptIndex != ((u8 *)&hand->unk_51C)[0])
        Anim_StartIndexed(entity, ((u8 *)&hand->unk_51C)[0], 2, 0);
    Anim_Update(entity);
}
