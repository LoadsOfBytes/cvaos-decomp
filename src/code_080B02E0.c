/* Weapon carried by an enemy: follows the parent's animation, falls and breaks. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "syscalls.h"
#include "code_08001194.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
#include "structs/entity_render_flags.h"

struct WeaponFrame {
    s16 frame;
    s16 pad;
    s32 angle;
    s8 dx;
    s8 dy;
    u8 pad2[2];
};


extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern struct EwramData_EntityData *Entity_CreateObject(u32 *);
extern void Sprite_GetPartCornerOffset(struct EwramData_EntityData *, s32, s16 *);
extern void EntityAccelerateY(struct EwramData_EntityData *, s32);
extern void EntityMove(struct EwramData_EntityData *);
extern void MinotaurAxeHitBox_Update(struct EwramData_EntityData *);
extern void MinotaurAxePiece_Update(struct EwramData_EntityData *);
extern u8 sMinotaurGfx[], sMinotaurPalette[], sMinotaurSpriteSheet[];
extern u8 sCommonObjectGfx[], sCommonObjectSpriteSheet[];
extern struct WeaponFrame *sMinotaurAxeFrames[];

static inline struct WeaponFrame *FindFrame(struct WeaponFrame *frame, struct EwramData_EntityData *parent)
{
    if (frame == NULL)
        return NULL;
    {
        u8 key = parent->ext.minotaur.animStep;
        while (frame->frame != -1)
        {
            if (key <= frame->frame)
                return frame;
            frame++;
        }
    }
    return NULL;
}

static inline s32 ScreenY(u16 y) { struct BgLayerState *bg = &gEwramData->bgInfo[1]; s32 b = bg->yPos.part16.integer; return (s16)y - b; }
static inline s32 ScreenX(s32 x) { struct BgLayerState *bg = &gEwramData->bgInfo[1]; return x - bg->xPos.part16.integer; }

#define SPIN(e) (e->ext.minotaurAxe.spin)
#define SPIN_SPEED(e) (e->ext.minotaurAxe.spinSpeed)
#define DROPPED(e) (e->ext.minotaurAxe.dropped)
#define TO_PIXELS(d) ((d) < 0 ? -(-(d) >> 16) : (d) >> 16)

void MinotaurAxe_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = ENTITY_OWNER(entity);
    s16 offset[2];
    s32 angle;

    if (DROPPED(entity) == 0)
    {
        if ((s16)parent->unk_518.part16.hp <= 0)
            DROPPED(entity) = 1;
        if (DROPPED(entity) == 0 && entity->state == 4
            && ENTITY_INDEX(parent) == ENTITY_PARENT_INDEX(entity)
            && (parent->ext.minotaur.flags & 1))
        {
            entity->state = 1;
            PlaySong(0x69);
        }
    }
    switch (entity->state)
    {
    case 0:
    {
        u16 slot;
        if (!Entity_LoadGraphics(entity, sMinotaurGfx, sMinotaurPalette, 0, sMinotaurSpriteSheet))
            goto remove;
        slot = ObjAffine_Alloc(1);
        if ((s16)slot < 0)
            goto remove;
        if (!(((u8 *)gEwramData)[0x8E] & 0x40))
        {
            struct EwramData_EntityData *hitbox;
            entity->attack += entity->attack >> 2;
            hitbox = Entity_CreateObject((u32 *)MinotaurAxeHitBox_Update);
            if (hitbox != NULL)
            {
                ENTITY_OWNER(hitbox) = entity;
                ENTITY_PARENT_INDEX(hitbox) = ENTITY_INDEX(entity);
                hitbox->attack = entity->attack;
                hitbox->unk_518.part16.hp = entity->unk_518.part16.hp;
            }
        }
        entity->affineIndex = slot;
        entity->affine = 1;
        entity->scrollWithBg = 1;
        Anim_StartIndexed(entity, 8, 0, 1);
        Anim_Update(entity);
        entity->spriteLayer = parent->spriteLayer;
        entity->state++;
    }
    case 1:
        if (ENTITY_INDEX(parent) != ENTITY_PARENT_INDEX(entity))
        {
            entity->state = 6;
            break;
        }
        if (parent->state == 3)
        {
            entity->state = 3;
            break;
        }

        if (!(parent->ext.minotaur.flags & 1))
        {
            entity->state = 2;
            break;
        }
        entity->flipX = parent->flipX;
        {
            s16 *ofs = offset;
            Sprite_GetPartCornerOffset(parent, 0, ofs);
            entity->posX.part16.integer = parent->posX.part16.integer + offset[0];
            entity->posY.part16.integer = ofs[1] + parent->posY.part16.integer;
        }
        {
            struct WeaponFrame *found = FindFrame(sMinotaurAxeFrames[parent->animScriptIndex], parent);
            if (found == NULL)
                break;
            SPIN(entity) = found->angle;
            if (entity->flipX)
                entity->posX.part16.integer -= found->dx;
            else
                entity->posX.part16.integer += found->dx;
            entity->posY.part16.integer += found->dy;
        }
        break;
    case 2:
        PlaySong(0x86);
        entity->velX.whole = -0x8000;
        if (entity->flipX)
            entity->velX.whole = -entity->velX.whole;
        entity->velY.whole = -0x42000;
        entity->accelY.whole = 0x1800;
        SPIN_SPEED(entity) = 0xF200;
        entity->state = 4;
        break;
    case 3:
        entity->velX.part16.integer = 0;
        entity->velY.whole = -0x30000;
        entity->accelY.whole = 0x2000;
        SPIN_SPEED(entity) = 0xF800;
        entity->state = 4;
        break;
    case 4:
    {
        u16 rx, ry;
        s32 dx, dy, hit, i, drop;
        struct EwramData_EntityData *piece;
        s32 graphics;
        SPIN(entity) += SPIN_SPEED(entity);
        EntityAccelerateY(entity, 0x80000);
        EntityMove(entity);
        if (entity->velY.whole <= 0x2FFFF)
            break;
        if (entity->velY.whole > 0x60000 && EntityIsOutsideScreen(entity, 4))
        {
            if (DROPPED(entity) == 0)
                parent->ext.minotaur.flags &= ~2;
            goto lost;
        }
        angle = (u16)SPIN(entity);
        if ((u32)(angle - 0x9000) > 0x2000)
            break;
        angle = SPIN(entity);
        if (entity->flipX)
            angle = -angle;
        rx = GetEntityRoomXPositionInteger(entity);
        ry = GetEntityRoomYPositionInteger(entity);
        angle -= 0x4000;
        dx = Sin(angle + 0x4000) << 4;
        dy = Sin(angle) << 4;
        rx = (s16)rx + TO_PIXELS(dx);
        ry = (s16)ry + TO_PIXELS(dy);
        hit = Collision_GetPushUp((s16)rx, (s16)ry + 10) << 16;
        if (hit == 0)
            break;
        PlaySong(0x7F);
        graphics = ObjGfx_Find(sCommonObjectGfx);
        i = 0;
        drop = hit;
        goto test;
    loop:
        {
            Sprite_Attach(piece, sCommonObjectSpriteSheet, graphics, 6);
            piece->animFrame = Mod(RandomNumberGenerator() >> 4, 3) + 0x40;
            piece->posX.part16.integer = ScreenX((s16)rx + RandomSigned(4));
            piece->posY.part16.integer = ScreenY(ry) - 8;
            piece->flipX = RandomNumberGenerator() & 1;
            piece->spriteLayer = 0x11;
            piece->scrollWithBg = 1;
            piece->drawFunc = (u32 *)Sprite_DrawEntity;
            i++;
        }
    test:
        if (i <= 3 && (piece = EntityCreateInRange(0x49, 0xCF, (u32 *)MinotaurAxePiece_Update)) != NULL)
            goto loop;
        if (ENTITY_INDEX(parent) == ENTITY_PARENT_INDEX(entity))
            parent->ext.minotaur.flags &= ~2;
        entity->posY.whole += drop;
        entity->state = 5;
        entity->timer = 0xC0;
        break;
    }
    case 5:
        if (EntityIsOutsideScreen(entity, 4))
            goto lost;
        if (entity->timer == 0x20)
            entity->drawSkip = 1;
        if (entity->drawSkip && !(entity->timer & 3))
        {
            s32 x = entity->posX.whole + (RandomSigned(0x100) << 13);
            s32 y = entity->posY.whole + ({ s32 t = (RandomSigned(0x100) << 13) + 0x100000; t; });
            Effect_SpawnShared(x, y, RandomNumberGenerator() & 1, 8);
        }
        if (entity->timer == 0)
            goto lost;
        entity->timer--;
        break;
    lost:
        entity->state = 6;
        break;
    case 6:
        ObjAffine_Free(entity->affineIndex);
    remove:
        EntityDelete(entity);
        return;
    }
    angle = SPIN(entity);
    if (entity->flipX)
        angle = -angle;
    entity->ext.minotaurAxe.angle = angle;
    ObjAffine_Set(angle, 0x10000, 0x10000, entity->flipX, 0, entity->affineIndex);
}
