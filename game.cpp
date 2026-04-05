#include "types.h"
#include <fstream>
#include <filesystem>
#include <ctime>

using namespace std;

static int t;
class agent;

float additionalReward(const State& s);
void saveQTable(const agent& a, const string& filename);
void loadQTable(agent& a, const string& filename);

int visitCount[STATE_NUMBER][ENCOUNTER_NUMBER] = {};

class Game 
{

public:
    explicit Game(uint32_t seed = 42u)
        : rng(seed)
    {
        encounters.push_back(std::make_unique<EventEncounter>());
        encounters.push_back(std::make_unique<BattleEncounter>());
        encounters.push_back(std::make_unique<ShopEncounter>());
        rebelion = std::make_unique<RebelionEncounter>();
        boss = std::make_unique<BossEncounter>();
    }

    void reset() { s = State{}; }

    int chooseActionHuman(const Encounter& e) 
    {
        const int n = e.actionCount(s);

        cout << "\nChoose action:\n";
        for (int i = 0; i < n; ++i) 
        {
            cout << "  [" << i << "] " << e.actionName(i) << "\n";
        }

        int a;
        while (true) 
        {
            cout << "Enter action number (0~" << (n - 1) << "): ";
            if (cin >> a && 0 <= a && a < n) 
            {
                return a;
            }
            // 입력이 숫자가 아니거나 범위 밖이면 처리
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Try again.\n";
        }   
    }

    int chooseActionGreedy(const Encounter& e, agent& a) 
    {
        float r = rng.uniform01();

        if(r < GREEDY_VALUE) {
            // 20% 확률로 랜덤 행동 선택 (탐험)
            const int n = e.actionCount(s);
            return rng.rangeInt(0, n);
        }
        else
        {
            // 80% 확률로 최적 행동 선택 (활용)
            int stateIndex = a.getStateIndex(s);
            int encounterIndex = a.getEncounterIndex(e.type());
            
            // Q-table에서 가장 높은 Q값을 가진 행동 선택
            float maxQValue = -std::numeric_limits<float>::infinity();
            int bestAction = 0;
            for (int action = 0; action < e.actionCount(s); ++action) 
            {
                float qValue = a.qTable[stateIndex][encounterIndex][action];
                if (qValue > maxQValue) 
                {
                    maxQValue = qValue;
                    bestAction = action;
                }
            }
            return bestAction;
        }
    }
    int chooseActionAgent(const Encounter& e, agent& a) 
    {
        int stateIndex = a.getStateIndex(s);
        int encounterIndex = a.getEncounterIndex(e.type());
        
        // Q-table에서 가장 높은 Q값을 가진 행동 선택(정책)
        float maxQValue = -std::numeric_limits<float>::infinity();
        int bestAction = 0;
        for (int action = 0; action < e.actionCount(s); ++action) 
        {
            float qValue = a.qTable[stateIndex][encounterIndex][action];
            if (qValue > maxQValue) 
            {
                maxQValue = qValue;
                bestAction = action;
            }
        }
        return bestAction;
    }
    // 1 turn 진행 (encounter 선택 + action + 적용 + stage 증가)
    StepResult stepOneTurn() 
    {
        const Encounter& e = pickEncounter();
        cout << "\n=== Turn: " << t << " ===";
        cout << "\n=== Encounter: " << toString(e.type( )) << " ===\n";

        int action = chooseActionHuman(e);
        e.apply(s, action, rng);

        // 승리/게임오버면 stage 올릴 필요 없음
        if (!s.isGameOver && !s.isVictory) {
            s.stage += 1;
        }

        // 턴 끝날 떄 연료 소모 (다음 행선지로 이동): fuel -1 (0 아래로 X)
        if (s.fuel > 0) s.fuel -= 1;

        return { e.type(), action };
    }

    StepResult greedyStepOneTurn(agent& a, const Encounter& e) 
    {
        int action = chooseActionGreedy(e, a);
        e.apply(s, action, rng);

        // 승리/게임오버면 stage 올릴 필요 없음
        if (!s.isGameOver && !s.isVictory) {
            s.stage += 1;
        }

        // 턴 끝날 떄 연료 소모 (다음 행선지로 이동): fuel -1 (0 아래로 X)
        if (s.fuel > 0) s.fuel -= 1;

        return { e.type(), action };
    }

    StepResult agentStepOneTurn(agent& a) 
    {
        const Encounter& e = pickEncounter();
        cout << "\n=== Turn: " << t << " ===";
        cout << "\n=== Encounter: " << toString(e.type( )) << " ===\n";

        int action = chooseActionAgent(e, a);
        cout << "\nAgent chooses action: " << e.actionName(action) << "\n";
        e.apply(s, action, rng);

        // 승리/게임오버면 stage 올릴 필요 없음
        if (!s.isGameOver && !s.isVictory) {
            s.stage += 1;
        }

        // 턴 끝날 떄 연료 소모 (다음 행선지로 이동): fuel -1 (0 아래로 X)
        if (s.fuel > 0) s.fuel -= 1;

        return { e.type(), action };
    }
    const Encounter& pickEncounter() 
    {
        if (s.stage >= BOSS_STAGE) {
            return *boss;
        }
        if (s.fuel == 0) {
            return *rebelion;
        }
        // Event/Battle/Shop 랜덤
        int idx = rng.rangeInt(0, (int)encounters.size());

        return *encounters[idx];
    }

    const State& state() const { return s; }

private:
    State s;
    Rng rng;
    

    vector<unique_ptr<Encounter>> encounters; // Event/Battle/Shop 풀
    unique_ptr<Encounter> rebelion;
    unique_ptr<Encounter> boss;

    // Boss 등장 stage 기준 (원하는 대로)
    static constexpr int BOSS_STAGE = 20;

    
};

// ------------ main ------------
static void printState(const State& s) {
    cout << "State: HP=" << s.hp
         << " Scrap=" << s.scrap
         << " Power=" << s.power
         << " Fuel=" << s.fuel
         << " Stage=" << s.stage
         << " Victory=" << (s.isVictory ? "true" : "false")
         << " GameOver=" << (s.isGameOver ? "true" : "false")
         << "\n";
}

int main() 
{
while(true)
{
        cout << "=== FTL-like Game Simulation ===\n";
    cout << "Select mode:\n";
    cout << "  [0] Train Agent (Q-learning)\n";
    cout << "  [1] Test Agent\n";
    cout << "  [2] Human Play\n";
    cout << "  [3] Exit\n";

    cout << "Enter mode number: ";
    int mode;
    while (true) {
        if (cin >> mode && 0 <= mode && mode <= 3) {
            break;
        }
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input. Try again: ";
    }

    cout << "=== Run Start ===\n";
    if(mode == 3) {
        cout << "Exiting program.\n";
        return 0;
    }
    // Human Play 모드
    if(mode == 2)
    {
        int score = 0;
        Game game(time(nullptr)); // 시드로 현재 시간 사용
        game.reset();

        printState(game.state());

        // 안전장치: 무한 루프 방지
        const int MAX_TURNS = 100;
        
        for (t = 1; t <= MAX_TURNS; ++t) {
            auto before = game.state();

            StepResult r = game.stepOneTurn();
            const State& after = game.state();

            // 로그 출력
            //cout << "\nTurn " << t << " Encounter=" << toString(r.encounterType)
            //     << " Action=" << r.action << "\n";
            cout << "\n\n"; 
            printState(after);
            
            if (after.isVictory) {
                cout << "\n=== VICTORY! ===\n";
                score += VICTORY_REWARD; // 승리 보상
                score += additionalReward(after);
                cout << "Final Score: " << score << "\n";
                break;
            }
            if (after.isGameOver) {
                cout << "\n=== GAME OVER ===\n";
                score -= GAME_OVER_PENALTY; // 게임 오버 패널티
                score += additionalReward(after); // 게임 오버 시에도 추가 보상/패널티 반영
                cout << "Final Score: " << score << "\n";
                break;
            }
        }
    }
    //train agent mode
    else if(mode == 0)
    {
        Game game(time(nullptr)); // 시드로 현재 시간 사용. // 우선 전체 훈련에서 시드를 고정한다. 
        
        //훈련 횟수 설정 
        //체크포인트 빈도수 설정 
        //훈련 시작 
        //체크포인트마다 Q-table 저장
        //반복 100번 완료마다 로그 출력

        int iterations = 1000;
        int checkpointInterval = 500;

        cout << "Enter number of training iterations: ";
        cin >> iterations;

        cout << "Enter checkpoint interval: ";
        cin >> checkpointInterval;

        agent a;

        std::time_t t = std::time(nullptr);
        std::tm* tm = std::localtime(&t);

        char buf[20];
        std::strftime(buf, sizeof(buf), "%m%d_%H%M%S", tm);

        std::filesystem::create_directory(buf);
        
        for(int i=0; i<iterations+1; i++)
        {
            
            const Encounter * lastEncounterType; // 마지막으로 만난 인카운터 타입 저장
            const Encounter * nextEncounterType; // 다음 턴에 만날 인카운터 타입 저장
            int gameOverCount = 0;

            if(i != 0 && i % checkpointInterval == 0) {
                cout << "Iteration: " << i << ", Game Over Count: " << gameOverCount << "\n";
            }
            if(i % checkpointInterval == 0) {
                saveQTable(a, string(buf) + "/" + "qtable_checkpoint_" + std::to_string(i) + ".txt");
            }
            game.reset();
            // 안전장치: 무한 루프 방지
            const int MAX_TURNS = 100;
            
            lastEncounterType = &game.pickEncounter(); // 초기 인카운터 타입 설정
            for (t = 1; t <= MAX_TURNS; ++t) 
            {
                auto before = game.state();  //현재 상태 저장 
                visitCount[a.getStateIndex(before)][a.getEncounterIndex(lastEncounterType->type())]++; // 방문 횟수 업데이트

                StepResult r = game.greedyStepOneTurn(a, *lastEncounterType); //행동 선택 
                const State& after = game.state(); //행동의 결과로 나온 다음 상태 저장
                nextEncounterType = &game.pickEncounter(); // 다음 인카운터 타입 설정
                
                float reward = 0; 

                if (after.isVictory) 
                {
                    //cout << "\n=== VICTORY! ===\n";
                    reward += VICTORY_REWARD; // 승리 보상
                    reward += additionalReward(after);
                }
                else if(after.isGameOver) 
                {
                    //cout << "\n=== GAME OVER ===\n";
                    reward -= GAME_OVER_PENALTY; // 게임 오버 패널티
                    reward += additionalReward(after); // 게임 오버 시에도 추가 보상/패널티 반영
                    gameOverCount++;
                }
                else
                {
                    reward -= GAME_STEP_PENALTY; //턴마다 패널티
                }
                a.learn(before, (*lastEncounterType).type(), r.action, reward, after, (*nextEncounterType).type()); //에이전트 학습
                lastEncounterType = nextEncounterType; // 다음 턴을 위해 마지막 인카운터 업데이트
                if(after.isVictory || after.isGameOver) 
                {
                    break; // 게임 종료 시 루프 탈출
                }   
            }
        }
        int moreThan10Count = 0;
        int moreThan100Count = 0;

        cout << "=== Training Complete ===\n";
        for(int state = 0; state < STATE_NUMBER; state++) {
            for(int encounter = 0; encounter < ENCOUNTER_NUMBER; encounter++) {
                if(visitCount[state][encounter] > 10) {
                    moreThan10Count++;
                }
                if(visitCount[state][encounter] > 100) {
                    moreThan100Count++;
                }
            }
        }
        cout << "visit more than 10 times: " << moreThan10Count << "\n";
        cout << "visit more than 100 times: " << moreThan100Count << "\n";
    }

    //test agent mode 
    else if(mode == 1)
    {
        int score = 0;
        string dirname;
        string filename;
        //select Q-table file
        while (true)
        {
            
            cout << "Enter Q-table directory name: ";
            cin >> dirname;
            int iteration;
            cout << "Enter agent iteration version to load: ";
            cin >> iteration;
            filename = "qtable_checkpoint_" + std::to_string(iteration) + ".txt";

            bool fileExists = std::filesystem::exists(dirname + "/" + filename);

            if (fileExists)
            {
                break; // 성공하면 탈출
            }
            else
            {
                std::cerr << "Error opening file: " << filename << "\n";
                std::cerr << "Try again.\n";
            }
        }
        
        agent a;
        loadQTable(a, dirname + "/" + filename);

        Game game(time(nullptr)); // 시드로 현재 시간 사용
        game.reset();

        printState(game.state());

        // 안전장치: 무한 루프 방지
        const int MAX_TURNS = 100;
        
        for (t = 1; t <= MAX_TURNS; ++t) {
            auto before = game.state();

            StepResult r = game.agentStepOneTurn(a);
            const State& after = game.state();

            // 로그 출력
            //cout << "\nTurn " << t << " Encounter=" << toString(r.encounterType)
            //     << " Action=" << r.action << "\n";
            cout << "\n\n"; 
            printState(after);

            if (after.isVictory) {
                cout << "\n=== VICTORY! ===\n";
                score += VICTORY_REWARD; // 승리 보상
                score += additionalReward(after);
                cout << "Final Score: " << score << "\n";
                break;
            }
            if (after.isGameOver) {
                cout << "\n=== GAME OVER ===\n";
                score -= GAME_OVER_PENALTY; // 게임 오버 패널티
                score += additionalReward(after); // 게임 오버 시에도 추가 보상/패널티 반영
                cout << "Final Score: " << score << "\n";
                break;
            }
        }
    }
    cout << "\n=== Run End ===\n";
}   
}

float additionalReward(const State& s) 
{
    // 추가 보상 계산 (예시: 스테이지가 높을수록 보상 증가)
    
    float extra = 0;
    /*
    게임 종료 시 가산점 

    hp30이상 +1
    hp40이상 +2 
    hp50이상 +3
    hp60이상 +4
    hp70이상 +5

    scrap15이상 +1
    scrap30이상 +2
    scrap45이상 +3
    scrap60이상 +4

    fuel1~3 +1
    fuel4~6 +2
    fuel7~9 +3
    fuel10+ +4

    power
    diff >=10  +1
    diff >=15  +2
    diff >=20  +3
    diff >= 25  +4
    diff >= 30  +5
    */

    if(s.hp >= 30) extra += 1;
    if(s.hp >= 40) extra += 1;
    if(s.hp >= 50) extra += 1;
    if(s.hp >= 60) extra += 1;
    if(s.hp >= 70) extra += 1;

    if(s.scrap >= 15) extra += 1;
    if(s.scrap >= 30) extra += 1;
    if(s.scrap >= 45) extra += 1;
    if(s.scrap >= 60) extra += 1;

    if(s.fuel >= 1 && s.fuel <= 3) extra += 1;
    if(s.fuel >= 4 && s.fuel <= 6) extra += 2;
    if(s.fuel >= 7 && s.fuel <= 9) extra += 3;
    if(s.fuel >= 10) extra += 4;

    float enemyStrength = ENEMY_BASE_POWER + ENEMY_POWER_SCALE * ((s.stage-1) / 3);
    float diff = s.power - enemyStrength;
    if(diff >= 10) extra += 1;
    if(diff >= 15) extra += 1;
    if(diff >= 20) extra += 1;
    if(diff >= 25) extra += 1;
    if(diff >= 30) extra += 1;  

    return extra;
}

void saveQTable(const agent& a, const string& filename) 
{
    ofstream outFile(filename);
    if (!outFile) 
    {
        cerr << "Error opening file for writing: " << filename << "\n";
        return;
    }

    for (int state = 0; state < STATE_NUMBER; ++state) 
    {
        outFile << "State " << state << "\n";

        for (int encounter = 0; encounter < ENCOUNTER_NUMBER; ++encounter) 
        {
            outFile << "  Encounter " << encounter << ": ";

            for (int action = 0; action < ACTION_NUMBER; ++action) {
                outFile << a.qTable[state][encounter][action] << " ";
            }

            outFile << "\n";
        }

        outFile << "\n"; // state 구분
    }
}
#include <fstream>
#include <iostream>
#include <string>

void loadQTable(agent& a, const std::string& filename)
{
    std::ifstream inFile(filename);
    if (!inFile)
    {
        std::cerr << "Error opening file for reading: " << filename << "\n";
        return;
    }

    std::string word;

    for (int state = 0; state < STATE_NUMBER; ++state)
    {
        // "State" 0 읽기
        inFile >> word;   // State
        if (word != "State")
        {
            std::cerr << "Format error: expected 'State'\n";
            return;
        }

        int savedState;
        inFile >> savedState;

        for (int encounter = 0; encounter < ENCOUNTER_NUMBER; ++encounter)
        {
            // "Encounter" encounter ":" 읽기
            inFile >> word;   // Encounter
            if (word != "Encounter")
            {
                std::cerr << "Format error: expected 'Encounter'\n";
                return;
            }

            int savedEncounter;
            inFile >> savedEncounter;

            char colon;
            inFile >> colon;  // :
            if (colon != ':')
            {
                std::cerr << "Format error: expected ':'\n";
                return;
            }

            for (int action = 0; action < ACTION_NUMBER; ++action)
            {
                inFile >> a.qTable[state][encounter][action];
                if (!inFile)
                {
                    std::cerr << "Format error while reading qTable data\n";
                    return;
                }
            }
        }
    }
}
