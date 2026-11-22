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
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define GRAHAM_FLAGS(e) e->unk_4F8.part16.unk_4FA
#define GRAHAM_SIDE_PART_0(e) e->unk_50C.whole
#define GRAHAM_SIDE_PART_1(e) e->unk_510.whole
#define GRAHAM_PART_SIDE(e) e->unk_4FC.part16.unk_4FE[0]
#define GRAHAM_PART_STEP(e) e->unk_4FC.part16.unk_4FC
#define GRAHAM_PART_FLASH(e) e->unk_500.part16.unk_500

/*
 * Scene links of the root Graham entity: entity index of the scene object
 * (object 0x20, Graham_CreateSceneObject), of the temporary intro helper and
 * of the projectile tracked in the defeat scene. Graham's form entities keep
 * the entity index of their linked child (combat body, attack effect,
 * transition part) in child index.
 */
#define GRAHAM_SCENE_OBJECT_INDEX(e) e->unk_4F8.part8.unk_4F8
#define GRAHAM_SCENE_HELPER_INDEX(e) e->unk_4F4.part8.unk_4F7
#define GRAHAM_DEFEAT_PROJECTILE_INDEX(e) e->unk_4FC.part16.unk_4FE[0]
#define GRAHAM_CHILD_INDEX(e) e->unk_4F4.part8.unk_4F6

/*
 * First form attacks (GrahamFirstForm_UpdateAttacks): sub-state a volley
 * resumes after turning to face the player, and which side the double volley
 * starts with.
 */
#define GRAHAM_TURN_RESUME(e) e->unk_500.part8.unk_500
#define GRAHAM_VOLLEY_SIDE(e) e->unk_504.part8.unk_507

#endif /* GRAHAM_ENTITY_H */
