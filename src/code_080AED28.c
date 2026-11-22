/* Parent-linked affine entity part: attachment, release, motion and deletion. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
#include "structs/entity_render_flags.h"
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, u8 *, void *, u8, void *);
extern void WaiterSkeletonThrownTray_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void WaiterSkeletonThrownTray_OnHit(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Sprite_GetPartCornerOffset(struct EwramData_EntityData *, s32, s16 *);
extern s32 Target_GetX(struct EwramData_EntityData *);
extern void Effect_SpawnRandom(s32, s32, s32, s32, s32);
extern void EntityDampVelocityX(struct EwramData_EntityData *, s32);
extern void EntityAccelerateY(struct EwramData_EntityData *, s32);
extern void EntityMoveX(struct EwramData_EntityData *);
extern void EntityMoveY(struct EwramData_EntityData *);
extern u8 sWaiterSkeletonGfx[], sSkeletonWaiterSkeletonPalette[], sWaiterSkeletonSpriteSheet[];
struct PartOffset { u16 index; s8 x, y; };
extern const struct PartOffset sWaiterSkeletonTrayOffsets[];
/* This halfword bitfield view preserves the original slot update access width. */

static inline void BoxA(struct EwramData_EntityData *e, s32 box) { HitBox_SetAttackRaw(e, box); }
static inline void BoxB(struct EwramData_EntityData *e, s32 box) { HitBox_SetBodyRaw(e, box); }

void WaiterSkeletonThrownTray_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->unk_4FC.unk_4FC_entity;
    u16 box[8];
    s16 offset[2];
    if (entity->state != 5 && (s16)entity->unk_518.part16.hp <= 0)
        entity->state = 5;
    if (entity->state <= 2 && ((u8)entity->unk_4F4.part8.unk_4F4 != (u8)parent->unk_4F4.part8.unk_4F4 || parent->state == 5)) {
        entity->hidden = 0;
        entity->spriteLayer = 19;
        entity->state = 3;
        Hit_Enable(entity, 10, WaiterSkeletonThrownTray_DamagePlayerOnContact, (s32)WaiterSkeletonThrownTray_OnHit);
        BoxA(entity, 0x0808FCFC);
        BoxB(entity, 0x0808FCFC);
        entity->hit.flags &= 0xFB;
        entity->hit.flags |= 2;
    }
    if (gEwramData->gameState.timeStopTimer && entity->state != 5) {
        Hit_CheckWithPlayer(box, entity);
        return;
    }
    switch (entity->state) {
    case 0: {
        s16 slot;
        if (!Entity_LoadGraphics(entity, sWaiterSkeletonGfx, sSkeletonWaiterSkeletonPalette, 0, sWaiterSkeletonSpriteSheet))
            goto delete;
        slot = ObjAffine_Alloc(1);
        if (slot < 0)
            goto delete;
        entity->affineIndex = slot;
        entity->affine = 1;
        entity->scrollWithBg = 1;
        entity->spriteLayer = parent->spriteLayer;
        Anim_StartIndexed(entity, entity->unk_4F8.part16.unk_4F8, 0, 1);
        entity->flipX = parent->flipX;
        entity->accelX.part16.integer = 10;
        if (entity->flipX)
            entity->accelX.part16.integer = -entity->accelX.part16.integer;
        { u16 *origin = &parent->posX.part16.integer;
          u16 x = entity->accelX.part16.integer;
          u16 parentX = *origin;
          x += parentX;
          entity->posX.part16.integer = x; }
        entity->posY.part16.integer = parent->posY.part16.integer - 16;
        if (parent->unk_504.whole & 1) {
            entity->hidden = 1;
            entity->state = 2;
            break;
        }
        entity->state = 1;
    }
    case 1:
        if ((u8)entity->unk_4F4.part8.unk_4F4 != (u8)parent->unk_4F4.part8.unk_4F4) {
            entity->state = 5;
            break;
        }
        if ((u8)(parent->animScriptIndex - 2) <= 4) {
            const struct PartOffset *point = &sWaiterSkeletonTrayOffsets[parent->animScriptIndex];
            Sprite_GetPartCornerOffset(parent, point->index, offset);
            entity->flipX = parent->flipX;
            { u16 *origin = &parent->posX.part16.integer;
          u16 x = offset[0];
          u16 parentX = *origin;
          x += parentX;
          entity->posX.part16.integer = x; }
            entity->posY.part16.integer = offset[1] + parent->posY.part16.integer;
            if (entity->flipX)
                entity->posX.part16.integer -= point->x;
            else
                entity->posX.part16.integer += point->x;
            entity->posY.part16.integer += point->y;
            if (parent->unk_504.whole & 4) {
                s32 vx;
                entity->state = 4;
                PlaySong(0x89);
                Hit_Enable(entity, 10, WaiterSkeletonThrownTray_DamagePlayerOnContact, (s32)WaiterSkeletonThrownTray_OnHit);
                BoxA(entity, 0x0808FCFC);
                BoxB(entity, 0x0808FCFC);
                entity->hit.flags &= 0xFB;
                if (Target_GetX(entity) - entity->posX.whole < 0)
                    vx = -0x14000 - ((entity->posX.whole - Target_GetX(entity)) >> 6);
                else
                    vx = -0x14000 - ((Target_GetX(entity) - entity->posX.whole) >> 6);
                entity->velX.whole = vx;
                entity->accelY.whole = 0x2000;
                if (entity->flipX)
                    entity->velX.whole = -vx;
                entity->velY.whole = -0x20000;
            }
        }
        break;
    case 2:
        if ((u8)entity->unk_4F4.part8.unk_4F4 != (u8)parent->unk_4F4.part8.unk_4F4) {
            entity->state = 5;
            break;
        }
        if (parent->state == 3) {
            entity->state = 3;
            break;
        }
        switch (parent->animStep) {
        case 1:
            entity->hidden = 0;
            entity->spriteLayer = parent->spriteLayer + 1;
            break;
        case 3:
            entity->spriteLayer = parent->spriteLayer;
            entity->flipX = parent->flipX;
            entity->accelX.part16.integer = -10;
            if (entity->flipX)
                entity->accelX.part16.integer = 10;
            { u16 *origin = &parent->posX.part16.integer;
          u16 x = entity->accelX.part16.integer;
          u16 parentX = *origin;
          x += parentX;
          { u16 *destination = &entity->posX.part16.integer;
            *destination = x;
            { u16 y = parent->posY.part16.integer - 16;
              /* Recompute the field address: pointer arithmetic folds into a different store. */
              destination = &entity->posY.part16.integer; *destination = y; } } }
            entity->state = 1;
            break;
        }
        break;
    case 3:
        entity->velX.whole = 0x8000;
        if (entity->flipX)
            entity->velX.whole = -entity->velX.whole;
        entity->velY.whole = -0x28000;
        entity->accelY.whole = 0x2000;
        PlaySong(0x89);
        entity->state = 4;
    case 4:
        entity->unk_504.whole += entity->velX.whole / 64;
        EntityDampVelocityX(entity, 60);
        EntityAccelerateY(entity, 0x80000);
        EntityMoveX(entity);
        EntityMoveY(entity);
        if (!EntityIsOutsideScreen(entity, 4))
            break;
        goto release;
    case 5:
        Effect_SpawnRandom(entity->posX.whole, entity->posY.whole, 0xC0000, 0xC0000, entity->spriteLayer - 1);
release:
        ObjAffine_Free(entity->affineIndex);
delete:
        EntityDelete(entity);
        return;
    }
    ObjAffine_Set(entity->unk_504.whole, 0x10000, 0x10000, entity->flipX, 0, entity->affineIndex);
    if (entity->state != 5)
        Hit_CheckWithPlayer(box, entity);
    Anim_Update(entity);
}
