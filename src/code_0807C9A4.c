/* Create Alura Une's projectile with a timed or immediate activation. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
extern void AluraUneThorn_Update(struct EwramData_EntityData *);
extern void AluraUneAttack_DamagePlayerOnContact(struct EwramData_EntityData *);
extern u8 sAluraUneGfx[], sAluraUneSpriteSheet[];
/**
 * @brief Spawn a thorn at xOffset from parent that rises from the floor after
 * delay frames (hit box {-2, -54, 4, 54}) and sinks again.
 */
void AluraUne_SpawnThorn(struct EwramData_EntityData *parent, s32 xOffset, s32 delay, s32 type)
{
    struct EwramData_EntityData *projectile = EntityCreateInRange(0x1B, 0x48, (u32 *)AluraUneThorn_Update);
    if (projectile)
    {
        s32 graphics = ObjGfx_Find(sAluraUneGfx);
        if (graphics < 0)
        {
            EntityDelete(projectile);
            goto finished;
        }
        Sprite_Attach(projectile, sAluraUneSpriteSheet, graphics, parent->unk_510.part8.basePalette);
        Anim_StartIndexed(projectile, 12, 0, 0);
        projectile->drawFunc = (u32 *)Sprite_DrawEntity;
        Hit_Enable(projectile, 8, AluraUneAttack_DamagePlayerOnContact, 0);
        projectile->hit.flags |= 4;
        projectile->hit.flags |= 2;
        projectile->scrollWithBg = 1;
        projectile->spriteLayer = parent->spriteLayer;
        projectile->hidden = 1;
        if (delay == -1)
        {
            s32 timer = RandomNumberGenerator() & 31;
            projectile->timer = timer + 32;
        }
        else projectile->timer = delay;
        projectile->unk_504.part16.unk_504 = type;
        if (delay == 0)
        {
            Anim_StartIndexed(projectile, 7, 0, 0);
            Anim_Update(projectile);
            projectile->state = 2;
            projectile->hidden = 0;
        }
        projectile->unk_4F8.unk_4F8_entity = parent;
        projectile->posX.part16.integer = parent->posX.part16.integer + xOffset;
        projectile->posY.part16.integer = parent->posY.part16.integer;
        projectile->flipX = parent->flipX;
        projectile->attack = parent->attack;
    }
finished:
    return;
}
