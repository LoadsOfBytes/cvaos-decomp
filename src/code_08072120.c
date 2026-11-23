/* Handle projectile impacts, debris and attachment to a moving entity. */
#include "code_08009A0.h"
#include "mudman_entity.h"
#include "code/code_08039340.h"

extern s32 Entity_MoveWithCollisionDefault(struct EwramData_EntityData *, s32, s32);
extern s32 EntityIsOutsideScreen(struct EwramData_EntityData *, s32);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void MudmanPiece_Update(struct EwramData_EntityData *);
extern u8 sMudmanGfx[], sMudmanSpriteSheet[];
static inline void EmitDebris(struct EwramData_EntityData *parent, s32 x, s32 y, s32 vx, s32 vy, s32 zero)
{
    struct EwramData_EntityData *part = EntityCreateInRange(0x49, 0xCF, (u32 *)MudmanPiece_Update);
    if (part)
    {
        s32 graphics;
        part->posX.whole = x;
        part->posY.whole = y;
        part->velX.whole = vx;
        part->velY.whole = vy;
        part->accelY.whole = 0x2800;
        part->spriteLayer = parent->spriteLayer - 1;
        graphics = ObjGfx_Find(sMudmanGfx);
        Sprite_Attach(part, sMudmanSpriteSheet, graphics, parent->unk_510.part8.basePalette);
        if (vx > 0) part->animFrame = 21;
        else part->animFrame = 18;
        if (vy > 0) part->flipY = 1;
        else part->flipY = 0;
        part->scrollWithBg = 1;
        part->drawFunc = (u32 *)Sprite_DrawEntity;
        part->timer = zero;
        asm(""); /* FAKEMATCH: keeps jump2 from cross-jumping the two inlined copies */
    }
}
/**
 * @brief Mud shot update: fly with gravity and collision; on a wall or the floor
 * splash into a piece and vanish; state 1 (struck) splashes; state 2
 * sticks to the entity it hit until the splat animation ends.
 */
void MudmanShot_Update(struct EwramData_EntityData *entity)
{
    struct BgLayerState *background = &gEwramData->bgInfo[1];
    u8 collision[16];
    s32 state = entity->state;
    switch (state)
    {
    case 0:
    {
        s32 impact = Entity_MoveWithCollisionDefault(entity, 0x10000, -0x40000);
        if (EntityIsOutsideScreen(entity, 1)) goto remove;
        if (impact & 12)
        {
            s32 vx = entity->velX.whole / 2;
            EmitDebris(entity, entity->posX.whole, entity->posY.whole, vx, -entity->velY.whole, 0);
            goto remove;
        }
        if (impact & 3)
        {
            entity->state = 2;
            entity->subState = state;
            Anim_StartIndexed(entity, 6, 0, 0);
            entity->animDone = 0;
        }
        break;
    }
    case 1:
        EmitDebris(entity, entity->posX.whole, entity->posY.whole, -entity->velX.whole / 2, 1, 0);
        goto remove;
    case 2:
        switch (entity->subState)
        {
        case 0:
            entity->subState = 1;
            EmitDebris(entity, entity->posX.whole, entity->posY.whole, -entity->velX.whole / 2, 1, 0);
            /* fall through */
        case 1:
        {
            struct EwramData_EntityData *parent = ENTITY_OWNER(entity);
            if (parent && parent->updateFunc)
            {
                entity->posX.whole += parent->posX.whole + background->xPos.whole - MUDMAN_SHOT_OWNER_X(entity);
                entity->posY.whole += parent->posY.whole + background->yPos.whole - MUDMAN_SHOT_OWNER_Y(entity);
                MUDMAN_SHOT_OWNER_X(entity) = parent->posX.whole + background->xPos.whole;
                MUDMAN_SHOT_OWNER_Y(entity) = parent->posY.whole + background->yPos.whole;
            }
            if (entity->animDone) goto remove;
            break;
        }
        }
        break;
    }
    goto finished;
remove:
    EntityDelete(entity);
    return;
finished:
    if (entity->state == 0) Hit_CheckWithPlayer(collision, entity);
    Anim_Update(entity);
}
