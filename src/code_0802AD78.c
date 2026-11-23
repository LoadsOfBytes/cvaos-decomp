/* Mandragora skill: spawn falling leaves around the caster while the flash lasts. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"

struct SoulGraphicsResources {
    void *graphics;
    void *palette;
    void *spriteSheet;
};
extern struct SoulGraphicsResources sSkillGraphicsSets[];

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

void SkillMandragoraUse(struct EwramData_EntityData *entity)
{
    s32 one;
    struct EwramData_EntityData *parent = entity->ext.redSoul.owner;

    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(25);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        Anim_StartIndexed(entity, 0, 0, 0);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 0xF;
        Entity_PlaceRelativeTo(entity, parent, 0x10, -0x1A);
        Hit_SetTimerOverride(1, 60);
        Hit_Enable(entity, 5, 0, 0);
        HitBox_SetAttackRaw(entity, 0x7F7FC0C0);
        entity->hit.flags |= 4;
        entity->scrollWithBg = 1;
        entity->objMode = 1;
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        PlaySong(0xDF);
        entity->state = 1;
        BgPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0, 0x10000, 0x18);
        ObjPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0, 0x10000, 0x18);
        /* Dead store: the register then appears before the CSE block of case 1,
           so cse2 keeps it as the class head instead of reusing the drawSkip constant. */
        one = 0;
    case 1:
        entity->timer++;
        if ((entity->timer & 3) == 0)
        {
            s16 a = RandomNumberGenerator();
            s32 a2 = a + 0x4000;
            s32 x = entity->posX.whole + Sin(a2) * 24;
            struct EwramData_EntityData *child = Effect_SpawnShared(x, entity->posY.whole + Sin(a) * 24, 0x10, 6);
            if (child != NULL)
            {
                child->drawSkip = 1;
                child->accelX.whole = Sin(a2) / 8;
                child->accelY.whole = Sin(a) / 8;
                {
                    s32 s = RandomNumberGenerator();
                    one = 1;
                    child->flipX = s & one;
                    child->flipY = RandomNumberGenerator() & one;
                }
            }
        }
        if (entity->animFlags & 4)
        {
            BgPalette_StopColorFade();
            ObjPalette_StopColorFade();
            Anim_StartIndexed(entity, 1, 0, 1);
            entity->timer = 0x20;
            entity->state = 2;
            entity->hit.flags |= 2;
        }
        break;
    case 2:
        if (entity->timer == 0)
        {
            entity->drawSkip = 1;
            entity->timer = 8;
            entity->state = 3;
            break;
        }
        entity->timer--;
        break;
    case 3:
        if (entity->timer == 0)
        {
            gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
            gEwramData->playerState.nbrActiveRedSoulEntities--;
            EntityDelete(entity);
            return;
        }
        entity->timer--;
        break;
    }
    if (EntityIsOutsideScreen(entity, 4) || !entity->roomActive)
    {
        entity->roomActive = 1;
        BgPalette_StopColorFade();
        ObjPalette_StopColorFade();
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    Anim_Update(entity);
}
