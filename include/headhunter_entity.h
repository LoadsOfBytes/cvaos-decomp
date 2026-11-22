#ifndef HEADHUNTER_ENTITY_H
#define HEADHUNTER_ENTITY_H

/*
 * Work slots of the Headhunter main entity:
 * - graphics: ObjGfx slot of its sprite sheet, shared by all parts and
 *   effects it spawns;
 * - palettes 0..3: the four palette variants of sHeadhunterPalette (body
 *   forms and parts; 3 = effects);
 * - flags: cleared every update; 1 = no contact damage this frame,
 *   2 = show the forced frame instead of the animation;
 * - forced frame: sprite frame shown while flag 2 is set;
 * - hit react: set by Headhunter_ApplyResolvedDamage when a hit should
 *   interrupt the current action.
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define HEADHUNTER_GRAPHICS(e) e->unk_500.whole
#define HEADHUNTER_PALETTE_0(e) e->unk_504.part8.unk_504
#define HEADHUNTER_PALETTE_1(e) e->unk_504.part8.unk_505
#define HEADHUNTER_PALETTE_2(e) e->unk_504.part8.unk_506
#define HEADHUNTER_PALETTE_3(e) e->unk_504.part8.unk_507
#define HEADHUNTER_FLAGS(e) e->unk_4FC.part8.unk_4FE
#define HEADHUNTER_FORCED_FRAME(e) e->unk_4FC.part8.unk_4FF
#define HEADHUNTER_HIT_REACT(e) e->unk_4FC.part8.unk_4FD

/*
 * Effects, shots and parts it spawns: the shared ObjGfx slot (copied from
 * graphics) and their first sprite frame (animation frame = base + random).
 */
#define HEADHUNTER_EFFECT_GRAPHICS(e) e->unk_500.whole
#define HEADHUNTER_EFFECT_FRAME(e) e->unk_4FC.part8.unk_4FF

/*
 * Extending whip attack (Headhunter_SpawnExtendingAttack,
 * HeadhunterAttackTip_Update - TU aliases BASE_X / BASE_Y / OFFSET_X /
 * OFFSET_Y): room-space base of the whip and the tip's 16.16 offset from it.
 * Its segments (Headhunter_CreateAttackSegment, HeadhunterAttackSegment_Update)
 * keep the tip and their index (1..9, placed at index / 10 of the offset).
 */
#define HEADHUNTER_WHIP_BASE_X(e) e->unk_4FC.whole
#define HEADHUNTER_WHIP_BASE_Y(e) e->unk_500.whole
#define HEADHUNTER_WHIP_OFFSET_X(e) e->unk_504.whole
#define HEADHUNTER_WHIP_OFFSET_Y(e) e->unk_4F4.whole
#define HEADHUNTER_WHIP_TIP(e) e->unk_4FC.unk_4FC_entity
#define HEADHUNTER_WHIP_INDEX(e) e->unk_504.part8.unk_504

/*
 * Transformation (code_08083A40.c - TU aliases PHASE / KIND / COUNT /
 * TARGET_X / TARGET_Y): form phase of the Headhunter (defeat in phases 1 and 2
 * starts a transformation; the parts wait for phase KIND + 1); slash arcs keep
 * the Headhunter. Kind of a child (TU aliases KIND): transformation part
 * 2..0, slash arc (1 / 2 drift with its facing), burst corner 0..3.
 * Transformation parts count their passes and keep the room-space point
 * they fly to.
 */
#define HEADHUNTER_FORM_PHASE(e) e->unk_4FC.part8.unk_4FC
#define HEADHUNTER_ARC_OWNER(e) e->unk_4F4.unk_4F4_entity
#define HEADHUNTER_KIND(e) e->unk_504.part8.unk_504
#define HEADHUNTER_PART_COUNT(e) e->unk_504.part8.unk_505
#define HEADHUNTER_PART_TARGET_X(e) e->unk_4FC.whole
#define HEADHUNTER_PART_TARGET_Y(e) e->unk_500.whole

/*
 * Effects (lightning, waves, orbiting particles ...): the Headhunter whose
 * flag bit 0 cancels them (TU alias OWNER_DONE reads HEADHUNTER_FLAGS of it);
 * orbiting particles keep the camera position of the last frame to stay
 * room-fixed. Intro (Headhunter AI TU): step of the intro sequence.
 */
#define HEADHUNTER_EFFECT_SOURCE(e) e->unk_4F4.unk_4F4_entity
#define HEADHUNTER_ORBIT_CAMERA_X(e) e->unk_4FC.whole
#define HEADHUNTER_ORBIT_CAMERA_Y(e) e->unk_500.whole
#define HEADHUNTER_INTRO_STEP(e) e->unk_4F8.part8.unk_4F8

#endif /* HEADHUNTER_ENTITY_H */
