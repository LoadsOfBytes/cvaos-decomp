/* Hit_ResolveDamage: matched (damage resolution as an inline whose
   working variables are parameters; template address passed as the last,
   written parameter). */
/* Resolve damage modifiers and create the shared hit effect. */
#include "code_08009A0.h"
#include "particle_effect.h"
#include "enemy_status.h"
#include "code_0800CB00.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "structs/entity_template.h"

struct HitEffectEntry {
    u8 animation;
    u8 palette;
    u16 song;
};
extern const struct HitEffectEntry sHitEffects[];
extern void Entity_ShowDamage(struct EwramData_EntityData *, s16, s32);
extern void Hit_GetContactPoint(struct EwramData_EntityData *, struct EwramData_EntityData *, s16 *, s16 *);
extern void HitEffect_Update(struct EwramData_EntityData *);

static inline void HighestEffect(u16 *out, u32 bits)
{
    s32 mask = 0x20;
    u16 result = 0;
    s32 i = 6;
    while (i > 0)
    {
        if (bits & mask)
        {
            result = i;
            break;
        }
        mask >>= 1;
        i--;
    }
    *out = result;
}

static inline void SpawnHitSpark(s16 x, s16 y, u16 effect)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)HitEffect_Update);

    if (child != NULL)
    {
        child->posX.part16.integer = x;
        child->posY.part16.integer = y;
        child->flipX = RandomNumberGenerator() & 1;
        PARTICLE_HIT_TYPE(child) = effect;
        child->subState = effect;
    }
}

static inline void ShowHit(struct EwramData_EntityData *other, struct EwramData_EntityData *entity, u16 effect)
{
    s16 x, y;

    Hit_GetContactPoint(other, entity, &x, &y);
    SpawnHitSpark(x, y, effect);
}

static inline u16 ResolveDamage(struct EwramData_EntityData *entity, s32 damage, u16 flags, u16 *out, u32 weakness, u32 attributes, const struct EntityTemplate *template)
{
    const struct EntityTemplate *t = template;

    if (flags & 2)
    {
        struct WeaponInfo *weapon = Weapon_GetEquippedInfo();

        if (weapon != NULL)
        {
            attributes = weapon->attackAttributes;
            weakness = 1;
            if (attributes & 0x1000)
                damage >>= 1;
        }
    }
    if (!gEwramData->gameState.linkPlay)
    {
        if (!weakness && (flags & 8))
        {
            void *soul = RedSoul_GetLastUsedInfo();

            weakness = 1;
            attributes = ((u16 *)soul)[6];
        }
    }
    else if (flags & 8)
    {
        struct WeaponInfo *weapon = Weapon_GetAttackerInfo(1);

        if (weapon != NULL)
        {
            attributes = weapon->attackAttributes;
            if (attributes & 0x1000)
                damage >>= 1;
        }
    }
    if (!weakness && (flags & 0x10))
        attributes = gEwramData->playerState.skillAttackAttributes;

    weakness = ((u16 *)t)[13] & attributes;
    if (weakness != 0)
    {
        if (ENEMY_STATUS(entity) == 0)
        {
            /* SImode mask register (a plain constant is narrowed to HImode). */
            ENEMY_STATUS(entity) = weakness & ({ u32 k_ = 0xFC0; k_; });
            if (ENEMY_STATUS(entity) & ENEMY_STATUS_PETRIFIED)
                return 0;
            if (ENEMY_STATUS(entity) & 0x800)
            {
                s16 old = entity->unk_518.part16.hp;
                s16 delta;

                entity->unk_518.part16.hp = entity->unk_510.part16.unk_512;
                entity->unk_510.part16.unk_512 = old;
                delta = (s16)entity->unk_518.part16.hp - old;
                if (delta >= 0)
                    Entity_ShowDamage(entity, delta, 1);
                else
                    Entity_ShowDamage(entity, delta, 2);
            }
            if (ENEMY_STATUS(entity) & 0x40)
            {
                entity->defense = 0;
                entity->attack -= entity->attack >> 2;
            }
        }
        if (weakness & 0x80)
            damage += (s16)entity->unk_518.part16.hp >> 2;
        weakness &= 0x3F;
        if (weakness)
        {
            if (weakness == 1)
                damage += damage >> 2;
            else
                damage <<= 1;
            HighestEffect(out, weakness);
            return damage;
        }
    }
    weakness = attributes & 0x3F;
    if (weakness)
    {
        u32 resisted = t->flags_1C & weakness;

        if ((resisted & 0xFFFE) != 0)
        {
            if ((weakness & ~1) == (resisted & 0xFFFE))
                return damage - (damage >> 1);
        }
        else if (weakness == resisted)
            return damage - (damage >> 1);
    }
    /* Dead store: makes the parameter writable, so the inliner copies the
       argument through a temporary (the template address add lands after
       the other parameter copies, as in the original). */
    template = 0;
    HighestEffect(out, attributes);
    if (*out == 1)
        *out = 0;
    return damage;
}

s16 Hit_ResolveDamage(struct EwramData_EntityData *entity, struct EwramData_EntityData *other, u16 *hit, s16 inputDamage)
{
    u16 effect = 0;
    s32 damage;

    damage = ResolveDamage(entity, inputDamage, hit[0], &effect, 0, 0, &sEnemyData[entity->unk_518.part8.enemyId]);
    PlaySong(sHitEffects[effect].song);
    damage = ((damage - (entity->defense >> 1)) * (256 - entity->defense)) >> 8;
    if (damage <= 0)
        damage = 1;
    if (ENEMY_STATUS(entity) & ENEMY_STATUS_PETRIFIED)
    {
        damage <<= 1;
        effect = 0;
    }
    ShowHit(other, entity, effect);
    Hud_RequestEnemyName((struct EnemyHudNameView *)entity);
    return damage;
}
