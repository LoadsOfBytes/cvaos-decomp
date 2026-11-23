#ifndef ENTITY_HIT_H
#define ENTITY_HIT_H

#include "gba.h"
#include "structs/ewram.h"
#include "structs/entity_boxes.h"
#include "player_level.h"

/* Hit bookkeeping block at offset 0x70 of an entity (see Hit_CheckPair,
 * Hit_CheckWithPlayer). Field names are guesses from usage. */

struct Box8
{
    u32 w0;
    u32 w1;
};

/* Output of the collision checks Hit_CheckPair / Hit_CheckWithPlayer. */
struct HitResult
{
    u16 flags;      /* bit 0: first entity was hit, bit 1/2/3/4: second entity hit by ... */
    u16 typeA;
    u16 typeB;
    u8 resA;
    u8 resB;
    struct Box8 box; /* copy of the overlap rectangle */
};

typedef void (*HitCallback)(u8 *self, u8 *other, struct HitResult *res);

struct HitInfo
{
    u8 type;            /* attack type, indexes sHitTimers */
    u8 noContactDamage; /* nonzero: only the attack box hurts */
    u8 flags;           /* bit 0: active, bit 1: cannot hit, bit 2: cannot be hit */
    u8 boxSlot;         /* box slot in gEntityBoxes */
    u8 hitBy[3];        /* attack types that already hit this entity */
    u8 unk_7;
    u8 hitTimer[3];     /* per-slot value from GetHitTimer() + 1 */
    u8 unk_B;
    HitCallback onHit;
    HitCallback onHurt;
};

#define HIT_INFO(e) ((struct HitInfo *)((u8 *)(e) + 0x70))

u32 HitBox_Overlap(struct EwramData_EntityData *e1, struct SpriteBox *hb1, struct EwramData_EntityData *e2, struct SpriteBox *hb2);
u32 Hit_Check(u8 *a, u8 *b);
u32 Hit_CheckPlayerAttack(u8 *e);
u32 Hit_CheckPlayerHurt(u8 *e);
u32 Hit_CheckPair(struct HitResult *res, u8 *a, u8 *b);
u32 Hit_CheckWithPlayer(struct HitResult *res, u8 *b);

#endif
