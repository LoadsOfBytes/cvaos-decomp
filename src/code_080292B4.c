/* SkillFlameDemonProjectileUpdate; graphics
 * loading as SkillLoadGraphicsInline in src/code/code_080211F0.c. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"

struct SoulGraphicsResources {
    void *graphics;
    void *palette;
    void *spriteSheet;
};
extern struct SoulGraphicsResources sSkillGraphicsSets[];
void SoulAttack_MarkHit(struct EwramData_EntityData *entity);

static inline void SkillLoadGraphicsInline(struct EwramData_EntityData *entity, u8 paletteSlot, void *gfxSrc, void *palette, void *sheet)
{
    gEwramData->playerState.redSoulGfx = ObjGfx_LoadResource(gfxSrc, 0x10000);
    ObjPalette_LoadRange(palette, paletteSlot, 1, 2);
    Sprite_Attach(entity, sheet, gEwramData->playerState.redSoulGfx, 2);
}

static inline struct SoulGraphicsResources *GetSet(s32 index)
{
    return &sSkillGraphicsSets[index];
}

/**
 * @brief Child of SkillFlameDemonUse: projectile (graphics set 17,
 * animation 3, box 0x1010F8F8, SoulAttack_MarkHit) moving until off
 * screen, hidden or marked; frees its red soul slot on deletion.
 */
void SkillFlameDemonProjectileUpdate(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(17);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        Anim_StartIndexed(entity, 3, 0, 1);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 0xD;
        Hit_Enable(entity, 5, SoulAttack_MarkHit, 0);
        HitBox_SetAttackRaw(entity, 0x1010F8F8);
        entity->hit.flags |= 4;
        entity->state++;
    case 1:
        EntityAccelerateAndMove(entity, 0x80000);
        Anim_Update(entity);
    }
    if (EntityIsOutsideScreen(entity, 2) || !entity->roomActive)
        entity->roomActive = 1;
    else if ((u8)entity->unk_4FC.part8.unk_4FC == 0)
        return;
    gEwramData->playerState.activeRedSoulEntities[entity->unk_4FC.part16.unk_4FE[0]] = NULL;
    gEwramData->playerState.nbrActiveRedSoulEntities--;
    EntityDelete(entity);
}
