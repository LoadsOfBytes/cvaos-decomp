#ifndef DEBUG_ENTITY_H
#define DEBUG_ENTITY_H

/*
 * Debug mode entities: the moving debug platform (DebugPlatform_*) sets its
 * carry flag when its floor correction catches the player
 * (DebugPlatform_GetCarryVelocity then moves the player with it and clears
 * it); the test enemy (DebugTestEnemy_Update) circles with this angle.
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define DEBUG_PLATFORM_CARRYING(e) e->unk_4FC.part8.unk_4FC
#define DEBUG_TEST_ENEMY_ANGLE(e) e->unk_4F8.whole

#endif /* DEBUG_ENTITY_H */
