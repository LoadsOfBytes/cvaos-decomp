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
extern void SkillSlimeTrailUpdate(struct EwramData_EntityData *);
extern void SkillSlimeTrailDraw(struct EwramData_EntityData *);
extern void SkillSlimeMoveAndBounce(struct EwramData_EntityData *);
extern struct EwramData_EntityData *RedSoul_CreateHistoryTrail(struct EwramData_EntityData *, u32 *, u32 *);

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


static inline s32 Range(s32 r, s32 mask, s32 base)
{
    return (r & mask) + base;
}
static inline s32 RandomRange(s32 mask, s32 base)
{
    return (RandomNumberGenerator() & mask) + base;
}

static inline s32 FixedMul(s32 a, s32 b)
{
    return (a >> 8) * (b >> 8);
}

/**
 * @brief 2CFC4 | Slime red soul: a bouncing blob (graphics set 33, animation
 * 0, layer 0xD) from the caster (0xC / -0x1A) with a SkillSlimeTrailUpdate trail,
 * box 0x0808FCFC in slot 5 (disabled), half size 4 x 4 (accelX.part16.integer / accelY.part16.integer),
 * Hit_SetTimerOverride(1, 15). Moves at speed unk_518 = 3 px along angle unk_510
 * (0x1000 + random, mirrored) bouncing with SkillSlimeMoveAndBounce for 256 frames,
 * switching to animation 1 when 32 frames remain.
 */
void SkillSlimeUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = RED_SOUL_OWNER(entity);
    s32 angle;

    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(33);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        Anim_StartIndexed(entity, 0, 0, 1);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 0xD;
        entity->scrollWithBg = 1;
        Entity_PlaceRelativeTo(entity, parent, 0xC, -0x1A);
        /* FAKEMATCH: the original loads both masks through reload copies
           (ldr r2 / movs r3 then mov into r1); opaque temps rebuild that shape */
        {
            s32 t;
            register s32 k asm("r1"); /* FAKEMATCH: pinned register */
            s32 c1;
            s32 c2;
            t = RandomNumberGenerator();
            c1 = 0x3FF;
            asm("" : "+r"(c1)); /* FAKEMATCH: opaque constant */
            k = c1;
            asm("" : "+r"(k)); /* FAKEMATCH: opaque copy */
            t &= k;
            c2 = 0x1000;

            asm("" :: "r"(c1)); /* FAKEMATCH: keeps c1 apart from k */
            k = c2;
            asm("" : "+r"(k)); /* FAKEMATCH: opaque copy */
            angle = t + k;
            asm("" :: "r"(c2)); /* FAKEMATCH: keeps c2 apart from k */
        }
        SKILL_SLIME_ANGLE(entity) = angle;
        SKILL_SLIME_SPEED(entity) = 0x30000;
        if (entity->flipX)
            /* FAKEMATCH: r3 looks busy here, so the reload rotation lands on r1 */
            { register u32 z_ asm("r3"); asm("" : "=r"(z_)); SKILL_SLIME_ANGLE(entity) = -0x8000 - angle; asm("" :: "r"(z_)); } /* FAKEMATCH */
        Hit_SetTimerOverride(1, 15);
        Hit_Enable(entity, 5, 0, 0);
        HitBox_SetAttackRaw(entity, 0x0808FCFC);
        { u8 zero = 0; entity->hit.flags |= 4; entity->hit.flags |= zero; entity->hit.flags &= ~2; }
        SKILL_SLIME_TIMER(entity) = 0x100;
        entity->accelX.part16.integer = 4;
        entity->accelY.part16.integer = 4;
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        RedSoul_CreateHistoryTrail(entity, (u32 *)SkillSlimeTrailUpdate, (u32 *)SkillSlimeTrailDraw);
        entity->state = 1;
    case 1:
        entity->velX.whole = FixedMul(Sin(SKILL_SLIME_ANGLE_WORD(entity) + 0x4000), SKILL_SLIME_SPEED(entity));
        entity->velY.whole = FixedMul(Sin(SKILL_SLIME_ANGLE_WORD(entity)), SKILL_SLIME_SPEED(entity));
        SkillSlimeMoveAndBounce(entity);
        SKILL_SLIME_TIMER(entity)--;
        if (entity->subState == 0 && SKILL_SLIME_TIMER(entity) == 0x20)
        {
            Anim_StartIndexed(entity, 1, 0, 0);
            entity->subState = 1;
        }
        if (SKILL_SLIME_TIMER(entity) != 0)
            break;
        entity->subState = 2;
    case 2:
        gEwramData->playerState.activeRedSoulEntities[RED_SOUL_SLOT(entity)] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    if (EntityIsOutsideScreen(entity, 6) || !entity->roomActive)
    {
        entity->roomActive = 1;
        gEwramData->playerState.activeRedSoulEntities[RED_SOUL_SLOT(entity)] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    Anim_Update(entity);
}
