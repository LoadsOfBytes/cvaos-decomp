#include "code_08009A0.h"
#include "head_stack_entity.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

struct HeadStackDebrisVariant {
    void *graphics;
    void *palette;
    void *spriteSheet;
    u8 firstDebrisFrame;
    u8 lastDebrisFrame;
    u8 pad_E[2];
};

extern const struct HeadStackDebrisVariant sUnk_081193DC[];
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern s32 Hit_CheckWithPlayer(void *res, struct EwramData_EntityData *b);
extern void HeadStackEffect_UpdateEmitter(struct EwramData_EntityData *entity);
extern void HeadStackProjectile_DamagePlayerOnContact(struct EwramData_EntityData *entity);

/**
 * @brief Play the owner variant attack animation, enable its attack box at step 4, and end with the animation or owner death.
 */
void HeadStackEffect_UpdateAttack(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    struct EwramData_EntityData *owner = ENTITY_OWNER(entity);
    struct EwramData_EntityData *child;

    switch (entity->state)
    {
    case 0:
    {
        const struct HeadStackDebrisVariant *t = &sUnk_081193DC[HEAD_STACK_TYPE(owner)];
        if (!Entity_LoadGraphics(entity, t->graphics, t->palette, 0, t->spriteSheet))
        {
            EntityDelete(entity);
            return;
        }
    }
        Anim_StartIndexed(entity, 4, 0, 0);
        entity->spriteLayer = 9;
        entity->objMode = 1;
        if (HEAD_STACK_TYPE(owner) != 0)
        {
            child = EntityCreateInRange(0x49, 0xCF, (u32 *)HeadStackEffect_UpdateEmitter);
            if (child != NULL)
            {
                child->posX.part16.integer = entity->posX.part16.integer;
                child->posY.part16.integer = entity->posY.part16.integer + 0x10;
                child->flipX = entity->flipX;
                child->scrollWithBg = 1;
            }
        }
        Hit_Enable(entity, 8, HeadStackProjectile_DamagePlayerOnContact, 0);
        HitBox_SetAttackRaw(entity, 0x1032F8CE);
        entity->hit.flags |= 2;
        entity->hit.flags |= 4;
        PlaySong(0x6E);
        entity->timer = 8;
        entity->state++;
    case 1:
        if (entity->animTimer == 0)
            if (entity->animStep == 4)
                entity->hit.flags &= 0xFD;
        break;
    }
    Hit_CheckWithPlayer(collision, entity);
    if (Anim_Update(entity) == 3 || owner->state == 2)
        entity->deleteRequested = 1;
}
