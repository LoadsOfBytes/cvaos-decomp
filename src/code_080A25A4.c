/* Chaos: the two rotating rings (create 25A4/27F8, update 26BC/2910). */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

/* allocation of 10 words in the Chaos core's unk_504 */
struct ChaosData {
    s32 graphics;
    s32 palette;
};

extern u8 sChaosSpriteSheet[];
extern s32 Entity_GetVisibilityPattern(s32);
void ChaosOuterRing_Update(struct EwramData_EntityData *entity);
void ChaosInnerRing_Update(struct EwramData_EntityData *entity);
void ChaosOrb_Update(struct EwramData_EntityData *entity);
extern void ChaosOrb_OnHit(struct EwramData_EntityData *, s32, u16 *);
extern void ChaosOrb_CreateHalo(struct EwramData_EntityData *, s32);
extern void ChaosOrb_CreateAura(struct EwramData_EntityData *, s32, s32, s32);

#define SPIN(e) (*(u16 *)&e->ext.chaosRing.spin)
#define ORBIT(e) (*(u16 *)&e->ext.chaosRing.orbit)
#define SPIN_BASE(e) (*(u16 *)&e->ext.chaosRing.spinOffset)
#define ORBIT_BASE(e) (*(u16 *)&e->ext.chaosRing.orbitOffset)
#define CENTER_X(e) (e->ext.chaosRing.centerX)
#define CENTER_Y(e) (e->ext.chaosRing.centerY)
#define RADIUS(e) (e->ext.chaosRing.radius)

/* Create the outer ring (frame 0) around (0x78, 0x50). */
struct EwramData_EntityData *Chaos_CreateOuterRing(struct EwramData_EntityData *owner, u16 spin, u16 orbit, u16 radius)
{
    struct EwramData_EntityData *ring = EntityCreateInRange(0x49, 0xCF, (u32 *)ChaosOuterRing_Update);
    struct ChaosData *data = (struct ChaosData *)owner->ext.chaos.data.value;
    s32 graphics;
    u16 slot;

    if (ring == NULL)
        return NULL;
    ring->posX.part16.integer = 0xFF80;
    ring->posY.part16.integer = 0xFF80;
    CENTER_X(ring) = 0x78;
    CENTER_Y(ring) = 0x50;
    ring->spriteLayer = 0x1A;
    graphics = data->graphics;
    ring->basePalette = data->palette;
    Sprite_Attach(ring, sChaosSpriteSheet, graphics, ring->basePalette);
    ring->animFrame = 0;
    if ((s32)((slot = ObjAffine_Alloc(1)) << 16) < 0)
    {
        EntityDelete(ring);
        return NULL;
    }
    ring->affineIndex = slot;
    ring->affine = 1;
    ObjAffine_Set(0, 0x10000, 0x10000, ring->flipX, ring->flipY, ring->affineIndex);
    RADIUS(ring) = radius;
    SPIN_BASE(ring) = spin;
    ORBIT_BASE(ring) = orbit;
    ring->enemyId = owner->enemyId;
    ring->ext.chaosChild.link = owner;
    ring->scrollWithBg = 0;
    ring->hit.flags |= 4;
    ring->hit.flags |= 2;
    ring->drawFunc = (u32 *)Sprite_DrawEntity;
    return ring;
}

/* Outer ring: spin and orbit (unless the game is paused by stateFlags bit 0),
 * blink out for 60 frames once the owner enters state 3. */
void ChaosOuterRing_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = entity->ext.chaosChild.link;
    struct EwramData *ewram = gEwramData;
    s32 x, y;

    if (owner == NULL || owner->updateFunc == NULL
        || owner->enemyId != entity->enemyId)
        goto delete;
    if (entity->state == 0)
    {
        if (!(ewram->gameState.stateFlags & 1))
        {
            SPIN(entity) += 0x70;
            ORBIT(entity) += 0x1E0;
        }
        if (owner->state == 3)
        {
            entity->state++;
            entity->timer = 60;
        }
    }
    else if (entity->state == 1)
    {
        SPIN(entity) += 0x70;
        ORBIT(entity) += 0x1E0;
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
        if ((s8)--entity->timer <= 0)
        {
            ObjAffine_Free(entity->affineIndex);
            EntityDelete(entity);
            return;
        }
    }
    x = Sin(ORBIT(entity) + ORBIT_BASE(entity) + 0x4000) * RADIUS(entity);
    y = Sin(ORBIT(entity) + ORBIT_BASE(entity)) * RADIUS(entity);
    entity->posX.whole = (CENTER_X(entity) << 16) + x;
    entity->posY.whole = (CENTER_Y(entity) << 16) + y;
    ObjAffine_Set(SPIN(entity) + SPIN_BASE(entity), 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
    return;
delete:
    ObjAffine_Free(entity->affineIndex);
    EntityDelete(entity);
}

/* Create the inner ring (frame 1). */
struct EwramData_EntityData *Chaos_CreateInnerRing(struct EwramData_EntityData *owner, u16 spin, u16 orbit, u16 radius)
{
    struct EwramData_EntityData *ring = EntityCreateInRange(0x49, 0xCF, (u32 *)ChaosInnerRing_Update);
    struct ChaosData *data = (struct ChaosData *)owner->ext.chaos.data.value;
    s32 graphics;
    u16 slot;

    if (ring == NULL)
        return NULL;
    ring->posX.part16.integer = 0xFF80;
    ring->posY.part16.integer = 0xFF80;
    CENTER_X(ring) = 0x78;
    CENTER_Y(ring) = 0x50;
    ring->spriteLayer = 0x1A;
    graphics = data->graphics;
    ring->basePalette = data->palette;
    Sprite_Attach(ring, sChaosSpriteSheet, graphics, ring->basePalette);
    ring->animFrame = 1;
    if ((s32)((slot = ObjAffine_Alloc(1)) << 16) < 0)
    {
        EntityDelete(ring);
        return NULL;
    }
    ring->affineIndex = slot;
    ring->affine = 1;
    ObjAffine_Set(0, 0x10000, 0x10000, ring->flipX, ring->flipY, ring->affineIndex);
    RADIUS(ring) = radius;
    SPIN_BASE(ring) = spin;
    ORBIT_BASE(ring) = orbit;
    ring->enemyId = owner->enemyId;
    ring->ext.chaosChild.link = owner;
    ring->scrollWithBg = 0;
    ring->hit.flags |= 4;
    ring->hit.flags |= 2;
    ring->drawFunc = (u32 *)Sprite_DrawEntity;
    return ring;
}

/* Inner ring: like ChaosOuterRing_Update, slower and spinning the other way. */
void ChaosInnerRing_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = entity->ext.chaosChild.link;
    struct EwramData *ewram = gEwramData;
    s32 x, y;

    if (owner == NULL || owner->updateFunc == NULL
        || owner->enemyId != entity->enemyId)
        goto delete;
    if (entity->state == 0)
    {
        if (!(ewram->gameState.stateFlags & 1))
        {
            SPIN(entity) += 0x60;
            ORBIT(entity) += 0x1C0;
        }
        if (owner->state == 3)
        {
            entity->state++;
            entity->timer = 60;
        }
    }
    else if (entity->state == 1)
    {
        SPIN(entity) += 0x60;
        ORBIT(entity) += 0x1C0;
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
        if ((s8)--entity->timer <= 0)
        {
            ObjAffine_Free(entity->affineIndex);
            EntityDelete(entity);
            return;
        }
    }
    x = Sin(ORBIT(entity) + ORBIT_BASE(entity) + 0x4000) * RADIUS(entity);
    y = Sin(ORBIT(entity) + ORBIT_BASE(entity)) * RADIUS(entity);
    entity->posX.whole = (CENTER_X(entity) << 16) + x;
    entity->posY.whole = (CENTER_Y(entity) << 16) + y;
    ObjAffine_Set(-(SPIN(entity) + SPIN_BASE(entity)), 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
    return;
delete:
    ObjAffine_Free(entity->affineIndex);
    EntityDelete(entity);
}

/* Create one of the three orbs (kind 0-2, 3000 HP, hit by ChaosOrb_OnHit,
 * box 0x350ADFFB = x -5, y -33, w 10, h 53) on the core's circle. */
struct EwramData_EntityData *Chaos_CreateOrb(struct EwramData_EntityData *owner, u16 spin, s32 anim, s32 kind)
{
    u32 box;
    struct EwramData_EntityData *orb = EntityCreateInRange(0x49, 0xCF, (u32 *)ChaosOrb_Update);
    struct ChaosData *data;
    s32 graphics;

    box = 0x350ADFFB;
    data = (struct ChaosData *)owner->ext.chaos.data.value;
    if (orb == NULL)
        goto fail;
    orb->posX.part16.integer = 0xFF80;
    orb->posY.part16.integer = 0xFF80;
    CENTER_X(orb) = 0x78;
    CENTER_Y(orb) = 0x50;
    orb->spriteLayer = 0x12;
    graphics = data->graphics;
    orb->basePalette = data->palette;
    Sprite_Attach(orb, sChaosSpriteSheet, graphics, orb->basePalette);
    Anim_StartIndexed(orb, anim, 2, 0);
    Anim_Update(orb);
    SPIN_BASE(orb) = spin;
    orb->enemyId = owner->enemyId;
    orb->ext.chaosChild.link = owner;
    orb->scrollWithBg = 0;
    orb->ext.chaosOrb.kind = kind;
    orb->attack = 0x50;
    orb->hp = 3000;
    Hit_Enable(orb, 8, NULL, (s32)ChaosOrb_OnHit);
    HitBox_SetBody(orb, &box);
    HitBox_SetAttack(orb, &box);
    orb->hit.flags &= ~4;
    orb->hit.flags |= 2;
    orb->drawFunc = (u32 *)Sprite_DrawEntity;
    switch (kind)
    {
    case 0:
        ChaosOrb_CreateAura(orb, 1, 0, 0xDC);
        break;
    case 1:
        ChaosOrb_CreateAura(orb, 0, 1, 0xDC);
        break;
    case 2:
        ChaosOrb_CreateAura(orb, 2, 2, 0xDC);
        break;
    }
    ChaosOrb_CreateHalo(orb, 0xDC);
    return orb;
fail:
    return NULL;
}
