/* Big Golem: falling projectile with a hit callback owned by the boss. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"
extern void BigGolemRock_Update(struct EwramData_EntityData *);
extern void BigGolemRock_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void BigGolemRock_OnKilled(struct EwramData_EntityData *, s32, u16 *);
extern u8 sCommonObjectGfx[], sCommonObjectSpriteSheet[];
/**
 * @brief Spawn a thrown rock at (x, y) with velocity (vx, vy): BigGolemRock_Update,
 * shared object graphics and a contact box (BigGolemRock_DamagePlayerOnContact);
 * deleted again when the graphics are missing.
 */
void BigGolem_SpawnRock(struct EwramData_EntityData *parent, s32 x, s32 y, s32 vx, s32 vy)
{
    u32 collision = 0x0808FCFC;
    struct EwramData_EntityData *shot = EntityCreateInRange(0x1B, 0x48, (u32 *)BigGolemRock_Update);
    if (shot != NULL)
    {
        s32 tiles = ObjGfx_Find(sCommonObjectGfx);
        if (tiles < 0)
        {
            EntityDelete(shot);
            return;
        }
        Sprite_Attach(shot, sCommonObjectSpriteSheet, tiles, 6);
        shot->animFrame = 0x42;
        shot->drawFunc = (u32 *)Sprite_DrawEntity;
        Hit_Enable(shot, 10, BigGolemRock_DamagePlayerOnContact, (s32)BigGolemRock_OnKilled);
        HitBox_SetAttack(shot, &collision);
        HitBox_SetBody(shot, &collision);
        shot->hit.flags &= ~4;
        shot->hit.flags &= ~2;
        shot->scrollWithBg = 1;
        shot->spriteLayer = parent->spriteLayer + 1;
        ENTITY_OWNER(shot) = parent;
        shot->posX.whole = x;
        shot->posY.whole = y;
        shot->velX.whole = vx;
        shot->velY.whole = vy;
        shot->accelY.whole = 0x2000;
        shot->flipX = parent->flipX;
        shot->attack = parent->attack;
    }
}
