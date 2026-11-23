/* Take damage; on death start the death state, otherwise occasionally emit
 * a piece (at most four pieces counted by the owner). */
#include "code/code_080211F0.h"
#include "code_08009A0.h"
#include "structs/ewram.h"
extern s32 Hit_ResolveDamage(struct EwramData_EntityData *, struct EwramData_EntityData *, u16 *, s16);
extern void Entity_ShowDamage(struct EwramData_EntityData *, s16, s32);
extern void Legion_SpawnDestructibleFragment(struct EwramData_EntityData *owner, s32 x, s32 y, s32 vx, s32 vy, s32 graphics);

/* The statement expressions keep (pos - 0x100000) as one sum: agbcc would
 * otherwise fold the constant into the random term. The 32-bit result of
 * Hit_ResolveDamage is narrowed only at its use. */
void LegionCorePart_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
{
    struct EwramData_EntityData *base = entity->ext.legionPart.owner;
    s16 scale = Player_CalculateAttackPower(hit[2]);
    struct EwramData_EntityData *owner = ENTITY_OWNER(base);
    s32 damage = Hit_ResolveDamage(entity, attacker, hit, scale);

    entity->unk_510.part8.hitFlashTimer = 8;
    {
        s32 hp = entity->unk_518.part16.hp;
        s32 applied = (s16)damage;
        entity->unk_518.part16.hp = hp - applied;
        Entity_ShowDamage(entity, applied, 0);
    }
    if ((s16)entity->unk_518.part16.hp <= 0)
    {
        entity->palette = entity->unk_510.part8.basePalette;
        entity->timer = 0x3C;
        entity->unk_510.part8.hitFlashTimer = 0;
        entity->state = 10;
        entity->subState = 0;
    }
    else if ((RandomNumberGenerator() & 0x3F) < 0x10 && owner->ext.raw.unk_500.whole <= 3)
    {
        s32 x, y, vx, vy;
        u8 type;

        owner->ext.raw.unk_500.whole++;
        type = entity->ext.legionPart.kind;
        if (type == 0)
            goto negative;
        if (type == 1)
        {
            x = (RandomNumberGenerator() & 0x1FFFFF) + ({ s32 t = entity->posX.whole - 0x100000; t; });
            y = (RandomNumberGenerator() & 0x1FFFFF) + ({ s32 t = entity->posY.whole - 0x100000; t; });
            vx = RandomNumberGenerator() & 0x7FFF;
            vy = -(RandomNumberGenerator() & 0x7FFF);
            Legion_SpawnDestructibleFragment(entity->ext.legionPart.owner, x, y, vx, vy, entity->ext.legionPart.tiles);
        }
        else if (type == 2)
        {
        negative:
            x = (RandomNumberGenerator() & 0x1FFFFF) + ({ s32 t = entity->posX.whole - 0x100000; t; });
            y = (RandomNumberGenerator() & 0x1FFFFF) + ({ s32 t = entity->posY.whole - 0x100000; t; });
            vx = -(RandomNumberGenerator() & 0x7FFF);
            vy = -(RandomNumberGenerator() & 0x7FFF);
            Legion_SpawnDestructibleFragment(entity->ext.legionPart.owner, x, y, vx, vy, entity->ext.legionPart.tiles);
        }
        else
        {
            x = (RandomNumberGenerator() & 0x1FFFFF) + ({ s32 t = entity->posX.whole - 0x100000; t; });
            y = (RandomNumberGenerator() & 0x1FFFFF) + ({ s32 t = entity->posY.whole - 0x100000; t; });
            vx = RandomNumberGenerator() & 0x7FFF;
            vy = -(RandomNumberGenerator() & 0x7FFF);
            Legion_SpawnDestructibleFragment(entity->ext.legionPart.owner, x, y, vx, vy, entity->ext.legionPart.tiles);
        }
    }
}
