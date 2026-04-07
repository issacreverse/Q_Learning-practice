#include "types.h"

agent::agent()
    : qTable(STATE_NUMBER * ENCOUNTER_NUMBER * ACTION_NUMBER, 0.0f) {}

float& agent::Q(int stateIndex, int encounterIndex, int actionIndex)
{
    return qTable[
        (stateIndex * ENCOUNTER_NUMBER + encounterIndex) * ACTION_NUMBER + actionIndex
    ];
}

const float& agent::Q(int stateIndex, int encounterIndex, int actionIndex) const
{
    return qTable[
        (stateIndex * ENCOUNTER_NUMBER + encounterIndex) * ACTION_NUMBER + actionIndex
    ];
}

void agent::learn(const State& state, const EncounterType& encounter, int action,
                  float reward, const State& nextState, const EncounterType& nextEncounter)
{
    int stateIndex = getStateIndex(state);
    int nextStateIndex = getStateIndex(nextState);

    int encounterIndex = getEncounterIndex(encounter);
    int nextEncounterIndex = getEncounterIndex(nextEncounter);

    float alpha = 0.1f;
    float gamma = 0.95f;

    Q(stateIndex, encounterIndex, action) +=
        alpha * (
            reward
            + gamma * maxQ(nextStateIndex, nextEncounterIndex)
            - Q(stateIndex, encounterIndex, action)
        );
}
int agent::getStateIndex(const State& s) {
    
    int hpIndex;
    int scrapIndex;
    int powerIndex;
    int fuelIndex;  
    int stageIndex;
    int bombIndex;
    int enounterIndex; 

    //hpIndex
    if(s.hp <= 4) hpIndex = 0;
    else if(s.hp <= 9) hpIndex = 1;
    else if(s.hp <= 14) hpIndex = 2;
    else if(s.hp <= 19) hpIndex = 3;
    else if(s.hp <= 29) hpIndex = 4;
    else if(s.hp <= 49) hpIndex = 5;
    else if(s.hp <= 69) hpIndex = 6;
    else if(s.hp <= 89) hpIndex = 7;
    else hpIndex = 8;
    
    //scrapIndex
    if(s.scrap <= 4) scrapIndex = 0;
    else if(s.scrap <= 9) scrapIndex = 1;
    else if(s.scrap <= 14) scrapIndex = 2;
    else if(s.scrap <= 25) scrapIndex = 3;
    else if(s.scrap <= 40) scrapIndex = 4;
    else scrapIndex = 5;

    //powerIndex
    if(s.power <= 29) powerIndex = 0;
    else if(s.power <= 34) powerIndex = 1;
    else if(s.power <= 44) powerIndex = 2;
    else if(s.power <= 54) powerIndex = 3;
    else if(s.power <= 64) powerIndex = 4;
    else if(s.power <= 74) powerIndex = 5;
    else if(s.power <= 84) powerIndex = 6;
    else if(s.power <= 94) powerIndex = 7;
    else powerIndex = 8;

    //fuelIndex
    if(s.fuel == 0) fuelIndex = 0;
    else if(s.fuel == 1) fuelIndex = 1;
    else if(s.fuel == 2) fuelIndex = 2;
    else if(s.fuel <= 4) fuelIndex = 3;
    else if(s.fuel <= 6) fuelIndex = 4;
    else if(s.fuel <= 9) fuelIndex = 5;
    else fuelIndex = 6;

    //stageIndex
    if(s.stage <= 9) stageIndex = 0;
    else if(s.stage <= 14) stageIndex = 1;
    else if(s.stage <= 19) stageIndex = 2;
    else stageIndex = 3;

    //bombIndex
    if(s.bomb == 0) bombIndex = 0;
    else if (s.bomb == 1) bombIndex = 1;
    else if (s.bomb == 2) bombIndex = 2;
    else bombIndex = 3;


    //return hpIndex * (6*9*7*4*4) + scrapIndex * (9*7*4*4) + powerIndex * (7*4*4) + fuelIndex * (4*4) + stageIndex * 4 + bombIndex;
    return hpIndex    * (SCRAP_BUCKET_COUNT * POWER_BUCKET_COUNT * FUEL_BUCKET_COUNT * STAGE_BUCKET_COUNT * BOMB_BUCKET_COUNT)
     + scrapIndex * (POWER_BUCKET_COUNT * FUEL_BUCKET_COUNT * STAGE_BUCKET_COUNT * BOMB_BUCKET_COUNT)
     + powerIndex * (FUEL_BUCKET_COUNT * STAGE_BUCKET_COUNT * BOMB_BUCKET_COUNT)
     + fuelIndex  * (STAGE_BUCKET_COUNT * BOMB_BUCKET_COUNT)
     + stageIndex * (BOMB_BUCKET_COUNT)
     + bombIndex;
}

int agent::getEncounterIndex(const EncounterType& encounter) 
{
    switch (encounter) 
    {
        case EncounterType::Event: return 0;
        case EncounterType::Battle: return 1;
        case EncounterType::Shop: return 2;
        case EncounterType::Rebelion: return 3;
        case EncounterType::Boss: return 4;
        default: return -1; // Invalid encounter type
    }
}

float agent::maxQ(int stateIndex, int encounterIndex)
{
    float maxValue = Q(stateIndex, encounterIndex, 0);

    for (int action = 1; action < ACTION_NUMBER; action++)
    {
        if (Q(stateIndex, encounterIndex, action) > maxValue)
        {
            maxValue = Q(stateIndex, encounterIndex, action);
        }
    }

    return maxValue;
}

void agent::maskBombActionsWhenNoBomb()
{
    for (int hp = 0; hp < HP_BUCKET_COUNT; ++hp)
    for (int scrap = 0; scrap < SCRAP_BUCKET_COUNT; ++scrap)
    for (int power = 0; power < POWER_BUCKET_COUNT; ++power)
    for (int fuel = 0; fuel < FUEL_BUCKET_COUNT; ++fuel)
    for (int stage = 0; stage < STAGE_BUCKET_COUNT; ++stage)
    {
        int bomb = 0; // 폭탄 없음 상태

        int stateIndex =
            hp    * (SCRAP_BUCKET_COUNT * POWER_BUCKET_COUNT * FUEL_BUCKET_COUNT * STAGE_BUCKET_COUNT * BOMB_BUCKET_COUNT)
          + scrap * (POWER_BUCKET_COUNT * FUEL_BUCKET_COUNT * STAGE_BUCKET_COUNT * BOMB_BUCKET_COUNT)
          + power * (FUEL_BUCKET_COUNT * STAGE_BUCKET_COUNT * BOMB_BUCKET_COUNT)
          + fuel  * (STAGE_BUCKET_COUNT * BOMB_BUCKET_COUNT)
          + stage * (BOMB_BUCKET_COUNT)
          + bomb;

        // encounter: Battle(1), Rebelion(3), Boss(4)
        Q(stateIndex, 1, 2) = -1000.0f;
        Q(stateIndex, 3, 2) = -1000.0f;
        Q(stateIndex, 4, 2) = -1000.0f;
    }
}


