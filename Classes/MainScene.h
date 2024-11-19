#pragma once
#include "cocos2d.h"
#include "Enemy_Metall.h"
#include "Enemy_Bunby.h"
#include "Enemy_Boss.h"
#include "Core.h"
#include "Utils.h"
#include <unordered_map>
#include <cmath>
#include "RestartHelper.h";
#include "SoundManager.h"


enum _moveState {
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

enum _attackState {
	ATK_PLACEHOLDER,
	DEFAULT_POSE,
	STOP_SHOOT,
	RUN_SHOOT,
	JUMP_SHOOT,
	FALL_SHOOT
};

enum _bullets {
	COMMON_BULLET,
	CHARGE_X1,
	CHARGE_X2
};



class MainScene : public cocos2d::Scene {
public:
	static cocos2d::Scene* createScene();
	virtual bool init();

	CREATE_FUNC(MainScene);

	void onEnter();
	void onExit();

	void update(float dt) override;

	void onKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);
	void onKeyReleased(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);

	void moveAnimState(_moveState state);
	void attackAnimState(_attackState state);
	void bulletParticleAnimState(_bullets state, float x, float y);
	void readyLogoAnim();

	cocos2d::Vec2 getPlayerWorldPos();
	
	std::string bgmPath = "Megaman/BGM/MainScene.mp3";
	float nowVolume = 0.5f;
	bool isRestartScene = false;

	std::unordered_map<size_t, cocos2d::Sprite*> _BossMoveCollider;
	std::unordered_map<size_t, cocos2d::Sprite*> _BladeBoundaryCollider;

private:
	cocos2d::LayerColor* wlayer;

	cocos2d::Sprite* EmptyHealthBar;
	cocos2d::Sprite* HealthBar;

	float playerHealth = 28.0f;

	bool isInited;
	cocos2d::PhysicsWorld* m_world;

	void createMyPhysicsWorld();

	void turnOnMoveSprite();
	void turnOffMoveSprite();
	void setReadyGame();
	void hideReadyLogo();
	void setOffStuck();
	void playTeleportSound();
	void playerDie();

	void pauseGame();
	void resumeGame();
	void restartGame();

	void flippedCharacter(bool state);

	void startBossStage();
	void endGame();
	void setTeleportVelocity();
	void replaceEndScene();

	cocos2d::Sprite* charLayer;
	cocos2d::Sprite* character;
	cocos2d::Sprite* attackChar;
	cocos2d::Sprite* jumpCheckBox;
	cocos2d::PhysicsBody* characterBody;
	cocos2d::Sprite* readyLogoSprite;
	cocos2d::EventListenerKeyboard* _listener;
	int _keyCode;

	bool readyGame = false;
	bool hideLogo = false;
	bool CollideEnemy = false;
	bool CollideEnemyBullet = false;
	bool isStuck = false;
	bool startStuckTimer = false;
	bool isDead = false;
	bool startPauseTimer = false;
	bool isChangeMoveSprite = false;

	float changeTime = 0;
	float atkChargeTime = 0;
	float jumpChargeTime = 0;
	float stuckTime = 0;
	float pauseTime = 0;

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
	int contactingDamage = 0;

	float followX = 0.0f;
	float followWidth = 1024.0f;
	
	bool isTriggingFirst = false;
	bool isTriggedFirst = false;
	bool isTriggingSecond = false;
	bool isTriggedSecond = false;

	bool cutsceneBoss = false;
	bool readyBossStage = false;

	bool isTriggingThird = false;
	bool isTriggedThird = false;

	bool isTeleport = false;

	bool x1Ready = false;
	bool x2Ready = false;
		
	_moveState moveInputState;
	_moveState moveBeforeState;

	_attackState attackInputState;
	_attackState attackBeforeState;

	_bullets chargeState;

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
	void createBullet(_bullets state);
	
	cocos2d::GLProgram* glProg;
	cocos2d::GLProgramState* glProgramState;

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::unordered_map<size_t, Enemy_Metall*> metalls;
	std::unordered_map<size_t, Enemy_Bunby*> bunbys;
	Enemy_Boss* madGrinder;

	int metallsNum;
	int bunbysNum;

	SoundManager* soundManager = new SoundManager();
};