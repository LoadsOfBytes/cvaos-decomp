/* Skeleton-family red soul (bone throw and variants), driven by sUnk_084F12C8. */
#include "code_08009A0.h"
#include "soul_skill_entity.h"
#include "red_soul_entity.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "code/code_08060B98.h"

#include "structs/entity_render_flags.h"


struct SoulGraphicsResources {
    void *graphics;
    void *palette;
    void *spriteSheet;
};

struct BoneSoulInfo {
    s32 velX;
    s32 velY;
    s32 gravity;
    u8 graphicsIndex;
    u8 frame;
    u8 animation;
    u8 palette;
    u16 song;
    u16 flags;
    u16 spin;
    u16 pad_16;
};

extern struct BoneSoulInfo sUnk_084F12C8[];
extern struct SoulGraphicsResources sSkillGraphicsSets[];
extern void BoneSoul_OnPlayerContact(struct EwramData_EntityData *);
extern void RedSoulEffect_UpdateAccelerated(struct EwramData_EntityData *);
extern void SoulBoneFragment_Update(struct EwramData_EntityData *);
extern void BoneSoul_SpawnShatterDebris(struct EwramData_EntityData *owner);
extern void BoneSoul_SpawnFlyingEffect(struct EwramData_EntityData *owner);
extern void Effect_SpawnParticleTemplateList(s16 x, s16 y, s32 index);

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

static inline void SetFrameBox(struct EwramData_EntityData *entity)
{
    u8 *spriteSlot = &entity->spriteSlot;
    struct SpriteBox *boxes = gEwramData->spriteSlots[*spriteSlot].sheet->frames[entity->animFrame].boxes;

    if (boxes != NULL)
    {
        entity->hit.flags &= ~2;
        HitBox_SetAttack(entity, boxes);
    }
    else
    {
        entity->hit.flags |= 2;
    }
}

static inline struct EwramData_EntityData *SpawnBone(s32 x, s32 y, u8 frame)
{
    struct EwramData_EntityData *bone = EntityCreateInRange(0x49, 0xCF, (u32 *)SoulBoneFragment_Update);

    if (bone == NULL)
        return NULL;
    {
        struct SoulGraphicsResources *t = GetSet(17);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(bone, 0, gfx, pal, oam);
    }
    bone->animFrame = frame;
    bone->drawFunc = (u32 *)Sprite_DrawEntity;
    bone->spriteLayer = 0xD;
    bone->posX.whole = x;
    bone->posY.whole = y;
    bone->scrollWithBg = 1;
    return bone;
}

void Skill_Skeleton_ZombieSoldier_RockArmor_WaiterSkeleton_GiantSkeleton_BomberArmor_Use(struct EwramData_EntityData *entity)
{
    struct BoneSoulInfo *info = &sUnk_084F12C8[(u16)RED_SOUL_PARAM(entity)];
    struct EwramData_EntityData *parent = RED_SOUL_OWNER(entity);
    s32 result;

    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(info->graphicsIndex);
        u8 palette = info->palette;
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, palette, gfx, pal, oam);
    }
        Entity_PlaceRelativeTo(entity, parent, 8, -0x1A);
        if (info->flags & 0x80)
        {
            u16 slot = ObjAffine_Alloc(0);
            u16 spin;

            if ((s16)slot < 0)
            {
                gEwramData->playerState.activeRedSoulEntities[RED_SOUL_SLOT(entity)] = NULL;
                EntityDelete(entity);
                return;
            }
            entity->affineIndex = slot;
            entity->affine = 1;
            spin = info->spin;
            SKILL_THROW_SPIN(entity) = spin;
            if (entity->flipX)
                SKILL_THROW_SPIN(entity) = -spin;
        }
        if (info->flags & 1)
            Anim_StartIndexed(entity, info->animation, 0, 1);
        else
            entity->animFrame = info->frame;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 0xD;
    {
        s32 vx = info->velX;
        SKILL_THROW_SPEED(entity) = vx;
        entity->velX.whole = vx;
        if (entity->flipX)
            entity->velX.whole = -vx;
    }
        entity->velY.whole = info->velY;
        entity->accelY.whole = info->gravity;
        entity->scrollWithBg = 1;
        entity->state++;
        Hit_Enable(entity, 5, BoneSoul_OnPlayerContact, 0);
        if (info->flags & 0x60)
        {
            HitBox_SetAttackRaw(entity, 0x2838E2E4);
            entity->hit.flags |= 2;
            entity->timer = 0x50;
        }
        else if (info->flags & 0x200)
            SetFrameBox(entity);
        else
            HitBox_SetAttackRaw(entity, 0x1010F8F8);
        entity->hit.flags |= 4;
        Hit_SetTimerOverride(1, 0x1E);
        if (info->flags & 0x400)
        {
            Hit_SetTimerOverride(1, 0x28);
            entity->timer = 0xC8;
        }
        if (info->flags & 8)
            gEwramData->playerState.enemyTarget = entity;
        if (info->song)
            PlaySong(info->song);
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        break;
    case 1:
        if (info->flags & 0x60)
        {
            if (--entity->timer == 0)
            {
                Effect_SpawnParticleTemplateList(entity->posX.part16.integer, entity->posY.part16.integer, 0);
                entity->hit.flags &= ~2;
                entity->palette = 4;
                entity->timer = 2;
                entity->state = 2;
                PlaySong(0xD7);
                if (info->flags & 0x20)
                    BgPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0x10000, 0x4000, 2);
            }
        }
        else if (info->flags & 0x400)
        {
            if (--entity->timer == 0x10)
                entity->drawSkip = 1;
            if (entity->timer <= 0xF && !(entity->timer & 1))
            {
                struct EwramData_EntityData *debris;

                debris = Effect_SpawnShared(entity->posX.whole + ({ s32 t = (RandomNumberGenerator() & 0x7FFFF) + -0x40000; t; }),
                                      /* -k: offset reloaded from the pool */ entity->posY.whole + ({ s32 t = (RandomNumberGenerator() & 0x7FFFF) + ({ s32 k = 0x40000; -k; }); t; }), 4, 6);
                if (debris != NULL)
                    debris->accelY.whole = (RandomNumberGenerator() & 0x3FF) + -0x1A00;
            }
            if (entity->timer == 0)
                entity->state = 3;
        }
        if (info->flags & 2)
        {
            if (!(info->flags & 0x400))
                result = Entity_Walk(entity, -SKILL_THROW_SPEED(entity), (u8 *)sUnk_084F12C8 + 0xC0, 0);
            else
                result = Entity_Walk(entity, -SKILL_THROW_SPEED(entity), (u8 *)sUnk_084F12C8 + 0xD4, 0);
            if (result & 3)
                SKILL_THROW_SPEED(entity) = 0;
            if (result & 8)
            {
                if (info->flags & 0x10)
                {
                    BoneSoul_SpawnShatterDebris(entity);
                    PlaySong(0xD3);
                    entity->state = 3;
                }
                if (info->flags & 0x420)
                    SKILL_THROW_SPEED(entity) = 0;
            }
            else if ((info->flags & 0x20) && !(entity->timer & 3))
                BoneSoul_SpawnFlyingEffect(entity);
        }
        else if (info->flags & 4)
        {
            s32 vy = entity->velY.whole;

            result = Entity_Walk(entity, -SKILL_THROW_SPEED(entity), (u8 *)sUnk_084F12C8 + 0xC0, 0);
            if (result & 3)
                entity->flipX ^= 1;
            if (result & 8)
            {
                if (vy > 0x10000)
                {
                    PlaySong(0x11B);
                    entity->velY.whole = -(vy / 2);
                }
                else
                    SKILL_THROW_SPEED(entity) = 0;
            }
        }
        else
        {
            EntityAccelerateAndMoveX(entity, 0x80000);
            EntityAccelerateAndMoveY(entity, 0x80000);
            if ((info->flags & 0x100) && (u16)entity->unk_504.part16.unk_504 != 0)
            {
                struct EwramData_EntityData *fx;
                s32 i;
                s32 x = entity->posX.whole;
                s32 y = entity->posY.whole;

                fx = EntityCreateInRange(0x49, 0xCF, (u32 *)RedSoulEffect_UpdateAccelerated);
                if (fx != NULL)
                {
                    {
                        struct SoulGraphicsResources *t = GetSet(17);
                        void *gfx = t->graphics;
                        void *pal = t->palette;
                        void *oam = t->spriteSheet;
                        SkillLoadGraphicsInline(fx, 0, gfx, pal, oam);
                    }
                    Anim_StartIndexed(fx, 2, 0, 0);
                    fx->spriteLayer = 0xD;
                    fx->posX.whole = x;
                    fx->posY.whole = y;
                    fx->scrollWithBg = 1;
                }
                PlaySong(0xD2);
                for (i = 0; i <= 2; i++)
                {
                    struct EwramData_EntityData *bone;
                    u8 frame = RandomNumberGenerator() % 2; /* % 2: result in the constant register */
                    s32 bx = entity->posX.whole;
                    s32 by = entity->posY.whole;

                    bone = SpawnBone(bx, by, frame + 10);

                    if (bone == NULL)
                        break;
                    { s32 t = (RandomNumberGenerator() & 7) - 4; bone->posX.part16.integer += t; }
                    { s32 t = (RandomNumberGenerator() & 7) - 4; bone->posY.part16.integer += t; }
                    bone->velX.whole = ({ s32 t = (RandomNumberGenerator() & 0xFF) - 0x80; -entity->velX.whole / 2 + (t << 9); });
                    bone->velY.whole = RandomNumberGenerator() | 0xFFFF8000;
                    bone->accelY.whole = 0x4000;
                    bone->posX.whole = entity->posX.whole;
                    bone->posY.whole = entity->posY.whole;
                }
                entity->hit.flags |= 2;
                entity->hidden = 1;
                entity->timer = 0x20;
                entity->state = 4;
            }
        }
        break;
    case 2:
        if (entity->timer == 0)
        {
            if (info->flags & 0x20)
                BgPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0x4000, 0x10000, 0x10);
            entity->timer = 0x10;
            entity->state = 4;
        }
        else
            entity->timer--;
        break;
    case 3:
        BgPalette_StopColorFade();
        ObjPalette_StopColorFade();
        gEwramData->playerState.enemyTarget = gEwramData->playerState.player;
        if ((info->flags & 0x80) && (ENTITY_RENDER_FLAGS16(entity) & 0x101F))
            ObjAffine_Free(entity->affineIndex);
        gEwramData->playerState.activeRedSoulEntities[RED_SOUL_SLOT(entity)] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    case 4:
        if (entity->timer != 0)
            entity->timer--;
        else
            entity->state = 3;
        break;
    }
    if (EntityIsOutsideScreen(entity, 4) || !entity->roomActive)
    {
        entity->roomActive = 1;
        entity->state = 3;
        entity->hidden = 1;
    }
    if (info->flags & 0x80)
    {
        SKILL_THROW_ANGLE(entity) += SKILL_THROW_SPIN(entity);
        ObjAffine_Set((s16)SKILL_THROW_ANGLE(entity), 0x10000, 0x10000, entity->flipX, 0, entity->affineIndex);
    }
    entity->roomActive = 1;
    if (info->flags & 1)
        Anim_Update(entity);
}
