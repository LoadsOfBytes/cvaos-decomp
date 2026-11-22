#ifndef KYOMA_DEMON_ENTITY_H
#define KYOMA_DEMON_ENTITY_H

/*
 * Kyoma Demon (EnemyKyomaDemon*, KyomaDemon_*):
 * - hidden: 1 while it lurks behind the scenery (no contact damage);
 * - lurk timer: frames until it next appears (random 64..127);
 * - home x / y: room pixel position of the spawn point;
 * - room x / y: 16.16 room position of the previous frame (Lurk compares it
 *   to detect that it stopped).
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define KYOMA_DEMON_HIDDEN(e) e->unk_4FC.part8.unk_4FC
#define KYOMA_DEMON_LURK_TIMER(e) e->unk_4FC.part16.unk_4FE[0]
#define KYOMA_DEMON_HOME_X(e) e->unk_4F8.part16.unk_4F8
#define KYOMA_DEMON_HOME_Y(e) e->unk_4F8.part16.unk_4FA
#define KYOMA_DEMON_ROOM_X(e) e->unk_500.whole
#define KYOMA_DEMON_ROOM_Y(e) e->unk_504.whole

#endif /* KYOMA_DEMON_ENTITY_H */
