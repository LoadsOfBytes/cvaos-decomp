/* Destructible fragment update, reconstructed from 0x0809F410. */
#include "code_08009A0.h"
#include "legion_entity.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
#include "structs/entity_render_flags.h"
extern u32 Entity_MoveWithCollisionDefault(struct EwramData_EntityData *, s32, s32);
extern s32 Collision_IsSlopeTile(s16, s16);
extern s32 Tile_Classify(s16, s16);
extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern s32 Entity_GetVisibilityPattern(u8);
extern void Effect_SpawnRandom(s32, s32, s32, s32, s32);
extern struct AnimScript sLegionDestructibleFragmentAnimScript[], sLegionAnimScript1[], sUnk_08528340[];

struct FragmentOwnerData {
    u8 pad[0x1C];
    s32 count;
};

void LegionDestructibleFragment_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner;
    struct BgLayerState *bg;
    s32 accelerationLimit;
    u32 box;
    struct FragmentOwnerData *ownerData;
    u32 state;
    u32 cur;
    u32 res;
    s32 x;
    u16 hitbox[8];
    owner = LEGION_PART_OWNER(entity);
    bg = &gEwramData->bgInfo[1];
    do { /* keeps the constant in a register (no move to its use in local-alloc) */
        accelerationLimit = 0x60000;
    } while (0);
    box = 0x1C06E4FD;
    ownerData = (void *)owner->unk_4F8.unk_4F8_entity;
    cur = entity->state;
    state = cur & 0xFF; /* not a plain copy: CSE must not learn cur from the state tests */
    if (state == 0) {
        s32 y;
        x = entity->posX.whole + bg->xPos.whole;
        y = entity->posY.whole + bg->yPos.whole;
        if ((u32)(x - 0xA00000) <= 0xC00000 && y > 0xCFFFFF && y <= 0x1600000) {
            EntityAccelerateAndMove(entity, accelerationLimit);
        } else if ((res = Entity_MoveWithCollisionDefault(entity, 0x40000, -0x200000)) & 4) {
            entity->state++;
            Anim_Start(entity, sLegionDestructibleFragmentAnimScript, 2, 1);
        }
    } else if (state == 1) {
        if (entity->animDone) {
            *(u8 *)&entity->state = cur + 1;
            Anim_Start(entity, sLegionAnimScript1, 2, 1);
            HitBox_SetBody(entity, &box);
            HitBox_SetAttack(entity, &box);
            *(u8 *)&entity->hit.flags &= 0xFB;
            *(u8 *)&entity->hit.flags &= 0xFD;
            entity->velY.whole = 0x40000;
            entity->accelY.whole = 0x1800;
            entity->accelX.whole = 0;
        }
    } else if (state == 2) {
        register u32 flags asm("r9"); /* FAKEMATCH: priority vs. the flipX byte pointer (sb/sl) */
        if (entity->flipX)
            entity->velX.whole = -0x4000;
        else
            entity->velX.whole = 0x4000;
        res = Entity_MoveWithCollisionDefault(entity, 0x40000, -0x200000);
        do { /* raises the refs of x (global-alloc priority) */
            u32 b_ = *(ENTITY_RENDER_FLAGS_ADDRESS(entity)) & 0x40;
            x = 8;
            if (b_)
                x -= 16;
        } while (0);
        flags = 3;
        flags &= res;
        if (flags)
            entity->flipX ^= 1;
        if (!Collision_IsSlopeTile(bg->xPos.part16.integer + entity->posX.part16.integer,
                          bg->yPos.part16.integer + entity->posY.part16.integer + 1)) {
            do { /* raises the refs of the posX/posY pointers */
                if (Tile_Classify(bg->xPos.part16.integer + entity->posX.part16.integer + x,
                                bg->yPos.part16.integer + entity->posY.part16.integer) || flags)
                    entity->flipX ^= 1;
            } while (0);
        }
        Enemy_UpdateHitFlash(entity);
        Hit_CheckWithPlayer(hitbox, entity);
        if (owner && owner->updateFunc && owner->unk_518.part8.enemyId == entity->unk_518.part8.enemyId
            && owner->state != 2) {
            Anim_Update(entity);
            return;
        }
        {
            s32 zero = 0;
            entity->state = 10;
            entity->subState = zero;
            entity->unk_518.part16.hp = zero;
            entity->palette = entity->unk_510.part8.basePalette;
            entity->timer = 40;
            entity->unk_510.part8.hitFlashTimer = zero;
            Anim_Start(entity, sUnk_08528340, 2, 1);
        }
    } else if (state == 10) {
        if (!(entity->timer & 3))
            Effect_SpawnRandom(entity->posX.whole, entity->posY.whole - accelerationLimit, 0x100000, 0xC0000, entity->spriteLayer - 1);
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
        if ((s8)--entity->timer <= 0) {
            ownerData->count--;
            EntityDelete(entity);
            return;
        }
    }
    Anim_Update(entity);
}
