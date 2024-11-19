#pragma once
#include "cocos2d.h"
#include "Core.h"
#include "Utils.h"
#include <unordered_map>
#include <cmath>
#include "ReplaceMainSceneHelper.h"
#include "SoundManager.h"

enum _TmoveState {
	MS_PLACEHOLDER,
	MS_INIT,
	MS_TELEPORT,
	MS_STOP,
	MS_RUN,
	MS_JUMP,
	MS_SLIDING,
	MS_FALL,
	MS_HURT
};

enum _TattackState {
	ATK_PLACEHOLDER,
	DEFAULT_POSE,
	STOP_SHOOT,
	RUN_SHOOT,
	JUMP_SHOOT,
	FALL_SHOOT
};

enum _Tbullets {
	COMMON_BULLET,
	CHARGE_X1,
	CHARGE_X2
};

class TutorialScene : public cocos2d::Scene {
public:
	std::string bgmPath = "Megaman/BGM/Tutorial.mp3";
	
	std::string warpUpPath = "Megaman/SFX/Megaman_Warp_Up.mp3";
	std::string warpDownPath = "Megaman/SFX/Megaman_Warp_Down.mp3";
	std::string jumpPath = "Megaman/SFX/Megaman_Jump.mp3";
	std::string landPath = "Megaman/SFX/Megaman_Land_Ground.mp3";
	std::string slidingPath = "Megaman/SFX/Megaman_Slide.mp3";
	std::string megamanTakeDamagePath = "Megaman/SFX/Megaman_Take_Damage.mp3";
	std::string megamanDeathPath = "Megaman/SFX/Megaman_Death.mp3";

	std::string bulletPath = "Megaman/SFX/Megaman_Common_Shoot.mp3";
	std::string chargeX1Path = "Megaman/SFX/Charge_X1_Shoot.mp3";
	std::string chargeX2Path = "Megaman/SFX/Charge_X2_Shoot.mp3";
	std::string startChargePath = "Megaman/SFX/Charge_Buster_Start.mp3";
	std::string chargingLoopPath = "Megaman/SFX/Charge_Buster_Loop.mp3";


	static cocos2d::Scene* createScene();
	virtual bool init();

	CREATE_FUNC(TutorialScene);

	void onEnter();
	void onExit();

	void update(float dt) override;

	void onKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);
	void onKeyReleased(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);

	void moveAnimState(_TmoveState state);
	void attackAnimState(_TattackState state);
	void bulletParticleAnimState(_Tbullets state, float x, float y);

	void teleportNextScene();
	void setTeleportVelocity();
	void replaceMainScene();

	float nowVolume = 0.5f;
	bool isReplaceScene = false;

private:
	cocos2d::PhysicsWorld* m_world;
	void createMyPhysicsWorld();

	cocos2d::Sprite* EmptyHealthBar;
	cocos2d::Sprite* HealthBar;

	float playerHealth = 28.0f;

	bool isInited;

	void turnOnMoveSprite();
	void turnOffMoveSprite();
	void flippedCharacter(bool state);

	cocos2d::Sprite* charLayer;
	cocos2d::Sprite* character;
	cocos2d::Sprite* attackChar;
	cocos2d::Sprite* jumpCheckBox;
	cocos2d::PhysicsBody* characterBody;
	cocos2d::Sprite* readyLogoSprite;
	cocos2d::EventListenerKeyboard* _listener;
	int _keyCode;

	float changeTime = 0;
	float atkChargeTime = 0;
	float jumpChargeTime = 0;
	
	bool fall;
	bool isJumping;
	bool movingLeft;
	bool movingRight;
	bool movingUp;
	bool readySlide;
	bool movingSliding;
	bool inputShoot;
	bool beforeRunShoot;
	int movingDir = 1;	// 0 : Left, 1 : Right
	float slidingClock;
	float slidingTime;
	float slidingSpeed;
	bool isChangeMoveSprite = false;

	bool x1Ready = false;
	bool x2Ready = false;

	bool canTeleport = false;
	bool isTeleport = false;

	_TmoveState moveInputState;
	_TmoveState moveBeforeState;

	_TattackState attackInputState;
	_TattackState attackBeforeState;

	_Tbullets chargeState;

	float atk_stop_pos[1][2] = { 23, 2 };
	float atk_jump_pos[5][2] = { {23, 2}, {24, 9}, {23, 12}, {24, 14}, {24, 13} };
	float atk_fall_pos[5][2] = { {24, 13}, {24, 14}, {23, 12}, {24, 9}, {23, 2} };
	float atk_run_pos[10][2] = { {23, 2}, {23, 4}, {23, 4}, {23, 2}, {23, 2}, {23, 1}, {24, 1}, {24, 3}, {24, 1}, {24, 1} };

	float explosionPos[16][2] = { {210.0f, 0.0f}, {0.0f, 210.0f}, {-210.0f, 0.0f}, {0.0f, -210.0f}, {145.4f, 145.4f}, {145.4f, -145.4f}, {-145.4f,145.4f}, {-145.4f, -145.4f}, {173.2f, 100.0f}, {100.0f, 173.2f}, {-173.2f, 100.0f}, {-100.0f, 173.2f}, {173.2f, -100.0f}, {100.0f, -173.2f}, {-173.2f, -100.0f}, {-100.0f, -173.2f} };

	cocos2d::Vec2 shootPosition;

	// 디버그용 점 그리기
	cocos2d::DrawNode* _node;

	cocos2d::Animate* atkAnimate;

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cocos2d::Size winSize;
	cocos2d::Vec2 spawnPosition;
	cocos2d::TMXTiledMap* tmap;

	cocos2d::TMXLayer* background;
	cocos2d::TMXLayer* portal;
	cocos2d::TMXLayer* block_1;
	cocos2d::TMXLayer* block_2;
	cocos2d::TMXLayer* sceneCollider;

	//cocos2d::Sprite* pBullet;
	//cocos2d::PhysicsBody* bulletPBody;
	cocos2d::Sprite* particleBullet;
	void createBullet(_Tbullets state);

	cocos2d::GLProgram* glProg;
	cocos2d::GLProgramState* glProgramState;

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	SoundManager* soundManager = new SoundManager();
};