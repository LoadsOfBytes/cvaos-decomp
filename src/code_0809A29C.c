/* BiphronShot_Update (Biphron projectile update). */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern s32 Entity_MoveWithCollisionDefault(struct EwramData_EntityData *entity, s32 landingVelocity, s32 ceilingOffset);
extern void Hit_CheckWithPlayer(void *res, struct EwramData_EntityData *b);
extern void BiphronShot_SpawnFlame(struct EwramData_EntityData *entity, s32 offsetX, s32 offsetY);

/**
 * @brief 9A29C | Projectile update: move (Entity_MoveWithCollisionDefault); delete when it
 * stops, lands (bit 2 clear) or leaves the screen; otherwise hitbox and
 * every 4 frames an effect (BiphronShot_SpawnFlame) with sound 0x77.
 */
void BiphronShot_Update(struct EwramData_EntityData *entity)
{
    u8 collision[0x10];
    s32 x = (s16)entity->posX.part16.integer;

    if (!(Entity_MoveWithCollisionDefault(entity, 0x40000, -0x200000) & 4)
        || x == (s16)entity->posX.part16.integer
        || EntityIsOutsideScreen(entity, 1))
    {
        EntityDelete(entity);
    }
    else
    {
        Hit_CheckWithPlayer(collision, entity);
        if (!(entity->timer++ & 3))
        {
            BiphronShot_SpawnFlame(entity, 0, 0);
            PlaySong(0x77);
        }
    }
}
