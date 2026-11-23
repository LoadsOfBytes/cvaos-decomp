/* Chaos core damage (2454) and its rotating background (24D4). */
#include "code_08009A0.h"
#include "syscalls.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "structs/main.h"

/* allocation of 10 words in the Chaos core's unk_504 */
struct ChaosData {
    s32 graphics;
    s32 palette;
    s32 palette2;
    s32 bgRotation; /* 0x0C */
    s32 bgAngle;    /* 0x10 */
    s32 bgScale;    /* 0x14 */
    s32 bgRadius;   /* 0x18 */
};

extern u16 Player_CalculateAttackPower(u16);
extern s32 Hit_ResolveDamage(struct EwramData_EntityData *, s32, u16 *, s16);
extern void Entity_ShowDamage(struct EwramData_EntityData *, s32, s32);
extern void Enemy_DropRewards(struct EwramData_EntityData *);
extern void BossDefeatEffect_Spawn(s32, s32);

#define TO_PIXELS(d) ((d) < 0 ? -(-(d) >> 16) : (d) >> 16)

/* Core hit: lose HP; at 0 start dying (state 3, BossDefeatEffect_Spawn at the core). */
void Chaos_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    s32 damage = Hit_ResolveDamage(entity, attacker, hit, (s16)Player_CalculateAttackPower(hit[2]));

    entity->unk_510.part8.hitFlashTimer = 8;
    {
        u16 hp = entity->unk_518.part16.hp;

        damage <<= 16;
        damage >>= 16;
        entity->unk_518.part16.hp = hp - damage;
    }
    Entity_ShowDamage(entity, damage, 0);
    if ((s16)entity->unk_518.part16.hp <= 0)
    {
        Enemy_DropRewards(entity);
        entity->palette = entity->unk_510.part8.basePalette;
        entity->timer = 60;
        entity->state = 3;
        entity->subState = 0;
        BossDefeatEffect_Spawn(entity->posX.whole, entity->posY.whole);
    }
}

/* Background entity of the core: set the BG2 affine transform (centre
 * (0x78, 0x50) moved on a circle, scale and rotation from ChaosData). */
void ChaosBackground_Update(struct EwramData_EntityData *entity)
{
    struct BgAffineSrcData src;
    struct EwramData_EntityData *core = entity->ext.chaosChild.link;
    struct ChaosData *data = (struct ChaosData *)core->ext.chaos.data.value;

    if (core == NULL || core->updateFunc == NULL
        || core->unk_518.part8.enemyId != entity->unk_518.part8.enemyId)
        return;
    src.texX = 0x8000;
    src.texY = 0x8000;
    src.scrX = TO_PIXELS(FixedMultiply(Sin(data->bgAngle + 0x4000), data->bgRadius)) + 0x78;
    src.scrY = TO_PIXELS(FixedMultiply(Sin(data->bgAngle), data->bgRadius)) + 0x50;
    src.sx = data->bgScale;
    src.sy = src.sx;
    src.alpha = data->bgRotation;
    BgAffineSet(&src, (struct BgAffineDstData *)&gDisplayRegisters.bg2PA, 1);
}
