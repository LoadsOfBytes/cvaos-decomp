/* HitEffect_Spawn / HitEffect_SpawnAtContact (effect spawners). */
#include "code_08009A0.h"
#include "structs/ewram.h"

extern void HitEffect_Update(struct EwramData_EntityData *entity);
extern void Hit_GetContactPoint(s32 a, struct EwramData_EntityData *b, s16 *outX, s16 *outY);

/**
 * @brief Spawn a hit effect (HitEffect_Update) at the pixel (x, y) with random
 * facing; type selects the animation (0 plain hit, 1..6 attribute index
 * from Attribute_GetHighestIndex as used by Player_ApplyContactDamage).
 */
void HitEffect_Spawn(u16 x, u16 y, u16 type)
{
    struct EwramData_EntityData *effect = EntityCreateInRange(0x49, 0xCF, (u32 *)HitEffect_Update);

    if (effect != NULL)
    {
        effect->posX.part16.integer = x;
        effect->posY.part16.integer = y;
        effect->flipX = RandomNumberGenerator() & 1;
        effect->ext.hitEffect.type = type;
        effect->subState = type;
    }
}

/* Same as HitEffect_Spawn with signed coordinates; inlined into HitEffect_SpawnAtContact
 * (the u16 parameters of HitEffect_Spawn would give ldrh instead of ldrsh). */
static inline void SpawnHitEffect(s16 x, s16 y, u16 type)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)HitEffect_Update);

    if (child != NULL)
    {
        child->posX.part16.integer = x;
        child->posY.part16.integer = y;
        child->flipX = RandomNumberGenerator() & 1;
        child->ext.hitEffect.type = type;
        child->subState = type;
    }
}

/**
 * @brief 6E6B4 | Spawn the hit effect of type type at the contact point
 * of attacker and entity (Hit_GetContactPoint).
 */
void HitEffect_SpawnAtContact(s32 attacker, struct EwramData_EntityData *entity, u16 type)
{
    s16 x;
    s16 y;

    Hit_GetContactPoint(attacker, entity, &x, &y);
    SpawnHitEffect(x, y, type);
}
