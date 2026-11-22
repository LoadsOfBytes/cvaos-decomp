/* Restore selected entity attributes from its object template. */
#include "structs/ewram.h"
#include "structs/entity_template.h"
void Enemy_ResetFromTemplate(struct EwramData_EntityData *entity)
{
    entity->spriteLayer = 18;
    entity->unk_518.part16.hp = sEnemyData[entity->unk_518.part8.enemyId].hp;
    entity->unk_510.part16.unk_512 = sEnemyData[entity->unk_518.part8.enemyId].unk_E;
    entity->attack = sEnemyData[entity->unk_518.part8.enemyId].attack;
    entity->defense = sEnemyData[entity->unk_518.part8.enemyId].defense;
}
