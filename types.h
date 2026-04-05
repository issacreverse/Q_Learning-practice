#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <random>
#include <algorithm>
#include <limits>

static const int ENEMY_BASE_POWER = 15;
static const float ENEMY_POWER_SCALE = 5.0f;
static const float GREEDY_VALUE = 0.2f; // 20% 확률로 랜덤 행동 선택
static const float GAME_STEP_PENALTY = 0.01f; // 매 턴마다 패널티
static const float VICTORY_REWARD = 30.0f; // 승리 보상
static const float GAME_OVER_PENALTY = 30.0f; // 게임 오버 패널티 

static const int STATE_NUMBER = 54432; // 상태의 총 개수 (9*6*9*7*4*4)
static const int ENCOUNTER_NUMBER = 5; // 인카운터의 총 개수 (event, battle, shop, rebelion, boss)
static const int ACTION_NUMBER = 3; // 행동의 총 개수 (최대 3개)

static const int BOMB_MAINTENANCE_COST = 2; // 폭탄 유지 비용
static const int BOMB_POWER_BONUS = 5; // 폭탄 사용 시 파워 보너스
static const int MAX_BOMB = 2; // 최대 폭탄 개수

// ------------ Game State  ------------
struct State {
    int hp = 100;
    int scrap = 30;
    int power = 25;
    int fuel = 10;
    int stage = 1;
    int bomb = 1;
    bool isVictory = false;
    bool isGameOver = false;
};

struct Rng {
    std::mt19937 gen;
    explicit Rng(uint32_t seed = 42u);

    float uniform01();
    int rangeInt(int minInclusive, int maxExclusive);
};

// ------------ Encounter Base ------------
enum class EncounterType { Event, Battle, Shop, Rebelion, Boss };


class Encounter {
public:
    virtual ~Encounter() = default;
    virtual EncounterType type() const = 0;
    virtual int actionCount(const State& s) const = 0;
    virtual const char* actionName(int a) const = 0;
    virtual void apply(State& s, int action, Rng& rng) const = 0;

protected:
    static void clampAndCheck(State& s);
};

std::string toString(EncounterType t);

// ------------ Environment ------------
struct StepResult {
    EncounterType encounterType;
    int action;
};

// ------------ Concrete Encounters ------------
class EventEncounter final : public Encounter {
public:
    EncounterType type() const override;
    int actionCount(const State&) const override;
    const char* actionName(int a) const override;

    void apply(State& s, int action, Rng& rng) const override;
};

class BattleEncounter final : public Encounter {
public:
    EncounterType type() const override;
    int actionCount(const State&) const override;
    const char* actionName(int a) const override;

    void apply(State& s, int action, Rng&) const override;
};

class ShopEncounter final : public Encounter {
public:
    EncounterType type() const override;
    int actionCount(const State&) const override;
    const char* actionName(int a) const override;

    void apply(State& s, int action, Rng&) const override;
};

class RebelionEncounter final : public Encounter {
public:
    EncounterType type() const override;
    int actionCount(const State&) const override;
    const char* actionName(int a) const override;

    void apply(State& s, int action, Rng&) const override;
};

class BossEncounter final : public Encounter {
public:
    EncounterType type() const override;
    int actionCount(const State&) const override;
    const char* actionName(int a) const override;

    void apply(State& s, int action, Rng&) const override;
};

class agent {
    //체력은 1부터 100까지  (1-4)(5-9)(10-14)(14-19)(20-29)(30-49)(50-69)(70-89)(90+) 총 9개
    //스크랩은 0부터 무한대까지 (0-4)(5-9)(10-14)(15-25)(25-40)(40+) 총 6개
    //파워는 25부터 무한대까지 (25-29)(30-34)(35-44)(45-54)(55-64)...(95+) 총 9개
    //연료는 0부터 무한대까지 (0)(1)(2)(3-4)(5-6)(7-9)(10+) 총 7개
    //스테이지는 1부터 20까지 (1-9)(10-14)(15-19)(20) 총 4개
    //폭탄은 0부터 2까지 (0)(1)(2)(3) 총 4개
    //상태 인덱스는 9*6*9*7*4*4 = 54432개

    //인카운터는 5개 (event, battle, shop, rebelion, boss)

    //event 인카운터 (do)(don't) 총 2개
    //battle 인카운터 (fight)(evade) 총 2개
    //shop 인카운터 (repair)(buy weapon)(buy fuel) 총 3개
    //rebellion 인카운터 (fight)(evade) 총 2개
    //boss 인카운터 (fight)(evade) 총 2개
    //행동 3개면 됨

    
    // Q-table 크기: 54432(상태) * 5(인카운터) * 3(행동) = 816,480 float entries (약 3.1MB)

    public:
    float qTable[STATE_NUMBER][ENCOUNTER_NUMBER][ACTION_NUMBER] = {0.0f};  // Q-table 

    uint32_t seed = 42u;
    
    void learn(const State& state, const EncounterType& encounter, int action, float reward, const State& nextState, const EncounterType& nextEncounter) ;
    
    int getStateIndex(const State& s);

    int getEncounterIndex(const EncounterType& encounter) ;
    
    float maxQ(int stateIndex, int encounterIndex);
};