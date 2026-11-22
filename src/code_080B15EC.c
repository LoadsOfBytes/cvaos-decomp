/* Disc Armor disc aiming. */
#include "gba.h"
#include "disc_armor_entity.h"
#include "syscalls.h"
#include "structs/ewram.h"

/* Integer part of a 16.16 value, rounded towards zero. */
#define INT_TOWARD_ZERO(v) ((v) < 0 ? -((-(v)) >> 16) : (v) >> 16)

/* Pixel difference pos - centre, with the 16.16 centre rounded towards zero. */
static inline s32 PixelDelta(s16 pos, s32 centre)
{
    if (centre < 0)
        return pos + ((-centre) >> 16);
    return pos - (centre >> 16);
}

/**
 * @brief B15EC | Disc Armor disc aiming: point `disc` from `owner`'s stored
 * centre (unk_4FC/unk_500, 16.16) towards `entity`.  unk_4F8 gets the angle,
 * unk_504 the distance (16.16 pixels) and the disc is moved onto the centre.
 * Returns that distance, or 0 without a disc.
 */
s32 DiscArmorRing_Aim(struct EwramData_EntityData *disc, struct EwramData_EntityData *entity,
                 struct EwramData_EntityData *owner)
{
    s32 dx, dy, distance;

    if (disc != NULL)
    {
        /* X via the inline helper, Y via the macro: this mix gives the
         * original register allocation. */
        DISC_ARMOR_RING_ANGLE(disc) = ArcTan2(PixelDelta(entity->posX.part16.integer, DISC_ARMOR_HAND_X(owner)),
                                           entity->posY.part16.integer - INT_TOWARD_ZERO(DISC_ARMOR_HAND_Y(owner)));
        dx = (entity->posX.whole - (s32)DISC_ARMOR_HAND_X(owner)) >> 16;
        dx *= dx;
        dy = (entity->posY.whole - DISC_ARMOR_HAND_Y(owner)) >> 16;
        dy *= dy;
        distance = Sqrt(dx + dy) << 16;
        DISC_ARMOR_RING_LENGTH(disc) = distance;
        disc->posX.whole = DISC_ARMOR_HAND_X(owner);
        disc->posY.whole = DISC_ARMOR_HAND_Y(owner);
        return distance;
    }
    return 0;
}
