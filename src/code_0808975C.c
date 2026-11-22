/* Create a hit effect relative to the body with supplied initial velocity. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
extern void GorgonBreath_Update(struct EwramData_EntityData *);
extern void GorgonBreath_DamagePlayerOnContact(struct EwramData_EntityData *);
extern u8 sGorgonGfx[], sGorgonSpriteSheet[];
/**
 * @brief Spawn a breath puff at offset (x, y) from parent moving with (vx, vy);
 * contact damage GorgonBreath_DamagePlayerOnContact (attribute 0x100).
 */
void GorgonBreath_Spawn(struct EwramData_EntityData *parent, s32 x, s32 y, s32 vx, s32 vy)
{
    struct EwramData_EntityData *part = EntityCreateInRange(0x49, 0xCF, (u32 *)GorgonBreath_Update);
    struct HitBoxRecord box = {-4, -4, 8, 8};
    if (part)
    {
        s32 graphics;
        part->posX.whole = parent->posX.whole + x;
        part->posY.whole = parent->posY.whole + y;
        part->velX.whole = vx;
        part->velY.whole = vy;
        part->spriteLayer = parent->spriteLayer - 1;
        graphics = ObjGfx_Find(sGorgonGfx);
        part->unk_510.part8.basePalette = parent->unk_510.part8.basePalette;
        Sprite_Attach(part, sGorgonSpriteSheet, graphics, parent->unk_510.part8.basePalette);
        part->unk_4FC.part8.unk_4FE = 25;
        Hit_Enable(part, 8, GorgonBreath_DamagePlayerOnContact, 0);
        HitBox_SetAttack(part, &box);
        part->hit.flags |= 4;
        part->hit.flags &= ~2;
        part->attack = parent->attack;
        part->unk_4F8.unk_4F8_entity = parent;
        part->scrollWithBg = 1;
        part->drawFunc = (u32 *)Sprite_DrawEntity;
        part->timer = 10;
    }
}
