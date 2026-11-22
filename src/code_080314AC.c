/* SkillAlastorUse: Alastor familiar attack. Only matches without -f2003-patch
 * (NO_2003_PATCH_SRCS in ../Makefile). (flipX test mask kept in a variable across the
 * position copies, explicit u32 negation of dx). */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

extern s32 BlueSoul_CheckEndCondition(struct EwramData_EntityData *);
extern void BlueSoul_ReleaseActiveResources(struct EwramData_EntityData *);
extern struct EwramData_EntityData *Player_GetSelectedOrNearestTarget(struct EwramData_EntityData *);
extern s32 Target_GetX(struct EwramData_EntityData *);
extern s32 Target_GetY(struct EwramData_EntityData *);
extern s32 Entity_StepTowardPoint(struct EwramData_EntityData *, s32, s32, s32);
extern s32 Entity_DistanceToPoint(struct EwramData_EntityData *, s32, s32);
extern void EntityDampVelocityX(struct EwramData_EntityData *, s32);

static inline void LoadGfx(struct EwramData_EntityData *entity, u8 paletteSlot, void *gfxSrc, void *palette, void *sheet)
{
    gEwramData->playerState.blueSoulGfx = ObjGfx_LoadResource(gfxSrc, 0x10000);
    ObjPalette_LoadRange(palette, paletteSlot, 1, 3);
    Sprite_Attach(entity, sheet, gEwramData->playerState.blueSoulGfx, 3);
}

#define FIXED_TO_INT(v) ((v) < 0 ? -(-(v) >> 16) : (v) >> 16)

void SkillAlastorUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->unk_4F8.unk_4F8_entity;
    s32 x = Target_GetX(entity);
    s32 y = Target_GetY(entity) - 0x280000;
    s32 dist;
    u16 slot;

    entity->roomActive = 1;
    {
    u8 six = 6;
    if (entity->state < six && BlueSoul_CheckEndCondition(entity))
        entity->state = six;
    }
    switch (entity->state)
    {
    case 0:
        slot = ObjAffine_Alloc(0);
        if ((s16)slot < 0)
        {
            if (entity->affineIndex || entity->affine)
                ObjAffine_Free(entity->affineIndex);
            gEwramData->playerState.activeBlueSoulEntities[entity->unk_508.part16.unk_508] = NULL;
            EntityDelete(entity);
            return;
        }
        entity->affineIndex = slot;
        entity->affine = 1;
    {
        u8 *graphics = (u8 *)0x081E3980;
        u8 *palette = (u8 *)0x0820B700;
        u8 *sprite = (u8 *)0x08237AFC;
        LoadGfx(entity, 0, graphics, palette, sprite);
    }
        entity->spriteLayer = 0xF;
        entity->animFrame = 0;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->scrollWithBg = 1;
        {
            u32 m;
            entity->flipX = parent->flipX;
            m = 0x40;
            entity->posX.part16.integer = parent->posX.part16.integer;
            entity->posY.part16.integer = parent->posY.part16.integer - 0x10;
            entity->unk_510.part16.unk_512 = 0x100;
            m &= ((u8 *)entity)[0x58];
        if (m)
            entity->unk_510.part16.unk_512 = -entity->unk_510.part16.unk_512;
        }
        entity->flipX ^= 1;
        Hit_Enable(entity, 6, 0, 0);
        entity->hit.flags |= 4;
        entity->hit.flags |= 2;
        Hit_SetTimerOverride(2, 0x3C);
        {
            u16 a = 0x19;
            u16 b = 1;
            gEwramData->playerState.skillBaseAttack = a;
            gEwramData->playerState.skillAttackAttributes = b;
        }
        entity->state = 1;
        PlaySong(0x19E);
    case 1:
        entity->unk_510.part16.unk_510 += entity->unk_510.part16.unk_512;
        if (entity->unk_50C.whole > 0xFFFF)
        {
            entity->state = 2;
            break;
        }
        entity->unk_50C.whole += 0x1000;
        break;
    case 2:
    {
        s32 a;
        entity->hit.flags |= 2;
        if ((RandomNumberGenerator() & 0x7F) == 0)
        {
            entity->state = 4;
            break;
        }
        dist = Entity_StepTowardPoint(entity, x, y, 5);
        entity->timer++;
        entity->unk_510.part16.unk_510 += Sin(entity->timer << 9) / 512;
        if (entity->velX.whole >= 0)
        {
            if (entity->unk_510.part16.unk_510 <= 0x1FFF)
                entity->unk_510.part16.unk_510 += (entity->velX.whole >> 8);
        }
        else
        {
            if (entity->unk_510.part16.unk_510 > -0x2000)
                entity->unk_510.part16.unk_510 += ((entity->velX.whole + 0xFF) >> 8);
        }
        if (dist <= 0x7FFF)
            entity->state = 3;
        break;
    }
    case 3:
        entity->hit.flags |= 2;
        if ((RandomNumberGenerator() & 0x7F) <= 1)
        {
            entity->state = 4;
            break;
        }
        entity->timer++;
        dist = entity->timer << 8;
        entity->velY.whole = Sin(dist + 0x4000) / 4;
        EntityDampVelocityX(entity, 0x40);
        EntityAccelerateAndMove(entity, 0x20000);
        if (Entity_DistanceToPoint(entity, x, y) > 0x300000)
            entity->state = 2;
        break;
    case 4:
        entity->unk_514.unk_514_32 = Player_GetSelectedOrNearestTarget(entity);
        if (entity->unk_514.unk_514_32 == NULL)
        {
            entity->state = 2;
            entity->timer = 0x80;
            break;
        }
        entity->unk_518.whole = -((RandomNumberGenerator() & 0xF) << 16);
        entity->state = 5;
    case 5:
    {
        struct EwramData_EntityData *target;
        s32 timer;
        entity->hit.flags &= ~2;
        target = entity->unk_514.unk_514_32;
        timer = entity->timer - 1;
        entity->timer = timer;
        if ((s16)target->unk_518.part16.hp > 0 && (u8)timer != 0)
        {
            dist = Entity_StepTowardPoint(entity, target->posX.whole, target->posY.whole + entity->unk_518.whole, 5);
            entity->unk_510.part16.unk_510 = entity->velX.whole / 64 + entity->unk_510.part16.unk_510 + entity->unk_510.part16.unk_512 * 4;
            if (dist > 0x1FFF)
                break;
        }
        entity->state = 2;
        break;
    }
    case 6:
        entity->unk_510.part16.unk_510 = entity->unk_510.part16.unk_512 * 16 + entity->unk_510.part16.unk_510;
        if (entity->unk_50C.whole <= 0)
        {
            BlueSoul_ReleaseActiveResources(entity);
            EntityDelete(entity);
            return;
        }
        entity->unk_50C.whole -= 0x1000;
        break;
    }
    if (entity->state == 5)
    {
        u16 a = entity->unk_510.part16.unk_510 - 0x4000;
        u16 dx = FIXED_TO_INT(Sin((s16)a + 0x4000) << 4);
        u16 dy = FIXED_TO_INT(Sin((s16)a) << 4);
        if (entity->flipX)
            dx = (u32)-(s32)(dx << 16) >> 16;
        HitBox_SetAttackRaw(entity, ((((s16)dx - 10) & 0xFF) + ((((s16)dy - 10) & 0xFF) << 8)) + 0x14140000);
    }
    ObjAffine_Set(entity->unk_510.part16.unk_510, entity->unk_50C.whole, entity->unk_50C.whole, entity->flipX, 0, entity->affineIndex);
}
