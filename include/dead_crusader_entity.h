#ifndef DEAD_CRUSADER_ENTITY_H
#define DEAD_CRUSADER_ENTITY_H

/*
 * Dead Crusader: guarding (crouched behind the shield after the player
 * attacked; DeadCrusader_OnHit then takes no damage). Cleared every frame
 * by EnemyDeadCrusaderUpdate and set again by DeadCrusader_UpdateWalk.
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define DEAD_CRUSADER_GUARDING(e) e->unk_504.part8.unk_505

#endif /* DEAD_CRUSADER_ENTITY_H */
