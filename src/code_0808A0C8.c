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
 * @brief Hit: resolve and show the damage, flash and bounce off the attacker; at
 * HP <= 0 drop the rewards and die (song 0x6E, state 1).
 */
void Ghost_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
{
    s32 damage = Hit_ResolveDamage(entity, attacker, hit, (s16)Player_CalculateAttackPower(hit[2]));
    entity->unk_510.part8.hitFlashTimer = 8;
    {
        s32 hp = entity->unk_518.part16.hp;
        damage = (s16)damage;
        entity->unk_518.part16.hp = hp - damage;
    }
    Entity_ShowDamage(entity, damage, 0);
    Bounce(entity, attacker);
    if ((s16)entity->unk_518.part16.hp <= 0)
    {
        Enemy_DropRewards(entity);
        PlaySong(0x6E);
        entity->palette = entity->unk_510.part8.basePalette;
        entity->timer = 8;
        entity->state = 1;
        entity->subState = 0;
    }
}
