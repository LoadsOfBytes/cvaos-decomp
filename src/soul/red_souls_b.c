/*
 * Red (bullet) soul skills, part 2: history trails, Ghost up to Slime, the shared red soul helpers (targets, graphics, effects) and draw callbacks.
 */
#include "core.h"
#include "bg_and_collision.h"
#include "graphics.h"
#include "structs/ewram.h"
#include "soul_skill_entity.h"
#include "gameplay.h"
#include "entity_util.h"
#include "sound.h"
#include "syscalls.h"
#include "structs/entity_render_flags.h"
#include "music.h"
#include "structs/main.h"
#include "macros.h"
#include "types.h"
#include "damage_numbers.h"
#include "entity_common.h"
#include "hitbox.h"
#include "math_anim.h"
#include "oam.h"
#include "obj_gfx.h"
#include "palette.h"
#include "red_souls_b.h"
#include "effects.h"
#include "data/data_0E0B68.h"
#include "data/data_118B48.h"
#include "data/data_4F0B18.h"
/* from player_core.h (not included: see the module notes) */
extern s32 Player_IsBossTargetLowHp(void);
extern void Player_StartContactImmunity(u8 frames);
extern struct EwramData_EntityData* RedSoul_CreateEntity(struct EwramData_EntityData *owner, u32 *update);
extern struct EwramData_EntityData* RedSoul_CreateTrailCopy(struct EwramData_EntityData *entity, u8 delay);
extern void Entity_PlaceRelativeTo(struct EwramData_EntityData *entity, struct EwramData_EntityData *anchor, s32 offsetX, s32 offsetY);

/* RedSoulHistoryTrail_Update. */
struct Unk_0802A330 {
    s16 x;
    s16 y;
};

void RedSoulHistoryTrail_Draw(struct EwramData_EntityData *entity);

static inline s32 NoRedSouls(struct EwramData *ew)
{
    s32 empty;
    if (ew->playerState.nbrActiveRedSoulEntities == 0 && ew->playerState.activeRedSoulEntities[0] == NULL)
        empty = 1;
    else
        empty = 0;
    return empty;
}

/**
 * @brief History trail update that installs its draw callback on the first
 * update: delete when no red soul is active or its slot is empty,
 * otherwise shift the 10-entry history and record the soul position.
 */
void RedSoulHistoryTrail_Update(struct EwramData_EntityData *entity)
{
    struct Unk_0802A330 *trail = (struct Unk_0802A330 *)&entity->ext;
    struct EwramData *ew = gEwramData;
    struct EwramData_EntityData *owner = ew->playerState.activeRedSoulEntities[entity->stage];
    s32 i;

    if (NoRedSouls(ew) || gEwramData->playerState.activeRedSoulEntities[entity->stage] == NULL)
    {
        EntityDelete(entity);
        return;
    }
    switch (entity->state)
    {
    case 0:
        entity->drawFunc = (u32 *)RedSoulHistoryTrail_Draw;
        entity->state++;
    }
    for (i = 9; i > 0; i--)
    {
        trail[i].x = trail[i - 1].x;
        trail[i].y = trail[i - 1].y;
    }
    trail[0].x = GetEntityRoomXPositionInteger(owner);
    trail[0].y = GetEntityRoomYPositionInteger(owner);
}

/* The s16 layer and u8 facing temporaries were found with a permuter. */
void RedSoulHistoryTrail_Update(struct EwramData_EntityData *entity);

/**
 * @brief Create a history trail entity (RedSoulHistoryTrail_Update) copying
 * the ghost sprite, palette and facing, with its position history
 * filled with the current position.
 */
struct EwramData_EntityData *SkillGhost_CreateHistoryTrail(struct EwramData_EntityData *owner)
{
    s32 graphics = Sprite_GetTileSlot(owner);
    u8 *index = &owner->spriteSlot;
    struct SpriteSheet *sprite = gEwramData->spriteSlots[*index].sheet;
    struct SpriteSheet *s = sprite;
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, (u32 *)RedSoulHistoryTrail_Update);
    struct Unk_0802A330 *trail;
    s32 i;

    if (entity == NULL)
        return NULL;
    if (sprite == NULL)
    {
        EntityDelete(entity);
        return NULL;
    }
    Sprite_Attach(entity, s, graphics, owner->palette);
    {
        s16 layer = owner->spriteLayer + 1;
        entity->spriteLayer = layer;
    }
    {
        u8 facing = owner->flipX;
        entity->flipX = facing;
    }
    entity->stage = owner->ext.redSoul.slot;
    trail = (struct Unk_0802A330 *)&entity->ext;
    for (i = 9; i != 0; i--)
    {
        trail[i].x = GetEntityRoomXPositionInteger(owner);
        trail[i].y = GetEntityRoomYPositionInteger(owner);
    }
    entity->scrollWithBg = 1;
    return entity;
}

/* Red soul projectile: homes on the nearest target, or wanders randomly. */
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

static inline s16 FixedDelta(s32 fixed, s32 base)
{
    s32 delta;

    if (fixed < 0)
        delta = -(-fixed >> 16) - base;
    else
        delta = (fixed >> 16) - base;
    return delta;
}

static inline s32 FixedMul(s32 a, s32 b)
{
    return (a >> 8) * (b >> 8);
}

/**
 * @brief Ghost spirit created by SkillGhostUse: appear, leave a trail, home
 * in on the nearest enemy and fly off.
 */
void SkillGhostSpiritUpdate(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *target;
    s32 i;
    s32 angle, diff;

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
        Anim_StartIndexed(entity, 1, 0, 0);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 0xD;
        entity->state = 1;
    case 1:
        if (Anim_Update(entity) != 3)
            break;
        SkillGhost_CreateHistoryTrail(entity);
        {
            u32 bestDistance = -1;
            struct EwramData_EntityData *nearest = NULL;
            struct EwramData_EntityData *found;

            if (gEwramData->playerState.selectedTarget != NULL)
                found = gEwramData->playerState.selectedTarget;
            else
            {
                for (i = 0; i < 4; i++)
                {
                    if (gEwramData->playerState.trackedEntities[i] != NULL)
                    {
                        s32 dx = (s16)gEwramData->playerState.trackedEntities[i]->posX.part16.integer - (s16)entity->posX.part16.integer;
                        s32 dy = gEwramData->playerState.trackedEntities[i]->posY.part16.integer - entity->posY.part16.integer;
                        u32 distance = dx * dx + dy * dy;
                        if (bestDistance > distance)
                        {
                            bestDistance = distance;
                            nearest = gEwramData->playerState.trackedEntities[i];
                        }
                    }
                }
                found = nearest;
            }
            entity->ext.skillHoming.target = found;
            if (found != NULL)
                entity->state = 2;
            else
                entity->state = 3;
        }
        Hit_Enable(entity, 5, SoulAttack_MarkHit, 0);
        HitBox_SetAttackRaw(entity, 0x1010F8F8);
        entity->hit.flags |= 4;
        entity->timer = 0x40;
        break;
    case 2:
    {
        s32 tx, ty;

        if (entity->ext.skillHoming.speed <= 0x3FFFF)
            entity->ext.skillHoming.speed += 0x2000;
        target = entity->ext.skillHoming.target;
        tx = target->posX.whole;
        ty = target->posY.whole;
        angle = SKILL_GHOST_HEADING(entity);
        {
            s32 dx, dy;

            dx = FixedDelta(tx, (s16)(dy = entity->posX.part16.integer));
            dy = FixedDelta(ty, (s16)entity->posY.part16.integer);
            diff = ArcTan2(dx, dy) - angle;
        }
        if (diff > 0)
        {
            if (diff <= 0x7FFF)
                angle += 0x300;
            else
                angle -= 0x300;
        }
        else if (diff < -0x8000)
            angle += 0x300;
        else
            angle -= 0x300;
        SKILL_GHOST_HEADING(entity) = angle;
        entity->velX.whole = FixedMul(Sin(SKILL_GHOST_HEADING(entity) + 0x4000), entity->ext.skillHoming.speed);
        entity->velY.whole = FixedMul(Sin(SKILL_GHOST_HEADING(entity)), entity->ext.skillHoming.speed);
        EntityAccelerateAndMove(entity, 0x80000);
        entity->timer--;
        if ((s16)target->hp <= 0 || (u8)entity->ext.redSoul.hit != 0 || entity->timer == 0)
            entity->state = 4;
        break;
    }
    case 3:
    {
        s32 dx, dy, tx, ty;

        if (entity->ext.skillHoming.speed <= 0x3FFFF)
            entity->ext.skillHoming.speed += 0x2000;
        tx = (RandomNumberGenerator() & 0x1FF) << 16;
        ty = (RandomNumberGenerator() & 0xFF) << 16;
        angle = SKILL_GHOST_HEADING(entity);
        {

            dx = FixedDelta(tx, (s16)(dy = entity->posX.part16.integer));
            dy = FixedDelta(ty, (s16)entity->posY.part16.integer);
            diff = ArcTan2(dx, dy) - angle;
        }
        if (diff > 0)
        {
            if (diff <= 0x7FFF)
                angle += 0x300;
            else
                angle -= 0x300;
        }
        else if (diff < -0x8000)
            angle += 0x300;
        else
            angle -= 0x300;
        SKILL_GHOST_HEADING(entity) = angle;
        entity->velX.whole = FixedMul(Sin(SKILL_GHOST_HEADING(entity) + 0x4000), entity->ext.skillHoming.speed);
        entity->velY.whole = FixedMul(Sin(SKILL_GHOST_HEADING(entity)), entity->ext.skillHoming.speed);
        EntityAccelerateAndMove(entity, 0x80000);
        if (--entity->timer == 0)
            entity->state = 4;
        break;
    }
    case 4:
        if (entity->ext.skillHoming.speed <= 0x3FFFF)
            entity->ext.skillHoming.speed += 0x2000;
        entity->velX.whole = FixedMul(Sin(SKILL_GHOST_HEADING(entity) + 0x4000), entity->ext.skillHoming.speed);
        entity->velY.whole = FixedMul(Sin(SKILL_GHOST_HEADING(entity)), entity->ext.skillHoming.speed);
        EntityAccelerateAndMove(entity, 0x80000);
        break;
    case 5:
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
    }
}

/* SkillGhostUse; structure as
 * SkillHarpyUse (this file). */

/**
 * @brief 2A8D4 | Ghost red soul: position at the caster (Entity_PlaceRelativeTo, 0
 * / -0x18), Hit_SetTimerOverride(1, 15), song 0xD6; spawn two SkillGhostSpiritUpdate ghosts
 * (RedSoul_CreateEntity) offset by sine/cosine << 4 around it, at angles -0x7800 and -0x1800
 * plus a random 0..0xFFF (stored in ENTITY_HALF_510); then free the red soul slot.
 */
void SkillGhostUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.redSoul.owner;
    s32 i;
    s32 angle;

    entity->roomActive = 1;
    switch (entity->state)
    {
    case 0:
        Entity_PlaceRelativeTo(entity, parent, 0, -0x18);
        entity->scrollWithBg = 1;
        Hit_SetTimerOverride(1, 15);
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        PlaySong(0xD6);
        entity->state = 1;
    case 1:
        for (i = 0, angle = 0; i <= 1; angle += 0x6000, i++)
        {
            struct EwramData_EntityData *child = RedSoul_CreateEntity(entity, (u32 *)SkillGhostSpiritUpdate);
            if (child == NULL)
                break;
            {
                s32 offset = (s32)(RandomNumberGenerator() % 0x1000) - 0x7800;
                SKILL_GHOST_HEADING(child) = angle + offset;
            }
            child->posX.whole = entity->posX.whole + (Sin(SKILL_GHOST_HEADING(child) + 0x4000) << 4);
            child->posY.whole = entity->posY.whole + (Sin(SKILL_GHOST_HEADING(child)) << 4);
        }
        entity->state = 2;
        break;
    case 2:
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        break;
    }
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

/**
 * @brief 2A9DC | Nightmare red soul: affine graphics set 23 (layer 0xD,
 * flipped against the caster, offset 8 px), moving at 0x4000 / 0x2000 with
 * acceleration 0x80000; sounds 0x19C and 0xE1. Every frame a Effect_SpawnShared
 * (type 7) particle appears at a random spot and drifts at a random angle;
 * the affine scale unk_508 grows by 0x1000 per frame until 0x10000, then the
 * slot is freed and two RedSoul_CreateTrailCopy children (types 7 and 3) are released.
 */
void SkillNightmareUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.redSoul.owner;
    struct EwramData_EntityData *child;
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
        struct SoulGraphicsResources *t = GetSet(23);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        Anim_StartIndexed(entity, 0, 0, 1);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 0xD;
        entity->scrollWithBg = 1;
        Entity_PlaceRelativeTo(entity, parent, 8, 0);
    {
        s32 vx = 0x4000;
        s32 vy;
        entity->velX.whole = vx;
        vy = 0x2000;
        entity->accelX.whole = vy;
        if (entity->flipX)
        {
            entity->velX.whole = -vx;
            entity->accelX.whole = -vy;
        }
    }
        entity->flipX ^= 1;
        Hit_SetTimerOverride(1, 20);
        Hit_Enable(entity, 5, 0, 0);
    {
        u8 zero = 0;
        entity->hit.flags |= 4;
        entity->hit.flags |= zero;
        entity->hit.flags &= ~2;
    }
        PlaySong(0x19C);
        PlaySong(0xE1);
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        entity->state = 1;
    case 1:
    {
        s32 r = (RandomNumberGenerator() & 0x7FFFF) - 0x40000;
        s32 x = entity->posX.whole + r;
        s32 y;
        r = (RandomNumberGenerator() & 0x7FFFF) - 0x140000;
        y = entity->posY.whole + r;
        child = Effect_SpawnShared(x, y, 0, 7);
    }
        if (child != NULL)
        {
            u16 angle = (RandomNumberGenerator() & 0x3FFF) - 0x2000;
            if (entity->flipX)
                angle = (u32)((angle << 16) + 0x80000000) >> 16;
            child->accelX.whole = Sin((s16)angle + 0x4000) / 32;
            child->accelY.whole = Sin((s16)angle) / 32;
        }
        if (entity->ext.skillNightmare.scaleX <= 0xFFFF)
        {
            entity->ext.skillNightmare.scaleX += 0x1000;
            ObjAffine_Set(0, entity->ext.skillNightmare.scaleX, 0x10000, entity->flipX, 0, entity->affineIndex);
            EntityAccelerateAndMove(entity, 0x80000);
            break;
        }
        ObjAffine_Free(entity->affineIndex);
        entity->affineIndex = 0;
        entity->affine = 0;
        entity->state = 2;
    {
        struct EwramData_EntityData *c = RedSoul_CreateTrailCopy(entity, 7);
        if (c != NULL)
        {
            c->drawSkip = 1;
            c->objMode = 1;
        }
        c = RedSoul_CreateTrailCopy(entity, 3);
        if (c != NULL)
            c->objMode = 1;
    }
    case 2:
        EntityAccelerateAndMove(entity, 0x80000);
        break;
    }
    Anim_Update(entity);
    SetFrameBox(entity);
    if (EntityIsOutsideScreen(entity, 3) || !entity->roomActive)
    {
        entity->roomActive = 1;
        if (ENTITY_RENDER_FLAGS16(entity) & 0x101F)
            ObjAffine_Free(entity->affineIndex);
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
    }
}

/* Mandragora skill: spawn falling leaves around the caster while the flash lasts. */
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

/* Inlined Entity_SetAttackBoxFromFrame (this file). */

/**
 * @brief Child of SkillSkullMillioneUse: skull (graphics set 7, animation 0,
 * box slot 5) animating with frame hit boxes until the animation ends
 * or it leaves the screen, then frees its red soul slot.
 */
void SkillSkullMillioneSkullUpdate(struct EwramData_EntityData *entity)
{
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
        Anim_StartIndexed(entity, 0, 0, 0);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 0xD;
        entity->scrollWithBg = 1;
        Hit_Enable(entity, 5, 0, 0);
        entity->hit.flags |= 4;
        entity->hit.flags |= 2;
        entity->state++;
    case 1:
        if (Anim_Update(entity) == 3)
            entity->state = 2;
        SetFrameBox(entity);
        break;
    case 2:
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
    }
}

/**
 * @brief 2B1B8 | Skull Millione red soul: graphics set 7, Hit_SetTimerOverride(1,
 * 12), song 0xC9; moves away from the caster (offset subState + 0x14 /
 * -0x14, subState growing by 1 per frame) and spawns a SkillSkullMillioneSkullUpdate child
 * at its position every second frame, three times; ends then or off screen.
 */
void SkillSkullMillioneUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.redSoul.owner;
    struct EwramData_EntityData *child;

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
        entity->scrollWithBg = 1;
        Hit_SetTimerOverride(1, 12);
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        PlaySong(0xC9);
        entity->state++;
    case 1:
        Entity_PlaceRelativeTo(entity, parent, entity->subState + 0x14, -0x14);
        entity->timer++;
        entity->subState++;
        if (entity->timer > 1)
        {
            entity->timer = 0;
            child = RedSoul_CreateEntity(entity, (u32 *)SkillSkullMillioneSkullUpdate);
            if (child != NULL)
            {
                child->posX.whole = entity->posX.whole;
                child->posY.whole = entity->posY.whole;
                child->flipX = entity->flipX;
            }
            if (++entity->stage > 2)
                entity->state = 2;
        }
        break;
    case 2:
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    if (EntityIsOutsideScreen(entity, 3) || !entity->roomActive)
    {
        entity->roomActive = 1;
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
    }
}

/**
 * @brief 2B350 | Fish Head red soul: a fireball (graphics set 17, animation
 * 0, layer 0xD) from the caster (8 / -0x1A) with velX 0x20000 (mirrored)
 * and acceleration accelX = velX / 32, box 0x0C0CFAFA in slot 5 with
 * SoulAttack_MarkHit on hit, Hit_SetTimerOverride(1, 20), song 0xD8. Ends on hit or off
 * screen.
 */
void SkillFishHeadUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.redSoul.owner;

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
        Anim_StartIndexed(entity, 0, 0, 1);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 0xD;
        entity->scrollWithBg = 1;
        Entity_PlaceRelativeTo(entity, parent, 8, -0x1A);
        entity->velX.whole = 0x20000;
        if (entity->flipX)
            entity->velX.whole = -entity->velX.whole;
        entity->accelX.whole = entity->velX.whole / 32;
        Hit_SetTimerOverride(1, 20);
        Hit_Enable(entity, 5, SoulAttack_MarkHit, 0);
        HitBox_SetAttackRaw(entity, 0x0C0CFAFA);
        entity->hit.flags |= 4;
        PlaySong(0xD8);
        gEwramData->playerState.nbrActiveRedSoulEntities++;
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

static inline u32 *Slot131CC(struct EwramData *e) { return &e->playerState.redSoulGfx; }

static inline u8 *NbrActive(struct EwramData *e) { return &e->playerState.nbrActiveRedSoulEntities; }

/* Gladiator red soul: a rolling boulder thrown from the caster; it bounces
 * off walls, slows down on the floor and crumbles (debris Effect_SpawnShared)
 * when it stops. */
void SkillGladiatorUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.redSoul.owner;
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
            gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
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
        entity->ext.skillGladiator.speed = -0x20000;
        if (entity->flipX)
            entity->ext.skillGladiator.speed = -entity->ext.skillGladiator.speed;
        Hit_SetTimerOverride(1, 30);
        Hit_Enable(entity, 5, 0, 0);
        HitBox_SetAttackRaw(entity, 0x2020F0F0);
        entity->hit.flags |= 4;
        entity->state = 1;
        (*NbrActive(*ew))++;
    }
        PlaySong(0xC5);
    case 1:
        result = Entity_Walk(entity, -entity->ext.skillGladiator.speed, sSkillGladiatorWalkProfile, 2);
        if (result & 0x40)
        {
            v = entity->ext.skillGladiator.speed;
            if (v > 0)
                goto dec;
            { register u32 z_ asm("r3"); asm("" : "=r"(z_)); /* FAKEMATCH: occupy r3 for reload rotation */
            v += 0x1000;
            asm("" :: "r"(z_)); } /* FAKEMATCH: occupy r3 for reload rotation */
            goto store;
        }
        else if (result & 0x20)
        {
            v = entity->ext.skillGladiator.speed;
            if (v > 0)
                v += 0x1000;
            else
            {
            dec:
                v -= 0x1000;
            }
        store:
            entity->ext.skillGladiator.speed = v;
        }
        else
        {
            if (entity->ext.skillGladiator.speed >= 0)
            {
                entity->ext.skillGladiator.speed -= 0x200;
                if (entity->ext.skillGladiator.speed <= 0)
                {
                    entity->ext.skillGladiator.speed = 0;
                    entity->timer = 0x10;
                    entity->drawSkip = 1;
                    entity->state = 2;
                }
            }
            else
            {
                entity->ext.skillGladiator.speed += 0x200;
                if (entity->ext.skillGladiator.speed > 0)
                {
                    entity->ext.skillGladiator.speed = 0;
                    entity->timer = 0x10;
                    entity->drawSkip = 1;
                    entity->state = 2;
                }
            }
        }
    roll:
        SKILL_GLADIATOR_ANGLE(entity) += -entity->ext.skillGladiator.speed / 64;
        if (result & 3)
        {
            PlaySong(0x15F);
            entity->ext.skillGladiator.speed = -entity->ext.skillGladiator.speed / 2;
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
            gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
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
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    ObjAffine_Set(SKILL_GLADIATOR_ANGLE(entity), 0x10000, 0x10000, 0, 0, entity->affineIndex);
}

/* Inlined Entity_SetAttackBoxFromFrame (this file). */

/**
 * @brief 2B808 | Durga red soul: a projectile (graphics set 24, frame 2,
 * layer 0xD) from the caster (8 / -0x1A) with velX 0x40000 (mirrored), box
 * slot 5 (no callback) with the current frame's hit box, Hit_SetTimerOverride(1,
 * 30), song 0xC7. Ends off screen.
 */
void SkillDurgaUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.redSoul.owner;

    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(24);
        s32 zero;
        zero = 0;
        /* the OR with this zero variable survives in the original */
        SkillLoadGraphicsInline(entity, zero, t->graphics, t->palette, t->spriteSheet);
        entity->animFrame = 2;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 0xD;
        entity->scrollWithBg = 1;
        Entity_PlaceRelativeTo(entity, parent, 8, -0x1A);
        entity->velX.whole = 0x40000;
        if (entity->flipX)
            entity->velX.whole = -entity->velX.whole;
        Hit_SetTimerOverride(1, 30);
        Hit_Enable(entity, 5, 0, 0);
        entity->hit.flags |= 4;
        entity->hit.flags |= zero;
        SetFrameBox(entity);
    }
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        PlaySong(0xC7);
        entity->state++;
    case 1:
        EntityAccelerateAndMove(entity, 0x80000);
    }
    if (EntityIsOutsideScreen(entity, 3) || !entity->roomActive)
    {
        entity->roomActive = 1;
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
    }
}

/* Inlined Entity_SetAttackBoxFromFrame (this file). */

/**
 * @brief 2B9A8 | Killer Mantle red soul: graphics set 26, animation 0,
 * layer 0xD, Hit_SetTimerOverride(1, 20), box slot 5 (no callback, disabled), song
 * 0xD1; follows the caster with the current frame's hit box until the
 * animation ends or it leaves the screen.
 */
void SkillKillerMantleUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.redSoul.owner;

    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(26);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        Anim_StartIndexed(entity, 0, 0, 0);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 0xD;
        entity->scrollWithBg = 1;
        Hit_SetTimerOverride(1, 20);
        Hit_Enable(entity, 5, 0, 0);
        entity->hit.flags |= 4;
        entity->hit.flags |= 2;
        PlaySong(0xD1);
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        entity->state++;
    case 1:
        Entity_PlaceRelativeTo(entity, parent, 0, 0);
        if (Anim_Update(entity) == 3)
            entity->state = 2;
        SetFrameBox(entity);
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

/**
 * @brief 2BB44 | Kyoma Demon red soul: affine sprite (graphics set 27,
 * frame 0, layer 0xD, objMode = 1) on the caster, song 0x97; for 33
 * frames scaled vertically by sin(timer << 10) (unk_508), then calls
 * Player_StartContactImmunity(180) on the player, sets hidden and waits 180 frames before freeing its
 * affine slot and red soul slot.
 */
void SkillKyomaDemonUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.redSoul.owner;
    u16 slot;

    entity->roomActive = 1;
    switch (entity->state)
    {
    case 0:
        slot = ObjAffine_Alloc(0);
        if ((s16)slot < 0)
        {
            gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
            EntityDelete(entity);
            break;
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
        entity->animFrame = 0;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 0xD;
        entity->objMode = 1;
        entity->scrollWithBg = 1;
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        PlaySong(0x97);
        entity->timer = 0x20;
        entity->state++;
    case 1:
        Entity_PlaceRelativeTo(entity, parent, 0, 0);
        entity->ext.skillKyomaDemon.scaleX = Sin(entity->timer << 10);
        ObjAffine_Set(0, entity->ext.skillKyomaDemon.scaleX, 0x10000, 0, 0, entity->affineIndex);
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
        entity->ext.skillKyomaDemonWait.timer = 0xB4;
        Player_StartContactImmunity(0xB4);
        entity->hidden = 1;
        entity->state++;
    case 2:
        if (entity->ext.skillKyomaDemonWait.timer != 0)
        {
            entity->ext.skillKyomaDemonWait.timer--;
            break;
        }
        ObjAffine_Free(entity->affineIndex);
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        break;
    }
}

/* Inlined Entity_SetAttackBoxFromFrame (this file). */

/**
 * @brief 2BD1C | Fireball of Flame Demon / Demon Lord (spawned by
 * SkillDemonLordUse): affine sprite (graphics set 28, frame 9, layer 0xD)
 * with the current frame's hit box in slot 5, spinning by mp = 0x234
 * (mirrored) while moving; every 8th frame it drops a Effect_SpawnShared flame
 * within +-8 / +-2 px with accelY -0x1000. Ends off screen.
 */
void SkillFlameDemonUpdate(struct EwramData_EntityData *entity)
{
    u16 slot;
    s32 spin;
    s32 timer;
    struct EwramData_EntityData *flame;

    switch (entity->state)
    {
    case 0:
        slot = ObjAffine_Alloc(0);
        if ((s16)slot < 0)
        {
            gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
            gEwramData->playerState.nbrActiveRedSoulEntities--;
            EntityDelete(entity);
            return;
        }
        entity->affineIndex = slot;
        entity->affine = 1;
    {
        struct SoulGraphicsResources *t = GetSet(28);
        s32 zero;
        zero = 0;
        /* the OR with this zero variable survives in the original */
        SkillLoadGraphicsInline(entity, zero, t->graphics, t->palette, t->spriteSheet);
        entity->animFrame = 9;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 0xD;
        Hit_Enable(entity, 5, 0, 0);
        entity->hit.flags |= 4;
        entity->hit.flags |= zero;
        SetFrameBox(entity);
    }
        spin = 0x234;
        SKILL_FLAME_DEMON_SPIN(entity) = spin;
        if (entity->flipX)
            SKILL_FLAME_DEMON_SPIN(entity) = -spin;
        entity->state++;
    case 1:
        SKILL_FLAME_DEMON_ANGLE(entity) += SKILL_FLAME_DEMON_SPIN(entity);
        EntityAccelerateAndMove(entity, 0x80000);
        ObjAffine_Set(SKILL_FLAME_DEMON_ANGLE(entity), 0x10000, 0x10000, entity->flipX, 0, entity->affineIndex);
        timer = entity->timer + 1;
        entity->timer = timer;
        if ((timer & 7) == 0)
        {
            s32 x, y;
            {
                s32 d = (RandomNumberGenerator() & 0xFFFFF) - 0x80000;
                x = entity->posX.whole + d;
            }
            {
                s32 d = (RandomNumberGenerator() & 0x3FFFF) - 0x20000;
                y = entity->posY.whole + d;
            }
            flame = Effect_SpawnShared(x, y, 0xF, 7);
            if (flame != NULL)
                flame->accelY.whole = -0x1000;
        }
    }
    if (EntityIsOutsideScreen(entity, 3) || !entity->roomActive)
    {
        entity->roomActive = 1;
        ObjAffine_Free(entity->affineIndex);
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
    }
}

/**
 * @brief 2BF68 | Demon Lord red soul: graphics set 28, animation 0, layer
 * 0xD, Hit_SetTimerOverride(1, 15), song 0x1A4, placed at the caster (8 / -0x10);
 * on the 29th frame spawn a SkillFlameDemonUpdate fireball (song 0x12A,
 * offset 0xA / -2, velX 0x18000, accelX 0x400, both negated when flipped);
 * ends with the animation. The original sets roomActive through the child
 * pointer, which is uninitialised when no fireball was spawned this frame.
 */
void SkillDemonLordUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.redSoul.owner;
    struct EwramData_EntityData *child;

    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(28);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        Anim_StartIndexed(entity, 0, 0, 0);
        entity->spriteLayer = 0xD;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->scrollWithBg = 1;
        Hit_SetTimerOverride(1, 15);
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        PlaySong(0x1A4);
        entity->state++;
        Entity_PlaceRelativeTo(entity, parent, 8, -0x10);
    case 1:
        if (++entity->timer == 0x1D)
        {
            child = RedSoul_CreateEntity(entity, (u32 *)SkillFlameDemonUpdate);
            if (child != NULL)
            {
                s32 speed, accel;
                PlaySong(0x12A);
                Entity_PlaceRelativeTo(child, entity, 0xA, -2);
                speed = 0x18000;
                child->velX.whole = speed;
                accel = 0x400;
                child->accelX.whole = accel;
                if (child->flipX)
                {
                    child->velX.whole = -speed;
                    child->accelX.whole = -accel;
                }
            }
        }
        if (Anim_Update(entity) == 3)
        {
            gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
            gEwramData->playerState.nbrActiveRedSoulEntities--;
            EntityDelete(entity);
            return;
        }
    }
    child->roomActive = 1;
}

/* Inlined Entity_SetAttackBoxFromFrame (this file). */

/**
 * @brief 2C0D4 | Legion red soul: graphics set 29, animation 0, layer 0xE,
 * Hit_SetTimerOverride(1, 15), box slot 5 (no callback, disabled), song 0x1A9;
 * follows the caster keeping the initial facing (stage) with the current
 * frame's hit box; starts sound 0xCE (Sound_PlayTracked) on frame 12 and stops
 * it (Sound_StopTracked) when the caster's unk_4F4 bit 7 is set or the animation
 * ends.
 */
void SkillLegionUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.redSoul.owner;

    entity->roomActive = 1;
    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(29);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        Anim_StartIndexed(entity, 0, 0, 0);
        entity->spriteLayer = 0xE;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->scrollWithBg = 1;
        Hit_SetTimerOverride(1, 15);
        Hit_Enable(entity, 5, 0, 0);
        entity->hit.flags |= 4;
        entity->hit.flags |= 2;
        Entity_PlaceRelativeTo(entity, parent, 0, 0);
        entity->stage = parent->flipX;
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        PlaySong(0x1A9);
        entity->state = 1;
    case 1:
        Entity_PlaceRelativeTo(entity, parent, 0, 0);
        entity->flipX = entity->stage;
        if (*(u16 *)&entity->animStep == 0xC)
            Sound_PlayTracked(0xCE);
        if (parent->ext.player.actionFlags & 0x80)
        {
            Sound_StopTracked(0xCE);
            gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
            gEwramData->playerState.nbrActiveRedSoulEntities--;
            EntityDelete(entity);
            return;
        }
    }
    if (Anim_Update(entity) == 3)
    {
        Sound_StopTracked(0xCE);
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    SetFrameBox(entity);
}

/* Inlined Entity_SetAttackBoxFromFrame (this file). */

/**
 * @brief 2C2C4 | Balore red soul: graphics set 30, animation 0, layer 0xE,
 * Hit_SetTimerOverride(1, 60), box slot 5 (no callback, disabled), song 0x19F or
 * 0x1A1 (Player_IsBossTargetLowHp); follows the caster with the current frame's hit
 * box while the caster's unk_4F4 & 0x180 equals 0x100, until the animation
 * ends.
 */
void SkillBaloreUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.redSoul.owner;

    entity->roomActive = 1;
    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(30);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        Anim_StartIndexed(entity, 0, 0, 0);
        entity->spriteLayer = 0xE;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->scrollWithBg = 1;
        Hit_SetTimerOverride(1, 60);
        Hit_Enable(entity, 5, 0, 0);
        entity->hit.flags |= 4;
        entity->hit.flags |= 2;
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        if (Player_IsBossTargetLowHp())
            PlaySong(0x19F);
        else
            PlaySong(0x1A1);
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

#define FIXED_TO_INT(v) ((v) < 0 ? -(-(v) >> 16) : (v) >> 16)

/* Inlined Entity_SetAttackBoxFromFrame (this file). */

/**
 * @brief 2C488 | Blue Crow red soul: graphics set 31, animation 2, layer
 * 0xD, Hit_SetTimerOverride(1, 20), song 0xEE, box slot 5 (no callback, disabled)
 * with the current frame's hit box. For 51 frames it circles a point 0x12
 * px above the caster facing the other way: angle ENTITY_HALF_510 from 0x1000
 * (-0x9000 flipped) turning by mp = -0x147 (mirrored), radius
 * unk_508 = 60 * sin(timer * 0x28F) px.
 */
void SkillBlueCrowUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.redSoul.owner;
    s32 dx, dy;
    s32 spin, angle;

    entity->roomActive = 1;
    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(31);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        Anim_StartIndexed(entity, 2, 0, 1);
        entity->spriteLayer = 0xD;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        Hit_SetTimerOverride(1, 20);
        entity->flipX = parent->flipX;
        SKILL_BLUE_CROW_ANGLE(entity) = 0x1000;
        spin = 0xFEB9;
        SKILL_BLUE_CROW_TURN(entity) = spin;
        if (entity->flipX)
        {
            angle = -0x9000;
            SKILL_BLUE_CROW_ANGLE(entity) = angle;
            SKILL_BLUE_CROW_TURN(entity) = -SKILL_BLUE_CROW_TURN(entity);
        }
        entity->flipX ^= 1;
        entity->timer = 0x32;
        Hit_Enable(entity, 5, 0, 0);
        entity->hit.flags |= 4;
        entity->hit.flags |= 2;
        PlaySong(0xEE);
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        entity->state++;
    case 1:
        entity->ext.skillBlueCrow.radius = FIXED_TO_INT(Sin(entity->timer * 0x28F) * 60);
        SKILL_BLUE_CROW_ANGLE(entity) += SKILL_BLUE_CROW_TURN(entity);
        dx = entity->ext.skillBlueCrow.radius * Sin(SKILL_BLUE_CROW_ANGLE(entity) + 0x4000);
        dy = entity->ext.skillBlueCrow.radius * Sin(SKILL_BLUE_CROW_ANGLE(entity));
        entity->posX.whole = parent->posX.whole + dx;
        entity->posY.whole = parent->posY.whole + dy - 0x120000;
        if (entity->timer != 0)
            entity->timer--;
        else
            entity->state = 2;
        break;
    case 2:
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    Anim_Update(entity);
    SetFrameBox(entity);
}

/**
 * @brief 2C6EC | Man Eater red soul: a seed (graphics set 32, one of three
 * random variants, frame base stage = 5 * variant) from the caster (8 /
 * -0x1A) with velX 0x12000 (mirrored), box 0x0C0CFAFA in slot 5 with
 * SoulAttack_MarkHit on hit, Hit_SetTimerOverride(1, 20), song 0x15E. The frame advances
 * every 10 frames for 30 frames, then it flies on for 48 frames; on a hit
 * it stops (frame base + 4) for 16 frames. Either way it then shows drawSkip
 * for 16 frames and ends; also ends off screen.
 */
void SkillManEaterUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.redSoul.owner;

    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(32);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        RandomNumberGenerator();
        entity->subState = RandomNumberGenerator() % 3;
        entity->stage = entity->subState * 5;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 0xD;
        entity->scrollWithBg = 1;
        Entity_PlaceRelativeTo(entity, parent, 8, -0x1A);
        entity->velX.whole = 0x12000;
        if (entity->flipX)
            entity->velX.whole = -entity->velX.whole;
        Hit_SetTimerOverride(1, 20);
        Hit_Enable(entity, 5, SoulAttack_MarkHit, 0);
        HitBox_SetAttackRaw(entity, 0x0C0CFAFA);
        entity->hit.flags |= 4;
        PlaySong(0x15E);
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        entity->state++;
    case 1:
        entity->animFrame = entity->stage + entity->ext.skillManEater.frames / 10;
        EntityAccelerateAndMove(entity, 0x80000);
        if (++entity->ext.skillManEater.frames > 0x1D)
        {
            entity->timer = 0x30;
            entity->state = 2;
        }
        break;
    case 2:
        EntityAccelerateAndMove(entity, 0x80000);
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
        entity->drawSkip = 1;
        entity->timer = 0x10;
        entity->state = 3;
        break;
    case 3:
        EntityAccelerateAndMove(entity, 0x80000);
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
        entity->state = 5;
        break;
    case 4:
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
        entity->drawSkip = 1;
        entity->timer = 0x10;
        entity->state = 3;
        break;
    case 5:
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    if (entity->state <= 2 && (u8)entity->ext.redSoul.hit != 0)
    {
        entity->velX.whole = 0;
        entity->animFrame = entity->stage + 4;
        entity->state = 4;
        entity->timer = 0x10;
    }
    if (EntityIsOutsideScreen(entity, 2) || !entity->roomActive)
    {
        entity->roomActive = 1;
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
    }
}

/* RedSoulHistoryTrail_UpdateUnhide; clone of
 * RedSoulHistoryTrail_Update (this file). */

/**
 * @brief As RedSoulHistoryTrail_Update, but on the first update clear hidden
 * instead of installing a draw callback.
 */
void RedSoulHistoryTrail_UpdateUnhide(struct EwramData_EntityData *entity)
{
    struct Unk_0802A330 *trail = (struct Unk_0802A330 *)&entity->ext;
    struct EwramData *ew = gEwramData;
    struct EwramData_EntityData *owner = ew->playerState.activeRedSoulEntities[entity->stage];
    s32 i;

    if (NoRedSouls(ew) || gEwramData->playerState.activeRedSoulEntities[entity->stage] == NULL)
    {
        EntityDelete(entity);
        return;
    }
    switch (entity->state)
    {
    case 0:
        entity->hidden = 0;
        entity->state++;
    }
    for (i = 9; i > 0; i--)
    {
        trail[i].x = trail[i - 1].x;
        trail[i].y = trail[i - 1].y;
    }
    trail[0].x = GetEntityRoomXPositionInteger(owner);
    trail[0].y = GetEntityRoomYPositionInteger(owner);
}

/* Red soul trail entity. The s16 layer
 * temporary was found with a permuter. */

/**
 * @brief Create a trail entity with the given update and draw callbacks that
 * copies the owner sprite, palette, facing and red soul slot, filling
 * the 10-entry room position history with the owner position. Return
 * NULL when no entity or sprite is available.
 */
struct EwramData_EntityData *RedSoul_CreateHistoryTrail(struct EwramData_EntityData *owner, u32 *update, u32 *draw)
{
    s32 i;
    s32 graphics = Sprite_GetTileSlot(owner);
    u8 *index = &owner->spriteSlot;
    struct SpriteSheet *tmp = gEwramData->spriteSlots[*index].sheet;
    struct SpriteSheet *sprite = tmp;
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, update);
    struct Unk_0802A330 *trail;

    if (entity == NULL)
        return NULL;
    if (sprite == NULL)
    {
        EntityDelete(entity);
        return NULL;
    }
    Sprite_Attach(entity, sprite, graphics, owner->palette);
    {
        s16 layer = owner->spriteLayer + 1;
        entity->spriteLayer = layer;
    }
    entity->flipX = owner->flipX;
    entity->stage = owner->ext.redSoul.slot;
    trail = (struct Unk_0802A330 *)&entity->ext;
    for (i = 9; i != 0; i--)
    {
        trail[i].x = GetEntityRoomXPositionInteger(owner);
        trail[i].y = GetEntityRoomYPositionInteger(owner);
    }
    entity->drawFunc = draw;
    entity->scrollWithBg = 1;
    return entity;
}

extern void SkillHarpyTrailDraw(struct EwramData_EntityData *);

/**
 * @brief Child of SkillHarpyUse: projectile (graphics set 24, frame 1, box
 * 0x0202FFFF, SoulAttack_MarkHit) with a history trail; stop on a hit
 * or at a wall 6 px ahead, linger 16 updates hidden, end off screen.
 */
void SkillHarpyProjectileUpdate(struct EwramData_EntityData *entity)
{
    u16 x, y;

    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(24);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        entity->animFrame = 1;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 0xD;
        if (entity->flipX)
            entity->velX.whole = -entity->velX.whole;
        Hit_Enable(entity, 5, SoulAttack_MarkHit, 0);
        HitBox_SetAttackRaw(entity, 0x0202FFFF);
        entity->hit.flags |= 4;
        entity->state = 1;
        RedSoul_CreateHistoryTrail(entity, (u32 *)RedSoulHistoryTrail_UpdateUnhide, (u32 *)SkillHarpyTrailDraw);
    case 1:
        EntityAccelerateAndMove(entity, 0x80000);
        if ((u8)entity->ext.redSoul.hit != 0)
        {
            entity->state = 4;
            break;
        }
        x = GetEntityRoomXPositionInteger(entity);
        y = GetEntityRoomYPositionInteger(entity);
        if (entity->flipX)
        {
            if (Collision_GetPushRight((s16)x - 6, (s16)y))
                entity->state = 2;
        }
        else if (Collision_GetPushLeft((s16)x + 6, (s16)y))
            entity->state = 2;
        break;
    case 2:
        entity->timer = 0x10;
        entity->drawSkip = 1;
        entity->state = 3;
    case 3:
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
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
    }
    entity->roomActive = 1;
}

void SkillHarpyUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.redSoul.owner;
    s32 i;
    switch (entity->state)
    {
    case 0:
        Hit_SetTimerOverride(1, 20);
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        entity->state = 1;
        Entity_PlaceRelativeTo(entity, parent, 20, -26);
        PlaySong(0xE6);
    case 1:
        for (i = 0; i <= 1; i++)
        {
            struct EwramData_EntityData *child = RedSoul_CreateEntity(entity, (u32 *)SkillHarpyProjectileUpdate);
            if (child == NULL) break;
            child->posX.whole = entity->posX.whole;
            child->posY.whole = entity->posY.whole;
            child->velX.whole = 0x50000;
            child->flipX = i;
        }
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

/* SkillSlimeTrailUpdate; clone of
 * RedSoulHistoryTrail_UpdateUnhide / RedSoulHistoryTrail_Update (this file). */

/**
 * @brief History trail of the SkillSlimeUse projectile: as
 * RedSoulHistoryTrail_UpdateUnhide, but set drawSkip once the soul
 * subState becomes nonzero.
 */
void SkillSlimeTrailUpdate(struct EwramData_EntityData *entity)
{
    struct Unk_0802A330 *trail = (struct Unk_0802A330 *)&entity->ext;
    struct EwramData *ew = gEwramData;
    struct EwramData_EntityData *owner = ew->playerState.activeRedSoulEntities[entity->stage];
    s32 i;

    if (NoRedSouls(ew) || gEwramData->playerState.activeRedSoulEntities[entity->stage] == NULL)
    {
        EntityDelete(entity);
        return;
    }
    switch (entity->state)
    {
    case 0:
        entity->hidden = 0;
        entity->state++;
        break;
    case 1:
        if (owner->subState != 0)
        {
            entity->drawSkip = 1;
            entity->state++;
        }
        break;
    }
    for (i = 9; i > 0; i--)
    {
        trail[i].x = trail[i - 1].x;
        trail[i].y = trail[i - 1].y;
    }
    trail[0].x = GetEntityRoomXPositionInteger(owner);
    trail[0].y = GetEntityRoomYPositionInteger(owner);
}

/**
 * @brief Move the SkillSlimeUse projectile by its velocity and resolve floor,
 * ceiling and wall contacts using its half sizes; reflect the angle
 * ENTITY_HALF_510 and add the floor slope on floor or ceiling hits.
 */
void SkillSlimeMoveAndBounce(struct EwramData_EntityData *entity)
{
    u8 wall = 0;
    u32 slope = 0;
    u8 tile;
    u16 x, y;
    u16 c;
    s16 r;

    entity->posX.whole += entity->velX.whole;
    entity->posY.whole += entity->velY.whole;
    x = GetEntityRoomXPositionInteger(entity);
    y = GetEntityRoomYPositionInteger(entity);
    if (entity->velY.whole >= 0)
    {
        r = Collision_GetPushUp((s16)x, (s16)y + entity->accelY.part16.integer);
        c = r;
        if (r != 0)
        {
            tile = gUnk_03002CB0.collisionProbeFlags;
            entity->posY.part16.integer += ({ s32 t_ = (s16)c; t_; });
            wall = 2;
        }
    }
    else
    {
        r = Collision_GetPushDown((s16)x, (s16)y - entity->accelY.part16.integer);
        c = r;
        if (r != 0)
        {
            tile = gUnk_03002CB0.collisionProbeFlags;
            entity->posY.part16.integer += ({ s32 t_ = (s16)c; t_; });
            wall = 2;
        }
    }
    y = GetEntityRoomYPositionInteger(entity);
    if (entity->velX.whole < 0)
    {
        r = Collision_GetPushRight((s16)x - entity->accelX.part16.integer, (s16)y);
        c = r;
        if (r != 0)
        {
            entity->posX.part16.integer += ({ s32 t_ = (s16)c; t_; });
            if (wall == 0)
                wall = 1;
        }
    }
    else if (entity->velX.whole > 0)
    {
        r = Collision_GetPushLeft((s16)x + entity->accelX.part16.integer, (s16)y);
        c = r;
        if (r != 0)
        {
            entity->posX.part16.integer += ({ s32 t_ = (s16)c; t_; });
            if (wall == 0)
                wall = 1;
        }
    }
    if (wall == 0)
        return;
    if (wall == 2 && (tile & 0xC0))
    {
        switch ((tile & 0xC0) >> 6)
        {
        case 1:
            slope = 0x4000;
            break;
        case 2:
            slope = 0x5556;
            break;
        case 3:
            slope = 0x63D8;
            break;
        }
        if (tile & 4)
            slope = (u32)(-(slope << 16)) >> 16;
    }
    if ((s16)slope != 0)
        PlaySong(0x16A);
    else
        PlaySong(0x16B);
    if (wall == 1)
        SKILL_SLIME_ANGLE(entity) = -0x8000 - SKILL_SLIME_ANGLE(entity);
    else
        SKILL_SLIME_ANGLE(entity) = -SKILL_SLIME_ANGLE(entity);
    SKILL_SLIME_ANGLE(entity) += ({ s32 t_ = (s16)slope; t_; });
}

extern void SkillSlimeTrailDraw(struct EwramData_EntityData *);

static inline s32 Range(s32 r, s32 mask, s32 base)
{
    return (r & mask) + base;
}
static inline s32 RandomRange(s32 mask, s32 base)
{
    return (RandomNumberGenerator() & mask) + base;
}

/**
 * @brief 2CFC4 | Slime red soul: a bouncing blob (graphics set 33, animation
 * 0, layer 0xD) from the caster (0xC / -0x1A) with a SkillSlimeTrailUpdate trail,
 * box 0x0808FCFC in slot 5 (disabled), half size 4 x 4 (accelX.part16.integer / accelY.part16.integer),
 * Hit_SetTimerOverride(1, 15). Moves at speed hp = 3 px along angle ENTITY_HALF_510
 * (0x1000 + random, mirrored) bouncing with SkillSlimeMoveAndBounce for 256 frames,
 * switching to animation 1 when 32 frames remain.
 */
void SkillSlimeUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.redSoul.owner;
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
        entity->ext.skillSlime.timer = 0x100;
        entity->accelX.part16.integer = 4;
        entity->accelY.part16.integer = 4;
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        RedSoul_CreateHistoryTrail(entity, (u32 *)SkillSlimeTrailUpdate, (u32 *)SkillSlimeTrailDraw);
        entity->state = 1;
    case 1:
        entity->velX.whole = FixedMul(Sin(SKILL_SLIME_ANGLE_WORD(entity) + 0x4000), SKILL_SLIME_SPEED(entity));
        entity->velY.whole = FixedMul(Sin(SKILL_SLIME_ANGLE_WORD(entity)), SKILL_SLIME_SPEED(entity));
        SkillSlimeMoveAndBounce(entity);
        entity->ext.skillSlime.timer--;
        if (entity->subState == 0 && entity->ext.skillSlime.timer == 0x20)
        {
            Anim_StartIndexed(entity, 1, 0, 0);
            entity->subState = 1;
        }
        if (entity->ext.skillSlime.timer != 0)
            break;
        entity->subState = 2;
    case 2:
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    if (EntityIsOutsideScreen(entity, 6) || !entity->roomActive)
    {
        entity->roomActive = 1;
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    Anim_Update(entity);
}

/* Red soul info and small geometry helpers 0x0802D1D8..0x0802D407.
 *
 * This file does not include player_core.h: that header declares
 * RedSoul_FindFreeEntitySlot as returning u8, but the original returns a sign-extended
 * s8 (-1 when no slot is free). Changing the header would change callers. */

/* Same layout as struct RedSoulActivationView in player_core.h, plus activeEntityLimit. */
struct RedSoulInfo {
    void *updateFunc; // TODO: function signature
    u16 castAnimKind;
    s16 mpCost;
    u8 activeEntityLimit;
    u8 pad_9[0xE - 0x9];
    u16 param;
}; /* size = 0x10 */

extern const struct RedSoulInfo sSomaRedSoulInfo[]; // Soma (currentCharacter == 0)
extern const struct RedSoulInfo sJuliusSubweaponInfo[]; // Julius mode (defined in player/player_level.c)

#define INT_TOWARD_ZERO(v) ((v) < 0 ? -((-(v)) >> 16) : (v) >> 16)

/**
 * @brief Return the cast animation for pose 0 (ground) or 1 (air) from the
 * sRedSoulCastAnims pair selected by the equipped red soul info castAnimKind.
 */
u8 RedSoul_GetCastAnim(s32 pose)
{
    const struct RedSoulInfo *info = RedSoul_GetInfo(gEwramData->playerStatus.equippedRedSoul);

    return sRedSoulCastAnims[info->castAnimKind][pose];
}

/**
 * @brief Return the first NULL active-red-soul slot among eight, or signed -1; preserve existing caller declaration differences.
 */
s8 RedSoul_FindFreeEntitySlot(void)
{
    s8 i;

    for (i = 0; i < 8; i++)
    {
        if (gEwramData->playerState.activeRedSoulEntities[i] == NULL)
            return i;
    }
    return -1;
}

/**
 * @brief Return the last-used red-soul record from the current-character table without adding index validation.
 */
const struct RedSoulInfo *RedSoul_GetLastUsedInfo(void)
{
    if (gEwramData->playerStatus.currentCharacter == 0)
        return &sSomaRedSoulInfo[gEwramData->playerState.lastUsedRedSoul];
    return &sJuliusSubweaponInfo[gEwramData->playerState.lastUsedRedSoul];
}

/**
 * @brief Return the indexed red-soul record from the current-character table, retaining the byte argument and unchecked index.
 */
const struct RedSoulInfo *RedSoul_GetInfo(u8 soul)
{
    if (gEwramData->playerStatus.currentCharacter == 0)
        return &sSomaRedSoulInfo[soul];
    return &sJuliusSubweaponInfo[soul];
}

/**
 * @brief Return the active-entity limit (unk_8) of lastUsedRedSoul from the
 * current character's table (Soma sSomaRedSoulInfo, Julius sJuliusSubweaponInfo).
 */
u8 RedSoul_GetActiveLimit(void)
{
    const struct RedSoulInfo *info;

    if (gEwramData->playerStatus.currentCharacter == 0)
        info = &sSomaRedSoulInfo[gEwramData->playerState.lastUsedRedSoul];
    else
        info = &sJuliusSubweaponInfo[gEwramData->playerState.lastUsedRedSoul];
    return info->activeEntityLimit;
}

/**
 * @brief 2D300 | EWRAM byte 0x1307D.
 */
u8 Hit_GetTimerOverride1(void)
{
    return gEwramData->hitTimerOverride[1];
}

/**
 * @brief Return ArcTan2 of target minus source integer positions; preserve the existing integer-field conversion behavior and separate matching copy.
 */
u16 Entity_AngleToEntity(struct EwramData_EntityData *entity, struct EwramData_EntityData *target)
{
    return ArcTan2(target->posX.part16.integer - entity->posX.part16.integer,
                   target->posY.part16.integer - entity->posY.part16.integer);
}

/**
 * @brief Return the angle to a 16.16 point, truncating negative fixed-point coordinates toward zero and preserving source integer-field conversions.
 */
u16 Entity_AngleToPoint(struct EwramData_EntityData *entity, s32 x, s32 y)
{
    return ArcTan2(INT_TOWARD_ZERO(x) - (s16)entity->posX.part16.integer,
                   INT_TOWARD_ZERO(y) - entity->posY.part16.integer);
}

/**
 * @brief Return squared integer-pixel distance to the target, retaining signed X conversion and existing Y arithmetic.
 */
s32 Entity_DistanceSquaredToEntity(struct EwramData_EntityData *entity, struct EwramData_EntityData *target)
{
    s32 dx = (s16)target->posX.part16.integer - (s16)entity->posX.part16.integer;
    s32 dy = target->posY.part16.integer - entity->posY.part16.integer;

    return dx * dx + dy * dy;
}

/**
 * @brief Return squared distance to a 16.16 point after truncation toward zero, preserving the original coordinate conversions.
 */
s32 Entity_DistanceSquaredToPoint(struct EwramData_EntityData *entity, s32 x, s32 y)
{
    s32 dx = INT_TOWARD_ZERO(x) - (s16)entity->posX.part16.integer;
    s32 dy = INT_TOWARD_ZERO(y) - entity->posY.part16.integer;

    return dx * dx + dy * dy;
}

/* Step toward a fixed-point target angle and wrap at 0x10000.
 * The step is not clamped to the remaining angle; equal angles retain the
 * original subtract branch. Target pixel conversion truncates toward zero. */
#undef INT_TOWARD_ZERO
#define INT_TOWARD_ZERO(x) ((x) < 0 ? -(-(x) >> 16) : (x) >> 16)
/**
 * @brief Add or subtract the supplied turn toward the target angle and wrap to 16 bits; preserve equal-angle/tie branches and possible overshoot without snapping.
 */
u32 Entity_StepAngleTowardPoint(struct EwramData_EntityData *entity, s32 x, s32 y, s32 angle, s32 turn)
{
    s32 dx = (s16)(INT_TOWARD_ZERO(x) - (s16)entity->posX.part16.integer);
    s32 dy = (s16)(INT_TOWARD_ZERO(y) - entity->posY.part16.integer);
    s32 delta = (u16)ArcTan2(dx, dy) - angle;
    if (delta > 0)
    {
        if (delta <= 0x7FFF) goto add;
        goto subtract;
    }
    else if (delta >= -0x8000) goto subtract;
add:
    angle += turn;
    goto finish;
subtract:
    angle -= turn;
finish:
    return angle & 0xFFFF;
}

/* Use the selected target, or the nearest of the four registered candidates. */
/**
 * @brief Return the selected target when non-NULL, otherwise the nearest of four non-NULL candidates; retain first-entry tie behavior and NULL when empty.
 */
struct EwramData_EntityData *Player_GetSelectedOrNearestTarget(struct EwramData_EntityData *entity)
{
    u32 bestDistance = -1;
    struct EwramData_EntityData *best = NULL;
    struct EwramData_EntityData *selected = gEwramData->playerState.selectedTarget;
    if (selected == NULL)
    {
        s32 i;
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
    return selected;
}

/* Soul helpers 0x0802D510..0x0802D603. */

/**
 * @brief Return the selected target or first non-NULL candidate in cyclic order from a random slot; return NULL when empty, without promising uniform selection.
 */
struct EwramData_EntityData *Player_GetSelectedOrRandomTarget(void)
{
    struct EwramData_EntityData *target = gEwramData->playerState.selectedTarget;
    s32 i;
    u32 slot;

    if (target != NULL)
        return target;
    i = 0;
    slot = RandomNumberGenerator() & 3;
    for (; i <= 3; i++)
    {
        target = gEwramData->playerState.trackedEntities[slot];
        if (target != NULL)
            return target;
        slot = (slot + 1) & 3;
    }
    return NULL;
}

/**
 * @brief Return 1 (and set flag 0x53D bit 2) when the entity is outside the
 * screen by the given margin or its 0x53D bit 2 is clear; else 0.
 */
s32 Entity_IsOffscreenOrInactive(struct EwramData_EntityData *entity, s32 margin)
{
    if (EntityIsOutsideScreen(entity, margin) || !entity->roomActive)
    {
        entity->roomActive = 1;
        return 1;
    }
    return 0;
}

/**
 * @brief 2D590 | Take the hit box of the current animation frame (spriteSlot /
 * animFrame): activate it with HitBox_SetAttack (flags bit 1 cleared), or set
 * flags bit 1 when the frame has none.
 */
void Entity_SetAttackBoxFromFrame(struct EwramData_EntityData *entity)
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

/**
 * @brief 2D5EC | Random value in [-range / 2, range / 2) (range a
 * power of two).
 */
s32 RandomSigned(u32 range)
{
    return (RandomNumberGenerator() & (range - 1)) - (range >> 1);
}

/* Graphics setup helpers, 0x0802D604..0x0802D723. */
/* Keep the graphics context access explicit, matching the original loaders. */
#define GRAPHICS_CONTEXT ((struct PlayerState *)((u8 *)gEwramData + OFFSET_OF(struct EwramData, playerState)))

struct GraphicsSetWithPalette {
    struct SoulGraphicsResources set;
    u8 pad_C[2];
    u8 paletteIndex;
    u8 pad_F;
};

/**
 * @brief Unreferenced: load red-soul graphics (into redSoulGfx), palette row and
 * attach the sprite with palette 2.
 */
void Skill_LoadGraphics(struct EwramData_EntityData *entity, u8 *resource, void *palette, void *sheet, u8 first)
{
    gEwramData->playerState.redSoulGfx = ObjGfx_LoadResource(resource, 0x10000);
    ObjPalette_LoadRange(palette, first, 1, 2);
    Sprite_Attach(entity, sheet, gEwramData->playerState.redSoulGfx, 2);
}

/**
 * @brief Unreferenced: as Skill_LoadGraphics from a graphics set with palette
 * index.
 */
void Skill_LoadGraphicsFromSet(struct EwramData_EntityData *entity, const struct GraphicsSetWithPalette *def)
{
    u8 paletteIndex = def->paletteIndex;
    u8 *graphics = def->set.graphics;
    void *palette = def->set.palette;
    void *sprites = def->set.spriteSheet;
    GRAPHICS_CONTEXT->redSoulGfx = ObjGfx_LoadResource(graphics, 0x10000);
    ObjPalette_LoadRange(palette, paletteIndex, 1, 2);
    Sprite_Attach(entity, sprites, GRAPHICS_CONTEXT->redSoulGfx, 2);
}

/**
 * @brief Unreferenced: as Skill_LoadGraphics from entry n of sSkillGraphicsSets with
 * the given palette row.
 */
void Skill_LoadGraphicsByIndex(struct EwramData_EntityData *entity, s32 setIndex, s32 paletteSlot)
{
    const struct SoulGraphicsResources *set = &sSkillGraphicsSets[setIndex];
    u8 paletteIndex = paletteSlot;
    u8 *graphics = set->graphics;
    void *palette = set->palette;
    void *sprites = set->spriteSheet;
    GRAPHICS_CONTEXT->redSoulGfx = ObjGfx_LoadResource(graphics, 0x10000);
    ObjPalette_LoadRange(palette, paletteIndex, 1, 2);
    Sprite_Attach(entity, sprites, GRAPHICS_CONTEXT->redSoulGfx, 2);
}

/* 2D724. */
/* Remove a red soul entity from the active list (ext.redSoul.slot). */
void RedSoul_RemoveActiveEntity(struct EwramData_EntityData *soul)
{
    struct EwramData *ewram = gEwramData;

    ewram->playerState.activeRedSoulEntities[soul->ext.redSoul.slot] = NULL;
    ewram->playerState.nbrActiveRedSoulEntities--;
}

static inline s32 NoRedSoulActive(void)
{
    struct EwramData *ewram = gEwramData;
    u8 count = ewram->playerState.nbrActiveRedSoulEntities;
    s32 none = 0;
    if (count == 0 && ewram->playerState.activeRedSoulEntities[0] == NULL)
        none = 1;
    return none;
}

/* Animate a red-soul effect with velocity and acceleration; discard it
 * when its soul is inactive or its animation finishes. */
/**
 * @brief Move with velocity and acceleration and delete at the end of the
 * animation or when no red soul is active.
 */
void RedSoulEffect_UpdateAccelerated(struct EwramData_EntityData *entity)
{
    if (NoRedSoulActive())
    {
        EntityDelete(entity);
        return;
    }
    if (entity->state == 0)
    {
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->state = 1;
    }
    entity->posX.whole += entity->velX.whole;
    entity->velX.whole += entity->accelX.whole;
    entity->posY.whole += entity->velY.whole;
    entity->velY.whole += entity->accelY.whole;
    if (Anim_Update(entity) == 3)
        EntityDelete(entity);
}

/* Create an effect at (x, y) from graphics set 080E13BC[index] (palette
 * slot 0, palette 2) with animation animation, priority 13; updated by
 * RedSoulEffect_UpdateAccelerated. GRAPHICS_CONTEXT as in this file keeps the original
 * reloads of the 0x131CC offset. */

/**
 * @brief Unreferenced: spawn a RedSoulEffect_UpdateAccelerated entity at (x, y)
 * with the graphics set n of sSkillGraphicsSets and the given animation.
 */
struct EwramData_EntityData *Skill_SpawnAcceleratedEffect(s32 x, s32 y, s32 index, s32 animation)
{
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, (u32 *)RedSoulEffect_UpdateAccelerated);
    const struct SoulGraphicsResources *set;
    u8 *graphics;
    void *palette, *sprites;

    if (entity == NULL)
        return NULL;
    set = &sSkillGraphicsSets[index];
    graphics = set->graphics;
    palette = set->palette;
    sprites = set->spriteSheet;
    GRAPHICS_CONTEXT->redSoulGfx = ObjGfx_LoadResource(graphics, 0x10000);
    ObjPalette_LoadRange(palette, 0, 1, 2);
    Sprite_Attach(entity, sprites, GRAPHICS_CONTEXT->redSoulGfx, 2);
    Anim_StartIndexed(entity, animation, 0, 0);
    entity->spriteLayer = 0xD;
    entity->posX.whole = x;
    entity->posY.whole = y;
    entity->scrollWithBg = 1;
    return entity;
}

/* Move a red-soul effect, deleting it when its soul is inactive or
 * when it leaves the extended screen bounds. */
/**
 * @brief Move a thrown bone fragment with acceleration; delete it outside the
 * screen area or when no red soul is active.
 */
void SoulBoneFragment_Update(struct EwramData_EntityData *entity)
{
    if (NoRedSoulActive())
    {
        EntityDelete(entity);
        return;
    }
    entity->posX.whole += entity->velX.whole;
    entity->velX.whole += entity->accelX.whole;
    entity->posY.whole += entity->velY.whole;
    entity->velY.whole += entity->accelY.whole;
    if (!Entity_IsInRect(entity, -16, 0, 0x110, 0xB0))
        EntityDelete(entity);
}

/* Create an effect at (x, y) from graphics set 080E13BC[index] with the
 * given frame, priority 13; updated by SoulBoneFragment_Update. */

/**
 * @brief Unreferenced: spawn a SoulBoneFragment_Update entity at (x, y) with
 * the graphics set n of sSkillGraphicsSets and the given frame.
 */
struct EwramData_EntityData *Skill_SpawnBoneFragment(s32 x, s32 y, s32 index, s32 frame)
{
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, (u32 *)SoulBoneFragment_Update);
    const struct SoulGraphicsResources *set;
    u8 *graphics;
    void *palette, *sprites;

    if (entity == NULL)
        return NULL;
    set = &sSkillGraphicsSets[index];
    graphics = set->graphics;
    palette = set->palette;
    sprites = set->spriteSheet;
    GRAPHICS_CONTEXT->redSoulGfx = ObjGfx_LoadResource(graphics, 0x10000);
    ObjPalette_LoadRange(palette, 0, 1, 2);
    Sprite_Attach(entity, sprites, GRAPHICS_CONTEXT->redSoulGfx, 2);
    entity->animFrame = frame;
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    entity->spriteLayer = 0xD;
    entity->posX.whole = x;
    entity->posY.whole = y;
    entity->scrollWithBg = 1;
    return entity;
}

/* Soul helpers 0x0802D9B4..0x0802DAD3. */
extern void RedSoulAnchor_Update(struct EwramData_EntityData *entity);

/**
 * @brief Hit callback of soul attack projectiles (Hit_Enable slot 5): set the
 * hit marker unk_4FC to 1.
 */
void SoulAttack_MarkHit(struct EwramData_EntityData *entity)
{
    entity->ext.redSoul.hit = 1;
}

/**
 * @brief Hit callback: unless already anchored, create a RedSoulAnchor_Update
 * entity linking the projectile to the target, set a 0x78-update timer
 * and attack box 0x1014F8F6; otherwise clear unk_50A.
 */
void RedSoul_AnchorToHitTargetTimed(struct EwramData_EntityData *entity, struct EwramData_EntityData *target)
{
    struct EwramData_EntityData *child;

    if (entity->ext.redSoul.anchor == NULL)
    {
        child = EntityCreateInRange(0x49, 0xCF, (u32 *)RedSoulAnchor_Update);
        if (child == NULL)
            return;
        entity->ext.redSoul.anchor = child;
        ENTITY_OWNER(child) = entity;
        child->ext.redSoulAnchor.target = target;
        ENTITY_INDEX(child) = ENTITY_INDEX(entity);
        entity->timer = 0x78;
        HitBox_SetAttackRaw(entity, 0x1014F8F6);
        child->accelX.whole = entity->posX.whole - target->posX.whole;
        child->accelY.whole = entity->posY.whole - target->posY.whole;
    }
    else
    {
        entity->ext.redSoul.anchorTimer = 0;
    }
}

/**
 * @brief Contact callback of the homing soul projectile: switch it to its
 * release state 3.
 */
void HomingSoulProjectile_OnPlayerContact(struct EwramData_EntityData *entity)
{
    entity->state = 3;
}

/**
 * @brief Draw callback (SkillChronomageUse): outside mode 6 and with unk_A074
 * bit 0 copy ext.skillChronomage.invert to paletteInvert, then draw.
 */
void Soul_DrawWithPaletteInvert(struct EwramData_EntityData *entity)
{
    struct EwramData *ewram = gEwramData;

    if (ewram->gameState.subMode != 6 && ewram->paused)
        ewram->paletteInvert = entity->ext.skillChronomage.invert;
    Sprite_Draw(entity);
}

/**
 * @brief 2DA5C | A random non-empty entry of trackedEntities[4] (NULL if all empty).
 */
struct EwramData_EntityData *Player_GetRandomTrackedEntity(void)
{
    s32 i = 0;
    u32 slot = RandomNumberGenerator() % 4;

    for (; i <= 3; i++)
    {
        if (gEwramData->playerState.trackedEntities[slot] != NULL)
            return gEwramData->playerState.trackedEntities[slot];
        slot = (slot + 1) % 4;
    }
    return NULL;
}

/**
 * @brief Move with acceleration (velocity limit 0x80000) and end outside
 * the screen.
 */
void Effect_UpdateBallisticDebris(struct EwramData_EntityData *entity)
{
    EntityAccelerateAndMoveX(entity, 0x80000);
    EntityAccelerateAndMoveY(entity, 0x80000);
    if (EntityIsOutsideScreen(entity, 4))
        entity->deleteRequested = 1;
}

/**
 * @brief Contact callback of the bone soul projectile: set its hit flag
 * (ext.skillBone.hit).
 */
void BoneSoul_OnPlayerContact(struct EwramData_EntityData *entity)
{
    entity->ext.skillBone.hit = 1;
}

/* Projectile 0x081C15F4 / 0x0820ED60 (palette 4, frame 0x35, priority 13):
 * set up on the first frame, then fall and flag itself (deleteRequested) for
 * removal once it leaves the screen. */

/**
 * @brief Droplet spawned by SkillMermanProjectileUpdate: move with
 * acceleration and delete off-screen.
 */
void SkillMermanDropletUpdate(struct EwramData_EntityData *entity)
{
    if (entity->state == 0)
    {
        s32 graphics = ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(entity, sCommonObjectSpriteSheet, graphics, 4);
        entity->animFrame = 0x35;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 0xD;
        entity->state++;
    }
    EntityAccelerateAndMove(entity, 0x80000);
    if (EntityIsOutsideScreen(entity, 1))
        entity->deleteRequested = 1;
}

/* Entity_DrawWithVelocityCopies (motion trail draw). */
/**
 * @brief Draw the entity plus two copies 0.5 and 1.5 velocity steps ahead,
 * then restore its position.
 */
void Entity_DrawWithVelocityCopies(struct EwramData_EntityData *entity)
{
    s32 oldX = entity->posX.whole;
    s32 oldY = entity->posY.whole;
    s32 i;

    entity->posX.whole = oldX + entity->velX.whole / 2;
    entity->posY.whole = oldY + entity->velY.whole / 2;
    for (i = 2; i != 0; i--)
    {
        Sprite_Draw(entity);
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
    }
    entity->posX.whole = oldX;
    entity->posY.whole = oldY;
}

/* Soul effect helpers 0x0802DB84..0x0802DCFF. */
/**
 * @brief Hit callback: unless the projectile already has an anchor (unk_504),
 * create a RedSoulAnchor_Update entity linking it to the hit target with
 * their current offset, and set hit flag 2.
 */
void RedSoul_AnchorToHitTarget(struct EwramData_EntityData *entity, struct EwramData_EntityData *target)
{
    if (entity->ext.redSoul.anchor == NULL)
    {
        struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)RedSoulAnchor_Update);

        if (child != NULL)
        {
            entity->ext.redSoul.anchor = child;
            ENTITY_OWNER(child) = entity;
            child->ext.redSoulAnchor.target = target;
            ENTITY_INDEX(child) = ENTITY_INDEX(entity);
            entity->hit.flags |= 2;
            child->accelX.whole = entity->posX.whole - target->posX.whole;
            child->accelY.whole = entity->posY.whole - target->posY.whole;
        }
    }
}

/**
 * @brief Draw callback of SkillCockatriceUse: draw 8 copies stepping by
 * (accelX, accelY) from 8 px below minus unk_508 * 8, then restore.
 */
void SkillCockatriceDraw(struct EwramData_EntityData *entity)
{
    s32 oldX = entity->posX.whole;
    s32 oldY = entity->posY.whole;
    s32 i;

    {
        s32 baseY = oldY + 0x80000;

        entity->posY.whole = baseY - entity->ext.skillCockatrice.scaleY * 8;
    }
    for (i = 8; i != 0; i--)
    {
        Sprite_Draw(entity);
        entity->posX.whole += entity->accelX.whole;
        entity->posY.whole += entity->accelY.whole;
    }
    entity->posX.whole = oldX;
    entity->posY.whole = oldY;
}

/**
 * @brief Draw frames 0x10..0x14 at 5 points of the 10-entry room position
 * history (every second entry from unk_4F4).
 */
void RedSoulHistoryTrail_Draw(struct EwramData_EntityData *entity)
{
    s32 i = 0;
    u16 *history = (u16 *)&entity->ext;

    for (; i <= 4; i++)
    {
        struct BgLayerState *bg = &gEwramData->bgInfo[1];

        entity->posX.part16.integer = history[0] - bg->xPos.part16.integer;
        entity->posY.part16.integer = history[1] - bg->yPos.part16.integer;
        entity->animFrame = i + 0x10;
        Sprite_Draw(entity);
        history += 4;
    }
}

/**
 * @brief Draw frames 3..5 at 3 history points stored from unk_4F8.
 */
void SkillHarpyTrailDraw(struct EwramData_EntityData *entity)
{
    s32 i = 0;
    u16 *history = (u16 *)&entity->ext;

    for (; i <= 2; i++)
    {
        struct BgLayerState *bg = &gEwramData->bgInfo[1];

        entity->posX.part16.integer = history[2] - bg->xPos.part16.integer;
        entity->posY.part16.integer = history[3] - bg->yPos.part16.integer;
        entity->animFrame = i + 3;
        Sprite_Draw(entity);
        history += 2;
    }
}

/**
 * @brief Draw frames 2, 2, 3, 3 .. 5 at 8 history points stored from unk_4F8.
 */
void SkillSlimeTrailDraw(struct EwramData_EntityData *entity)
{
    s32 i = 0;
    u16 *history = (u16 *)&entity->ext;

    for (; i <= 7; i++)
    {
        struct BgLayerState *bg = &gEwramData->bgInfo[1];

        entity->posX.part16.integer = history[2] - bg->xPos.part16.integer;
        entity->posY.part16.integer = history[3] - bg->yPos.part16.integer;
        entity->animFrame = (i >> 1) + 2;
        Sprite_Draw(entity);
        history += 2;
    }
}
