#include "code_08009A0.h"
#include "gladiator_entity.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"
/* Shot of the arm: facing of the arm at the shot; never read. */
#define GLADIATOR_SHOT_UNUSED_DIR(e) e->unk_4FC.part8.unk_4FD

extern void GladiatorShot_Update(struct EwramData_EntityData *entity);
extern void GladiatorShot_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
extern void GladiatorShot_CreateTip(struct EwramData_EntityData *entity);
extern u8 sGladiatorGfx[], sGladiatorSpriteSheet[];

/**
 * @brief A15CC | Gladiator shot: entity GladiatorShot_Update at (dx, dy) from the
 * owner with velocity (vx, vy), graphics sGladiatorSpriteSheet / sGladiatorGfx
 * with the owner's palette, animation 0xE, box slot 8 (GladiatorShot_OnHit) with
 * box 0x0404FEFE (boxes off), layer one below, owner in unk_4F8, the
 * owner's facing in unk_4FD and its attack; own palette with blending and
 * an affine matrix pointing along the facing (unk_502 0 / 0x8000); then
 * GladiatorShot_CreateTip. Deleted when the graphics or palette allocation fails.
 */
void GladiatorArm_FireShot(struct EwramData_EntityData *entity, s32 dx, s32 dy, s32 vx, s32 vy)
{
    u32 box = 0x0404FEFE;
    struct EwramData_EntityData *shot = EntityCreateInRange(0x1B, 0x48, (u32 *)GladiatorShot_Update);
    s32 tiles;
    u16 palette;

    if (shot == NULL)
        return;
    tiles = ObjGfx_Find(sGladiatorGfx);
    if (tiles < 0)
        goto delete;
    Sprite_Attach(shot, sGladiatorSpriteSheet, tiles, entity->unk_510.part8.basePalette);
    Anim_StartIndexed(shot, 0xE, 0, 0);
    Anim_Update(entity);
    shot->drawFunc = (u32 *)Sprite_DrawEntity;
    Hit_Enable(shot, 8, NULL, (s32)GladiatorShot_OnHit);
    HitBox_SetBody(shot, &box);
    shot->hit.flags &= 0xFB;
    shot->hit.flags |= 2;
    shot->scrollWithBg = 1;
    shot->spriteLayer = entity->spriteLayer - 1;
    shot->unk_510.part8.basePalette = entity->unk_510.part8.basePalette;
    shot->unk_518.part8.enemyId = entity->unk_518.part8.enemyId;
    ENTITY_OWNER(shot) = entity;
    shot->posX.whole = entity->posX.whole + dx;
    shot->posY.whole = entity->posY.whole + dy;
    shot->velX.whole = vx;
    shot->velY.whole = vy;
    GLADIATOR_SHOT_UNUSED_DIR(shot) = entity->flipX;
    shot->attack = entity->attack;
    if (entity->flipX)
        GLADIATOR_SHOT_ANGLE(shot) = 0;
    else
        GLADIATOR_SHOT_ANGLE(shot) = 0x8000;
    palette = ObjAffine_Alloc(1);
    if ((s16)palette < 0)
    {
    delete:
        EntityDelete(shot);
        return;
    }
    shot->affineIndex = palette;
    shot->affine = 1;
    ObjAffine_Set((u16)GLADIATOR_SHOT_ANGLE(shot) - 0x8000, 0x10000, 0x10000, shot->flipX, shot->flipY, shot->affineIndex);
    GladiatorShot_CreateTip(shot);
}
