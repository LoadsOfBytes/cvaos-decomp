#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void BeamSkeletonBeamSprite_Update(struct EwramData_EntityData *entity);
extern void BeamSkeletonBeam_Update(struct EwramData_EntityData *entity);
extern void Enemy_OnContact(struct EwramData_EntityData *entity);
extern s8 ObjPalette_FindOrLoad(void *palette);
extern void Hit_Enable(struct EwramData_EntityData *entity,s32 mode,void *callback,s32 flags);
extern void HitBox_SetAttackRaw(struct EwramData_EntityData *entity,s32 flags);
extern u8 sBeamSkeletonPalette[];
extern u8 sBeamSkeletonSpriteSheet[];

/**
 * @brief Create a stationary beam sprite and moving contact entity using
 * the owner graphics and affine index. Preserve the distinct 1.5 and
 * integer 3/2 damage expressions and all allocation-failure behavior.
 */
void BeamSkeleton_CreateBeam(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *beam = EntityCreateInRange(0x49,0xCF,(u32 *)BeamSkeletonBeamSprite_Update);
    s32 i;
    if (beam != NULL)
    {
        s32 graphics;
        void *sprite;
        beam->flipX = entity->flipX;
        beam->posX.whole = -0xB0000;
        beam->posY.whole = -0x110000;
        if (beam->flipX)
            beam->posX.whole = -beam->posX.whole;
        beam->posX.whole += entity->posX.whole;
        beam->posY.whole += entity->posY.whole;
        beam->unk_4F4.part8.unk_4F6 = entity->unk_4F4.part8.unk_4F4;
        beam->scrollWithBg = 1;
        beam->drawFunc = (u32 *)Sprite_DrawEntity;
        sprite = sBeamSkeletonSpriteSheet;
        graphics = Sprite_GetTileSlot(entity);
        Sprite_Attach(beam,sprite,graphics,ObjPalette_FindOrLoad(sBeamSkeletonPalette));
        beam->animFrame = 21;
        beam->objMode = 1;
        beam->spriteLayer = 18;
        beam->affineIndex = entity->affineIndex;
        beam->affine = 1;
        beam->attack = entity->attack * 1.5;
        beam->defense = entity->defense;
        beam->shrinkPartOffsets = 1;
    }
    for (i = 0; i < 1; i++)
    {
        beam = EntityCreateInRange(0x49,0xCF,(u32 *)BeamSkeletonBeam_Update);
        if (beam != NULL)
        {
            s32 graphics;
            beam->flipX = entity->flipX;
            beam->posX.whole = -0xB0000;
            beam->posY.whole = -0x110000;
            if (beam->flipX)
                beam->posX.whole = -beam->posX.whole;
            beam->posX.whole += entity->posX.whole;
            beam->posY.whole += entity->posY.whole;
            beam->unk_4F4.part8.unk_4F6 = entity->unk_4F4.part8.unk_4F4;
            beam->scrollWithBg = 1;
            beam->drawFunc = (u32 *)Sprite_DrawEntity;
            graphics = Sprite_GetTileSlot(entity);
            Sprite_Attach(beam,sBeamSkeletonSpriteSheet,graphics,ObjPalette_FindOrLoad(sBeamSkeletonPalette));
            beam->animFrame = 22;
            beam->spriteLayer = 9;
            beam->affineIndex = entity->affineIndex;
            beam->attack = entity->attack * 3 / 2;
            beam->defense = entity->defense;
            Hit_Enable(beam,8,Enemy_OnContact,0);
            HitBox_SetAttackRaw(beam,0x0C08FAFC);
            beam->hit.flags |= 4;
            beam->timer = i;
        }
    }
}
