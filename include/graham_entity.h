#ifndef GRAHAM_ENTITY_H
#define GRAHAM_ENTITY_H

/*
 * Graham family state flags: set on the main entity and copied every frame to
 * the body parts through their owner pointers (2 = first form hit reaction,
 * 4 / 8 = transition steps, 0x10 / 0x20 = attack phases read by the parts,
 * 0x40, 0x80 = scene objects created).
 * Body overlay: its two side part entities (Graham_CreateSidePart 0 / 1).
 * Side part: which side it is on (toggled during the attack), the step in its
 * wait table and a flash countdown.
 * The macros expand to member chains or to the ENTITY_* views of the common
 * block (structs/ewram.h) without added parentheses; use them with a plain
 * base expression.
 */
#define GRAHAM_SIDE_PART_1(e) ENTITY_WORD_510(e)

/*
 * Scene links of the root Graham entity: entity index of the scene object
 * (object 0x20, Graham_CreateSceneObject), of the temporary intro helper and
 * of the projectile tracked in the defeat scene. Graham's form entities keep
 * the entity index of their linked child (combat body, attack effect,
 * transition part) in child index.
 */

/*
 * First form attacks (GrahamFirstForm_UpdateAttacks): sub-state a volley
 * resumes after turning to face the player, and which side the double volley
 * starts with.
 */

/* Scene (Graham_SpawnSceneProjectile): emitter counter, cycles through the
 * five spawn points. */

#endif /* GRAHAM_ENTITY_H */
