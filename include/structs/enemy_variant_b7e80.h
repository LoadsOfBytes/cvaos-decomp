#ifndef ENEMY_VARIANT_B7E80_H
#define ENEMY_VARIANT_B7E80_H

#include "types.h"

/* Variants selected by SkullMillioneFamily_Init: Skull Millione, Cagnazzo, Lubicant.
 * Layout is established by the effect, graphics and movement helpers. */
struct SkullMillioneFamilyVariant {
    void *sprites;
    u8 palette;
    u8 behaviorFlags;
    s8 swayMultiplier;
    u8 activationDelay;
    u16 effectType;
    u16 effectFlags;
    s32 approachSpeed;
    s32 diveSpeed;
}; /* size = 0x14 */

extern const struct SkullMillioneFamilyVariant sSkullMillioneFamilyVariants[];

#endif
