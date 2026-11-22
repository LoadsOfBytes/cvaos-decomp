/* Man Eater head: spawns the body chain, steers towards the player, orbits.
 *: one function-level u16 (angle) holds both the
 * segment index and the ArcTan2 result, as in the original allocation. */
#include "code_08009A0.h"
#include "man_eater_entity.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/entity_render_flags.h"
extern s32 ArcTan2(s16 x, s16 y);
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern struct EwramData_EntityData *Entity_CreateObject(void *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern s32 Target_GetPixelX(struct EwramData_EntityData *);
extern s32 Target_GetPixelY(struct EwramData_EntityData *);
extern s32 Target_IsNearEntity(struct EwramData_EntityData *, u16, u16);
extern u8 Angle_TurnTowards(u16 *, s32, s32);
extern void Enemy_OnContact(struct EwramData_EntityData *);
extern void ManEaterPart_OnHit(struct EwramData_EntityData *);
extern void ManEaterChainAnchor_Update(struct EwramData_EntityData *);
extern void ManEaterSegment_Update(struct EwramData_EntityData *);
extern u8 sManEaterGfx[], sManEaterPalette[], sManEaterSpriteSheet[];

struct Flip8 {
    u8 slot:5;
    u8 hidden:1;
    u8 flipX:1;
    u8 flipY:1;
};

struct OrbitPoint {
    u16 angle;
    s16 radius;
};
extern const struct OrbitPoint sManEaterHeadOrbitPoints[];

void ManEaterHead_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = &gEwramData->entityData[(u8)MAN_EATER_OWNER_INDEX(entity)];
    const struct OrbitPoint *orbit = &sManEaterHeadOrbitPoints[MAN_EATER_ORBIT_SLOT(entity)];
    struct EwramData_EntityData *prev, *link, *base, *fx;
    s32 i;
    s16 a, b;
    u16 angle;
    s32 dx, dy, lim;
    u32 ang;
    s32 x, y;

    if (entity->enemyFlags & 2)
    {
        if (MAN_EATER_HEAD_FLAGS(entity) & 4)
        {
            if (entity->subState == 0)
                goto delete;
            entity->subState--;
        }
        else
        {
            MAN_EATER_HEAD_FLAGS(entity) |= 4;
            entity->subState = 0x18;
        }
    }
    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, sManEaterGfx, sManEaterPalette, 0, sManEaterSpriteSheet))
            goto delete;
        Anim_StartIndexed(entity, 4, 0, 1);
        prev = entity;
        base = Entity_CreateObject(ManEaterChainAnchor_Update);
        if (base == NULL)
        {
        delete:
            EntityDelete(entity);
            return;
        }
        entity->unk_518.part8.enemyId = parent->unk_518.part8.enemyId;
        Enemy_ResetFromTemplate(entity);
        entity->spriteLayer = 0x12;
        Hit_Enable(entity, 8, Enemy_OnContact, (s32)ManEaterPart_OnHit);
        HitBox_SetBodyRaw(entity, 0x1010F8F8);
        HitBox_SetAttackRaw(entity, 0x1010F8F8);
        i = 0;
        while (i <= 5 && (link = Entity_CreateObject(ManEaterSegment_Update)) != NULL)
        {
            angle = i + MAN_EATER_ORBIT_SLOT(entity) * 4;
            MAN_EATER_OWNER_INDEX(link) = ENTITY_INDEX(entity);
            MAN_EATER_PREV_INDEX(link) = ENTITY_INDEX(prev);
            MAN_EATER_ANGLE(link) = 0xAAA * angle - 0x2000;
            MAN_EATER_ORBIT_SLOT(link) = i;
            MAN_EATER_BASE_INDEX(link) = ENTITY_INDEX(base);
            prev = link;
            i++;
        }
        link = base;
        MAN_EATER_OWNER_INDEX(link) = ENTITY_INDEX(entity);
        MAN_EATER_PREV_INDEX(link) = ENTITY_INDEX(prev);
        entity->scrollWithBg = 1;
        entity->state = 1;
        break;
    case 1:
        if (MAN_EATER_LUNGE_SPEED(entity) < 0x60000)
        {
            if (!(entity->timer & 0xF))
            {
                link = Effect_SpawnShared(entity->posX.whole, entity->posY.whole, 0xD, 8);
                if (link)
                {
                    ((struct Flip8 *)(ENTITY_RENDER_FLAGS_ADDRESS(link)))->flipX = RandomNumberGenerator() & 1;
                    ((struct Flip8 *)(ENTITY_RENDER_FLAGS_ADDRESS(link)))->flipY = RandomNumberGenerator() & 1;
                }
            }
            entity->timer++;
            MAN_EATER_LUNGE_SPEED(entity) += 0x1800;
        }
        else if ((RandomNumberGenerator() & 0x3FF) < 0x10 && Target_IsNearEntity(entity, 0xC8, 0x78))
        {
            entity->state = 2;
            MAN_EATER_HEAD_FLAGS(entity) |= 1;
            entity->timer = 0x80;
        }
        dx = (s16)(Target_GetPixelX(entity) - entity->posX.part16.integer);
        dy = (s16)(Target_GetPixelY(entity) - entity->posY.part16.integer);
        angle = ArcTan2(dx, dy);
        Angle_TurnTowards((u16 *)&MAN_EATER_ANGLE(entity), 0x40, angle);
        if (MAN_EATER_ANGLE(entity) > MAN_EATER_HOME_ANGLE(entity) + 0x2000)
            MAN_EATER_ANGLE(entity) = MAN_EATER_HOME_ANGLE(entity) + 0x2000;
        else if (MAN_EATER_ANGLE(entity) < MAN_EATER_HOME_ANGLE(entity) - 0x2000)
            MAN_EATER_ANGLE(entity) = MAN_EATER_HOME_ANGLE(entity) - 0x2000;
        break;
    case 2:
        if ((RandomNumberGenerator() & 1) && (MAN_EATER_HEAD_FLAGS(entity) & 2))
            entity->timer = 0;
        if (entity->timer == 0)
        {
            entity->timer = 0x30;
            entity->state = 3;
            MAN_EATER_HEAD_FLAGS(entity) &= ~1;
            break;
        }
        entity->timer--;
        break;
    case 3:
        if (entity->timer != 0)
            entity->timer--;
        else
            entity->state = 1;
        break;
    }
    MAN_EATER_HEAD_FLAGS(entity) &= ~2;
    a = parent->unk_4FC.part16.unk_4FC + orbit->angle;
    x = parent->posX.whole + Sin(a + 0x4000) * orbit->radius;
    y = parent->posY.whole + Sin(a) * orbit->radius;
    b = MAN_EATER_BASE_ANGLE(entity) + parent->unk_4FC.part16.unk_4FC;
    entity->posX.whole = x + Sin(b + 0x4000) * 10;
    entity->posY.whole = y + Sin(b) * 10;
    Anim_Update(entity);
}
