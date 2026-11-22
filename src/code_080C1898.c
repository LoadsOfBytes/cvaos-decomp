/* Altair creation, update, graphics setup and hit response. */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void Entity_EnableBoxes(struct EwramData_EntityData *, void *, void *);
extern s32 Entity_GetTargetDistanceX(struct EwramData_EntityData *);
extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *);
extern s32 Entity_IsTimeStopped(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *);
extern void Altair_UpdateFlight(struct EwramData_EntityData *);
extern void Altair_UpdateDeath(struct EwramData_EntityData *);
extern void Altair_UpdateSpawner(struct EwramData_EntityData *);
extern s32 Altair_CreateCarriedEnemy(struct EwramData_EntityData *, s32, s32, s32);
extern void Altair_UpdateBestiaryPreview(struct EwramData_EntityData *);
extern void Altair_DamagePlayerOnContact(struct EwramData_EntityData *);
extern s16 Hit_ResolveDamage(struct EwramData_EntityData *, s32, u16 *, s32);
extern void Entity_ShowDamage(struct EwramData_EntityData *, s16, s32);
extern void Enemy_DropRewards(struct EwramData_EntityData *);
extern u8 sAltairPalette[], sAltairSpriteSheet[], sAltairGfx[];
u8 Altair_LoadGraphics(struct EwramData_EntityData *);
void Altair_OnHit(struct EwramData_EntityData *, struct EwramData_EntityData *, u16 *);

#include "code_080D73B8.h"
#include "structs/entity_template.h"
extern struct EwramData_EntityData *Altair_CreateFeather(struct EwramData_EntityData *);
extern void AltairFeather_SetPhase(struct EwramData_EntityData *, s32);
extern void Enemy_UpdateFragmentDeathWithSpread(struct EwramData_EntityData *, s32, s32, s32);
extern void EnemyAltairCreate(struct EwramData_EntityData *);

/**
 * @brief Death state (2): terminate an attached child, create five feathers,
 * then move and update fragment death. Preserve the existing feather
 * allocation assumptions and phase selection calls.
 */
void Altair_UpdateDeath(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *attached = entity->unk_4F8.unk_4F8_entity;
    struct EwramData_EntityData *child;
    u32 randomMask;
    s32 count;
    switch (entity->subState)
    {
    case 0:
        if (attached != NULL && entity->unk_500.part8.unk_500 != 0 && attached->state != 0xFF)
        {
            attached->state = 0xFF;
            attached->subState = 0;
        }
        randomMask = 1;
        for (count = 4; count >= 0; count--)
        {
            child = Altair_CreateFeather(entity);
            if (child != NULL && (RandomNumberGenerator() & randomMask)) AltairFeather_SetPhase(child, 0);
            else AltairFeather_SetPhase(child, 1);
        }
        PlaySong(0x72);
        Anim_StartIndexed(entity, 3, 0, 1);
        entity->unk_4FC.part8.unk_4FC = 3;
        entity->velY.whole = 0xC000;
        entity->timer = 0x3F;
        entity->subState++;
        /* fall through */
    case 1:
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
        Enemy_UpdateFragmentDeathWithSpread(entity, 0xF, 0x100000, 0xC0000);
        break;
    }
}

/**
 * @brief Hidden spawner state (5): randomly create a flight instance while
 * the active count allows it, copy enemy parameters and link it back
 * to this spawner. Flight instances decrement the count on requested deletion.
 */
void Altair_UpdateSpawner(struct EwramData_EntityData *entity)
{
    u32 random = RandomNumberGenerator() & 0x3F;
    struct EwramData_EntityData *child;
    if (random != 0 || entity->unk_500.part16.unk_502 > 1) return;
    child = EntityCreateInRange(0x1B, 0x48, (u32 *)EntityUpdateNothing);
    if (child != NULL)
    {
        child->posY.whole = entity->posY.whole;
        child->scrollWithBg = 1;
        child->unk_518.part8.spawnIndex = random;
        child->unk_518.part8.enemyId = entity->unk_518.part8.enemyId;
        child->unk_514.part16.spawnParam0 = ((u16)entity->unk_500.part16.unk_502 & 1) + 2;
        child->unk_514.part16.spawnParam1 = entity->unk_514.part16.spawnParam1;
        child->drawFunc = (u32 *)Sprite_DrawEntity;
        child->unk_518.part16.hp = sEnemyData[entity->unk_518.part8.enemyId].hp;
        child->unk_510.part16.unk_512 = sEnemyData[entity->unk_518.part8.enemyId].unk_E;
        child->updateFunc = entity->updateFunc;
        child->unk_504.unk_504_entity = entity;
        EnemyAltairCreate(child);
        entity->unk_500.part16.unk_502++;
    }
}

/**
 * @brief Resolve damage, apply hit flash and subtract HP; on defeat drop
 * rewards, restore the palette, reset movement and enter death state 2.
 */
void Altair_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
{
    s32 damage = Player_CalculateAttackPower(hit[2]);
    damage = Hit_ResolveDamage(entity, (s32)attacker, hit, (s16)damage);
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
        entity->timer = 0x32;
        entity->state = 2;
        entity->subState = 0;
        entity->velX.whole = 0;
        entity->accelX.whole = 0;
        entity->velY.whole = 0;
        entity->accelY.whole = 0x2000;
    }
}

