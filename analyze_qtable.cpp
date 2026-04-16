#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>
#include <cstdio>

int main() {
    const int HP_BUCKET = 9;
    const int SCRAP_BUCKET = 6;
    const int POWER_BUCKET = 9;
    const int FUEL_BUCKET = 7;
    const int STAGE_BUCKET = 4;
    const int BOMB_BUCKET = 4;
    
    const int TOTAL_STATES = HP_BUCKET * SCRAP_BUCKET * POWER_BUCKET * 
                             FUEL_BUCKET * STAGE_BUCKET * BOMB_BUCKET;
    
    auto decode_state = [](int idx, int& hp, int& scrap, int& power, 
                          int& fuel, int& stage, int& bomb) {
        bomb = idx % BOMB_BUCKET;
        idx /= BOMB_BUCKET;
        stage = idx % STAGE_BUCKET;
        idx /= STAGE_BUCKET;
        fuel = idx % FUEL_BUCKET;
        idx /= FUEL_BUCKET;
        power = idx % POWER_BUCKET;
        idx /= POWER_BUCKET;
        scrap = idx % SCRAP_BUCKET;
        hp = idx / SCRAP_BUCKET;
    };
    
    std::ifstream file("0411_023649\\qtable_checkpoint_5440000000.txt");
    if (!file.is_open()) {
        std::cerr << "File not found!" << std::endl;
        return 1;
    }
    
    long long total_q_values = 0;
    long long zero_q_values = 0;
    long long reachable_q_values = 0;
    long long reachable_zero_q_values = 0;
    
    std::string line;
    int current_state = -1;
    
    while (std::getline(file, line)) {
        // State 라인 파싱
        if (line.find("State ") == 0) {
            sscanf(line.c_str(), "State %d", &current_state);
            continue;
        }
        
        // Encounter 라인 파싱
        if (line.find("  Encounter ") != std::string::npos && current_state >= 0) {
            float v1, v2, v3;
            int encounter_id;
            sscanf(line.c_str(), "  Encounter %d: %f %f %f", &encounter_id, &v1, &v2, &v3);
            
            // 상태 디코딩
            int hp, scrap, power, fuel, stage, bomb;
            decode_state(current_state, hp, scrap, power, fuel, stage, bomb);
            
            // 도달 불가능 판별
            bool is_unreachable = (bomb >= 2) || (stage == 3);  // bomb 2,3 또는 stage=20
            
            std::vector<float> values = {v1, v2, v3};
            for (float val : values) {
                total_q_values++;
                if (val == 0.0f) {
                    zero_q_values++;
                }
                
                if (!is_unreachable) {
                    reachable_q_values++;
                    if (val == 0.0f) {
                        reachable_zero_q_values++;
                    }
                }
            }
        }
    }
    
    file.close();
    
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "Q-테이블 분석 결과" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    
    std::cout << std::fixed << std::setprecision(2);
    
    std::cout << "\n【 전체 Q-값 】" << std::endl;
    std::cout << "  총 Q-값: " << total_q_values << std::endl;
    std::cout << "  0인 Q-값: " << zero_q_values << " (" 
              << (double)zero_q_values / total_q_values * 100 << "%)" << std::endl;
    std::cout << "  탐색된 Q-값: " << (total_q_values - zero_q_values) << std::endl;
    std::cout << "  탐색 비율: " << (double)(total_q_values - zero_q_values) / total_q_values * 100 
              << "%" << std::endl;
    
    std::cout << "\n【 도달 가능한 상태만 (bomb ≤ 1, stage < 20) 】" << std::endl;
    std::cout << "  총 Q-값: " << reachable_q_values << std::endl;
    std::cout << "  0인 Q-값 (탐색 안 함): " << reachable_zero_q_values << " (" 
              << (double)reachable_zero_q_values / reachable_q_values * 100 << "%)" << std::endl;
    std::cout << "  탐색된 Q-값: " << (reachable_q_values - reachable_zero_q_values) << " (" 
              << (double)(reachable_q_values - reachable_zero_q_values) / reachable_q_values * 100 << "%)" << std::endl;
    std::cout << "  미탐색 비율: " << (double)reachable_zero_q_values / reachable_q_values * 100 
              << "%" << std::endl;
    
    return 0;
}
