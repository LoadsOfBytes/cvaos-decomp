/* SkillArachneUse: matched (bone loop as in the skeleton skill:
   SpawnBone + scatter; one variable holds the linked entity pointer and its flag). */
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
extern void RedSoul_AnchorToHitTarget(struct EwramData_EntityData *, struct EwramData_EntityData *);
extern void SoulBoneFragment_Update(struct EwramData_EntityData *);

struct SkillShotInfo {
    s32 velX;
    s32 velY;
    s32 accelY;
    u8 set;
    u8 pad_D;
    u8 animation;
    u8 loop;
    u16 song;
    u16 flags;
    u8 pad_14[4];
};
extern struct SkillShotInfo sUnk_084F12C8[];

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

#define DEL(e) do { gEwramData->playerState.activeRedSoulEntities[(e)->unk_4FC.part16.unk_4FE[0]] = NULL; gEwramData->playerState.nbrActiveRedSoulEntities--; EntityDelete(e); } while (0)
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

void SkillArachneUse(struct EwramData_EntityData *entity)
{
    struct SkillShotInfo *info = &sUnk_084F12C8[(u16)entity->unk_500.part16.unk_502];
    struct EwramData_EntityData *parent = entity->unk_4F8.unk_4F8_entity;
    u32 v;
    s32 i;

    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(info->set);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        Anim_StartIndexed(entity, info->animation, 0, info->loop);
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
        if (entity->unk_504.unk_504_entity == NULL)
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
        v = (u32)entity->unk_504.unk_504_entity;
        if (((struct EwramData_EntityData *)v)->unk_508.part8.unk_508 != 0)
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
        v = (u32)entity->unk_504.unk_504_entity;
        v = ((struct EwramData_EntityData *)v)->unk_508.part8.unk_508;
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
