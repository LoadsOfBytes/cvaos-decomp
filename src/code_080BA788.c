/* Man Eater head: spawns the body chain, steers towards the player, orbits.
 *: one function-level u16 (angle) holds both the
 * segment index and the ArcTan2 result, as in the original allocation. */
#include "code_08009A0.h"
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
    struct EwramData_EntityData *parent = &gEwramData->entityData[(u8)entity->ext.manEater.ownerIndex];
    const struct OrbitPoint *orbit = &sManEaterHeadOrbitPoints[entity->ext.manEater.orbitSlot];
    struct EwramData_EntityData *prev, *link, *base, *fx;
    s32 i;
    s16 a, b;
    u16 angle;
    s32 dx, dy, lim;
    u32 ang;
    s32 x, y;

    if (entity->enemyFlags & 2)
    {
        if (entity->ext.manEater.headFlags & 4)
        {
            if (entity->subState == 0)
                goto delete;
            entity->subState--;
        }
        else
        {
            entity->ext.manEater.headFlags |= 4;
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
            angle = i + entity->ext.manEater.orbitSlot * 4;
            link->ext.manEater.ownerIndex = ENTITY_INDEX(entity);
            link->ext.manEater.prevIndex = ENTITY_INDEX(prev);
            link->ext.manEater.angle = 0xAAA * angle - 0x2000;
            link->ext.manEater.orbitSlot = i;
            link->ext.manEater.baseIndex = ENTITY_INDEX(base);
            prev = link;
            i++;
        }
        link = base;
        link->ext.manEater.ownerIndex = ENTITY_INDEX(entity);
        link->ext.manEater.prevIndex = ENTITY_INDEX(prev);
        entity->scrollWithBg = 1;
        entity->state = 1;
        break;
    case 1:
        if (entity->ext.manEater.lungeSpeed < 0x60000)
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
            entity->ext.manEater.lungeSpeed += 0x1800;
        }
        else if ((RandomNumberGenerator() & 0x3FF) < 0x10 && Target_IsNearEntity(entity, 0xC8, 0x78))
        {
            entity->state = 2;
            entity->ext.manEater.headFlags |= 1;
            entity->timer = 0x80;
        }
        dx = (s16)(Target_GetPixelX(entity) - entity->posX.part16.integer);
        dy = (s16)(Target_GetPixelY(entity) - entity->posY.part16.integer);
        angle = ArcTan2(dx, dy);
        Angle_TurnTowards((u16 *)&entity->ext.manEater.angle, 0x40, angle);
        if (entity->ext.manEater.angle > entity->ext.manEater.homeAngle + 0x2000)
            entity->ext.manEater.angle = entity->ext.manEater.homeAngle + 0x2000;
        else if (entity->ext.manEater.angle < entity->ext.manEater.homeAngle - 0x2000)
            entity->ext.manEater.angle = entity->ext.manEater.homeAngle - 0x2000;
        break;
    case 2:
        if ((RandomNumberGenerator() & 1) && (entity->ext.manEater.headFlags & 2))
            entity->timer = 0;
        if (entity->timer == 0)
        {
            entity->timer = 0x30;
            entity->state = 3;
            entity->ext.manEater.headFlags &= ~1;
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
    entity->ext.manEater.headFlags &= ~2;
    a = parent->ext.manEaterBody.angle + orbit->angle;
    x = parent->posX.whole + Sin(a + 0x4000) * orbit->radius;
    y = parent->posY.whole + Sin(a) * orbit->radius;
    b = entity->ext.manEater.baseAngle + parent->ext.manEaterBody.angle;
    entity->posX.whole = x + Sin(b + 0x4000) * 10;
    entity->posY.whole = y + Sin(b) * 10;
    Anim_Update(entity);
}
