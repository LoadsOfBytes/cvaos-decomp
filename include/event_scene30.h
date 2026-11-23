#ifndef EVENT_SCENE30_H
#define EVENT_SCENE30_H

/*
 * Event scene 30 keeps its work slots in ext.event30 (controller),
 * ext.event30Effect, ext.event30Ring, ext.event30Fragment and
 * ext.event30Driver. Views that do not fit a member:
 * - jitter y: vertical jitter offset of the controller, the high byte of
 *   ext.event30.jitterX (TU alias JITTER_Y);
 * - effect angle: affine rotation angle of the effect sprites (halo, centre
 *   effect, orbit ring), kept in the entity field unk_51C.
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define EVENT30_JITTER_Y(e) e->ext.raw.unk_508.part8.unk_50B
#define EVENT30_EFFECT_ANGLE(e) e->unk_51C

#endif /* EVENT_SCENE30_H */
