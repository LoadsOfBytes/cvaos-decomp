/* Create one segment of Alura Une's attack on the selected side. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
extern void AluraUneRootWave_Update(struct EwramData_EntityData *);
extern void AluraUneAttack_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *);
extern u8 sAluraUneGfx[], sAluraUneSpriteSheet[];
/**
 * @brief Spawn a root wave (animation 9) moving along the floor in the facing
 * direction.
 */
void AluraUne_SpawnRootWave(struct EwramData_EntityData *parent, s32 index, s32 facing)
{
    struct EwramData_EntityData *part = EntityCreateInRange(0x1B, 0x48, (u32 *)AluraUneRootWave_Update);
    if (part)
    {
        s32 graphics = ObjGfx_Find(sAluraUneGfx);
        if (graphics < 0)
        {
            EntityDelete(part);
            goto finished;
        }
        Sprite_Attach(part, sAluraUneSpriteSheet, graphics, parent->basePalette);
        Anim_StartIndexed(part, 9, 0, 0);
        Anim_Update(part);
        part->drawFunc = (u32 *)Sprite_DrawEntity;
        Entity_UpdateBoxes(part);
        Hit_Enable(part, 8, AluraUneAttack_DamagePlayerOnContact, 0);
        part->hit.flags &= ~2;
        part->hit.flags |= 4;
        part->scrollWithBg = 1;
        part->spriteLayer = parent->spriteLayer;
        ENTITY_OWNER(part) = parent;
        part->posX.part16.integer = parent->posX.part16.integer;
        part->posY.part16.integer = parent->posY.part16.integer;
        part->flipX = facing;
        part->attack = parent->attack;
        if (part->flipX) { part->posX.part16.integer -= 128; part->posX.part16.integer += index * 6; }
        else { part->posX.part16.integer += 128; part->posX.part16.integer -= index * 6; }
    }
finished:
    return;
}
