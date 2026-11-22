#include "code_08009A0.h"
#include "balore_entity.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

extern u8 sBaloreSpriteSheet[];
extern void BaloreHandHitBox_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void BaloreHandHitBox_OnHit(struct EwramData_EntityData *);
extern s32 Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
#define HIT_FLAGS(e) (((u8 *)&(e)->unk_51C)[1])
#define TO_PIXELS(x) ((x) < 0 ? -(-(x) >> 16) : (x) >> 16)

/* Balore hand's attached collision part: follows its angle and extension. */
void BaloreHandHitBox_Update(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    struct EwramData_EntityData *body = BALORE_BODY(entity);
    struct EwramData_EntityData *hand = BALORE_HAND(entity);
    u16 matrix;
    s32 angle, radius, x, y;
    u8 flags;
    u8 state;
    if (entity->state != 0 && hand->state == 10)
        entity->state = 3;
    entity->accelX.whole = entity->posX.whole;
    entity->accelY.whole = entity->posY.whole;
    switch (entity->state)
    {
    case 0:
        matrix = ObjAffine_Alloc(1);
        if ((s16)matrix < 0) goto delete;
        entity->affineIndex = matrix;
        entity->affine = 1;
        entity->spriteLayer = 0x19;
        Sprite_Attach(entity, sBaloreSpriteSheet, (s8)BALORE_GFX(body), hand->unk_510.part8.basePalette);
        Anim_StartIndexed(entity, 0x30, 2, 0);
        Anim_Update(entity);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        Hit_Enable(entity, 8, BaloreHandHitBox_DamagePlayerOnContact, (s32)BaloreHandHitBox_OnHit);
        HitBox_SetAttackRaw(entity, 0x2828ECEC);
        HitBox_SetBodyRaw(entity, 0x2828ECEC);
        entity->state = 1;
        break;
    case 1:
        if (hand->state == 7)
        {
            entity->hit.flags |= 2;
            entity->state = 2;
        }
        break;
    case 2:
        state = hand->state;
        if (state == 1)
        {
            entity->hit.flags &= 0xFD;
            entity->state = state;
        }
        break;
    case 3:
        ObjAffine_Free(entity->affineIndex);
    delete:
        EntityDelete(entity);
        return;
    }
    entity->palette = hand->palette;
    entity->hidden = hand->hidden;
    flags = HIT_FLAGS(hand);
    HIT_FLAGS(entity) = flags;
    if (entity->hidden) return;
    if (hand->flipX)
        angle = BALORE_ARM_ANGLE(hand);
    else
        angle = BALORE_ARM_ANGLE(hand) + 0x8000;
    radius = TO_PIXELS(BALORE_ARM_LENGTH(hand));
    x = radius * Sin(angle + 0x4000);
    entity->posX.whole = hand->posX.whole + x;
    y = Sin(angle) * radius;
    entity->posY.whole = hand->posY.whole + y;
    entity->velX.whole = entity->posX.whole - entity->accelX.whole;
    entity->velY.whole = entity->posY.whole - entity->accelY.whole;
    if (HIT_FLAGS(entity) & 4) Player_QueueEntity(entity, 3);
    Hit_CheckWithPlayer(collision, entity);
    ObjAffine_Set(BALORE_ARM_ANGLE(hand), 0x10000, 0x10000, hand->flipX, 0, entity->affineIndex);
    if (entity->animScriptIndex != ((u8 *)&hand->unk_51C)[0])
        Anim_StartIndexed(entity, ((u8 *)&hand->unk_51C)[0], 2, 0);
    Anim_Update(entity);
}

