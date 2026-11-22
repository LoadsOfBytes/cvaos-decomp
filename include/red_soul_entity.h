#ifndef RED_SOUL_ENTITY_H
#define RED_SOUL_ENTITY_H

/*
 * Common work slots of red soul entities (Soma's bullet souls and Julius'
 * sub-weapons, created from RedSoulInfo.updateFunc) and of the helper
 * entities created with RedSoul_CreateEntity:
 * - owner: the entity that created it (the player for a cast);
 * - slot: index in playerState.activeRedSoulEntities, cleared when the
 *   entity ends;
 * - id: the red soul that was cast (playerStatus.equippedRedSoul; casts only);
 * - param: RedSoulInfo.param, the variant of a shared update function
 *   (e.g. which skeleton bone, Arachne / Axe Armor variant) or a duration
 *   (Chronomage time stop); casts only;
 * - hit: set to 1 by SoulAttack_MarkHit, the hit callback (Hit_Enable slot 5)
 *   of soul attacks that end or react on their first hit.
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define RED_SOUL_OWNER(e) e->unk_4F8.unk_4F8_entity
#define RED_SOUL_SLOT(e) e->unk_4FC.part16.unk_4FE[0]
#define RED_SOUL_ID(e) e->unk_500.part16.unk_500
#define RED_SOUL_PARAM(e) e->unk_500.part16.unk_502
#define RED_SOUL_HIT(e) e->unk_4FC.part8.unk_4FC

/*
 * Hit anchors (RedSoul_AnchorToHitTarget / _AnchorToHitTargetTimed): a soul
 * attack that sticks to the enemy it hit creates an anchor entity (anchor;
 * anchor timer counts the frames since, timed variant). The anchor
 * (RedSoulAnchor_Update, owner = the attack) keeps the hit enemy (anchor
 * target), moves the attack along with it and sets target dead when the
 * enemy's hp runs out.
 */
#define RED_SOUL_ANCHOR(e) e->unk_504.unk_504_entity
#define RED_SOUL_ANCHOR_TIMER(e) e->unk_508.part16.unk_50A
#define RED_SOUL_ANCHOR_TARGET(e) e->unk_504.unk_504_entity
#define RED_SOUL_ANCHOR_TARGET_DEAD(e) e->unk_508.part8.unk_508

#endif /* RED_SOUL_ENTITY_H */
