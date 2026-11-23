#ifndef ALURA_UNE_ENTITY_H
#define ALURA_UNE_ENTITY_H

/*
 * Alura Une (AluraUne_*): awake flag (set the first time the player comes
 * near or it is damaged; that first time it attacks at once) and hit flag
 * (set by AluraUne_OnHit, ends the idle wait; cleared by the attacks).
 * Thorns (AluraUne_SpawnThorn, AluraUneThorn_Update): frames they stay out.
 * Root wave (AluraUneRootWave_Update): jumps done (turns after the third,
 * deleted after six).
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define ALURA_UNE_AWAKE(e) e->unk_504.part8.unk_504
#define ALURA_UNE_HIT(e) e->unk_504.part8.unk_505
#define ALURA_UNE_THORN_TIME(e) e->unk_504.part16.unk_504
#define ALURA_UNE_WAVE_STEP(e) e->unk_4FC.part8.unk_4FC

#endif /* ALURA_UNE_ENTITY_H */
