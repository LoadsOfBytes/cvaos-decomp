#ifndef ENTITY_FUNC_H
#define ENTITY_FUNC_H

struct EwramData_EntityData;

/* Per-frame callback of an entity: EwramData_EntityData.updateFunc / drawFunc,
 * also the update function passed to the entity creation helpers. */
typedef void (*EntityFunc)(struct EwramData_EntityData *);

#endif // ENTITY_FUNC_H
