/* Create a short-lived attack box and advance its collision lifetime. */
#include "code_08009A0.h"
#include "persephone_entity.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Persephone_DamagePlayerOnContact(struct EwramData_EntityData *);
extern u8 sPersephoneGfx[], sPersephoneSpriteSheet[];
extern void PersephoneAttackBox_Update(struct EwramData_EntityData *);
/**
 * @brief Attack box update: contact test, delete when its counter (PERSEPHONE_BOX_TIME) ends.
 */
void PersephoneAttackBox_Update(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    Hit_CheckWithPlayer(collision, entity);
    if (PERSEPHONE_BOX_TIME(entity)-- <= 0) EntityDelete(entity);
}
