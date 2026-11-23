#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_080211F0.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

void PlaySong(u32 song);
extern void Hit_CheckWithPlayer(void *res, struct EwramData_EntityData *b);
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *entity, void *graphics, void *sprite, u8 palette, void *paletteData);
extern s16 Entity_GetTargetDistance(struct EwramData_EntityData *entity);
extern void Effect_SpawnParticleTemplateList(s32 x, s32 y, s32 index);
extern void BomberArmorBomb_DamagePlayerOnContact(void);
extern void ArmorProjectile_OnHit(void);
extern void BomberArmorBombTrail_Update(struct EwramData_EntityData *entity);

/**
 * @brief B243C | Bouncing boss projectile (like RockArmorRock_Update): arcs towards
 * the player leaving BomberArmorBombTrail_Update trails every fourth frame, halves and
 * reverses its arc on wall hits, explodes (Effect_SpawnParticleTemplateList) 16 frames after
 * landing or when its HP runs out. Frozen while timeStopTimer is set.
 */
void BomberArmorBomb_Update(struct EwramData_EntityData *entity)
{
    u8 hit[0x10];
    struct EwramData_EntityData *trail;
    s32 result;

    Hit_CheckWithPlayer(hit, entity);
    if (entity->state != 0)
    {
        if ((s16)entity->unk_518.part16.hp == 0 && entity->state != 3)
        {
            Effect_SpawnParticleTemplateList((s16)entity->posX.part16.integer, (s16)entity->posY.part16.integer, 1);
            entity->state = 3;
        }
        if (entity->state != 3 && gEwramData->gameState.timeStopTimer != 0)
            return;
    }
    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, (void *)0x081E41BC, (void *)0x0820B83C, 0, (void *)0x0823EC00))
            goto delete;
        Anim_StartIndexed(entity, 3, 0, 1);
        entity->spriteLayer = 0x13;
        Hit_Enable(entity, 8, BomberArmorBomb_DamagePlayerOnContact, (s32)ArmorProjectile_OnHit);
        HitBox_SetAttackRaw(entity, 0x3030E8E8);
        HitBox_SetBodyRaw(entity, 0x1010F8F8);
        entity->hit.flags |= 2;
        entity->ext.bomberArmorBomb.speed = -0x2000 - (Entity_GetTargetDistance(entity) << 10);
        entity->posY.whole += -0x3A0000;
        if (entity->flipX)
            entity->ext.bomberArmorBomb.speed = -entity->ext.bomberArmorBomb.speed;
        entity->velY.whole = -0x28000;
        entity->accelY.whole = 0x2000;
        entity->attack += entity->attack >> 2;
        entity->scrollWithBg = 1;
        entity->state = 1;
    case 1:
        if ((entity->timer & 3) == 0)
        {
            trail = EntityCreateInRange(0x49, 0xCF, (u32 *)BomberArmorBombTrail_Update);
            if (trail != NULL)
            {
                trail->posX.whole = entity->posX.whole;
                trail->posY.whole = entity->posY.whole - 0x80000;
            }
        }
        entity->timer++;
        result = Entity_Walk(entity, entity->ext.bomberArmorBomb.speed, (void *)0x08528738, 2);
        if (result & 3)
            entity->ext.bomberArmorBomb.speed = -(s32)entity->ext.bomberArmorBomb.speed / 2;
        if (result & 8)
        {
            entity->state = 2;
            entity->timer = 0x10;
        }
        break;
    case 2:
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
        entity->hit.flags &= ~2;
        Effect_SpawnParticleTemplateList((s16)entity->posX.part16.integer, (s16)entity->posY.part16.integer, 0);
        entity->state = 3;
        break;
    case 3:
        PlaySong(0x76);
        goto delete;
    }
    if (EntityIsOutsideScreen(entity, 3))
    {
    delete:
        EntityDelete(entity);
        return;
    }
    Anim_Update(entity);
}
