#ifndef EVENT_SCENE30_H
#define EVENT_SCENE30_H

/*
 * Event scene 30 transformation controller (EventScene30_UpdateTransformation,
 * whose TU-local aliases are CENTER_X / CENTER_Y / SCALE_X / TIMER / JITTER_Y):
 * screen centre of the transformation, its scale (the halo and centre effect
 * use it as their radius), the step timer the effects watch (e.g. 263, 255,
 * 160, 0) and the vertical jitter offset.
 * Its effect sprites (halo, centre effect, orbit ring) keep their affine
 * rotation angle in the entity field unk_51C and their affine scale.
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define EVENT30_CENTER_X(e) e->unk_504.part16.unk_504
#define EVENT30_CENTER_Y(e) e->unk_504.part16.unk_506[0]
#define EVENT30_SCALE(e) e->unk_500.part16.unk_500
#define EVENT30_TIMER(e) e->unk_508.part16.unk_508
#define EVENT30_JITTER_Y(e) e->unk_508.part8.unk_50B
#define EVENT30_EFFECT_ANGLE(e) e->unk_51C
#define EVENT30_EFFECT_SCALE(e) e->unk_500.whole

/*
 * Orbit ring effect (EventScene30_SpawnOrbitRing, EventScene30OrbitRing_Update):
 * index of its spawn point, orbit centre, orbit angle, ring size (from the
 * spawn point's scale), x / y orbit radius and the outward spread counter.
 */
#define EVENT30_RING_INDEX(e) e->unk_4F8.part8.unk_4FB
#define EVENT30_RING_CENTER_X(e) e->unk_504.part16.unk_504
#define EVENT30_RING_CENTER_Y(e) e->unk_504.part16.unk_506[0]
#define EVENT30_RING_ANGLE(e) e->unk_508.part16.unk_508
#define EVENT30_RING_SIZE(e) e->unk_508.part16.unk_50A
#define EVENT30_RING_RADIUS_X(e) e->unk_50C.part8.unk_50C
#define EVENT30_RING_RADIUS_Y(e) e->unk_50C.part8.unk_50D
#define EVENT30_RING_SPREAD(e) e->unk_50C.part16.unk_50E

/*
 * Controller: horizontal jitter (read as an s8 by the centre effect).
 * Effects: spin step of the affine angle (centre and spinning effects).
 * Fragments (EventScene30_SpawnOrbitFragments, *Fragment_Update): sway / orbit
 * phase, orbit direction (index & 1) and fragment index.
 */
#define EVENT30_JITTER_X(e) e->unk_508.part16.unk_50A
#define EVENT30_EFFECT_SPIN(e) e->unk_508.part16.unk_50A
#define EVENT30_FRAGMENT_PHASE(e) e->unk_4FC.part16.unk_4FC
#define EVENT30_FRAGMENT_DIR(e) e->unk_4F8.part8.unk_4FA
#define EVENT30_FRAGMENT_INDEX(e) e->unk_4F8.part8.unk_4FB

/*
 * Controller phases (EventScene30_UpdateTransformation and the unreferenced
 * EventScene30_UpdateUnusedZoom; TU aliases PHASE / INIT / ANGLE / SCALE_Y /
 * SWEEP): phase, first-frame flag of the phase, BG2 affine angle (high byte),
 * vertical scale and the scanline sweep counter of the split zoom. The unused
 * zoom also keeps a texture y (high byte) of its split halves.
 */
#define EVENT30_PHASE(e) e->unk_4F8.part8.unk_4F9
#define EVENT30_PHASE_INIT(e) e->unk_4F8.part8.unk_4FA
#define EVENT30_ANGLE(e) e->unk_4F8.part8.unk_4F8
#define EVENT30_SCALE_Y(e) e->unk_500.part16.unk_502
#define EVENT30_SWEEP(e) e->unk_4FC.part16.unk_4FC
#define EVENT30_ZOOM_TEX_Y(e) e->unk_50C.part16.unk_50E

/*
 * Scene driver (EventScene30_Update): phase of the player's floating sway
 * during the transformation (Sin of it is passed to the scripted animation).
 */
#define EVENT30_FLOAT_PHASE(e) e->unk_4FC.part16.unk_4FC

#endif /* EVENT_SCENE30_H */
