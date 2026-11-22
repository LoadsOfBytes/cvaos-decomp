/* Chaos second form: summoned part (create 5270). */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

struct Chaos2Data {
    s32 graphics;
    s32 palette;
    s32 palette2;
};
struct HitBox {
    s8 x;
    s8 y;
    u8 w;
    u8 h;
};

extern u8 sChaos2SpriteSheet[];
extern void Hit_SetTimerOverride(s32, s32);
extern void Chaos2SummonedPart_Update(struct EwramData_EntityData *);
extern void Chaos2SummonedPart_OnHit(struct EwramData_EntityData *);
extern void Chaos2SummonedPart_OnContact(struct EwramData_EntityData *);

/* Create a summoned part at the core + (x, y) moving with (vx, vy)
 * (600 HP, box x -3, y -3, w 6, h 6, hit by Chaos2SummonedPart_OnContact / Chaos2SummonedPart_OnHit). */
void Chaos2_SummonPart(struct EwramData_EntityData *core, s32 x, s32 y, s32 vx, s32 vy)
{
    struct EwramData_EntityData *part = EntityCreateInRange(0x49, 0xCF, (u32 *)Chaos2SummonedPart_Update);
    struct HitBox box = { -3, -3, 6, 6 };
    struct Chaos2Data *data = (struct Chaos2Data *)core->unk_504.whole;

    if (part != NULL)
    {
        s32 graphics;

        part->posX.whole = core->posX.whole + x;
        part->posY.whole = core->posY.whole + y;
        part->velX.whole = vx;
        part->velY.whole = vy;
        part->spriteLayer = 0x11;
        graphics = data->graphics;
        part->unk_510.part8.basePalette = data->palette2;
        Sprite_Attach(part, sChaos2SpriteSheet, graphics, part->unk_510.part8.basePalette);
        Anim_StartIndexed(part, 6, 2, 1);
        part->animMode = 2;
        part->animTimer = RandomNumberGenerator() % 6;
        part->unk_518.part8.enemyId = core->unk_518.part8.enemyId;
        part->unk_4F4.unk_4F4_entity = core;
        part->scrollWithBg = 0;
        part->attack = 0x6E;
        part->unk_518.part16.hp = 600;
        part->enemyFlags |= 1;
        Hit_Enable(part, 9, Chaos2SummonedPart_OnContact, (s32)Chaos2SummonedPart_OnHit);
        Hit_SetTimerOverride(3, 2);
        HitBox_SetBody(part, &box);
        HitBox_SetAttack(part, &box);
        part->hit.flags |= 4;
        part->hit.flags |= 2;
        part->drawFunc = (u32 *)Sprite_DrawEntity;
    }
}
