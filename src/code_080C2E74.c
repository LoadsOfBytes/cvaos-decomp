/* Ghost Dancer's renderer and initial visibility/collision state. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void Entity_EnableBoxes(struct EwramData_EntityData *,void *,void *);
extern s32 ObjPalette_FindOrLoadVariant(void *,s32);
extern void GhostDancer_DamagePlayerOnContact(void);
extern void GhostDancer_OnHit(void);
extern u8 sGhostDancerPalette[],sGhostDancerSpriteSheet[],sGhostDancerGfx[];
/**
 * @brief Find or load graphics and the selected palette variant, attach
 * the sprite and start animation 0. Delete and return 0 on failure;
 * return 1 on success.
 */
u8 GhostDancer_LoadGraphics(struct EwramData_EntityData *entity)
{
    u8 *palette=sGhostDancerPalette;
    u8 *animation=sGhostDancerSpriteSheet;
    u8 *graphics=sGhostDancerGfx;
    s32 slot=ObjGfx_Find(graphics);
    if(slot<0)
    {
        slot=ObjGfx_LoadResource(graphics,0);
        if(slot<0) goto fail;
    }
    entity->unk_510.part8.basePalette=ObjPalette_FindOrLoadVariant(palette,(u8)entity->unk_514.part16.spawnParam0);
    if(entity->unk_510.part8.basePalette==255) goto fail;
    if(!Sprite_Attach(entity,animation,slot,entity->unk_510.part8.basePalette)) goto fail;
    if(!Anim_StartIndexed(entity,0,0,1)) goto fail;
    Anim_Update(entity);
    entity->drawFunc=(u32 *)Sprite_DrawEntity;
    return 1;
fail:
    EntityDelete(entity);
    return 0;
}
