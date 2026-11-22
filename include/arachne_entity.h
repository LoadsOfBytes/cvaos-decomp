#ifndef ARACHNE_ENTITY_H
#define ARACHNE_ENTITY_H

/*
 * Arachne work slots:
 * - walk phase: 16.16 position in the leg cycle (wraps at 0x240000; the legs
 *   pick their pose from its integer part), walk speed: signed step added to
 *   it every frame (eased to +-0x10000);
 * - gait: leg pose mode (0 walk, 1 / 2 recoil), wait: frames to wait;
 * - attack angle: aim of the web shot (swept by 0x60 per frame);
 * Legs keep their index (0..n) in leg index.
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define ARACHNE_WALK_PHASE(e) e->unk_4FC.whole
#define ARACHNE_WALK_SPEED(e) e->unk_500.whole
#define ARACHNE_GAIT(e) e->unk_504.part8.unk_505
#define ARACHNE_WAIT(e) e->unk_504.part8.unk_506
#define ARACHNE_ATTACK_ANGLE(e) e->unk_4F8.whole
#define ARACHNE_LEG_INDEX(e) e->unk_504.part8.unk_504

#endif /* ARACHNE_ENTITY_H */
