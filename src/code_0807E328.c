/* Giant Ghost's graphics, collision effect and death particles. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_080211F0.h"
extern s8 ObjPalette_FindOrLoadVariant(void *, u8);
extern void Effect_UpdateTimedParticle(struct EwramData_EntityData *);
extern u8 sGiantGhostGfx[], sGiantGhostPalette[], sGiantGhostPaletteAnim[], sGiantGhostSpriteSheet[];
extern u8 sCommonObjectGfx[], sCommonObjectSpriteSheet[];
/**
 * @brief Contact: mark it (unk_505 = 1) and Player_ApplyContactDamage with
 * attribute 0x10 at the entity position.
 */
void GiantGhost_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage effect;
    effect.attack = entity->attack;
    effect.attributes = 16;
    effect.unused = 0;
    effect.reaction = 0;
    effect.sourceX = entity->posX.whole;
    effect.sourceY = entity->posY.whole;
    entity->ext.giantGhost.hitPlayer = 1;
    Player_ApplyContactDamage(&effect);
}
