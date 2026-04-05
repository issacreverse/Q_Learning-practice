#include "types.h"

void agent::learn(const State& state, const EncounterType& encounter, int action, float reward, const State& nextState, const EncounterType& nextEncounter) 
{
    int stateIndex = getStateIndex(state);
    int nextStateIndex = getStateIndex(nextState);
    
    int encounterIndex = getEncounterIndex(encounter);
    int nextEncounterIndex = getEncounterIndex(nextEncounter);
    
    // Q-learning 업데이트
    float alpha = 0.1f; // 학습률
    float gamma = 0.9f; // 할인율
    
    qTable[stateIndex][encounterIndex][action] += alpha * (reward + gamma * maxQ(nextStateIndex, nextEncounterIndex) - qTable[stateIndex][encounterIndex][action]);
}
int agent::getStateIndex(const State& s) {
    
    int hpIndex;
    int scrapIndex;
    int powerIndex;
    int fuelIndex;  
    int stageIndex;
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


    return hpIndex * (6*9*7*4) + scrapIndex * (9*7*4) + powerIndex * (7*4) + fuelIndex * 4 + stageIndex;
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
    float maxValue = qTable[stateIndex][encounterIndex][0];
    for (int action = 1; action < 3; action++) 
    {
        if (qTable[stateIndex][encounterIndex][action] > maxValue) 
        {
            maxValue = qTable[stateIndex][encounterIndex][action];
        }
    }
    return maxValue;
}
