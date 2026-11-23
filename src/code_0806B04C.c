/* Restore selected entity attributes from its object template. */
#include "structs/ewram.h"
#include "structs/entity_template.h"
void Enemy_ResetFromTemplate(struct EwramData_EntityData *entity)
{
    entity->spriteLayer = 18;
    entity->hp = sEnemyData[entity->enemyId].hp;
    entity->mp = sEnemyData[entity->enemyId].mp;
    entity->attack = sEnemyData[entity->enemyId].attack;
    entity->defense = sEnemyData[entity->enemyId].defense;
}
