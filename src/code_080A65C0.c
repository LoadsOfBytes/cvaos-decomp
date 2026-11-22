/* Chaos second form: create a moving child from angle and shared graphics. */
#include "code_08009A0.h"
#include "chaos_entity.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"
struct ChaosChildGfx { s32 tiles; s32 unused; s32 palette; };
extern u8 sChaos2SpriteSheet[];
extern void Chaos2SnakeShot_Update(struct EwramData_EntityData *);
extern void Chaos2SnakeShot_DamagePlayerOnContact(struct EwramData_EntityData *);

/**
 * @brief Fire a shot from the snake head at angle (minus 0x1300) that bounces
 * inside the screen (Chaos2SnakeShot_Update).
 */
void Chaos2SnakeHead_FireShot(struct EwramData_EntityData *parent, s32 angle)
{
    /* Packed box: x=-4, y=-4, width=8, height=8. */
    u32 collision = 0x0808FCFC;
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)Chaos2SnakeShot_Update);
    struct ChaosChildGfx *gfx = (struct ChaosChildGfx *)CHAOS_LINK(parent)->unk_504.unk_504_entity;
    angle -= 0x1300;
    if (child != NULL)
    {
        s32 x, y, tiles;
        x = Sin(angle + 0x4000);
        x = FixedMultiply(x, 0x1C0000);
        y = Sin(angle);
        y = FixedMultiply(y, 0x1C0000);
        child->posX.whole = parent->posX.whole + x;
        child->posY.whole = parent->posY.whole + y;
        x = Sin(angle + 0x4000);
        x = FixedMultiply(x, 0x30000);
        y = Sin(angle);
        y = FixedMultiply(y, 0x30000);
        child->velX.whole = x;
        child->velY.whole = y;
        child->spriteLayer = parent->spriteLayer + 1;
        tiles = gfx->tiles;
        child->unk_510.part8.basePalette = gfx->palette;
        child->palette = (u8)gfx->palette;
        Sprite_Attach(child, sChaos2SpriteSheet, tiles, child->unk_510.part8.basePalette);
        Anim_StartIndexed(child, 7, 2, 1);
        Anim_Update(child);
        child->animFrame = 0x1E;
        child->animMode = 2;
        child->unk_518.part8.enemyId = parent->unk_518.part8.enemyId;
        CHAOS_LINK(child) = parent;
        child->scrollWithBg = 1;
        child->attack = 0x82;
        Hit_Enable(child, 8, Chaos2SnakeShot_DamagePlayerOnContact, 0);
        HitBox_SetAttack(child, &collision);
        child->hit.flags |= 4;
        child->hit.flags &= ~2;
        child->drawFunc = (u32 *)Sprite_DrawEntity;
    }
}
