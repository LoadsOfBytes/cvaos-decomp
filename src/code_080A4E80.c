/* Chaos second form: part update (4E80). */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *);
extern s32 Entity_GetVisibilityPattern(s32);
extern void Effect_SpawnRandom(s32, s32, s32, s32, s32);

#define FLASH(e) ((e)->unk_504.part16.unk_504)

/* Part: hurt the player, restore the frame after a hit flash, break when
 * the core is gone or dying; broken parts smoke and blink out. */
void Chaos2Part_Update(struct EwramData_EntityData *part)
{
    u8 collision[16];
    struct EwramData_EntityData *core = part->unk_4F4.unk_4F4_entity;

    if (part->state == 0)
    {
        if (part->unk_510.part8.hitFlashTimer != 0)
            Enemy_UpdateHitFlash(part);
        if (part->animScriptIndex == 1 && (s16)FLASH(part)-- <= 0)
            Anim_StartIndexed(part, 0, 2, 1);
        Hit_CheckWithPlayer(collision, part);
        if (core != NULL && core->updateFunc != NULL
            && core->unk_518.part8.enemyId == part->unk_518.part8.enemyId
            && core->state != 3)
        {
            if (RandomNumberGenerator() % 16)
                Anim_Update(part);
            return;
        }
        part->state = 10;
        part->subState = 0;
        part->unk_518.part16.hp = 0;
        part->palette = part->unk_510.part8.basePalette;
        part->timer = 40;
        part->unk_510.part8.hitFlashTimer = 0;
    }
    else if (part->state == 10)
    {
        if ((part->timer & 3) == 0)
            Effect_SpawnRandom(part->posX.whole, part->posY.whole - 0x60000, 0x100000, 0xC0000, part->spriteLayer - 1);
        part->hidden = Entity_GetVisibilityPattern(part->timer);
        if ((s8)--part->timer <= 0)
        {
            EntityDelete(part);
            return;
        }
    }
    Anim_Update(part);
}
