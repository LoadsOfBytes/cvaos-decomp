/* Create a short-lived attack box and advance its collision lifetime. */
#include "code_08009A0.h"
#include "persephone_entity.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Persephone_DamagePlayerOnContact(struct EwramData_EntityData *);
extern u8 sPersephoneGfx[], sPersephoneSpriteSheet[];
extern void PersephoneAttackBox_Update(struct EwramData_EntityData *);
/**
 * @brief Spawn an invisible one-frame attack box {-20, -24, 12, 14} at (x, y)
 * facing like parent, with its attack.
 */
void Persephone_SpawnAttackBox(struct EwramData_EntityData *parent, s32 x, s32 y)
{
    struct EwramData_EntityData *part = EntityCreateInRange(0x49, 0xCF, (u32 *)PersephoneAttackBox_Update);
    struct HitBoxRecord box = {-20, -24, 12, 14};
    if (part)
    {
        s32 graphics;
        part->posX.whole = x;
        part->posY.whole = y;
        part->flipX = parent->flipX;
        part->spriteLayer = parent->spriteLayer - 1;
        graphics = ObjGfx_Find(sPersephoneGfx);
        Sprite_Attach(part, sPersephoneSpriteSheet, graphics, parent->unk_510.part8.basePalette);
        part->attack = parent->attack;
        ENTITY_OWNER(part) = parent;
        part->animFrame = 20;
        PERSEPHONE_BOX_TIME(part) = 1;
        part->scrollWithBg = 1;
        part->drawFunc = (u32 *)Sprite_DrawEntity;
        Hit_Enable(part, 8, Persephone_DamagePlayerOnContact, 0);
        HitBox_SetAttack(part, &box);
        part->hit.flags &= ~2;
        part->hit.flags |= 4;
    }
}
