#ifndef JOINT_ENTITY_H
#define JOINT_ENTITY_H

/*
 * Work slots shared by the multi-part ("joint") enemies Golem, GreatArmor,
 * Devil, CreakingSkull, Stolas and BigGolem. The owner keeps its parts in
 * unk_500 (GolemPart/EntityJointPart arrays) and selects a pose: the index
 * into its s<Owner>PartPoses / s<Owner>PartAngles tables (read by
 * <Owner>_UpdatePart / _InterpolateJoint and the create functions). Each
 * part entity stores the index of the owner part it follows and the part it
 * turns towards; the owner's hurt mask has one bit per part whose box
 * damages the player (part updates test (mask >> partIndex) & 1).
 *
 * The macros expand to the original member chains without added
 * parentheses so that the preprocessed code is token-identical; use them
 * only with a plain identifier argument.
 */
#define JOINT_POSE(e) e->unk_504.part8.unk_504
#define JOINT_HURT_MASK(e) e->unk_504.part16.unk_506[0]
#define JOINT_PART_INDEX(e) e->unk_504.part8.unk_504
#define JOINT_PART_ANCHOR(e) e->unk_504.part8.unk_505

#endif /* JOINT_ENTITY_H */
