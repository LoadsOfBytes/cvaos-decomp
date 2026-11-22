/* Belmont melee strike: animation choice, sound cues and active hitbox. */
#include "code_08009A0.h"
#include "belmont_entity.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern u16 sBelmontMeleeSounds[];
/**
 * @brief Melee state (7): select the strike animation, play event sounds
 * and gate contact damage by animation step, then return to combat.
 */
void Belmont_UpdateMeleeAttack(struct EwramData_EntityData *entity)
{
    switch(entity->subState)
    {
        case 0:
            if(Player_GetFocusedActionFlags() & 0x400)
            {
                if(entity->animScriptIndex!=6)
                    Anim_StartIndexed(entity,6,1,0);
            }
            else
            {
                if(entity->animScriptIndex!=5)
                    Anim_StartIndexed(entity,5,1,0);
            }
            entity->subState++;
            entity->stage=0;
        case 1:
        {
            u8 *frame=&entity->animStep;
            if(*frame==4)
            {
                u16 *songs=sBelmontMeleeSounds;
                PlaySong(songs[RandomNumberGenerator()%2]);
            }
            if(*frame==8)
                PlaySong(0xFC);
            if(*frame>7)
                entity->attack=BELMONT_ATTACK(entity);
            else
                entity->attack=0;
            if(entity->animDone)
            {
                entity->state=2;
                entity->stage=0;
                entity->subState=0;
                entity->attack=BELMONT_ATTACK_OFF(entity);
            }
            break;
        }
    }
}
