// UnitData.h
#ifndef __UNIT_DATA_H__
#define __UNIT_DATA_H__

#include "cocos2d.h"
#include <vector>

// ���幥��ƫ������
enum class TargetPriority {
    ANY = 0,        // ���⽨�� (Ұ����)
    RESOURCE = 1,   // ��Դ���� (�粼��)
    DEFENSE = 2     // ������ (����)
};

// ����ö��
// ע�⣺ֻ��Ҫ���ҷ���(RIGHT)�Ķ�����Դ��������ͨ��ͼƬ��תʵ��
// ֡�����淶��{spriteFrameName}_{anim_key}_{frame_number}.png
// ���磺goblin_walk_1.png, goblin_walk_2.png
enum class Direction {
    LEFT = 0,       // ��
    RIGHT = 1       // ��
};

// ��������,���ڴ洢���ֱ�������
struct UnitConfig {
// ������Ϣ
    int id;                     // ���ֱ����1001
    std::string name;           // ��"Goblin"
    std::string spriteFrameName;// ��"goblin01.png" 
    
// �������� - �޸�Ϊ���ȼ���vector����
    std::vector<float> HP;      // ÿ���ȼ���Ѫ��
    std::vector<float> SPEED;   // ÿ���ȼ����ƶ��ٶ�
    std::vector<float> DP;      // ÿ���ȼ��ķ�����
    std::vector<float> ATK;     // ÿ���ȼ��Ĺ�����
    std::vector<float> RANGE;   // ÿ���ȼ��Ĺ�����Χ
    
    TargetPriority aiType;      // �磺2 - ����ֶξ�������Ǹ粼��
    
    // ��������
    bool ISREMOTE;
    bool ISFLY;

    // ���ȼ�
    int MAXLEVEL;

    // ��Դ����
    int COST_COIN;
    int COST_ELIXIR;
    int COST_POPULATION;
    int TRAIN_TIME; // ѵ��ʱ��,��λ��
    // �������ã��Ӿ���ͼ���ж�ȡ֡��
    // ֡�����淶��{spriteFrameName}_{anim_key}_{frame_number}.png
    // ���磺goblin_walk_1.png, goblin_walk_2.png, ...
    std::string anim_walk = "walk";        // �ƶ������ؼ���
    int anim_walk_frames = 6;              // �ƶ�����֡��
    float anim_walk_delay = 0.08f;         // ÿ֡�ӳ٣��룩
    
    std::string anim_attack = "attack";    // ���������ؼ���
    int anim_attack_frames = 8;            // ��������֡��
    float anim_attack_delay = 0.06f;       // ÿ֡�ӳ٣��룩
    
    std::string anim_idle = "idle";        // ���������ؼ��֣���ѡ��
    int anim_idle_frames = 4;              // ��������֡��
    float anim_idle_delay = 0.15f;         // ÿ֡�ӳ٣��룩

    std::string anim_dead = "dead";
    int anim_dead_frames = 4;              // ��������֡��
    float anim_dead_delay = 0.06f;         // ÿ֡�ӳ٣��룩
};

#endif // __UNIT_DATA_H__