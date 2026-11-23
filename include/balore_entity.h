#ifndef BALORE_ENTITY_H
#define BALORE_ENTITY_H

/*
 * Balore keeps its work slots in ext.balore (body), ext.balorePart,
 * ext.baloreHitBox, ext.baloreHand, ext.baloreEffect, ext.balorePiece,
 * ext.baloreOverlay and ext.baloreEmitter. Views that do not fit a member:
 * - hands: bytes 0 / 1 of the entity field unk_51C - animation their hit
 *   boxes follow, and hit flags copied to the hit boxes (1 = strong hit 90 /
 *   reaction 3, 2 = 80, else 55; 4 = solid for the player). Byte views, so
 *   the macros keep the original cast form;
 * - emitter (BaloreEmitter_Draw): 16.16 y step between its 8 drawn copies
 *   (outside the work area; the x step is ext.baloreEmitter.stepX);
 * - body: ext.balore.gfx read through the signed byte view of its slot
 *   (bytes[3]).
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define BALORE_HAND_ANIM(e) ((u8 *)&e->unk_51C)[0]
#define BALORE_HAND_HIT_FLAGS(e) ((u8 *)&e->unk_51C)[1]
#define BALORE_EMITTER_STEP_Y(e) e->unk_510.whole
#define BALORE_GFX_BYTE(e) e->ext.raw.unk_4F8.bytes[3]

#endif /* BALORE_ENTITY_H */
