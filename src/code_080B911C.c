/* Witch affine spell projectile. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "macros.h"
extern void EntityMoveX(struct EwramData_EntityData *);
extern void EntityMoveY(struct EwramData_EntityData *);
extern s32 Target_GetX(struct EwramData_EntityData *);
extern s32 Target_GetY(struct EwramData_EntityData *);
extern s32 WitchSpell_MoveTowardStoredPoint(struct EwramData_EntityData *);
extern void WitchSpellTrailParticle_Update(struct EwramData_EntityData *);
extern void WitchSpell_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern u8 sWitchGfx[], sWitchPalette[], sWitchSpriteSheet[];

/**
 * @brief Allocate an affine spell sprite, grow it, steer toward a sampled
 * target point, then continue moving. Emit trail particles and release
 * the affine slot on removal; contact applies the spell damage callback.
 */
void WitchSpell_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = &gEwramData->entityData[entity->unk_50C.part8.unk_50C];
    struct EwramData_EntityData *child;
    struct BgLayerState *bg;
    u32 box[4];
    u16 affine;
    u16 offset;
    s32 timer;
    s32 random;
    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, sWitchGfx, sWitchPalette, 0, sWitchSpriteSheet)) goto remove;
        affine = ObjAffine_Alloc(1);
        if ((s16)affine < 0) goto remove;
        entity->affineIndex = affine;
        entity->affine = 1;
        entity->spriteLayer = 3;
        Anim_StartIndexed(entity, 4, 0, 1);
        Hit_Enable(entity, 8, WitchSpell_DamagePlayerOnContact, 0);
        HitBox_SetBodyRaw(entity, 0x2020F0F0);
        HitBox_SetAttackRaw(entity, 0x2020F0F0);
        entity->hit.flags |= 4;
        offset = 0xFFE0;
        entity->velX.whole = -0x18000;
        entity->unk_4F8.part16.unk_4FA = 0xFD00;
        if (entity->flipX)
        {
            entity->velX.whole = -entity->velX.whole;
            offset = 0x20;
            entity->unk_4F8.part16.unk_4FA = -entity->unk_4F8.part16.unk_4FA;
        }
        {
            u16 *x = &parent->posX.part16.integer;
            s32 value = (s16)offset;
            value += *x;
            x = &entity->posX.part16.integer;
            *x = value;
        }
        entity->posY.part16.integer = (u16)parent->posY.part16.integer - 0x10;
        PlaySong(0x12A);
        entity->state = 1;
        /* fall through */
    case 1:
        if (entity->unk_4F4.whole <= 0xFFFF) entity->unk_4F4.whole += 0x2000;
        else
        {
            s32 targetX = Target_GetX(entity);
            bg = (struct BgLayerState *)((u8 *)gEwramData + OFFSET_OF(struct EwramData, bgInfo[1]));
            entity->unk_500.whole = bg->xPos.whole + targetX;
            {
                s32 targetY = Target_GetY(entity);
                bg = (struct BgLayerState *)((u8 *)gEwramData + OFFSET_OF(struct EwramData, bgInfo[1]));
                entity->unk_504.whole = bg->yPos.whole + targetY - 0x100000;
            }
            entity->unk_4FC.part16.unk_4FC = 0x20;
            entity->state = 2;
        }
        break;
    case 2:
        if (WitchSpell_MoveTowardStoredPoint(entity)) entity->state = 3;
        if (entity->unk_4FC.part16.unk_4FC != 0) entity->unk_4FC.part16.unk_4FC--;
        else entity->state = 3;
        break;
    case 3:
        EntityMoveX(entity);
        EntityMoveY(entity);
        break;
    case 4:
        ObjAffine_Free(entity->affineIndex);
remove:
        EntityDelete(entity);
        return;
    }
    Hit_CheckWithPlayer(box, entity);
    if (EntityIsOutsideScreen(entity, 3)) entity->state = 4;
    timer = entity->timer;
    timer++;
    entity->timer = timer;
    if (!(timer & 1))
    {
        child = EntityCreateInRange(0x49, 0xCF, (u32 *)WitchSpellTrailParticle_Update);
        if (child != NULL)
        {
            {
                s32 offsetX = RandomSigned(0x20) << 16;
                s32 x = entity->posX.whole;
                x += offsetX;
                child->posX.whole = x;
            }
            {
                s32 offsetY = RandomSigned(0x20) << 16;
                s32 y = entity->posY.whole;
                y += offsetY;
                child->posY.whole = y;
            }
            random = RandomSigned(0x8000);
            child->velX.whole = entity->velX.whole / 4 + random;
            random = RandomSigned(0x2000);
            {
                s32 speed = entity->velY.whole / 1024;
                speed -= 0x14000;
                random += speed;
                child->velY.whole = random;
            }
            child->accelX.whole = -child->velX.whole / 64;
            child->accelY.whole = -child->velY.whole / 64;
        }
    }
    ObjAffine_Set((s16)entity->unk_4F8.part16.unk_4F8, entity->unk_4F4.whole, entity->unk_4F4.whole, entity->flipX, 0, entity->affineIndex);
    Anim_Update(entity);
    entity->unk_4F8.part16.unk_4F8 += entity->unk_4F8.part16.unk_4FA;
}
