#include "types.h"

using namespace std;

string toString(EncounterType t) {
    switch (t) {
        case EncounterType::Event: return "Event";
        case EncounterType::Battle: return "Battle";
        case EncounterType::Shop: return "Shop";
        case EncounterType::Rebelion: return "Rebelion";
        case EncounterType::Boss: return "Boss";
    }
    return "Unknown";
}

void Encounter::clampAndCheck(State& s) {
    if (s.hp <= 0) {
        s.hp = 0;
        s.isGameOver = true;
    }
}

// ------------ Event ------------

EncounterType EventEncounter::type() const {
    return EncounterType::Event;
}

int EventEncounter::actionCount(const State&) const {
    return 3;
}

const char* EventEncounter::actionName(int a) const {
    return (a == 0) ? "Search" : (a == 1) ? "Don't Search" : "Deep Search(Fuel-1)";
}

void EventEncounter::apply(State& s, int action, Rng& rng, bool isTraining) const {
    if (s.isGameOver || s.isVictory) return;

    int scrapScale = int(s.stage / 3);
    // 내가 강할 수록 이벤트 스크랩 보상이 줄어듦. 
    float enemyStrength = ENEMY_BASE_POWER + ENEMY_POWER_SCALE * ((s.stage-1) / 3);
    float diff = s.power - enemyStrength;

    if(diff >= 25) scrapScale -= 8;
    else if(diff >= 0) scrapScale -= 3;

    if (action == 0) {
        float r = rng.uniform01();
        //cout << "r: " << r;
        if (r < 0.5f) {s.hp -= 10; if(!isTraining) cout << "Bad outcome: HP -10\n";}
        else {
            if (r < 0.9f)
            {
                s.scrap += (10 + scrapScale); 
                if(!isTraining) cout << "Good outcome: Scrap +" << (10 + scrapScale) << "\n";
            }
                
            else if (r < 0.95f) {
                s.scrap += (10 + scrapScale);
                s.fuel += 2;
                if(!isTraining) cout << "Good outcome: Scrap +" << (10 + scrapScale) << ", Fuel +" << 2 << "\n";
            }
            else if (r < 0.97f)
            {
                s.power += 5; 
                if(!isTraining) cout << "Good outcome: Power +" << 5 << "\n";
            }
            else {
                s.hp += 10;
                if(!isTraining) cout << "Good outcome: HP +" << 10 << "\n";
                if (s.hp >= 100) s.hp = 100;
            }
        }
    }
    else if (action == 2) {
        if(s.fuel > 0) s.fuel -= 1;
        else {
            if(!isTraining) cout << "No fuel for Deep Search. Action has no effect.\n";
            // 연료가 없으면 Deep Search 못함. 대신 Don't Search와 같은 효과
            return;
        }

        float r = rng.uniform01();
        //cout << "r: " << r;
        if (r < 0.3f)
        {
            s.hp -= 10;
            if(!isTraining) cout << "Bad outcome: HP -10\n";
        } 
        else {
            if (r < 0.5f)
            {
                s.scrap += (5 + scrapScale);
                if(!isTraining) cout << "Good outcome: Scrap +" << (5 + scrapScale) << "\n";
            }
                
            else if (r < 0.7f) {
                s.scrap += (10 + scrapScale);
                if(!isTraining) cout << "Good outcome: Scrap +" << (10 + scrapScale) << "\n";
            }
            else if (r < 0.85f)
            {
                s.scrap += (10 + scrapScale + scrapScale);
                if(!isTraining) cout << "Good outcome: Scrap +" << (10 + scrapScale + scrapScale) << "\n";
            }
            else {
                s.bomb += 1;
                if (s.bomb > MAX_BOMB) s.bomb = MAX_BOMB;
                if(!isTraining) cout << "Good outcome: Bomb +" << 1 << "\n";
            }
        }
    }
    else {
        if(!isTraining) cout << "No Search action taken. No outcome.\n";
    }

    clampAndCheck(s);
}

// ------------ Battle ------------

EncounterType BattleEncounter::type() const {
    return EncounterType::Battle;
}

int BattleEncounter::actionCount(const State&) const {
    return 3;
}

const char* BattleEncounter::actionName(int a) const {
    return (a == 0) ? "Fight" : (a == 1) ? "Evade" : "Fight(Use Bomb)";
}

void BattleEncounter::apply(State& s, int action, Rng& rng, bool isTraining) const {
    if (s.isGameOver || s.isVictory) return;

    float enemyStrength = ENEMY_BASE_POWER + ENEMY_POWER_SCALE * ((s.stage-1) / 3);
    float diff = s.power - enemyStrength;

    int scrapScale = int(s.stage / 3);
    if(action == 2) {
        diff += s.bomb*BOMB_POWER_BONUS; // 폭탄 사용 시 파워 차이에 보너스
        s.bomb = 0; // 폭탄 사용 후 폭탄 0개
    }

    if (action != 1) {
        if (diff >= 25) 
        {
            s.scrap += (10 + scrapScale);
            if(!isTraining) cout << "Victory: Scrap +" << (10 + scrapScale) << "\n";
        }
        else if (diff >= 0) { s.scrap += (10 + scrapScale); s.hp -= 10; if(!isTraining) cout << "Minor Victory: Scrap +" << (10 + scrapScale) << ", HP -10\n";}
        else
        {
            s.hp -= 15;
            if(!isTraining) cout << "Defeat: HP -15\n";
        } 
    } else {
        s.hp -= 5;
        if(!isTraining) cout << "Evaded: HP -5\n";
    }
    clampAndCheck(s);
}

// ------------ Shop ------------

EncounterType ShopEncounter::type() const {
    return EncounterType::Shop;
}

int ShopEncounter::actionCount(const State&) const {
    return 3;
}

const char* ShopEncounter::actionName(int a) const {
    switch (a) {
        case 0: return "Repair(10 scrap -> 20 hp)";
        case 1: return "BuyWeapons(15 scrap -> 15 power)";
        default: return "BuyFuel(5 scrap -> 3 fuel)";
    }
}

void ShopEncounter::apply(State& s, int action, Rng& rng, bool isTraining) const {
    if (s.isGameOver || s.isVictory) return;

    const int repairCost = 10, repairGain = 20;
    const int weaponCost = 15, powerGain = 15;
    const int fuelCost = 5, fuelGain = 3;

    if (action == 0) {
        if (s.scrap >= repairCost) {
            s.scrap -= repairCost;
            s.hp += repairGain;
            if (s.hp >= 100) s.hp = 100;
            if(!isTraining) cout << "Repaired: Scrap -" << repairCost << ", HP +" << repairGain << "\n";
        }
    }
    else if (action == 1) {
        if (s.scrap >= weaponCost) {
            s.scrap -= weaponCost;
            s.power += powerGain;
            if(!isTraining) cout << "Bought Weapons: Scrap -" << weaponCost << ", Power +" << powerGain << "\n";
        }
    }
    else {
        if (s.scrap >= fuelCost) {
            s.scrap -= fuelCost;
            s.fuel += fuelGain;
            if(!isTraining) cout << "Bought Fuel: Scrap -" << fuelCost << ", Fuel +" << fuelGain << "\n";
        }
    }

    clampAndCheck(s);
}

// ------------ Rebelion ------------

EncounterType RebelionEncounter::type() const {
    return EncounterType::Rebelion;
}

int RebelionEncounter::actionCount(const State&) const {
    return 3;
}

const char* RebelionEncounter::actionName(int a) const {
    return (a == 0) ? "Fight" : (a == 1) ? "Evade" : "Fight(Use Bomb)";
}

void RebelionEncounter::apply(State& s, int action, Rng& rng, bool isTraining) const {
    if (s.isGameOver || s.isVictory) return;

    float enemyStrength = ENEMY_BASE_POWER + ENEMY_POWER_SCALE * ((s.stage-1) / 3);
    float diff = s.power - enemyStrength;
    if(action == 2) {
        diff += s.bomb*BOMB_POWER_BONUS; // 폭탄 사용 시 파워 차이에 보너스
        s.bomb = 0; // 폭탄 사용 후 폭탄 0개
    }
    if (action != 1) {
        if (diff >= 25) {s.fuel += 3; if(!isTraining)cout << "Victory: Fuel +" << 3 << "\n";}
        else if (diff >= 0) { s.fuel += 3; s.hp -= 10; if(!isTraining)cout << "Minor Victory: Fuel +" << 3 << ", HP -10\n";}
        else {s.hp -= 15; if(!isTraining)cout << "Defeat: HP -15\n";}
    } else{
        s.hp -= 8;
        if(!isTraining) cout << "Evaded: HP -8\n";
    } 

    clampAndCheck(s);
}

// ------------ Boss ------------

EncounterType BossEncounter::type() const {
    return EncounterType::Boss;
}

int BossEncounter::actionCount(const State&) const {
    return 3;
}

const char* BossEncounter::actionName(int a) const {
    return (a == 0) ? "Fight" : (a == 1) ? "Evade" : "Fight(Use Bomb)";
}

void BossEncounter::apply(State& s, int action, Rng& rng, bool isTraining) const {
    if (s.isGameOver || s.isVictory) return;

    if (action == 1) {
        s.hp = 0;
        s.isGameOver = true;
        return;
    }

    float enemyStrength = ENEMY_BASE_POWER + ENEMY_POWER_SCALE * ((s.stage-1) / 3);
    float diff = s.power - enemyStrength;
    if(action == 2) {
        diff += s.bomb*BOMB_POWER_BONUS; // 폭탄 사용 시 파워 차이에 보너스
        s.bomb = 0; // 폭탄 사용 후 폭탄 0개
    }
    if (diff >= 25) {
        s.isVictory = true;
        if(!isTraining) cout << "The enemy flagship has been annihilated!" << "\n";
    }
    else if (diff >= 0) {
        s.hp -= 20;
        if (s.hp <= 0) {
            s.hp = 0;
            s.isGameOver = true;
            return;
        }
        s.isVictory = true;
    }
    else {
        s.hp = 0;
        s.isGameOver = true;
    }
}