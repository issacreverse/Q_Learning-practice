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
    return 2;
}

const char* EventEncounter::actionName(int a) const {
    return (a == 0) ? "Do" : "Don't";
}

void EventEncounter::apply(State& s, int action, Rng& rng) const {
    if (s.isGameOver || s.isVictory) return;

    int scrapScale = int(s.stage / 3);

    if (action == 0) {
        float r = rng.uniform01();
        //cout << "r: " << r;

        if (r < 0.5f) s.hp -= 10;
        else {
            if (r < 0.9f)
                s.scrap += (10 + scrapScale);
            else if (r < 0.95f) {
                s.scrap += (10 + scrapScale);
                s.fuel += 2;
            }
            else if (r < 0.97f)
                s.power += 5;
            else {
                s.hp += 10;
                if (s.hp >= 100) s.hp = 100;
            }
        }
    }

    clampAndCheck(s);
}

// ------------ Battle ------------

EncounterType BattleEncounter::type() const {
    return EncounterType::Battle;
}

int BattleEncounter::actionCount(const State&) const {
    return 2;
}

const char* BattleEncounter::actionName(int a) const {
    return (a == 0) ? "Fight" : "Evade";
}

void BattleEncounter::apply(State& s, int action, Rng&) const {
    if (s.isGameOver || s.isVictory) return;

    float enemyStrength = ENEMY_BASE_POWER + ENEMY_POWER_SCALE * ((s.stage-1) / 3);
    float diff = s.power - enemyStrength;

    int scrapScale = int(s.stage / 3);

    if (action == 0) {
        if (diff >= 25) s.scrap += (10 + scrapScale);
        else if (diff >= 0) { s.scrap += (10 + scrapScale); s.hp -= 10; }
        else s.hp -= 15;
    } else {
        s.hp -= 5;
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

void ShopEncounter::apply(State& s, int action, Rng&) const {
    if (s.isGameOver || s.isVictory) return;

    const int repairCost = 10, repairGain = 20;
    const int weaponCost = 15, powerGain = 15;
    const int fuelCost = 5, fuelGain = 3;

    if (action == 0) {
        if (s.scrap >= repairCost) {
            s.scrap -= repairCost;
            s.hp += repairGain;
            if (s.hp >= 100) s.hp = 100;
        }
    }
    else if (action == 1) {
        if (s.scrap >= weaponCost) {
            s.scrap -= weaponCost;
            s.power += powerGain;
        }
    }
    else {
        if (s.scrap >= fuelCost) {
            s.scrap -= fuelCost;
            s.fuel += fuelGain;
        }
    }

    clampAndCheck(s);
}

// ------------ Rebelion ------------

EncounterType RebelionEncounter::type() const {
    return EncounterType::Rebelion;
}

int RebelionEncounter::actionCount(const State&) const {
    return 2;
}

const char* RebelionEncounter::actionName(int a) const {
    return (a == 0) ? "Fight" : "Evade";
}

void RebelionEncounter::apply(State& s, int action, Rng&) const {
    if (s.isGameOver || s.isVictory) return;

    float enemyStrength = ENEMY_BASE_POWER + ENEMY_POWER_SCALE * ((s.stage-1) / 3);
    float diff = s.power - enemyStrength;

    if (action == 0) {
        if (diff >= 25) s.fuel += 3;
        else if (diff >= 0) { s.fuel += 3; s.hp -= 10; }
        else s.hp -= 15;
    } else {
        s.hp -= 8;
    }

    clampAndCheck(s);
}

// ------------ Boss ------------

EncounterType BossEncounter::type() const {
    return EncounterType::Boss;
}

int BossEncounter::actionCount(const State&) const {
    return 2;
}

const char* BossEncounter::actionName(int a) const {
    return (a == 0) ? "Fight" : "Evade";
}

void BossEncounter::apply(State& s, int action, Rng&) const {
    if (s.isGameOver || s.isVictory) return;

    if (action == 1) {
        s.hp = 0;
        s.isGameOver = true;
        return;
    }

    float enemyStrength = ENEMY_BASE_POWER + ENEMY_POWER_SCALE * ((s.stage-1) / 3);
    float diff = s.power - enemyStrength;

    if (diff >= 25) {
        s.isVictory = true;
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