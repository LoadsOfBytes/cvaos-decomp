#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"
extern void Chaos2Shot_Update(struct EwramData_EntityData *);
extern void Chaos2Shot_DamagePlayerOnContact(struct EwramData_EntityData *);
extern u8 sChaos2SpriteSheet[];
struct ChaosAngleFrames { u16 entries[13][2]; };
extern const struct ChaosAngleFrames sChaos2ShotAngleFrames;
struct ChaosShotGfx { s32 tiles; s32 unused; s32 palette; };
/* Create a shot, selecting its frame and flips from 13 angle sectors. */
void Chaos2_SpawnShot(struct EwramData_EntityData *parent, s32 dx, s32 dy, s32 angle)
{
    u32 collision = 0x0808FCFC;
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)Chaos2Shot_Update);
    struct ChaosAngleFrames frames = sChaos2ShotAngleFrames;
    struct ChaosShotGfx *gfx = (struct ChaosShotGfx *)parent->ext.chaos.data.ptr;
    if (child != NULL)
    {
        s32 vx, vy, tiles, index, zero;
        child->posX.whole = parent->posX.whole + dx;
        child->posY.whole = parent->posY.whole + dy;
        vx = FixedMultiply(Sin(angle + 0x4000), 0x30000);
        vy = FixedMultiply(Sin(angle), 0x30000);
        child->velX.whole = vx;
        child->velY.whole = vy;
        child->spriteLayer = 0x11;
        tiles = gfx->tiles;
        child->unk_510.part8.basePalette = gfx->palette;
        child->palette = (u8)gfx->palette;
        Sprite_Attach(child, sChaos2SpriteSheet, tiles, child->unk_510.part8.basePalette);
        index = (angle & 0xFFFF) / 0x1555;
        child->animFrame = frames.entries[index][0];
        child->animMode = 2;
        if (frames.entries[index][1] & 1)
            child->flipX = 1;
        if (frames.entries[index][1] & 2)
            child->flipY = 1;
        child->unk_518.part8.enemyId = parent->unk_518.part8.enemyId;
        child->ext.chaosChild.link = parent;
        zero = 0;
        child->scrollWithBg = zero;
        child->attack = 0x64;
        Hit_Enable(child, 8, Chaos2Shot_DamagePlayerOnContact, 0);
        HitBox_SetAttack(child, &collision);
        child->hit.flags |= 4;
        child->hit.flags |= zero;
        child->hit.flags &= ~2;
        child->drawFunc = (u32 *)Sprite_DrawEntity;
        child->timer = 78;
    }
}
