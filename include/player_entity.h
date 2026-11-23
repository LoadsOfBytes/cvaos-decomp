#ifndef PLAYER_ENTITY_H
#define PLAYER_ENTITY_H

/*
 * Work slots of the player entity (entity slot 0, Soma and Julius code):
 * - anim palette: palette index last loaded for the current animation
 *   (sSomaAnimPalettes[anim] is compared with it before ObjPalette_LoadRange);
 * - invulnerable timer: nonzero blocks Player_ApplyContactDamage; souls and
 *   damage set it, Soma_Update/Julius_Update count it down;
 * - floor kind: 0xFF = none yet this frame, else the floor/slope kind found by
 *   the collision probes; reset to 0xFF after use;
 * - conveyor velocity: 16.16 x velocity added to posX each frame
 *   (+/- gameState.conveyorSpeed on conveyor tiles), cleared after use.
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define PLAYER_ANIM_PALETTE(e) e->unk_508.part8.unk_50A
#define PLAYER_INVULNERABLE_TIMER(e) e->unk_500.part8.unk_502
#define PLAYER_FLOOR_KIND(e) e->unk_50C.part8.unk_50D
#define PLAYER_CONVEYOR_VELOCITY(e) e->unk_510.whole

/*
 * More player work slots:
 * - action flags: the player state bit field (bit 1 airborne, 0x10 double
 *   jump / ability jump used, 0x20 weapon attack, 0x400 crouching, 0x1000
 *   standing on a solid floor, 0x2000/0x4000 on a slope, 0x80000 an attack
 *   connected this frame, 0x200000 the requested animation has been started,
 *   ...); weapons, souls and enemies read it through the owner pointer, and
 *   the link partner entity keeps the same bits;
 * - air time: frames since leaving the floor, counted up to 16 (a jump sets it
 *   to 16); a ground jump is still allowed while it is <= 3;
 * - drop-through timer: set to 16 (32) by down + jump; while it is nonzero the
 *   floor probes are skipped and no jump starts;
 * - requested anim: animation index Soma_Update/Julius_Update start at the end
 *   of the frame (0xFF = none);
 * - frame counter: incremented once per player update;
 * - wall probes: address of the list of wall probe offsets used by the
 *   horizontal collision (chosen per state in Soma_Update/Julius_Update);
 * - on stairs: 1 when a stair tile was found in front of the feet;
 * - slope level: steepest slope class (collision flags >> 6) under the feet,
 *   SlopeSlowdown divides velX by it;
 * - draw offset: signed pixel offset added to the position while
 *   Player_DrawWithOffset draws the sprite;
 * - blink timer: contact immunity frames; the sprite blinks (drawSkip) while
 *   it counts down;
 * - up hold timer: frames up has been held while standing (look-up animation
 *   after 4);
 * - weapon slot: index into playerState.weaponEntities and the attacker info
 *   of the weapon this entity attacks with (0 = player, 1 = link partner).
 */
#define PLAYER_ACTION_FLAGS(e) e->unk_4F4.whole
#define PLAYER_AIR_TIME(e) e->unk_4F8.part16.unk_4F8
#define PLAYER_DROP_THROUGH_TIMER(e) e->unk_4F8.part16.unk_4FA
#define PLAYER_REQUESTED_ANIM(e) e->unk_504.part16.unk_504
#define PLAYER_FRAME_COUNTER(e) e->unk_504.part16.unk_506[0]
#define PLAYER_WALL_PROBES(e) e->unk_4FC.whole
#define PLAYER_ON_STAIRS(e) e->unk_500.part8.unk_500
#define PLAYER_SLOPE_LEVEL(e) e->unk_500.part8.unk_501
#define PLAYER_DRAW_OFFSET_X(e) e->unk_508.part8.unk_508
#define PLAYER_DRAW_OFFSET_Y(e) e->unk_508.part8.unk_509
#define PLAYER_BLINK_TIMER(e) e->unk_518.part16.hp
#define PLAYER_UP_HOLD_TIMER(e) e->unk_50C.part8.unk_50E
#define PLAYER_WEAPON_SLOT(e) e->unk_50C.part8.unk_50C

/*
 * Solid objects (Player_RegisterSolidObject, playerState.solidObject): their
 * callback table (first word = flags, Player_GetSolidObjectFlags), stored
 * through the entity-pointer view and read through the word-pointer view
 * (TU alias FUNCS of the player TUs).
 */
#define SOLID_OBJECT_CALLBACK_TABLE(e) e->unk_4F4.unk_4F4_entity
#define SOLID_OBJECT_CALLBACKS(e) e->unk_4F4.unk_4F4_p32

/* Transformation souls (Skill_Devil_Manticore_Curly_Use): form index (the
 * soul's effect value), picks the per-form dash animations
 * (Soma_UpdateTransformationDash). */
#define PLAYER_TRANSFORM_FORM(e) e->unk_508.part8.unk_50B

#endif /* PLAYER_ENTITY_H */
