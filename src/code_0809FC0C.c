/* Spawn a shot (LegionShot_Update) at the owner + (dx, dy) moving at speed 8 in
 * the given angle; graphics and palette come from the owner's parent data,
 * the frame and flip from a 24-entry table indexed by the angle. */
#include "code_08009A0.h"
#include "legion_entity.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

extern s32 FixedMultiply(s32 a, s32 b);
extern void LegionShot_Update(struct EwramData_EntityData *entity);
extern void LegionShot_DamagePlayerOnContact(void);
extern u8 sLegionSpriteSheet[];

struct ShotFrame {
    u16 frame;
    u16 flags;
};

struct ShotFrames {
    struct ShotFrame f[24];
};

struct ShotData {
    s32 graphics;
    u8 pad_4[0x24 - 4];
    s32 palette;
};

extern const struct ShotFrames sLegionShotFrames;

void LegionShellPart_FireShot(struct EwramData_EntityData *owner, s32 dx, s32 dy, s32 unused, s32 angle)
{
    s32 speed = 0x80000;
    u8 zero = 0;
    struct HitBoxRecord box = {-4, -4, 8, 8};
    struct EwramData_EntityData *shot = EntityCreateInRange(0x49, 0xCF, (u32 *)LegionShot_Update);
    struct ShotFrames frames = sLegionShotFrames;
    struct EwramData_EntityData *parent = LEGION_PART_OWNER(owner);
    struct ShotData *data = (struct ShotData *)LEGION_DATA_PTR(parent);
    s32 vx, vy, graphics, index;
    u16 *flags;

    if (shot == NULL)
        return;
    shot->posX.whole = owner->posX.whole + dx;
    shot->posY.whole = owner->posY.whole + dy;
    vx = FixedMultiply(Sin(angle + 0x4000), speed);
    vy = FixedMultiply(Sin(angle), speed);
    shot->velX.whole = vx;
    shot->velY.whole = vy;
    shot->spriteLayer = owner->spriteLayer + 1;
    graphics = data->graphics;
    shot->unk_510.part8.basePalette = data->palette;
    shot->palette = data->palette;
    Sprite_Attach(shot, sLegionSpriteSheet, graphics, shot->unk_510.part8.basePalette);
    angle &= 0xFFFF;
    index = angle / 0xAAA;
    shot->animFrame = frames.f[index].frame;
    shot->animMode = 2;
    {
        u16 *base = &frames.f[0].flags;
        flags = base + index * 2;
    }
    if (*flags & 1)
        shot->flipX = 1;
    if (*flags & 2)
        shot->flipY = 1;
    shot->unk_518.part8.enemyId = owner->unk_518.part8.enemyId;
    LEGION_PART_OWNER(shot) = owner;
    shot->scrollWithBg = 1;
    shot->attack = parent->attack + (parent->attack >> 2);
    Hit_Enable(shot, 8, LegionShot_DamagePlayerOnContact, 0);
    HitBox_SetAttack(shot, &box);
    shot->hit.flags |= 4;
    shot->hit.flags |= zero;
    shot->hit.flags &= ~2;
    shot->drawFunc = (u32 *)Sprite_DrawEntity;
}
