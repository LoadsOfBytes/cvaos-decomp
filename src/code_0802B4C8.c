#include "code_08009A0.h"
#include "soul_skill_entity.h"
#include "red_soul_entity.h"
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
extern u8 sSkillGladiatorWalkProfile[];

static inline void SkillLoadGraphicsInline(struct EwramData_EntityData *entity, u8 paletteSlot, void *gfxSrc, void *palette, void *sheet)
{
    gEwramData->playerState.redSoulGfx = ObjGfx_LoadResource(gfxSrc, 0x10000);
    ObjPalette_LoadRange(palette, paletteSlot, 1, 2);
    Sprite_Attach(entity, sheet, gEwramData->playerState.redSoulGfx, 2);
}

static inline u32 *Slot131CC(struct EwramData *e) { return &e->playerState.redSoulGfx; }

static inline u8 *NbrActive(struct EwramData *e) { return &e->playerState.nbrActiveRedSoulEntities; }

static inline struct SoulGraphicsResources *GetSet(s32 index)
{
    return &sSkillGraphicsSets[index];
}

/* Gladiator red soul: a rolling boulder thrown from the caster; it bounces
 * off walls, slows down on the floor and crumbles (debris Effect_SpawnShared)
 * when it stops. */
void SkillGladiatorUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = RED_SOUL_OWNER(entity);
    s32 oldVelY = entity->velY.whole;
    s32 result;
    s32 v;
    u16 slot;

    switch (entity->state)
    {
    case 0:
        slot = ObjAffine_Alloc(0);
        if ((s16)slot < 0)
        {
            gEwramData->playerState.activeRedSoulEntities[RED_SOUL_SLOT(entity)] = NULL;
            EntityDelete(entity);
            return;
        }
        entity->affineIndex = slot;
        entity->affine = 1;
    {
        struct SoulGraphicsResources *t = GetSet(24);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        struct EwramData **ew;
        {
            u32 r = ObjGfx_LoadResource(gfx, 0x10000);
            ew = &gEwramData;
            *Slot131CC(*ew) = r;
        }
        ObjPalette_LoadRange(pal, 0, 1, 2);
        Sprite_Attach(entity, oam, *Slot131CC(*ew), 2);
        entity->animFrame = 0;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 0xD;
        entity->scrollWithBg = 1;
        Entity_PlaceRelativeTo(entity, parent, 8, -0x1A);
        entity->accelY.whole = 0x2000;
        entity->velY.whole = -0x28000;
        SKILL_GLADIATOR_SPEED(entity) = -0x20000;
        if (entity->flipX)
            SKILL_GLADIATOR_SPEED(entity) = -SKILL_GLADIATOR_SPEED(entity);
        Hit_SetTimerOverride(1, 30);
        Hit_Enable(entity, 5, 0, 0);
        HitBox_SetAttackRaw(entity, 0x2020F0F0);
        entity->hit.flags |= 4;
        entity->state = 1;
        (*NbrActive(*ew))++;
    }
        PlaySong(0xC5);
    case 1:
        result = Entity_Walk(entity, -SKILL_GLADIATOR_SPEED(entity), sSkillGladiatorWalkProfile, 2);
        if (result & 0x40)
        {
            v = SKILL_GLADIATOR_SPEED(entity);
            if (v > 0)
                goto dec;
            { register u32 z_ asm("r3"); asm("" : "=r"(z_)); /* FAKEMATCH: occupy r3 for reload rotation */
            v += 0x1000;
            asm("" :: "r"(z_)); } /* FAKEMATCH: occupy r3 for reload rotation */
            goto store;
        }
        else if (result & 0x20)
        {
            v = SKILL_GLADIATOR_SPEED(entity);
            if (v > 0)
                v += 0x1000;
            else
            {
            dec:
                v -= 0x1000;
            }
        store:
            SKILL_GLADIATOR_SPEED(entity) = v;
        }
        else
        {
            if (SKILL_GLADIATOR_SPEED(entity) >= 0)
            {
                SKILL_GLADIATOR_SPEED(entity) -= 0x200;
                if (SKILL_GLADIATOR_SPEED(entity) <= 0)
                {
                    SKILL_GLADIATOR_SPEED(entity) = 0;
                    entity->timer = 0x10;
                    entity->drawSkip = 1;
                    entity->state = 2;
                }
            }
            else
            {
                SKILL_GLADIATOR_SPEED(entity) += 0x200;
                if (SKILL_GLADIATOR_SPEED(entity) > 0)
                {
                    SKILL_GLADIATOR_SPEED(entity) = 0;
                    entity->timer = 0x10;
                    entity->drawSkip = 1;
                    entity->state = 2;
                }
            }
        }
    roll:
        SKILL_GLADIATOR_ANGLE(entity) += -SKILL_GLADIATOR_SPEED(entity) / 64;
        if (result & 3)
        {
            PlaySong(0x15F);
            SKILL_GLADIATOR_SPEED(entity) = -SKILL_GLADIATOR_SPEED(entity) / 2;
        }
        if ((result & 8) && entity->velY.whole == 0 && oldVelY > 0)
            PlaySong(0x15F);
        break;
    case 2:
        if (!(entity->timer & 3))
        {
            s32 d, x, d2, y;
            register u32 m asm("r6"); /* FAKEMATCH: mask register */
            d = RandomNumberGenerator();
            m = 0x1FFFFF;
            d = (d & m) - 0x100000;
            x = entity->posX.whole + d;
            d2 = (RandomNumberGenerator() & m) + ({ s32 k = 0x100000; -k; });
            y = entity->posY.whole + d2;
            Effect_SpawnShared(x, y, 0, 7);
        }
        if (entity->timer == 0)
        {
            ObjAffine_Free(entity->affineIndex);
            gEwramData->playerState.activeRedSoulEntities[RED_SOUL_SLOT(entity)] = NULL;
            gEwramData->playerState.nbrActiveRedSoulEntities--;
            EntityDelete(entity);
            return;
        }
        entity->timer--;
        break;
    }
    if (EntityIsOutsideScreen(entity, 3) || !entity->roomActive)
    {
        entity->roomActive = 1;
        ObjAffine_Free(entity->affineIndex);
        gEwramData->playerState.activeRedSoulEntities[RED_SOUL_SLOT(entity)] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    ObjAffine_Set(SKILL_GLADIATOR_ANGLE(entity), 0x10000, 0x10000, 0, 0, entity->affineIndex);
}
