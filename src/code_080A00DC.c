#include "code_08009A0.h"
#include "kyoma_demon_entity.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "code/code_080211F0.h"
extern s32 Enemy_IsStunned(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void KyomaDemon_UpdateLurk(struct EwramData_EntityData *);
extern void KyomaDemon_UpdateAttack(struct EwramData_EntityData *);
extern void KyomaDemon_UpdateDeath(struct EwramData_EntityData *);

void EnemyKyomaDemonUpdate(struct EwramData_EntityData *entity)
{
    u32 box[4];
    struct BgLayerState *background = &gEwramData->bgInfo[1];
    u8 frame;
    if (entity->state != 2)
    {
        if (Entity_IsInRect(entity, -16, 0, 0x110, 0xB0)) Player_TrackEntity(entity);
    }
    if (Enemy_IsStunned(entity))
    {
        if (entity->state != 2 && KYOMA_DEMON_HIDDEN(entity) == 0)
            Hit_CheckWithPlayer(box, entity);
        return;
    }
    switch (entity->state)
    {
    case 0: KyomaDemon_UpdateLurk(entity); break;
    case 1: KyomaDemon_UpdateAttack(entity); break;
    case 2: KyomaDemon_UpdateDeath(entity); break;
    case 3: Anim_Update(entity); return;
    }
    frame = entity->animFrame;
    Anim_Update(entity);
    if (frame != entity->animFrame) Entity_UpdateBoxes(entity);
    if (entity->state != 2 && KYOMA_DEMON_HIDDEN(entity) == 0)
        Hit_CheckWithPlayer(box, entity);
    KYOMA_DEMON_ROOM_X(entity) = entity->posX.whole + background->xPos.whole;
    KYOMA_DEMON_ROOM_Y(entity) = entity->posY.whole + background->yPos.whole;
}
