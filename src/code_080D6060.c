/* Effects 0x080D6060..0x080D6177 (GrahamDeathSpark_Update from the template RisingEffect_Update). */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "code/code_080211F0.h"
#include "structs/ewram.h"

extern u8 sCommonObjectGfx[];
extern u8 sCommonObjectSpriteSheet[];

/**
 * @brief Attach shared effect graphics, play animation 1 and rise;
 * delete on animation completion.
 */
void GrahamDeathSpark_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
        case 0:
        {
            s32 gfx = ObjGfx_Find(sCommonObjectGfx);

            Sprite_Attach(entity, sCommonObjectSpriteSheet, gfx, 7);
            Anim_StartIndexed(entity, 1, 0, 0);
            entity->scrollWithBg = 1;
            entity->drawFunc = (u32 *)Sprite_DrawEntity;
            entity->state++;
            break;
        }
        case 1:
            entity->posY.whole -= 0x8000;
            break;
    }
    if (Anim_Update(entity) == 3)
        EntityDelete(entity);
}

/**
 * @brief Animate while visible. When the animation is done, advance to
 * the next state, hide and request deletion.
 */
void GrahamTransitionPart_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
        case 0:
            if (entity->animDone)
            {
                entity->state = 1;
                entity->stage = 0;
                entity->subState = 0;
            }
            break;
        case 1:
            entity->hidden = 1;
            entity->deleteRequested = 1;
            break;
    }
    if (!entity->hidden)
        Anim_Update(entity);
}

/**
 * @brief Apply stored damage with effect type 2 and zero flags, then
 * select removal state 0xF0 with counters reset.
 */
void GrahamSpreadProjectile_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attributes = 2;
    contactDamage.unk_4 = 0;
    contactDamage.attack = entity->attack;
    contactDamage.reaction = 0;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
    entity->state = 0xF0;
    entity->stage = 0;
    entity->subState = 0;
}
