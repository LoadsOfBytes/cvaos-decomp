/* Bounce away from contact or a hit, using the direction between entities. */
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code_080D73B8.h"
#include "syscalls.h"
extern s32 Sin(s32);
extern s32 FixedMultiply(s32, s32);
extern s32 Hit_ResolveDamage(struct EwramData_EntityData *, struct EwramData_EntityData *, u16 *, s16);
extern void Entity_ShowDamage(struct EwramData_EntityData *, s32, s32);
extern void Enemy_DropRewards(struct EwramData_EntityData *);
static inline void Bounce(struct EwramData_EntityData *entity, struct EwramData_EntityData *other)
{
    s32 x = entity->posX.whole - other->posX.whole;
    s32 y = entity->posY.whole - other->posY.whole;
    s32 pixelX, pixelY;
    u16 direction;
    s32 parameter;
    if (x < 0) pixelX = (s16)(-((-x) >> 16));
    else pixelX = x >> 16;
    if (y < 0) pixelY = (s16)(-((-y) >> 16));
    else pixelY = y >> 16;
    direction = ArcTan2(pixelX, pixelY);
    parameter = direction;
    x = Sin(parameter + 0x4000);
    y = Sin(parameter);
    /* The direction has been consumed; reuse the temporary for speed. */
    parameter = 0x20000;
    entity->velX.whole = FixedMultiply(x, parameter);
    entity->velY.whole = FixedMultiply(y, parameter);
}
/**
 * @brief Contact: Player_ApplyContactDamage with (attack, 0, 0, 0), then bounce
 * off the other entity.
 */
void Ghost_DamagePlayerOnContact(struct EwramData_EntityData *entity, struct EwramData_EntityData *other)
{
    struct PlayerContactDamage effect;
    effect.attack = entity->attack;
    effect.attributes = 0;
    effect.unk_4 = 0;
    effect.reaction = 0;
    effect.sourceX = entity->posX.whole;
    effect.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&effect);
    Bounce(entity, other);
}
