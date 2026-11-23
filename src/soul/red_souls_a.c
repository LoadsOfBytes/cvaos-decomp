/*
 * Red (bullet) soul skills, part 1: Student Witch, Bone soul effects and the soul attacks from Skeleton up to Ukoback.
 */
#include "core.h"
#include "sound.h"
#include "graphics.h"
#include "gameplay.h"
#include "player_core.h"
#include "macros.h"
#include "structs/ewram.h"
#include "soul_skill_entity.h"
#include "structs/entity_render_flags.h"
#include "music.h"
#include "bg_and_collision.h"
#include "syscalls.h"
#include "damage_numbers.h"
#include "entity_common.h"
#include "entity_motion.h"
#include "hitbox.h"
#include "math_anim.h"
#include "oam.h"
#include "obj_gfx.h"
#include "palette.h"
#include "red_souls_b.h"
#include "effects.h"
#include "data/data_118B48.h"
#include "data/data_4F0B18.h"
#include "data/data_0E0B68.h"

/* Red soul skill parameters (sUnk_084F12C8, 0x18 bytes, indexed by the
 * soul's parameter). */
struct RedSoulShotInfo {
    s32 velX;
    s32 velY;
    s32 accelY; // gravity
    u8 graphicsIndex; // into sSkillGraphicsSets
    u8 frame;
    u8 animation;
    u8 param; // palette (bone souls) / animation flags (skill shots)
    u16 song;
    u16 flags;
    u16 spin;
    u16 pad_16;
};

extern struct RedSoulShotInfo sUnk_084F12C8[];
#define CONTEXT ((struct PlayerState *)((u8 *)gEwramData + OFFSET_OF(struct EwramData, playerState)))

static inline u8 Enable(u8 value) { return value | 4; }
/* Student Witch soul projectile: fall, slide, wait after collision, then
 * release its active-soul slot. */
void SkillStudentWitchUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.redSoul.owner;
    s32 collision;
    switch (entity->state)
    {
    case 0:
        {
            const struct SoulGraphicsResources *sets = sSkillGraphicsSets;
            const struct SoulGraphicsResources *set = sets + 4;
            void *graphics = sets[4].graphics;
            void *palette = set->palette;
            void *sprites = set->spriteSheet;
            s32 animation;
            CONTEXT->redSoulGfx = ObjGfx_LoadResource(graphics, 0x10000);
            ObjPalette_LoadRange(palette, 0, 1, 2);
            Sprite_Attach(entity, sprites, CONTEXT->redSoulGfx, 2);
            animation = 0;
            if ((RandomNumberGenerator() & 7) == 0) animation = 1;
            animation = (u32)animation << 16;
            Anim_StartIndexed(entity, animation >> 16, 0, 1);
            entity->spriteLayer = 13;
            entity->drawFunc = (u32 *)Sprite_DrawEntity;
            Entity_PlaceRelativeTo(entity, parent, 8, -26);
            entity->accelY.whole = 0x2000;
            entity->velY.whole = 0;
            entity->scrollWithBg = 1;
            Hit_Enable(entity, 5, NULL, 0);
            HitBox_SetAttackRaw(entity, 0x1018F0F4);
            entity->hit.flags = Enable(entity->hit.flags);
            Hit_SetTimerOverride(1, 10);
            CONTEXT->nbrActiveRedSoulEntities++;
            entity->state++;
        }
        /* fall through */
    case 1:
        collision = Entity_Walk(entity, -0x20000, sSkillStudentWitchWalkProfile, 8);
        if (collision & 8)
        {
            if (!entity->subState)
            {
                entity->subState = 1;
                PlaySong(0xD0);
            }
            if (collision & 3)
            {
                entity->state = 2;
                entity->timer = 16;
                entity->drawSkip = 1;
            }
        }
        break;
    case 2:
        if (entity->timer) entity->timer--;
        else entity->state = 3;
        break;
    case 3:
        CONTEXT->activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        CONTEXT->nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    if (EntityIsOutsideScreen(entity, 4) || !entity->roomActive)
    {
        entity->roomActive = 1;
        entity->state = 3;
        entity->hidden = 1;
    }
    entity->roomActive = 1;
    Anim_Update(entity);
}

/* Burst of four sparks (frames 0x40/0x41, palette 6) within +-4 px of the
 * owner, random horizontal speed and an upward kick, gravity 0x2000;
 * updated by Effect_UpdateBallisticDebris. Stops at the first failed spawn. */

/**
 * @brief Spawn four ballistic debris pieces with random offsets and velocity
 * at the bone projectile.
 */
void BoneSoul_SpawnShatterDebris(struct EwramData_EntityData *owner)
{
    s32 i;
    for (i = 0; i <= 3; i++)
    {
        struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, (u32 *)Effect_UpdateBallisticDebris);
        s32 graphics, offset;
        if (entity == NULL)
            return;
        graphics = ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(entity, sCommonObjectSpriteSheet, graphics, 6);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 0xD;
        entity->animFrame = (RandomNumberGenerator() & 1) + 0x40;
        offset = (RandomNumberGenerator() & 7) - 4;
        entity->posX.part16.integer = offset + owner->posX.part16.integer;
        offset = (RandomNumberGenerator() & 7) - 4;
        entity->posY.part16.integer = offset + owner->posY.part16.integer;
        entity->velX.whole = (RandomNumberGenerator() & 0xFF) << 8;
        if (RandomNumberGenerator() & 1)
            entity->velX.whole = -entity->velX.whole;
        entity->velY.whole = -0x10000 - ((RandomNumberGenerator() & 0xFF) << 8);
        entity->accelY.whole = 0x2000;
        entity->scrollWithBg = 1;
    }
}

/* Move the red-soul effect and release its slot when it expires. */
/**
 * @brief Move with acceleration and delete the effect when it leaves the
 * screen.
 */
void BoneSoulEffect_Update(struct EwramData_EntityData *entity)
{
    EntityAccelerateAndMoveX(entity, 0x80000);
    EntityAccelerateAndMoveY(entity, 0x80000);
    if (EntityIsOutsideScreen(entity, 4)) entity->state = 1;
    if (--entity->timer == 0) entity->state = 1;
    if (!entity->roomActive) entity->state = 1;
    if (entity->state != 0)
    {
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
    }
    else Anim_Update(entity);
}

struct EffectPoint { s16 x, y; };
/**
 * @brief Create a red-soul effect (animation 5, update BoneSoulEffect_Update)
 * at the bone projectile.
 */
void BoneSoul_SpawnFlyingEffect(struct EwramData_EntityData *owner)
{
    struct RedSoulShotInfo *info = &sUnk_084F12C8[(u16)owner->ext.redSoul.param];
    struct EwramData_EntityData *entity = RedSoul_CreateEntity(owner, (u32 *)BoneSoulEffect_Update);
    struct EffectPoint point;
    if (entity != NULL)
    {
        Sprite_GetPartCornerOffset(owner, 1, &point.x);
        Sprite_Attach(entity, sSkillGraphicsSets[info->graphicsIndex].spriteSheet, gEwramData->playerState.redSoulGfx, 2);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 13;
        entity->flipX = owner->flipX;
        entity->posX.part16.integer = owner->posX.part16.integer + point.x / 2;
        entity->posY.part16.integer = owner->posY.part16.integer + point.y - 4;
        Anim_StartIndexed(entity, 5, 0, 1);
        Anim_Update(entity);
        entity->timer = 10;
    }
}

/* Skeleton-family red soul (bone throw and variants), driven by sUnk_084F12C8. */

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
    struct RedSoulShotInfo *info = &sUnk_084F12C8[(u16)entity->ext.redSoul.param];
    struct EwramData_EntityData *parent = entity->ext.redSoul.owner;
    s32 result;

    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(info->graphicsIndex);
        u8 palette = info->param;
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
                gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
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
        entity->ext.skillThrow.speed = vx;
        entity->velX.whole = vx;
        if (entity->flipX)
            entity->velX.whole = -vx;
    }
        entity->velY.whole = info->velY;
        entity->accelY.whole = info->accelY;
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
                result = Entity_Walk(entity, -entity->ext.skillThrow.speed, (u8 *)sUnk_084F12C8 + 0xC0, 0);
            else
                result = Entity_Walk(entity, -entity->ext.skillThrow.speed, (u8 *)sUnk_084F12C8 + 0xD4, 0);
            if (result & 3)
                entity->ext.skillThrow.speed = 0;
            if (result & 8)
            {
                if (info->flags & 0x10)
                {
                    BoneSoul_SpawnShatterDebris(entity);
                    PlaySong(0xD3);
                    entity->state = 3;
                }
                if (info->flags & 0x420)
                    entity->ext.skillThrow.speed = 0;
            }
            else if ((info->flags & 0x20) && !(entity->timer & 3))
                BoneSoul_SpawnFlyingEffect(entity);
        }
        else if (info->flags & 4)
        {
            s32 vy = entity->velY.whole;

            result = Entity_Walk(entity, -entity->ext.skillThrow.speed, (u8 *)sUnk_084F12C8 + 0xC0, 0);
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
                    entity->ext.skillThrow.speed = 0;
            }
        }
        else
        {
            EntityAccelerateAndMoveX(entity, 0x80000);
            EntityAccelerateAndMoveY(entity, 0x80000);
            if ((info->flags & 0x100) && (u16)entity->ext.skillBone.hit != 0)
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
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
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

/**
 * @brief Child of SkillWingedSkeletonUse: attack box 0x1010F8F8 (slot 5,
 * timer override 20) kept at the owner position plus its unk_508/unk_50C
 * offset; frees its red soul slot when the owner reaches state 3.
 */
void SkillWingedSkeletonHitBoxUpdate(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = entity->ext.redSoul.owner;
    entity->roomActive = 1;
    if (owner->state == 3)
    {
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    if (entity->state == 0)
    {
        Hit_SetTimerOverride(1, 20);
        Hit_Enable(entity, 5, NULL, 0);
        HitBox_SetAttackRaw(entity, 0x1010F8F8);
        entity->hit.flags |= 4;
        entity->state = 1;
    }
    entity->posX.whole = owner->posX.whole + owner->ext.skillWingedSkeleton.tipX;
    entity->posY.whole = owner->posY.whole + owner->ext.skillWingedSkeleton.tipY;
}

#define FIXED_TO_INT(v) ((v) < 0 ? -(-(v) >> 16) : (v) >> 16)

/**
 * @brief 26008 | Winged Skeleton red soul: graphics set 0 in an affine slot,
 * the SkillWingedSkeletonHitBoxUpdate child, placed at the caster (8 / -0x1A), song 0xE4. It
 * flies off at 0x40000 / -0x12000 (gravity 0x2000) with the sprite angle
 * ENTITY_HALF_510 turning by mp = 0x100 from -0x800 towards 0x4000 (mirrored);
 * its velocity unk_508 / unk_50C follows the angle. When the point one step
 * ahead hits a solid tile (Collision_GetTile, not Collision_IsPointSolid) it waits 32
 * frames, then frees its slot.
 */
void SkillWingedSkeletonUse(struct EwramData_EntityData *entity)
{
    u16 slot;

    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(0);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        entity->animFrame = 0;
        slot = ObjAffine_Alloc(0);
        if ((s16)slot < 0)
        {
            gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
            EntityDelete(entity);
            return;
        }
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->affineIndex = slot;
        entity->affine = 1;
    {
        struct EwramData_EntityData *child = RedSoul_CreateEntity(entity, (u32 *)SkillWingedSkeletonHitBoxUpdate);
        if (child == NULL)
        {
            ObjAffine_Free(entity->affineIndex);
            gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = child;
            EntityDelete(entity);
            return;
        }
    }
        Entity_PlaceRelativeTo(entity, entity->ext.redSoul.owner, 8, -0x1A);
        entity->scrollWithBg = 1;
        entity->spriteLayer = 0xD;
        entity->state++;
    {
        u16 angle, step;
        s32 vx;
        angle = 0xF800;
        SKILL_WINGED_SKELETON_ANGLE(entity) = angle;
        step = 0x100;
        SKILL_WINGED_SKELETON_TURN_STEP(entity) = step;
        vx = 0x40000;
        entity->velX.whole = vx;
        entity->velY.whole = -0x12000;
        entity->accelY.whole = 0x2000;
        if (entity->flipX)
        {
            SKILL_WINGED_SKELETON_ANGLE(entity) = -angle;
            SKILL_WINGED_SKELETON_TURN_STEP(entity) = -step;
            entity->velX.whole = -vx;
        }
    }
        PlaySong(0xE4);
        gEwramData->playerState.nbrActiveRedSoulEntities++;
    case 1:
        if (EntityIsOutsideScreen(entity, 4) || !entity->roomActive)
        {
            entity->roomActive = 1;
            entity->state = 3;
            entity->hidden = 1;
            break;
        }
    {
        s32 angle;
        u16 x, y;
        u16 step = SKILL_WINGED_SKELETON_TURN_STEP(entity);
        if (SKILL_WINGED_SKELETON_TURN_STEP(entity) != 0)
        {
            u16 cur = SKILL_WINGED_SKELETON_ANGLE(entity);
            if (SKILL_WINGED_SKELETON_ANGLE(entity) <= 0x3FFF)
                SKILL_WINGED_SKELETON_ANGLE(entity) = cur + step;
        }
        else
        {
            u16 cur = SKILL_WINGED_SKELETON_ANGLE(entity);
            if (SKILL_WINGED_SKELETON_ANGLE(entity) > 0x4000)
                SKILL_WINGED_SKELETON_ANGLE(entity) = cur + step;
        }
        angle = SKILL_WINGED_SKELETON_ANGLE(entity);
        if (entity->flipX)
            angle += -0x8000;
        entity->ext.skillWingedSkeleton.tipX = Sin(angle + 0x4000) << 4;
        entity->ext.skillWingedSkeleton.tipY = Sin(angle) << 4;
        EntityAccelerateAndMoveX(entity, 0x80000);
        EntityAccelerateAndMoveY(entity, 0x80000);
        x = (s16)gEwramData->bgInfo[1].xPos.part16.integer + (s16)entity->posX.part16.integer + FIXED_TO_INT(entity->ext.skillWingedSkeleton.tipX);
        y = (s16)gEwramData->bgInfo[1].yPos.part16.integer + (s16)entity->posY.part16.integer + FIXED_TO_INT(entity->ext.skillWingedSkeleton.tipY);
        if (Collision_GetTile((s16)x, (s16)y) && !Collision_IsPointSolid((s16)x, (s16)y))
        {
            entity->state = 2;
            entity->timer = 0x20;
        }
    }
        break;
    case 2:
        entity->drawSkip = 1;
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
        entity->state = 3;
        break;
    case 3:
        ObjAffine_Free(entity->affineIndex);
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    entity->roomActive = 1;
    ObjAffine_Set(SKILL_WINGED_SKELETON_ANGLE(entity), 0x10000, 0x10000, entity->flipX, 0, entity->affineIndex);
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

/* Inlined Entity_SetAttackBoxFromFrame (src/soul/red_souls_b.c). */

/**
 * @brief 265C0 | Une red soul: a seed (graphics set 0, animation 3) thrown
 * from the caster (8 / -0x1A) with velY -0x18000, gravity 0x2000 and
 * horizontal speed unk_508 = 0xA000, box slot 5 (SoulAttack_MarkHit on hit,
 * disabled), Hit_SetTimerOverride(1, 20). It moves with Entity_Walk (probe
 * sSkillUneWalkProfile), halving and reversing unk_508 on wall contact (bits 0-1);
 * on landing (bit 3, song 0xE0) it waits 0x80 frames or until hit, then
 * sprouts (animation 4, song 0xE8) with the frame hit box until it ends.
 */
void SkillUneUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.redSoul.owner;
    s32 result;

    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(0);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        Anim_StartIndexed(entity, 3, 0, 1);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        Entity_PlaceRelativeTo(entity, parent, 8, -0x1A);
        entity->ext.skillWalker.speed = 0xA000;
        entity->velY.whole = -0x18000;
        entity->accelY.whole = 0x2000;
        entity->scrollWithBg = 1;
        entity->state++;
        Hit_SetTimerOverride(1, 20);
        Hit_Enable(entity, 5, SoulAttack_MarkHit, 0);
        entity->hit.flags |= 4;
        entity->hit.flags |= 2;
        gEwramData->playerState.nbrActiveRedSoulEntities++;
    case 1:
        result = Entity_Walk(entity, -entity->ext.skillWalker.speed, sSkillUneWalkProfile, 0);
        if (result & 3)
            entity->ext.skillWalker.speed = -entity->ext.skillWalker.speed / 2;
        if (!(result & 8))
            break;
        PlaySong(0xE0);
        SetFrameBox(entity);
        entity->hit.flags &= ~2;
        entity->state = 2;
        entity->timer = 0x80;
        break;
    case 2:
        entity->timer--;
        if ((u8)entity->ext.redSoul.hit != 0 || entity->timer == 0)
        {
            Anim_StartIndexed(entity, 4, 0, 0);
            entity->state = 3;
            PlaySong(0xE8);
        }
        break;
    case 3:
        SetFrameBox(entity);
        if (entity->animFlags & 4)
            entity->state = 4;
        break;
    case 4:
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    if (EntityIsOutsideScreen(entity, 4) || !entity->roomActive)
    {
        entity->roomActive = 1;
        entity->state = 4;
        entity->hidden = 1;
    }
    entity->roomActive = 1;
    Anim_Update(entity);
}

#define ABS(x) ((x) < 0 ? -(x) : (x))

/**
 * @brief Child of SkillRedMinotaurUse: orbiting hit box (slot 5, box
 * 0x4040E0E0, timer override 60) placed 80 units along the owner angle
 * (ENTITY_HALF_510); enabled between owner states 2 and 3, removed with its red
 * soul slot at owner state 4.
 */
void SkillRedMinotaurHitBoxUpdate(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.redSoul.owner;
    s32 angle, angle2;
    s32 dx, dy;

    entity->roomActive = 1;
    if (parent->state == 4)
    {
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    switch (entity->state)
    {
    case 0:
        Hit_SetTimerOverride(1, 60);
        Hit_Enable(entity, 5, 0, 0);
        HitBox_SetAttackRaw(entity, 0x4040E0E0);
        entity->hit.flags |= 2;
        entity->hit.flags |= 4;
        entity->state = 1;
        break;
    case 1:
        if (parent->state == 2)
        {
            entity->hit.flags &= ~2;
            entity->state++;
        }
        break;
    case 2:
        if (parent->state == 3)
            entity->hit.flags |= 2;
        break;
    }
    angle = SKILL_RED_MINOTAUR_ANGLE(parent);
    angle2 = angle - 0x4000;
    dx = Sin(angle) * 80;
    dy = Sin(angle2) * 80;
    entity->posX.whole = parent->posX.whole + dx;
    entity->posY.whole = parent->posY.whole + dy;
    dy = ABS(FIXED_TO_INT(dy));
    if (parent->ext.skillRedMinotaur.scale > 0xFFFF)
    {
        if (entity->posY.part16.integer < -dy || entity->posY.part16.integer > dy + 0xA0)
            parent->hidden = 1;
        else
            parent->hidden = 0;
    }
}

/**
 * @brief 269BC | Red Minotaur red soul: an affine axe (graphics set 5,
 * frame 0) at the caster (0 / -0x1A) with the orbiting hit box
 * SkillRedMinotaurHitBoxUpdate; songs 0x1A0 and 0xCF. Angle ENTITY_HALF_510 starts at 0x4000 with
 * turn speed mp = -0x800 and acceleration unk_50C = 0x40 (all mirrored
 * when flipped); the scale unk_508 grows by 0x2000 up to 0x20000, the swing
 * lasts 0x50 frames, then it decelerates by 2 * unk_50C while shrinking.
 */
void SkillRedMinotaurUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.redSoul.owner;
    struct EwramData_EntityData *child;
    s32 zero;
    u16 slot;
    s32 angle, speed, accel;

    switch (entity->state)
    {
    case 0:
        zero = 0;
    {
        struct SoulGraphicsResources *t = GetSet(5);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        entity->animFrame = zero;
        slot = ObjAffine_Alloc(0);
        if ((s16)slot < 0)
        {
            gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = (void *)zero;
            EntityDelete(entity);
            return;
        }
        child = RedSoul_CreateEntity(entity, (u32 *)SkillRedMinotaurHitBoxUpdate);
        if (child == NULL)
        {
            ObjAffine_Free(entity->affineIndex);
            gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = child;
            EntityDelete(entity);
            return;
        }
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 0xD;
        entity->affineIndex = slot;
        entity->affine = 1;
        entity->doubleSize = 1;
        Entity_PlaceRelativeTo(entity, parent, 0, -0x1A);
        entity->scrollWithBg = 1;
        entity->state++;
        angle = 0x4000;
        SKILL_RED_MINOTAUR_ANGLE(entity) = angle;
        speed = 0xF800;
        SKILL_RED_MINOTAUR_SWING_SPEED(entity) = speed;
        accel = 0x40;
        entity->ext.skillRedMinotaur.swingAccel = accel;
        if (entity->flipX)
        {
            SKILL_RED_MINOTAUR_ANGLE(entity) = -angle;
            SKILL_RED_MINOTAUR_SWING_SPEED(entity) = -speed;
            entity->ext.skillRedMinotaur.swingAccel = -accel;
        }
        entity->timer = 0x50;
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        PlaySong(0x1A0);
        PlaySong(0xCF);
    case 1:
        if (entity->ext.skillRedMinotaur.scale <= 0x1FFFF)
            entity->ext.skillRedMinotaur.scale += 0x2000;
        else
            entity->state = 2;
    case 2:
        SKILL_RED_MINOTAUR_ANGLE(entity) += SKILL_RED_MINOTAUR_SWING_SPEED(entity);
        SKILL_RED_MINOTAUR_SWING_SPEED(entity) += entity->ext.skillRedMinotaur.swingAccel;
        if (entity->timer != 0)
            entity->timer--;
        else
            entity->state = 3;
        break;
    case 3:
        SKILL_RED_MINOTAUR_ANGLE(entity) += SKILL_RED_MINOTAUR_SWING_SPEED(entity);
        SKILL_RED_MINOTAUR_SWING_SPEED(entity) -= entity->ext.skillRedMinotaur.swingAccel * 2;
        if (entity->ext.skillRedMinotaur.scale > 0)
            entity->ext.skillRedMinotaur.scale -= 0x2000;
        else
            entity->state = 4;
        break;
    case 4:
        ObjAffine_Free(entity->affineIndex);
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    if (EntityIsOutsideScreen(entity, 4) || !entity->roomActive)
    {
        entity->roomActive = 1;
        entity->state = 4;
        entity->hidden = 1;
    }
    ObjAffine_Set(SKILL_RED_MINOTAUR_ANGLE(entity), entity->ext.skillRedMinotaur.scale, entity->ext.skillRedMinotaur.scale, entity->flipX, 0, entity->affineIndex);
}

/* Inlined Entity_SetAttackBoxFromFrame (src/soul/red_souls_b.c). */

/**
 * @brief 26C54 | Bat red soul: graphics set 3, animation 0, layer 0xD,
 * placed at the caster (0 / -0x1A), Hit_SetTimerOverride(1, 15), box slot 5 (no
 * callback), song 0xE2; then animate with the current frame's hit box until
 * the animation ends, then free the red soul slot.
 */
void SkillBatUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.redSoul.owner;

    if (entity->state == 0)
    {
        {
            struct SoulGraphicsResources *t = GetSet(3);
            void *gfx = t->graphics;
            void *pal = t->palette;
            void *oam = t->spriteSheet;
            SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
        }
        Anim_StartIndexed(entity, 0, 0, 0);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 0xD;
        Entity_PlaceRelativeTo(entity, parent, 0, -0x1A);
        entity->scrollWithBg = 1;
        entity->state++;
        Hit_SetTimerOverride(1, 15);
        Hit_Enable(entity, 5, 0, 0);
        entity->hit.flags |= 4;
        entity->hit.flags |= 2;
        PlaySong(0xE2);
        gEwramData->playerState.nbrActiveRedSoulEntities++;
    }
    entity->roomActive = 1;
    if (Anim_Update(entity) == 3)
    {
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    SetFrameBox(entity);
}

/**
 * @brief Child of SkillMermanUse: projectile (graphics set 0, animation 6,
 * box 0x1010F8F8) accelerating horizontally for 16 updates and dropping
 * one falling spark, then moving without its box until off screen.
 */
void SkillMermanProjectileUpdate(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child;

    entity->roomActive = 1;
    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(0);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        Anim_StartIndexed(entity, 6, 0, 1);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 0xD;
        Hit_Enable(entity, 5, 0, 0);
        HitBox_SetAttackRaw(entity, 0x1010F8F8);
        entity->hit.flags |= 4;
        entity->timer = 0x10;
        entity->state++;
    case 1:
        EntityAccelerateAndMoveX(entity, 0x80000);
        if (entity->timer != 0)
        {
            if (entity->timer == entity->subState)
            {
                child = EntityCreateInRange(0x49, 0xCF, (u32 *)SkillMermanDropletUpdate);
                if (child != NULL)
                {
                    child->scrollWithBg = 1;
                    child->posX.whole = entity->posX.whole;
                    child->posY.whole = entity->posY.whole;
                    child->velX.whole = entity->velX.whole;
                    child->velY.whole = ((RandomNumberGenerator() & 0xF) << 14) - 0x38000;
                    child->accelY.whole = 0x2000;
                }
            }
            entity->timer--;
            break;
        }
        entity->hit.flags |= 2;
        entity->state = 2;
    case 2:
        EntityAccelerateAndMove(entity, 0x80000);
        break;
    case 3:
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    if (EntityIsOutsideScreen(entity, 3) || !entity->roomActive)
    {
        entity->roomActive = 1;
        entity->state = 3;
        entity->hidden = 1;
    }
    Anim_Update(entity);
}

static inline void FreeRedSoul(struct EwramData_EntityData *entity)
{
    gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
    gEwramData->playerState.nbrActiveRedSoulEntities--;
    EntityDelete(entity);
}

/**
 * @brief 26F74 | Merman red soul: graphics set 0, animation 0, layer 0xD,
 * Hit_SetTimerOverride(1, 30), placed at the caster (6 / -0x1A), song 0xED; spawns
 * five SkillMermanProjectileUpdate projectiles with speeds 1..5 px (mirrored with the
 * caster), accelX = -velX / 128, gravity 0x2000 and spark frame i + 2; then
 * follows the caster (0 / -0x1A) until its animation ends.
 */
void SkillMermanUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.redSoul.owner;
    struct EwramData_EntityData *child;
    s32 i;

    entity->roomActive = 1;
    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(0);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        Anim_StartIndexed(entity, 0, 0, 0);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 0xD;
        entity->scrollWithBg = 1;
        Hit_SetTimerOverride(1, 30);
        Entity_PlaceRelativeTo(entity, parent, 6, -0x1A);
        PlaySong(0xED);
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        entity->state++;
        for (i = 0; i <= 4; i++)
        {
            child = RedSoul_CreateEntity(entity, (u32 *)SkillMermanProjectileUpdate);
            if (child == NULL)
                continue;
            child->posX.whole = entity->posX.whole;
            child->posY.whole = entity->posY.whole;
            child->velX.whole = (i + 1) << 16;
            child->accelY.whole = 0x2000;
            child->flipX = entity->flipX;
            if (child->flipX)
                child->velX.whole = -child->velX.whole;
            child->accelX.whole = -child->velX.whole / 128;
            child->subState = i + 2;
        }
    case 1:
        Entity_PlaceRelativeTo(entity, parent, 0, -0x1A);
        break;
    case 2:
        FreeRedSoul(entity);
        return;
    }
    if (Anim_Update(entity) == 3)
        FreeRedSoul(entity);
}

/**
 * @brief Child of SkillDiscArmorUse: affine sprite (graphics set 7, frame 2)
 * at the owner target point, moving along the owner angle and drawn
 * rotated by it and stretched by the owner distance (at most 64 px).
 * Ends with owner state 2; the scale is uninitialised outside state 1.
 */
void SkillDiscArmorScaledSpriteUpdate(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.redSoul.owner;
    s32 scale;
    u16 slot;
    s32 dx, dy, d;

    entity->roomActive = 1;
    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(7);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        slot = ObjAffine_Alloc(0);
        if ((s16)slot < 0)
        {
            gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
            gEwramData->playerState.nbrActiveRedSoulEntities--;
            EntityDelete(entity);
            return;
        }
        entity->animFrame = 2;
        entity->drawFunc = (u32 *)Entity_DrawWithVelocityCopies;
        entity->spriteLayer = 0xD;
        entity->affineIndex = slot;
        entity->affine = 1;
        entity->flipX = parent->flipX ? 1 : 0;
        entity->state = 1;
    case 1:
        entity->posX.whole = parent->accelX.whole;
        entity->posY.whole = parent->accelY.whole;
        dx = (entity->posX.whole - parent->posX.whole) >> 16;
        dx *= dx;
        dy = (entity->posY.whole - parent->posY.whole) >> 16;
        dy *= dy;
        d = Sqrt(dx + dy) << 16;
        if (d > 0x400000)
            d = 0x400000;
        scale = (d >> 1) / 32;
        entity->velX.whole = Sin(SKILL_DISC_ARMOR_ANGLE(parent) + 0x4000) << 5;
        entity->velY.whole = Sin(SKILL_DISC_ARMOR_ANGLE(parent)) << 5;
        entity->velX.whole = entity->velX.whole / 0x10000 * scale;
        entity->velY.whole = entity->velY.whole / 0x10000 * scale;
        if (parent->state != 2)
            break;
    case 2:
        ObjAffine_Free(entity->affineIndex);
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    ObjAffine_Set(SKILL_DISC_ARMOR_ANGLE(parent), scale, 0x10000, entity->flipX, 0, entity->affineIndex);
}

/**
 * @brief 27340 | Disc Armor red soul: affine disc (graphics set 7, frame 1,
 * layer 0xD, box 0x2020F0F0 in slot 5, Hit_SetTimerOverride(1, 10), song 0xD4) with
 * the SkillDiscArmorScaledSpriteUpdate trail. For 27 frames it circles a point 0x18 px above the
 * caster: angle ENTITY_HALF_510 from 0x2000 (0x6000 flipped) turning by mp =
 * -0x276 (mirrored), radius unk_508 = 60 * sin(timer * 1260) px; it spins
 * by spawnParam0 += 0x1B00 and is scaled by the same sine (unk_50C).
 */
void SkillDiscArmorUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.redSoul.owner;
    u16 slot;
    s32 dx, dy;
    s32 scale, rot;

    entity->roomActive = 1;
    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(7);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        entity->animFrame = 1;
        slot = ObjAffine_Alloc(0);
        if ((s16)slot < 0)
        {
            gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
            EntityDelete(entity);
            return;
        }
        entity->scrollWithBg = 1;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 0xD;
        entity->affineIndex = slot;
        entity->affine = 1;
        entity->flipX = parent->flipX ? 1 : 0;
        SKILL_DISC_ARMOR_ANGLE(entity) = 0x2000;
        SKILL_DISC_ARMOR_TURN(entity) = -0x276;
        if (entity->flipX)
        {
            SKILL_DISC_ARMOR_ANGLE(entity) = 0x6000;
            SKILL_DISC_ARMOR_TURN(entity) = -SKILL_DISC_ARMOR_TURN(entity);
        }
        Hit_SetTimerOverride(1, 10);
        Hit_Enable(entity, 5, 0, 0);
        HitBox_SetAttackRaw(entity, 0x2020F0F0);
        entity->hit.flags |= 4;
        entity->timer = 0x1A;
        entity->state = 1;
        PlaySong(0xD4);
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        RedSoul_CreateEntity(entity, (u32 *)SkillDiscArmorScaledSpriteUpdate);
    case 1:
        entity->accelX.whole = parent->posX.whole;
        entity->accelY.whole = parent->posY.whole - 0x180000;
        entity->ext.skillDiscArmor.radius = FIXED_TO_INT(Sin(entity->timer * 1260) * 60);
        SKILL_DISC_ARMOR_ANGLE(entity) += SKILL_DISC_ARMOR_TURN(entity);
        entity->spawnParam0 += 0x1B00;
        dx = entity->ext.skillDiscArmor.radius * Sin(SKILL_DISC_ARMOR_ANGLE(entity) + 0x4000);
        dy = entity->ext.skillDiscArmor.radius * Sin(SKILL_DISC_ARMOR_ANGLE(entity));
        entity->posX.whole = entity->accelX.whole + dx;
        entity->posY.whole = entity->accelY.whole + dy;
        if (entity->timer != 0)
            entity->timer--;
        else
            entity->state = 2;
        break;
    case 2:
        ObjAffine_Free(entity->affineIndex);
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    entity->ext.skillDiscArmor.scale = Sin(entity->timer * 1260);
    rot = entity->spawnParam0;
    if (entity->flipX)
        rot = -rot;
    ObjAffine_Set(rot, entity->ext.skillDiscArmor.scale, entity->ext.skillDiscArmor.scale, entity->flipX, 0, entity->affineIndex);
}

/* Inlined Entity_SetAttackBoxFromFrame (src/soul/red_souls_b.c). */

/**
 * @brief 27608 | Werejaguar / Weretiger red soul: graphics set 8 with the
 * animation in unk_502, layer 0xE, Hit_SetTimerOverride(1, 15), box slot 5 (no
 * callback), song 0xEB; follows the caster with the current frame's hit box
 * while the caster's unk_4F4 & 0x180 equals 0x100, until the animation ends.
 */
void Skill_Werejaguar_Weretiger_Use(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.redSoul.owner;

    entity->roomActive = 1;
    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(8);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        Anim_StartIndexed(entity, entity->ext.redSoul.param, 0, 0);
        entity->spriteLayer = 0xE;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->scrollWithBg = 1;
        Hit_SetTimerOverride(1, 15);
        Hit_Enable(entity, 5, 0, 0);
        entity->hit.flags = (entity->hit.flags | 4) & ~2;
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        PlaySong(0xEB);
        entity->state = 1;
    case 1:
        Entity_PlaceRelativeTo(entity, parent, 0, 0);
        if ((parent->ext.player.actionFlags & 0x180) == 0x100)
            break;
    case 2:
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    if (Anim_Update(entity) == 3)
    {
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    SetFrameBox(entity);
}

/**
 * @brief 277B0 | Lightning Doll red soul: graphics set 9, animation 0,
 * layer 0xE, Hit_SetTimerOverride(1, 10), box slot 5, random vertical flip, placed
 * at the caster (0x10 / -0x1C) with the caster's facing kept in stage;
 * song 0x1A2 or 0x1A9 (Player_IsBossTargetLowHp) and 0xE9. Follows the caster while
 * its unk_4F4 bit 8 is set during the first frames, then the box width
 * (timer, from 0x10) grows by 8 per frame; on frame 8 the palette
 * sSkillLightningDollPalette is loaded once. Ends with the animation or off screen.
 */
void SkillLightningDollUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.redSoul.owner;

    entity->roomActive = 1;
    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(9);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        Anim_StartIndexed(entity, 0, 0, 0);
        entity->spriteLayer = 0xE;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->scrollWithBg = 1;
        Hit_SetTimerOverride(1, 10);
        Hit_Enable(entity, 5, 0, 0);
        entity->hit.flags |= 4;
        entity->flipY = RandomNumberGenerator() & 1;
        Entity_PlaceRelativeTo(entity, parent, 0x10, -0x1C);
        entity->stage = parent->flipX;
        entity->timer = 0x10;
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        if (Player_IsBossTargetLowHp())
            PlaySong(0x1A2);
        else
            PlaySong(0x1A9);
        PlaySong(0xE9);
        entity->state = 1;
    case 1:
        if (entity->animStep > 3 && (s8)entity->timer >= 0)
            entity->timer += 8;
        if (entity->animStep <= 3 && (parent->ext.player.actionFlags & 0x100))
            Entity_PlaceRelativeTo(entity, parent, 0x10, -0x1C);
        entity->flipX = entity->stage;
        HitBox_SetAttackRaw(entity, (entity->timer << 16) + 0x2000F000);
        if (entity->subState == 0 && entity->animStep == 8)
        {
            ObjPalette_LoadRange(sSkillLightningDollPalette, 1, 1, 2);
            entity->subState = 1;
        }
        if (Anim_Update(entity) == 3)
        {
            gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
            gEwramData->playerState.nbrActiveRedSoulEntities--;
            EntityDelete(entity);
            return;
        }
    }
    if (EntityIsOutsideScreen(entity, 2) || !entity->roomActive)
    {
        entity->roomActive = 1;
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
    }
}

/**
 * @brief 27A04 | Altair red soul: graphics set 10, animation 1, layer 0xD,
 * Hit_SetTimerOverride(1, 30), placed at the caster (0x14 / -0x1A) facing the other
 * way, velX -0x28000 (mirrored), box 0x1020F8F0 in slot 5 disabled, song
 * 0xD6. When the animation ends (animFlags bit 2) it switches to animation 2
 * with blinking palette (2 / 4), then to the looping animation 0 with the
 * box enabled, flying off at 0x80000 acceleration (song 0xEE on frame 2).
 */
void SkillAltairUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.redSoul.owner;
    s32 state;
    s32 timer;

    entity->roomActive = 1;
    state = entity->state;
    switch (state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(10);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        Anim_StartIndexed(entity, 1, 0, 0);
        entity->spriteLayer = 0xD;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        Hit_SetTimerOverride(1, 30);
        Entity_PlaceRelativeTo(entity, parent, 0x14, -0x1A);
        entity->flipX ^= 1;
        entity->velX.whole = -0x28000;
        if (entity->flipX)
            entity->velX.whole = -entity->velX.whole;
        entity->scrollWithBg = 1;
        entity->state = 1;
        Hit_Enable(entity, 5, 0, 0);
        HitBox_SetAttackRaw(entity, 0x1020F8F0);
        entity->hit.flags |= 4;
        entity->hit.flags |= 2;
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        PlaySong(0xD6);
    case 1:
        if (entity->animFlags & 4)
        {
            entity->palette = 4;
            Anim_StartIndexed(entity, 2, 0, 0);
            entity->state = 2;
        }
        break;
    case 2:
        timer = entity->timer + 1;
        entity->timer = timer;
        if (timer & state) /* state == 2 here */
            entity->palette = 2;
        else
            entity->palette = 4;
        if (entity->animFlags & 4)
        {
            entity->palette = 2;
            Anim_StartIndexed(entity, 0, 0, 1);
            entity->state = 3;
            entity->hit.flags &= ~2;
        }
        break;
    case 3:
        EntityAccelerateAndMoveX(entity, 0x80000);
        if (*(u16 *)&entity->animStep == 2)
            PlaySong(0xEE);
        break;
    }
    if (EntityIsOutsideScreen(entity, 2) || !entity->roomActive)
    {
        entity->roomActive = 1;
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    Anim_Update(entity);
}

extern s32 Collision_FindWaterSurfaceOffset(s32, s32);

static inline struct EwramData_EntityData *NearestTarget(struct EwramData_EntityData *entity)
{
    u32 bestDistance = -1;
    struct EwramData_EntityData *best = NULL;
    struct EwramData_EntityData *selected = gEwramData->playerState.selectedTarget;
    s32 i;
    if (selected != NULL)
        return selected;
    for (i = 0; i < 4; i++)
    {
        struct EwramData_EntityData *target = gEwramData->playerState.trackedEntities[i];
        if (target != NULL)
        {
            s32 dx = (s16)target->posX.part16.integer - (s16)entity->posX.part16.integer;
            s32 dy = target->posY.part16.integer - entity->posY.part16.integer;
            u32 distance = dx * dx + dy * dy;
            if (bestDistance > distance)
            {
                bestDistance = distance;
                best = target;
            }
        }
    }
    return best;
}

void SkillKillerFishUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.redSoul.owner;

    entity->roomActive = 1;
    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(11);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        Anim_StartIndexed(entity, 1, 0, 0);
        entity->spriteLayer = 0xD;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        Hit_SetTimerOverride(1, 30);
        Entity_PlaceRelativeTo(entity, parent, 0x14, -0xC);
        entity->flipX ^= 1;
    {
        s32 vx = -0x28000;
        entity->velX.whole = vx;
        if (entity->flipX)
            entity->velX.whole = -vx;
    }
        entity->scrollWithBg = 1;
        entity->state = 1;
        Hit_Enable(entity, 5, 0, 0);
        HitBox_SetAttackRaw(entity, 0x2030F0E8);
        entity->hit.flags |= 4;
        entity->hit.flags |= 2;
        PlaySong(0x19C);
        gEwramData->playerState.nbrActiveRedSoulEntities++;
    case 1:
        if (!(entity->animFlags & 4))
            break;
    {
        s16 x = GetEntityRoomXPositionInteger(entity);
        s16 y = GetEntityRoomYPositionInteger(entity);
        if (Collision_IsPointSolid(x, y) || Collision_IsPointSolid(x, y - 0x10))
        {
            struct EwramData_EntityData *target = NearestTarget(entity);
            if (target != NULL)
            {
                register s32 dy asm("r4"); /* FAKEMATCH: pinned register */
                s32 ady;
                {
                    register s32 a asm("r1") = target->posY.whole; /* FAKEMATCH: pinned register */
                    register s32 b asm("r0") = entity->posY.whole; /* FAKEMATCH: pinned register */
                    dy = a - b;
                }
                ady = dy;
                if (dy < 0)
                    ady = -dy;
                if (ady > 0x200000)
                {
                    s32 q = dy;
                    if (dy < 0)
                        q += 127;
                    entity->velY.whole = q >> 7;
                    entity->accelY.whole = entity->velY.whole / 128;
                }
            }
            entity->hit.flags &= ~2;
            entity->state = 3;
            PlaySong(0xDA);
        }
        else
        {
            entity->velX.whole /= 2;
            entity->velY.whole = -0x38000;
            entity->accelY.whole = 0x2000;
            entity->flipY = 1;
            entity->state = 2;
            entity->drawSkip = 1;
            PlaySong(0x1AD);
            PlaySong(0xDB);
        }
    }
        Anim_StartIndexed(entity, 0, 0, 1);
        break;
    case 2:
        EntityAccelerateAndMove(entity, 0x80000);
        entity->timer++;
        if ((entity->timer & 3) == 0)
        {
            s32 x, y;
            {
                s32 d = (RandomNumberGenerator() & 0x7FFFF) - 0x40000;
                x = entity->posX.whole + d;
            }
            {
                /* -k keeps the offset out of CSE (reloaded from the pool as in the original) */
                s32 d = (RandomNumberGenerator() & 0x7FFFF) + ({ s32 k = 0x40000; -k; });
                y = entity->posY.whole + d;
            }
            Effect_SpawnShared(x, y, 0, 7);
        }
        break;
    case 3:
    {
        s32 r = Entity_MoveAndCollide(entity, 0x80000, -0x70000, 1, 0x30000);
        if (r & 3)
        {
            Effect_SpawnShared(entity->posX.whole, entity->posY.whole, 0, 7);
            entity->state = 4;
        }
        if (!(r & 0x80))
        {
            entity->posY.part16.integer += Collision_FindWaterSurfaceOffset((s16)entity->posX.part16.integer, (s16)entity->posY.part16.integer - 7);
            entity->velY.whole = 0;
            entity->accelY.whole = 0x400;
        }
    }
        break;
    case 4:
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    if (EntityIsOutsideScreen(entity, 4) || !entity->roomActive)
    {
        entity->roomActive = 1;
        entity->state = 4;
        entity->hidden = 1;
    }
    Anim_Update(entity);
}

/* SkillNeedlesDraw. */
struct Unk_08028018 {
    s16 radius;
    s16 angle;
};

/* Four (radius 8.8, angle) pairs at entity offset 0x20 (unk_504..basePalette);
 * addressed from unk_4F4 + 0x10 like the original. */
struct Unk_08028018_Block {
    u8 pad_0[0x10];
    struct Unk_08028018 parts[4];
};

/**
 * @brief Draw callback set by SkillNeedlesUse: draw once unless ext.skillNeedles.spread is
 * set; otherwise draw up to four copies on circles around the entity,
 * alternating halves by timer parity, with animFrame set to the index.
 */
void SkillNeedlesDraw(struct EwramData_EntityData *entity)
{
    s32 x, y;
    s32 i;
    struct Unk_08028018_Block *block;

    if ((u8)entity->ext.skillNeedles.spread == 0)
    {
        Sprite_Draw(entity);
        return;
    }
    x = entity->posX.whole;
    y = entity->posY.whole;
    block = (struct Unk_08028018_Block *)&entity->ext;
    for (i = 0; i <= 3; i++)
    {
        if ((entity->timer & 1) != (i & 1))
        {
            s32 r = block->parts[i].radius / 256;
            s32 a = block->parts[i].angle;
            entity->posX.whole = r * Sin(a + 0x4000) + x;
            entity->posY.whole = r * Sin(a) + y;
            entity->animFrame = i;
            Sprite_Draw(entity);
        }
    }
    entity->posX.whole = x;
    entity->posY.whole = y;
}

/* Four (radius 8.8, angle) pairs at entity offset 0x20, drawn by
 * SkillNeedlesDraw (this file). */

/**
 * @brief 280B0 | Needles red soul: graphics set 12, animation 0, layer
 * 0xD, drawn by SkillNeedlesDraw, at the caster (0x14 / -0x1A), box 0x1010F8F8
 * in slot 5 with SoulAttack_MarkHit on hit, Hit_SetTimerOverride(1, 15), song 0xE0. On a
 * hit or after 128 frames it bursts (song 0xD7, Effect_SpawnShared flame): hops
 * up with a random velY and gravity 0x2000 while four needles spread out
 * at angles 0xF000 + i * 0x3333 (+ random), radius growing by timer * 32
 * for 33 frames.
 */
void SkillNeedlesUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.redSoul.owner;
    struct Unk_08028018_Block *block;
    s32 i;
    s32 angle;

    entity->roomActive = 1;
    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(12);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        Anim_StartIndexed(entity, 0, 0, 1);
        entity->spriteLayer = 0xD;
        entity->drawFunc = (u32 *)SkillNeedlesDraw;
        Hit_SetTimerOverride(1, 15);
        Entity_PlaceRelativeTo(entity, parent, 0x14, -0x1A);
        entity->scrollWithBg = 1;
        Hit_Enable(entity, 5, SoulAttack_MarkHit, 0);
        HitBox_SetAttackRaw(entity, 0x1010F8F8);
        entity->hit.flags |= 4;
        entity->timer = 0x80;
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        PlaySong(0xE0);
        entity->state = 1;
    case 1:
        Anim_Update(entity);
        if (--entity->timer == 0)
            entity->ext.redSoul.hit = 1;
        if ((u8)entity->ext.redSoul.hit == 0)
            break;
        entity->velY.whole = -0x10000 - (RandomNumberGenerator() & 0x7FFF);
        entity->accelY.whole = 0x2000;
        Effect_SpawnShared(entity->posX.whole, entity->posY.whole, 0xF, 7);
        block = (struct Unk_08028018_Block *)&entity->ext;
        for (i = 0, angle = 0; i <= 3; i++)
        {
            block->parts[i].radius = 0;
            block->parts[i].angle = angle + 0xF000 + (RandomNumberGenerator() & 0x3FF);
            angle += 0x3333;
        }
        PlaySong(0xD7);
        entity->timer = 0x20;
        entity->state = 2;
        break;
    case 2:
        EntityAccelerateAndMoveY(entity, 0x80000);
        block = (struct Unk_08028018_Block *)&entity->ext;
        for (i = 0; i <= 3; i++)
            block->parts[i].radius += entity->timer << 5;
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
    case 3:
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        break;
    }
    if (EntityIsOutsideScreen(entity, 3) || !entity->roomActive)
    {
        entity->roomActive = 1;
        entity->state = 3;
    }
}

/**
 * @brief 282DC | Dryad red soul: a seed (graphics set 13, frame 0, layer
 * 0xD) thrown from the caster (0x14 / -0x1A) with velX 0x10000 (mirrored),
 * velY -0x20000 and gravity 0x2000, box 0x0202FFFF in slot 5 with
 * RedSoul_AnchorToHitTargetTimed on hit (linked entity in unk_504), Hit_SetTimerOverride(1, 20), song
 * 0xE5. After a hit it plays animation 0 (song 0xD5 and Player_RestoreHP(8) on
 * the first frame) for up to 21 frames while the link holds and the target
 * keeps its box, then stays 16 frames with drawSkip set.
 */
void SkillDryadUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.redSoul.owner;
    struct EwramData_EntityData *link;
    struct EwramData_EntityData *target;
    s32 zero;
    s32 r;

    switch (entity->state)
    {
    case 0:
        zero = 0;
    {
        struct SoulGraphicsResources *t = GetSet(13);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        entity->animFrame = zero;
        entity->spriteLayer = 0xD;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->scrollWithBg = 1;
        entity->roomActive = 1;
        Hit_SetTimerOverride(1, 20);
        Hit_Enable(entity, 5, RedSoul_AnchorToHitTargetTimed, 0);
        HitBox_SetAttackRaw(entity, 0x0202FFFF);
        entity->hit.flags |= 4;
        Entity_PlaceRelativeTo(entity, parent, 0x14, -0x1A);
        entity->velY.whole = -0x20000;
        entity->accelY.whole = 0x2000;
        entity->velX.whole = 0x10000;
        if (entity->flipX)
            entity->velX.whole = -entity->velX.whole;
        entity->state = 1;
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        PlaySong(0xE5);
    case 1:
        EntityAccelerateAndMove(entity, 0x40000);
        if (entity->ext.redSoul.anchor == NULL)
            break;
        entity->state = 2;
        Anim_StartIndexed(entity, 0, 0, 0);
    case 2:
        link = entity->ext.redSoul.anchor;
        if (link == NULL)
            goto animate;
        if (link->ext.redSoulAnchor.targetDead != 0)
            goto stop;
        target = link->ext.redSoulAnchor.target;
        if (entity->ext.redSoul.anchorTimer == 0)
        {
            PlaySong(0xD5);
            Player_RestoreHP(8);
        }
        if (entity->ext.redSoul.anchorTimer++ > 0x14)
            goto stop;
        if (!(target->hit.flags & 4))
            goto animate;
        entity->state = 4;
        break;
    stop:
        entity->state = 4;
        break;
    animate:
        r = Anim_Update(entity);
        if (r == 3)
        {
            entity->timer = 0x10;
            entity->state = r;
        }
        break;
    case 3:
        entity->drawSkip = 1;
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
    case 4:
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    if (EntityIsOutsideScreen(entity, 4) || !entity->roomActive)
    {
        entity->roomActive = 1;
        entity->state = 4;
    }
}

/* Inlined Entity_SetAttackBoxFromFrame (src/soul/red_souls_b.c). */

/**
 * @brief 284FC | Werewolf red soul: graphics set 14, animation 0, layer
 * 0xD at the caster (0x14 / -0xA), needs Entity_DropToFloor; Hit_SetTimerOverride(1, 12),
 * box slot 5 (no callback), song 0xD9. Dashes with Entity_Walk (speed
 * 0x4000, probe sUnk_084F13B4) with the current frame's hit box until the
 * animation ends.
 */
void SkillWerewolfUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.redSoul.owner;

    entity->roomActive = 1;
    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(14);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        Anim_StartIndexed(entity, 0, 0, 0);
        entity->spriteLayer = 0xD;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->scrollWithBg = 1;
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        Entity_PlaceRelativeTo(entity, parent, 0x14, -0xA);
        if (Entity_DropToFloor(entity) == 0)
        {
            gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
            gEwramData->playerState.nbrActiveRedSoulEntities--;
            EntityDelete(entity);
            break;
        }
        Hit_SetTimerOverride(1, 12);
        Hit_Enable(entity, 5, 0, 0);
        entity->hit.flags = (entity->hit.flags | 4) & ~2;
        PlaySong(0xD9);
        entity->state = 1;
    case 1:
        Entity_Walk(entity, -0x4000, sUnk_084F13B4, 4);
        if (Anim_Update(entity) == 3)
        {
            gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
            gEwramData->playerState.nbrActiveRedSoulEntities--;
            EntityDelete(entity);
            break;
        }
        SetFrameBox(entity);
        break;
    }
}

/* SkillBiphronAttackUpdate; graphics
 * loading as in SkillFlameDemonProjectileUpdate (this file). */

/* Inlined Entity_SetAttackBoxFromFrame (src/soul/red_souls_b.c). */

/**
 * @brief Child of SkillBiphronUse: graphics set 15, animation 0, box slot 5
 * and sound 0xD9; animate with frame hit boxes until the animation
 * ends, then free its red soul slot and delete.
 */
void SkillBiphronAttackUpdate(struct EwramData_EntityData *entity)
{
    entity->roomActive = 1;
    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(15);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        Anim_StartIndexed(entity, 0, 0, 0);
        entity->spriteLayer = 0xF;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        Hit_Enable(entity, 5, 0, 0);
        entity->hit.flags = (entity->hit.flags | 4) & ~2;
        PlaySong(0xD9);
        entity->state++;
    case 1:
        if (Anim_Update(entity) == 3)
        {
            gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
            gEwramData->playerState.nbrActiveRedSoulEntities--;
            EntityDelete(entity);
            return;
        }
        SetFrameBox(entity);
    }
}

/**
 * @brief 28800 | Biphron red soul: graphics set 15 at the caster (0x14 /
 * -0xA), needs Entity_DropToFloor; Hit_SetTimerOverride(1, 12), song 0x1A4. For 65
 * frames it moves with Entity_Walk (probe sUnk_084F13B4) at a speed
 * unk_508 growing from 0x8000 by 0x1000 up to 0x20000, turning around on
 * walls, and spawns a SkillBiphronAttackUpdate attack at its position every 8 frames.
 */
void SkillBiphronUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.redSoul.owner;
    struct EwramData_EntityData *child;

    entity->roomActive = 1;
    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(15);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        entity->scrollWithBg = 1;
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        Entity_PlaceRelativeTo(entity, parent, 0x14, -0xA);
        if (Entity_DropToFloor(entity) == 0)
        {
            gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
            gEwramData->playerState.nbrActiveRedSoulEntities--;
            EntityDelete(entity);
            break;
        }
        Hit_SetTimerOverride(1, 12);
        entity->state = 1;
        entity->ext.skillWalker.speed = 0x8000;
        PlaySong(0x1A4);
        entity->timer = 0x40;
    case 1:
        if (entity->ext.skillWalker.speed <= 0x1FFFF)
            entity->ext.skillWalker.speed += 0x1000;
        if (Entity_Walk(entity, -entity->ext.skillWalker.speed, sUnk_084F13B4, 4) & 3)
            entity->flipX ^= 1;
        if (!(entity->timer & 7))
        {
            child = RedSoul_CreateEntity(entity, (u32 *)SkillBiphronAttackUpdate);
            if (child != NULL)
            {
                child->posX.whole = entity->posX.whole;
                child->posY.whole = entity->posY.whole;
                child->flipX = entity->flipX;
            }
        }
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        break;
    }
}

/* Inlined Entity_SetAttackBoxFromFrame (src/soul/red_souls_b.c). */

/**
 * @brief 289B4 | Beam Skeleton red soul: graphics set 19, animation 0,
 * layer 0xD, Hit_SetTimerOverride(1, 16), song 0x1A9, box slot 5 (no callback,
 * disabled) with the current frame's hit box; facing like the caster and
 * kept at pixel offset (0x14 mirrored, -0x10) from it, stored in the
 * integer halves of velX / velY. Sound 0xCE plays from frame 10 until the
 * animation ends or it leaves the screen.
 */
void SkillBeamSkeletonUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.redSoul.owner;

    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(19);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        Anim_StartIndexed(entity, 0, 0, 0);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 0xD;
        entity->scrollWithBg = 1;
        Hit_SetTimerOverride(1, 16);
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        entity->flipX = parent->flipX;
        entity->velX.part16.integer = 0x14;
        entity->velY.part16.integer = -0x10;
        if (entity->flipX)
            entity->velX.part16.integer = -entity->velX.part16.integer;
        PlaySong(0x1A9);
        Hit_Enable(entity, 5, 0, 0);
        entity->hit.flags |= 4;
        entity->hit.flags |= 2;
        entity->state++;
    case 1:
        entity->posX.part16.integer = parent->posX.part16.integer + entity->velX.part16.integer;
        entity->posY.part16.integer = parent->posY.part16.integer + entity->velY.part16.integer;
        if (*(u16 *)&entity->animStep == 10)
            Sound_PlayTracked(0xCE);
        if (Anim_Update(entity) == 3)
            entity->state = 2;
        SetFrameBox(entity);
        break;
    case 2:
        Sound_StopTracked(0xCE);
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    if (EntityIsOutsideScreen(entity, 4) || !entity->roomActive)
    {
        entity->roomActive = 1;
        entity->state = 2;
    }
}

/**
 * @brief Child of SkillSkullArcherUse: arrow (graphics set 18, frame 1,
 * velX 0x60000, box 0x0202FF08) whose hit callback links a follower
 * entity; after a hit it stays 16 updates hidden, also ends off screen.
 */
void SkillSkullArcherArrowUpdate(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(18);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        entity->animFrame = 1;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 0xD;
        Hit_Enable(entity, 5, RedSoul_AnchorToHitTarget, 0);
        HitBox_SetAttackRaw(entity, 0x0202FF08);
        entity->hit.flags |= 4;
        entity->velX.whole = 0x60000;
        if (entity->flipX)
            entity->velX.whole = -entity->velX.whole;
        PlaySong(0xC8);
        entity->state++;
    case 1:
        EntityAccelerateAndMove(entity, 0x80000);
        if (entity->ext.redSoul.anchor != NULL)
        {
            entity->state = 2;
            entity->timer = 0x10;
            entity->drawSkip = 1;
        }
        break;
    case 2:
        if (entity->ext.redSoul.anchor->ext.redSoulAnchor.targetDead != 0)
        {
            gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
            gEwramData->playerState.nbrActiveRedSoulEntities--;
            EntityDelete(entity);
            return;
        }
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
    if (EntityIsOutsideScreen(entity, 2) || !entity->roomActive)
    {
        entity->roomActive = 1;
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
    }
}

/**
 * @brief 28D68 | Skull Archer red soul: graphics set 18, animation 0, layer
 * 0xD, placed at the caster (Entity_PlaceRelativeTo, -0xC / -0x18), Hit_SetTimerOverride(1, 10);
 * when the animation reaches frame 14 (animStep read as a halfword) spawn the
 * arrow SkillSkullArcherArrowUpdate at its position; ends when the animation ends or the
 * entity leaves the screen, then frees the red soul slot.
 */
void SkillSkullArcherUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.redSoul.owner;

    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(18);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        Anim_StartIndexed(entity, 0, 0, 0);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 0xD;
        entity->scrollWithBg = 1;
        Entity_PlaceRelativeTo(entity, parent, -0xC, -0x18);
        Hit_SetTimerOverride(1, 10);
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        entity->state++;
    case 1:
        if (*(u16 *)&entity->animStep == 0xE)
        {
            struct EwramData_EntityData *child = RedSoul_CreateEntity(entity, (u32 *)SkillSkullArcherArrowUpdate);
            if (child != NULL)
                Entity_PlaceRelativeTo(child, entity, 0, 0);
        }
        if (Anim_Update(entity) == 3)
            entity->state = 2;
        break;
    case 2:
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    if (EntityIsOutsideScreen(entity, 4) || !entity->roomActive)
    {
        entity->roomActive = 1;
        entity->state = 2;
    }
}

/* SkillArachneUse: matched (bone loop as in the skeleton skill:
   SpawnBone + scatter; one variable holds the linked entity pointer and its flag). */

#define DEL(e) do { gEwramData->playerState.activeRedSoulEntities[(e)->ext.redSoul.slot] = NULL; gEwramData->playerState.nbrActiveRedSoulEntities--; EntityDelete(e); } while (0)

void SkillArachneUse(struct EwramData_EntityData *entity)
{
    struct RedSoulShotInfo *info = &sUnk_084F12C8[(u16)entity->ext.redSoul.param];
    struct EwramData_EntityData *parent = entity->ext.redSoul.owner;
    u32 v;
    s32 i;

    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(info->graphicsIndex);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        Anim_StartIndexed(entity, info->animation, 0, info->param);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->scrollWithBg = 1;
        Entity_PlaceRelativeTo(entity, parent, 8, -0x1A);
    {
        s32 vx = info->velX;
        entity->velX.whole = vx;
        entity->accelY.whole = info->accelY;
        entity->velY.whole = info->velY;
        if (entity->flipX)
            entity->velX.whole = -vx;
    }
        Hit_SetTimerOverride(1, 30);
        Hit_Enable(entity, 5, RedSoul_AnchorToHitTarget, 0);
        { u8 v_ = entity->hit.flags; v_ |= 4; entity->hit.flags = v_; }
        if (info->song != 0)
            PlaySong(info->song);
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        entity->state++;
    case 1:
        EntityAccelerateAndMove(entity, 0x80000);
        Anim_Update(entity);
        SetFrameBox(entity);
        if (entity->ext.redSoul.anchor == NULL)
            break;
        if (!(info->flags & 0x100))
        {
            entity->state = 2;
            entity->timer = 0x10;
            entity->drawSkip = 1;
            entity->animFrame = 5;
            break;
        }
        PlaySong(0xD2);
        for (i = 0; i <= 2; i++)
        {
            struct EwramData_EntityData *bone;
            u8 frame = RandomNumberGenerator() % 2;
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
        Anim_StartIndexed(entity, 2, 0, 0);
        entity->state = 3;
        break;
    case 2:
        v = (u32)entity->ext.redSoul.anchor;
        if (((struct EwramData_EntityData *)v)->ext.redSoulAnchor.targetDead != 0)
        {
            DEL(entity);
            return;
        }
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
        DEL(entity);
        return;
    case 3:
        v = (u32)entity->ext.redSoul.anchor;
        v = ((struct EwramData_EntityData *)v)->ext.redSoulAnchor.targetDead;
        if (v != 0)
        {
            DEL(entity);
            return;
        }
        if (Anim_Update(entity) == 3)
        {
            DEL(entity);
            return;
        }
        break;
    }
    if (EntityIsOutsideScreen(entity, 2) || !entity->roomActive)
    {
        entity->roomActive = 1;
        DEL(entity);
    }
}

/* SkillFlameDemonProjectileUpdate; graphics
 * loading as SkillLoadGraphicsInline in src/player/player_core.c. */

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
    else if ((u8)entity->ext.redSoul.hit == 0)
        return;
    gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
    gEwramData->playerState.nbrActiveRedSoulEntities--;
    EntityDelete(entity);
}

/**
 * @brief 293C4 | Flame Demon red soul: graphics set 17 at the caster (8 /
 * -0x10), Hit_SetTimerOverride(1, 4), song 0xD8 and a Effect_SpawnShared flame; fans out
 * 3 (1 in 8: 5) SkillFlameDemonProjectileUpdate fireballs at speed 2 px, starting angle
 * velX = -(0x1000 / n) * (n / 2) and step velY = 0x1000 / n (both kept in
 * the entity's velocity fields), mirrored with the facing, each with
 * acceleration velocity / 64. The original then sets roomActive through the
 * last child pointer (NULL or uninitialised in some paths).
 */
void SkillFlameDemonUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.redSoul.owner;
    struct EwramData_EntityData *child;
    s32 i, n;

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
        entity->scrollWithBg = 1;
        Entity_PlaceRelativeTo(entity, parent, 8, -0x10);
        Hit_SetTimerOverride(1, 4);
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        entity->state++;
        n = 3;
        if ((RandomNumberGenerator() & 7) == 0)
            n = 5;
        entity->velY.whole = -0x1000 / n;
        entity->velX.whole = entity->velY.whole * (n >> 1);
        entity->velY.whole = 0x1000 / n;
        for (i = 0; i < n; i++)
        {
            child = RedSoul_CreateEntity(entity, (u32 *)SkillFlameDemonProjectileUpdate);
            if (child == NULL)
                break;
            Entity_PlaceRelativeTo(child, entity, 0, 0);
            child->velX.whole = Sin(entity->velX.whole + 0x4000) << 1;
            child->velY.whole = Sin(entity->velX.whole) << 1;
            if (child->flipX)
                child->velX.whole = -child->velX.whole;
            child->accelX.whole = child->velX.whole / 64;
            child->accelY.whole = child->velY.whole / 64;
            entity->velX.whole += entity->velY.whole;
        }
        PlaySong(0xD8);
        Effect_SpawnShared(entity->posX.whole, entity->posY.whole, 0xC, 6);
        break;
    case 1:
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    child->roomActive = 1;
}

/* SkillCockatriceEffectUpdate (pattern of
 * SkillFlameDemonProjectileUpdate). */

/**
 * @brief Child of SkillCockatriceUse: effect (graphics set 27) following its
 * owner until the animation ends or it leaves the screen; frees its
 * red soul slot on deletion.
 */
void SkillCockatriceEffectUpdate(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = entity->ext.redSoul.owner;

    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(27);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        Anim_StartIndexed(entity, 0, 0, 0);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 0xD;
        entity->state++;
    case 1:
        Entity_PlaceRelativeTo(entity, owner, 0, 0);
        if (Anim_Update(entity) == 3)
        {
            gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
            gEwramData->playerState.nbrActiveRedSoulEntities--;
            EntityDelete(entity);
            return;
        }
    }
    if (EntityIsOutsideScreen(entity, 2) || !entity->roomActive)
    {
        entity->roomActive = 1;
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
    }
}

/**
 * @brief 2968C | Cockatrice red soul: an affine breath (graphics set 27,
 * frame 1, layer 0xD, drawn by SkillCockatriceDraw) at pixel offset (8 mirrored,
 * -0x1A) from the caster, with the SkillCockatriceEffectUpdate child, Hit_SetTimerOverride(1, 60),
 * box slot 5, sound 0xCD while active. For 33 frames the angle ENTITY_HALF_510
 * (0x2800, mirrored) turns by mp = -0x40, the vertical scale is
 * sin(timer << 10) and the box (16 x 16) is moved along the angle by
 * (timer & 7) * 32 * sin/cos.
 */
void SkillCockatriceUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.redSoul.owner;
    u16 slot;
    u16 bx, by;

    switch (entity->state)
    {
    case 0:
        slot = ObjAffine_Alloc(0);
        if ((s16)slot < 0)
        {
            gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
            EntityDelete(entity);
            return;
        }
        entity->affineIndex = slot;
        entity->affine = 1;
    {
        struct SoulGraphicsResources *t = GetSet(27);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        entity->scrollWithBg = 1;
        entity->animFrame = 1;
        entity->drawFunc = (u32 *)SkillCockatriceDraw;
        entity->spriteLayer = 0xD;
        SKILL_COCKATRICE_ANGLE(entity) = 0x2800;
        SKILL_COCKATRICE_TURN(entity) = -0x40;
        entity->velX.part16.integer = 8;
        entity->velY.part16.integer = -0x1A;
        if (parent->flipX)
        {
            entity->velX.part16.integer = -entity->velX.part16.integer;
            SKILL_COCKATRICE_ANGLE(entity) = -0x8000 - SKILL_COCKATRICE_ANGLE(entity);
            SKILL_COCKATRICE_TURN(entity) = -SKILL_COCKATRICE_TURN(entity);
        }
        entity->flipX = 0;
        Hit_SetTimerOverride(1, 60);
        Hit_Enable(entity, 5, 0, 0);
        entity->hit.flags |= 4;
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        Sound_PlayTracked(0xCD);
        entity->timer = 0x20;
        entity->state++;
        RedSoul_CreateEntity(entity, (u32 *)SkillCockatriceEffectUpdate);
    case 1:
        entity->ext.skillCockatrice.scaleY = Sin(entity->timer << 10);
        SKILL_COCKATRICE_ANGLE(entity) += SKILL_COCKATRICE_TURN(entity);
        entity->posX.whole = parent->posX.whole + entity->velX.whole;
        entity->posY.whole = parent->posY.whole + entity->velY.whole;
        entity->accelX.whole = Sin(SKILL_COCKATRICE_ANGLE(entity) + 0x4000) << 5;
        entity->accelY.whole = Sin(SKILL_COCKATRICE_ANGLE(entity)) << 5;
        bx = FIXED_TO_INT(entity->accelX.whole) * (entity->timer & 7);
        by = FIXED_TO_INT(entity->accelY.whole) * (entity->timer & 7);
        HitBox_SetAttackRaw(entity, (((s16)bx - 8) & 0xFF) + ((((s16)by - 8) & 0xFF) << 8) + 0x10100000);
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
    case 2:
        Sound_StopTracked(0xCD);
        ObjAffine_Free(entity->affineIndex);
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    ObjAffine_Set(SKILL_COCKATRICE_ANGLE(entity), 0x10000, entity->ext.skillCockatrice.scaleY, 0, 0, entity->affineIndex);
    if (EntityIsOutsideScreen(entity, 2) || !entity->roomActive)
    {
        entity->roomActive = 1;
        Sound_StopTracked(0xCD);
        ObjAffine_Free(entity->affineIndex);
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
    }
}

/* Inlined Entity_SetAttackBoxFromFrame (src/soul/red_souls_b.c). */

/**
 * @brief 299A0 | Fleaman red soul: a hopping flea (graphics set 20, layer
 * 0xD) from the caster (8 / -0x1A) facing the other way, gravity 0x2000,
 * box slot 5 (no callback) with the current frame's hit box,
 * Hit_SetTimerOverride(1, 20), song 0x19C. For 160 frames it moves with
 * Entity_Walk (speed 0x18000, probe sUnk_084F13B4 + 0xA): frame by velY, turns
 * on walls, hops again on landing (velY -0x10000, or 1 in 4 a random
 * -5..-4 px). Then animation 1 with drawSkip set and random Effect_SpawnShared
 * flames every 4 frames for 16 frames.
 */
void SkillFleamanUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.redSoul.owner;
    s32 r;

    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(20);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 0xD;
        entity->scrollWithBg = 1;
        Entity_PlaceRelativeTo(entity, parent, 8, -0x1A);
        entity->accelY.whole = 0x2000;
        entity->flipX ^= 1;
        Hit_SetTimerOverride(1, 20);
        Hit_Enable(entity, 5, 0, 0);
        entity->hit.flags |= 4;
        entity->timer = 0xA0;
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        PlaySong(0x19C);
        entity->state = 1;
    case 1:
        r = Entity_Walk(entity, 0x18000, sUnk_084F13B4 + 0xA, 8);
        if (entity->velY.whole < -0x8000)
            entity->animFrame = 0;
        else if (entity->velY.whole > 0x8000)
            entity->animFrame = 2;
        else
            entity->animFrame = 1;
        if (r & 3)
            entity->flipX ^= 1;
        if (r & 8)
        {
            if (RandomNumberGenerator() & 3)
                entity->velY.whole = -0x10000;
            else
                entity->velY.whole = (RandomNumberGenerator() & 0xFFFF) + ({ s32 k = 0x50000; -k; });
        }
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
        Anim_StartIndexed(entity, 1, 0, 0);
        entity->drawSkip = 1;
        entity->state = 2;
        entity->timer = 0x10;
        break;
    case 2:
        if (!(entity->timer & 3))
        {
            s32 x, y;
            {
                s32 d = (RandomNumberGenerator() & 0x7FFFF) - 0x40000;
                x = entity->posX.whole + d;
            }
            {
                /* -k keeps the offset out of CSE (reloaded from the pool as in the original) */
                s32 d = (RandomNumberGenerator() & 0x7FFFF) + ({ s32 k = 0x40000; -k; });
                y = entity->posY.whole + d;
            }
            Effect_SpawnShared(x, y, 0, 7);
        }
        Anim_Update(entity);
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
    if (EntityIsOutsideScreen(entity, 2) || !entity->roomActive)
    {
        entity->roomActive = 1;
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    SetFrameBox(entity);
}

/* SkillTinyDevilUpdate: homing bat of SkillTinyDevilUse; matched.
 * Case 2 uses its own target variable (otherwise the case 1 one lives across calls). */

static inline s32 FixedMul(s32 a, s32 b)
{
    return (a >> 8) * (b >> 8);
}

/* Homing bat of SkillTinyDevilUse: when the animation ends it targets
 * selectedTarget or a random non-empty entry of trackedEntities and turns towards it
 * for up to 64 frames, then flies straight. */
void SkillTinyDevilUpdate(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *target;
    struct EwramData_EntityData *child;
    struct EwramData_EntityData *victim;
    s32 r, i, j;
    s32 angle, diff;
    s32 timer;

    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *set = GetSet(21);
        void *gfx = set->graphics;
        void *pal = set->palette;
        void *oam = set->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        Anim_StartIndexed(entity, 0, 0, 0);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 0xD;
        entity->state = 1;
    case 1:
        r = Anim_Update(entity);
        if (r != 3)
            break;
        if (gEwramData->playerState.selectedTarget != NULL)
            target = gEwramData->playerState.selectedTarget;
        else
        {
            i = 0;
            j = RandomNumberGenerator() & r;
            for (; i <= 3; i++)
            {
                if (gEwramData->playerState.trackedEntities[j] != NULL)
                {
                    target = gEwramData->playerState.trackedEntities[j];
                    goto found;
                }
                j = (j + 1) & 3;
            }
            target = NULL;
        }
    found:
        entity->ext.skillHoming.target = target;
        if (target != NULL)
            entity->state = 2;
        else
            entity->state = 3;
        Hit_Enable(entity, 5, SoulAttack_MarkHit, 0);
        HitBox_SetAttackRaw(entity, 0x1010F8F8);
        entity->hit.flags |= 4;
        entity->timer = 0x40;
        child = RedSoul_CreateTrailCopy(entity, 7);
        if (child != NULL)
        {
            child->drawSkip = 1;
            child->objMode = 1;
        }
        child = RedSoul_CreateTrailCopy(entity, 3);
        if (child != NULL)
            child->objMode = 1;
        PlaySong(0x134);
        break;
    case 2:
        if (entity->ext.skillHoming.speed <= 0x4FFFF)
            entity->ext.skillHoming.speed += 0x4000;
        victim = entity->ext.skillHoming.target;
        {
            s32 tx = victim->posX.whole;
            s32 ty = victim->posY.whole;
            angle = (s16)SKILL_TINY_DEVIL_HEADING(entity);
            diff = (u16)ArcTan2((s16)(FIXED_TO_INT(tx) - (s16)entity->posX.part16.integer),
                                (s16)(FIXED_TO_INT(ty) - (s16)entity->posY.part16.integer)) - angle;
        }
        if (diff > 0)
        {
            if (diff <= 0x7FFF)
                angle += 0x100;
            else
                angle -= 0x100;
        }
        else
        {
            if (diff < -0x8000)
                angle += 0x100;
            else
                angle -= 0x100;
        }
        SKILL_TINY_DEVIL_HEADING(entity) = angle;
        entity->velX.whole = FixedMul(Sin((s16)SKILL_TINY_DEVIL_HEADING(entity) + 0x4000), entity->ext.skillHoming.speed);
        entity->velY.whole = FixedMul(Sin((s16)SKILL_TINY_DEVIL_HEADING(entity)), entity->ext.skillHoming.speed);
        EntityAccelerateAndMove(entity, 0x80000);
        timer = entity->timer - 1;
        entity->timer = timer;
        if ((s16)victim->hp <= 0 || (u8)entity->ext.redSoul.hit != 0 || (u8)timer == 0)
            entity->state = 3;
        break;
    case 3:
        if (entity->ext.skillHoming.speed <= 0x4FFFF)
            entity->ext.skillHoming.speed += 0x4000;
        entity->velX.whole = FixedMul(Sin((s16)SKILL_TINY_DEVIL_HEADING(entity) + 0x4000), entity->ext.skillHoming.speed);
        entity->velY.whole = FixedMul(Sin((s16)SKILL_TINY_DEVIL_HEADING(entity)), entity->ext.skillHoming.speed);
        EntityAccelerateAndMove(entity, 0x80000);
        break;
    case 4:
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    if (EntityIsOutsideScreen(entity, 2) || !entity->roomActive)
    {
        entity->roomActive = 1;
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    if (entity->state > 1)
        entity->animFrame = (((s16)SKILL_TINY_DEVIL_HEADING(entity) + 0x4000) / 0x1000) & 0xF;
}

/* SkillTinyDevilUse; graphics
 * loading as SkillFlameDemonProjectileUpdate, spawning as SkillHarpyUse. */

/**
 * @brief 29FB0 | Tiny Devil red soul: graphics set 21, position at the
 * caster (Entity_PlaceRelativeTo, 8 / -0x1A), Hit_SetTimerOverride(1, 20), song 0x1A3; spawn
 * up to three SkillTinyDevilUpdate children at the caster's position with
 * angles -0x1800, 0, 0x1800 in ENTITY_HALF_510 (mirrored as -0x8000 - angle when
 * facing left); then free the red soul slot.
 */
void SkillTinyDevilUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.redSoul.owner;
    s32 i;
    s32 angle;

    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(21);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        entity->scrollWithBg = 1;
        Entity_PlaceRelativeTo(entity, parent, 8, -0x1A);
        Hit_SetTimerOverride(1, 20);
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        PlaySong(0x1A3);
        for (i = 0, angle = 0; i <= 2; angle += 0x1800, i++)
        {
            struct EwramData_EntityData *child = RedSoul_CreateEntity(entity, (u32 *)SkillTinyDevilUpdate);
            s32 a;
            if (child == NULL)
                break;
            child->posX.whole = entity->posX.whole;
            child->posY.whole = entity->posY.whole;
            a = angle - 0x1800;
            SKILL_TINY_DEVIL_HEADING(child) = a;
            if (entity->flipX)
                SKILL_TINY_DEVIL_HEADING(child) = -0x8000 - a;
        }
        entity->state = 1;
        break;
    case 1:
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        break;
    }
}

/**
 * @brief 2A0DC | Ukoback red soul: affine fireball (graphics set 22,
 * animation 1, layer 0xD) at the caster (8 / -0x1A), box 0x1010F8F8 in slot
 * 5 with SoulAttack_MarkHit on hit, Hit_SetTimerOverride(1, 20), song 0xDC. Grows (scale
 * unk_508 += 0x1000 up to 0x10000) and stays 240 frames or until hit, then
 * shrinks by 0x2000 per frame and ends; also ends off screen.
 */
void SkillUkobackUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.redSoul.owner;
    u16 slot;

    switch (entity->state)
    {
    case 0:
        slot = ObjAffine_Alloc(0);
        if ((s16)slot < 0)
        {
            gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
            EntityDelete(entity);
            return;
        }
        entity->affineIndex = slot;
        entity->affine = 1;
    {
        struct SoulGraphicsResources *t = GetSet(22);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        Anim_StartIndexed(entity, 1, 0, 1);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 0xD;
        entity->scrollWithBg = 1;
        Entity_PlaceRelativeTo(entity, parent, 8, -0x1A);
        Hit_SetTimerOverride(1, 20);
        Hit_Enable(entity, 5, SoulAttack_MarkHit, 0);
        HitBox_SetAttackRaw(entity, 0x1010F8F8);
        entity->hit.flags |= 4;
        entity->timer = 0xF0;
        PlaySong(0xDC);
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        entity->state = 1;
    case 1:
        if (entity->ext.skillUkoback.scale <= 0xFFFF)
            entity->ext.skillUkoback.scale += 0x1000;
        if ((u8)entity->ext.redSoul.hit != 0)
            entity->timer = 0;
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
        entity->state = 2;
    case 2:
        if (entity->ext.skillUkoback.scale <= 0)
        {
            ObjAffine_Free(entity->affineIndex);
            gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
            gEwramData->playerState.nbrActiveRedSoulEntities--;
            EntityDelete(entity);
            return;
        }
        entity->ext.skillUkoback.scale -= 0x2000;
        break;
    }
    Anim_Update(entity);
    ObjAffine_Set(0, entity->ext.skillUkoback.scale, entity->ext.skillUkoback.scale, entity->flipX, 0, entity->affineIndex);
    if (EntityIsOutsideScreen(entity, 2) || !entity->roomActive)
    {
        entity->roomActive = 1;
        ObjAffine_Free(entity->affineIndex);
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
    }
}
