#ifndef BLUE_SOUL_ENTITY_H
#define BLUE_SOUL_ENTITY_H

/*
 * Common work slots of blue soul entities (created by Soma_HandleBlueSoulInput
 * from BlueSoulInfo.updateFunc) and of their helper entities (created with
 * BlueSoul_CreateEntity):
 * - owner: the entity that created it (the player for the soul itself);
 * - slot: index in playerState.activeBlueSoulEntities (0 for the soul),
 *   cleared when the entity ends;
 * - id: the equipped blue soul; param / value: BlueSoulInfo.effectParam /
 *   effectValue (variant or strength, e.g. Devil / Manticore / Curly 0..2);
 * - MP interval / counter: one MP is paid every 60 / mpPerSecond frames,
 *   counted by BlueSoul_ShouldEnd / BlueSoul_CheckEndCondition.
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define BLUE_SOUL_OWNER(e) e->unk_4F8.unk_4F8_entity
#define BLUE_SOUL_SLOT(e) e->unk_508.part16.unk_508
#define BLUE_SOUL_ID(e) e->unk_4FC.part16.unk_4FC
#define BLUE_SOUL_PARAM(e) e->unk_4FC.part16.unk_4FE[0]
#define BLUE_SOUL_VALUE(e) e->unk_500.whole
#define BLUE_SOUL_MP_INTERVAL(e) e->unk_504.part8.unk_504
#define BLUE_SOUL_MP_COUNTER(e) e->unk_504.part16.unk_506[0]

#endif /* BLUE_SOUL_ENTITY_H */
