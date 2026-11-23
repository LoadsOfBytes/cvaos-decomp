#ifndef EVENT_SCENE03_H
#define EVENT_SCENE03_H

/*
 * Event scene 3 controller (sEventSceneCreateFuncs[3], EventScene03_*):
 * - actor 0 / 1 / 8: the helper entities created from the object table
 *   (Entity_CreateFromObjectTable kinds 0, 1 and 8); actor 0 is placed next to
 *   the player, actor 1 walks in and plays the attack animation, actor 8
 *   becomes EventScene03MainPart_Update and finally turns into soul orbs and
 *   debris;
 * - progress: sequence bits read by the parts through their parent index:
 *   8 = parts start moving, 0x10 = actor 1 attack started, 2 = its animation
 *   finished, 4 = scene over (parts end).
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */

/*
 * Homing orbs (EventScene03_SpawnSoulOrb / _SpawnTargetOrb and their
 * updates): heading angle, turn rate (grows by 42 up to 0x4000; the soul orb
 * starts with its mode), 16.16 speed (grows to 0x80000) and the target
 * entity of the target orb.
 */

/*
 * Parts (EventScene03_Spawn*Part, *Part_Update): spawn flags (bit 1 hidden,
 * 4 / 8 / 0x10 variant choices), entity index of the linked part and the wave
 * phase of the floating motion.
 */

/* Camera step of the scene (EventScene03_Init, EventScene03_ScrollCamera):
 * byte 0 of unk_51C (byte view, original cast form kept). */
#define EVENT_SCENE03_CAMERA_STEP(e) ((u8 *)&e->unk_51C)[0]

#endif /* EVENT_SCENE03_H */
