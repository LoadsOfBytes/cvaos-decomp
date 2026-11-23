#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

struct EffectGraphics { void *graphics, *palette, *sprite; u8 rest[4]; };
extern const struct EffectGraphics sUnk_081193DC[];
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern s32 Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Entity_PlaceRelativeToCopy(struct EwramData_EntityData *, struct EwramData_EntityData *, s32, s32);
extern void Effect_SpawnAt(s32, s32, s32);
extern void HeadStackProjectile_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void HeadStackProjectile_ClearHealth(struct EwramData_EntityData *);
extern void HeadStackEffect_UpdateParticle(struct EwramData_EntityData *);

/* Enemy projectile: emits trailing particles every eight frames. */
/**
 * @brief Initialize a moving attack projectile and emit trailing variant particles; delete when health reaches zero or it leaves the screen.
 */
void HeadStackProjectile_Update(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    s32 vx;
    u32 timer;
    struct EwramData_EntityData *owner = ENTITY_OWNER(entity);
    struct EwramData_EntityData *child;
    switch (entity->state)
    {
    case 0:
    {
        const struct EffectGraphics *t = &sUnk_081193DC[owner->ext.headStack.type];
        if (!Entity_LoadGraphics(entity, t->graphics, t->palette, 0, t->sprite))
        {
            EntityDelete(entity);
            return;
        }
    }
        Anim_StartIndexed(entity, 5, 0, 1);
        entity->spriteLayer = 9;
        Entity_PlaceRelativeToCopy(entity, owner, -8, -12);
        vx = -0x20000;
        entity->velX.whole = vx;
        if (entity->flipX) entity->velX.whole = -vx;
        PlaySong(0x7A);
        entity->unk_518.part16.hp = 1;
        Hit_Enable(entity, 10, HeadStackProjectile_DamagePlayerOnContact, (s32)HeadStackProjectile_ClearHealth);
        HitBox_SetAttackRaw(entity, 0x0808FCFC);
        HitBox_SetBodyRaw(entity, 0x0808FCFC);
        entity->state++;
    case 1:
        timer = entity->timer + 1;
        entity->timer = timer;
        if ((timer & 7) == 0)
        {
            child = EntityCreateInRange(0x49, 0xCF, (u32 *)HeadStackEffect_UpdateParticle);
            if (child != NULL)
            {
                child->posX.part16.integer = entity->posX.part16.integer + RandomSigned(4);
                child->posY.part16.integer = entity->posY.part16.integer + RandomSigned(8);
                child->velX.whole = -(entity->velX.whole + RandomSigned(0x8000)) / 4;
                child->velY.whole = -0x1000;
                child->accelX.whole = -0x100;
                child->scrollWithBg = 1;
                child->subState = entity->ext.headStack.type;
            }
        }
        EntityAccelerateAndMove(entity, 0x80000);
        break;
    }
    Hit_CheckWithPlayer(collision, entity);
    Anim_Update(entity);
    if ((s16)entity->unk_518.part16.hp == 0)
    {
        Effect_SpawnAt(entity->posX.whole, entity->posY.whole, entity->spriteLayer);
        entity->deleteRequested = 1;
    }
    if (EntityIsOutsideScreen(entity, 1)) entity->deleteRequested = 1;
}


