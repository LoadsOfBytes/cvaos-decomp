/* Balore-area effect spawning and timer phases.
 * The local register bindings preserve agbcc's callback reloads and child
 * stores. Matching does not establish the original source spelling. */
#include "code/code_08040A38.h"
#include "balore_entity.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

extern struct EwramData_EntityData *Entity_CreateObject(u32 *);
extern void BaloreAffineEffect_Update(struct EwramData_EntityData *);
extern void BaloreEmitter_Update(struct EwramData_EntityData *);

/**
 * @brief Attack state (state 3): spawn the affine effect and the spark emitter
 * (BaloreAffineEffect_Update / BaloreEmitter_Update), then return to the
 * hover state.
 */
void Balore_UpdateAttack(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *effect;

    switch (entity->subState)
    {
    case 0:
        {
            register u32 *callback asm("r0") = (u32 *)BaloreAffineEffect_Update; /* FAKEMATCH: pinned register */
            effect = Entity_CreateObject(callback);
        }
        if (effect != NULL)
        {
            effect->posX.whole = entity->posX.whole;
            effect->posY.whole = entity->posY.whole;
            effect->scrollWithBg = 1;
            effect->ext.balorePart.body = entity;
        }
        {
            u32 *callback = (u32 *)BaloreAffineEffect_Update;
            effect = Entity_CreateObject(callback);
        }
        if (effect != NULL)
        {
            effect->posX.whole = entity->posX.whole;
            effect->posY.whole = entity->posY.whole;
            effect->scrollWithBg = 1;
            {
                register struct EwramData_EntityData *child asm("r1") = effect; /* FAKEMATCH: pinned register */
                child->subState = 1;
                child->ext.balorePart.body = entity;
            }
        }
        PlaySong(0x1D5);
        entity->defense = 10;
        HitBox_SetBodyRaw(entity, 0x1010F0F0);
        entity->subState = 1;
        entity->timer = 0x60;
    case 1:
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
        effect = Entity_CreateObject((u32 *)BaloreEmitter_Update);
        if (effect != NULL)
        {
            effect->posX.whole = entity->posX.whole;
            effect->posY.whole = entity->posY.whole;
            effect->scrollWithBg = 1;
            effect->ext.balorePart.body = entity;
        }
        entity->subState = 2;
        break;
    case 2:
        if ((s16)entity->hp <= 999)
            entity->timer = 0x50;
        else
            entity->timer = 0x80;
        entity->subState = 1;
        break;
    }
}
