/* Spawn of three linked parts. */
#include "code_08009A0.h"
#include "graham_projectile.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, u8 *, void *, u8, void *);
extern struct EwramData_EntityData *Entity_CreateObject(u32 *);
extern void Entity_EnableBoxes(struct EwramData_EntityData *, void *, void *);
extern void GrahamSpreadProjectile_Update(struct EwramData_EntityData *);
extern void GrahamSpreadProjectile_DamagePlayerOnContact(void);
extern void GrahamSpreadProjectile_OnHit(void);
extern u8 sGrahamPalette1[], sGrahamSpriteSheet1[], sGrahamGfx1[];

/**
 * @brief Create three damaging projectiles at the owner with indexed
 * launch slopes. Return success. Preserve the existing uninitialized
 * array entries and full-array cleanup on allocation/graphics failure.
 */
s32 Graham_SpawnSpreadProjectiles(struct EwramData_EntityData *entity)
{
    /* graphics pointers are loaded before the loop in the original */
    u8 *gfx2 = sGrahamPalette1;
    u8 *gfx3 = sGrahamSpriteSheet1;
    u8 *gfx1 = sGrahamGfx1;
    struct EwramData_EntityData *parts[3];
    struct EwramData_EntityData **p;
    s32 ok = 1;
    s32 i;

    i = 0;
    p = parts;
    while (i < 3)
    {
        *p = Entity_CreateObject((u32 *)GrahamSpreadProjectile_Update);
        if (*p == NULL)
        {
            ok = 0;
            break;
        }
        if (!Entity_LoadGraphics(*p, gfx1, gfx2, 4, gfx3))
        {
            ok = 0;
            goto fail;
        }
        ENTITY_PARENT_INDEX((*p)) = ENTITY_INDEX(entity);
        (*p)->spriteLayer = entity->spriteLayer - 1;
        (*p)->flipX = 1 - entity->flipX;
        (*p)->posX.whole = entity->posX.whole;
        (*p)->posY.whole = entity->posY.whole - 0x200000;
        (*p)->scrollWithBg = 1;
        (*p)->attack = 0x46;
        (*p)->ext.grahamSpread.index = i;
        Anim_StartIndexed(*p, 8, 0, 1);
        Anim_Update(*p);
        Entity_EnableBoxes(*p++, GrahamSpreadProjectile_DamagePlayerOnContact, GrahamSpreadProjectile_OnHit);
        i++;
    }
    if (!ok)
    {
    fail:
        for (p = parts, i = 2; i >= 0; p++, i--)
        {
            if (*p != NULL)
                EntityDelete(*p);
        }
    }
    return ok;
}
