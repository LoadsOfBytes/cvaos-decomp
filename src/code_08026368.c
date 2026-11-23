#include "code_08009A0.h"
#include "soul_skill_entity.h"
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
extern void SoulAttack_MarkHit(struct EwramData_EntityData *);
extern u16 sSkillSirenNotes[];

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
 * @brief 26368 | Siren red soul: a note (graphics set 0, frame 2/3; with
 * unk_502 set graphics set 7, frame 4/5 and accelX 0x800) chosen at random
 * together with its song (sSkillSirenNotes), placed at the caster (8 / -0x1A),
 * flying at 1 px per frame with a sine wave in velY (phase unk_508 random,
 * += 0x400), box 0x0808FCFC in slot 5 with SoulAttack_MarkHit on hit; with unk_502
 * set it also gives RedSoul_CreateTrailCopy(7) / (3) objMode = 1. Ends on hit or off
 * screen.
 */
void SkillSirenUse(struct EwramData_EntityData *entity)
{
    s32 note;
    struct EwramData_EntityData *child;

    switch (entity->state)
    {
    case 0:
        note = RandomNumberGenerator() & 1;
        if ((u16)entity->ext.redSoul.param == 0)
        {
            {
                struct SoulGraphicsResources *t = GetSet(0);
                void *gfx = t->graphics;
                void *pal = t->palette;
                void *oam = t->spriteSheet;
                SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
            }
            entity->animFrame = note + 2;
        }
        else
        {
            {
                struct SoulGraphicsResources *t = GetSet(7);
                void *gfx = t->graphics;
                void *pal = t->palette;
                void *oam = t->spriteSheet;
                SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
            }
            entity->animFrame = note + 4;
            entity->accelX.whole = 0x800;
        }
        PlaySong(sSkillSirenNotes[note]);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        Entity_PlaceRelativeTo(entity, entity->ext.redSoul.owner, 8, -0x1A);
        entity->spriteLayer = 0xD;
        entity->scrollWithBg = 1;
        entity->velX.whole = 0x10000;
        if (entity->flipX)
        {
            entity->velX.whole = -entity->velX.whole;
            entity->accelX.whole = -entity->accelX.whole;
        }
        entity->flipX = 0;
        entity->state++;
        Hit_SetTimerOverride(1, 10);
        Hit_Enable(entity, 5, SoulAttack_MarkHit, 0);
        HitBox_SetAttackRaw(entity, 0x0808FCFC);
        entity->hit.flags |= 4;
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        entity->ext.skillSiren.phase = RandomNumberGenerator();
        if ((u16)entity->ext.redSoul.param != 0)
        {
            child = RedSoul_CreateTrailCopy(entity, 7);
            if (child != NULL)
            {
                child->drawSkip = 1;
                child->objMode = 1;
            }
            child = RedSoul_CreateTrailCopy(entity, 3);
            if (child != NULL)
                child->objMode = 1;
        }
    case 1:
        entity->velY.whole = Sin(entity->ext.skillSiren.phase);
        entity->ext.skillSiren.phase += 0x400;
        EntityAccelerateAndMove(entity, 0x80000);
        if ((u8)entity->ext.redSoul.hit != 0)
            entity->state = 2;
        if (EntityIsOutsideScreen(entity, 1))
            entity->state = 2;
        break;
    case 2:
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    entity->roomActive = 1;
}
