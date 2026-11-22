/* Child projectile/hitbox creation. */
#include "code_08009A0.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, u8 *, void *, u8, void *);
extern void RipperProjectile_Update(struct EwramData_EntityData *);
extern void RipperProjectile_DamagePlayerOnContact(void);
extern void RipperProjectile_OnHit(void);
extern u8 sFleamanRipperPalette[], sFleamanRipperSpriteSheet[], sFleamanRipperGfx[];

/**
 * @brief Create and initialize the thrown projectile with owner pointer,
 * seven-eighths owner damage and hit callbacks. Return the child or NULL.
 * Preserve initialization after a failed graphics load and EntityDelete.
 */
struct EwramData_EntityData *Ripper_CreateProjectile(struct EwramData_EntityData *owner)
{
    /* the original loads the graphics pointers before the create call */
    u8 *gfx2 = sFleamanRipperPalette;
    u8 *gfx3 = sFleamanRipperSpriteSheet;
    u8 *gfx1 = sFleamanRipperGfx;
    struct EwramData_EntityData *e = EntityCreateInRange(0x1B, 0x48, (u32 *)RipperProjectile_Update);

    if (e != NULL)
    {
        if (!Entity_LoadGraphics(e, gfx1, gfx2, 1, gfx3))
            EntityDelete(e);
        e->unk_508.unk_508_entity = owner;
        e->attack = owner->attack - (owner->attack >> 3);
        e->scrollWithBg = 1;
        e->animFrame = 0xB;
        Hit_Enable(e, 10, RipperProjectile_DamagePlayerOnContact, (s32)RipperProjectile_OnHit);
        HitBox_SetAttackRaw(e, 0x0808FCFC);
        HitBox_SetBodyRaw(e, 0x0808FCFC);
        e->hit.flags &= ~4;
        e->hit.flags &= ~2;
    }
    return e;
}
