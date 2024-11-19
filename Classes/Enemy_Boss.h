#pragma once
#include "cocos2d.h"
#include "Core.h"
#include "Utils.h"
#include <iostream>
#include <random>
#include "SoundManager.h"

enum _bossState {
	BOSS_PLACEHOLDER,
	BOSS_INIT,
	BOSS_IDLE,
	BOSS_FORWARD,
	BOSS_BACKWARD,
	BOSS_ATTACK,
	BOSS_RETURN_BLADE,
	BOSS_JUMP,
	BOSS_DEAD
};

class Enemy_Boss : public cocos2d::Node {
public:
	Enemy_Boss::Enemy_Boss(cocos2d::Layer* layer, float posX, float posY);

	static Enemy_Boss* create(cocos2d::Layer* layer, float posX, float posY);

	[[nodiscard]] bool init() override;
	void pause() override;
	void resume() override;

	void setPlayerPos(cocos2d::PhysicsBody* playerBody);

	void AnimState(_bossState state);
	void update(float dt) override;
	//void checkPlayerPos();
	void appearBoss();
	void setReadySequence();
	void setReadyBool();
	void bossSequence();
	void moveForward();
	void moveBackward();
	void attackBoss();
	void stopBoss();
	void createBlade();
	void returnBlade(cocos2d::PhysicsBody* bladeBody);
	void createSmoke(cocos2d::Sprite* smokeSprite);
	void nextSequence();
	void cameraShaking();
	void die();
	void removeObject();
	void createDeadExplosion();

	cocos2d::Sprite* bossSprite;
	cocos2d::Sprite* bossHitboxSprite;
	cocos2d::Sprite* bossBladeSprite;
	cocos2d::Sprite* bossSmoke;
	cocos2d::Sprite* bossSmoke_1;
	cocos2d::Sprite* bossSmoke_2;
	cocos2d::Sprite* bossSmoke_3;
	cocos2d::Sprite* bossSmoke_4;

	cocos2d::PhysicsBody* bossBody;
	cocos2d::PhysicsBody* bossHitbox;

	_bossState bossState = BOSS_IDLE;
	_bossState bossBeforeState = BOSS_PLACEHOLDER;

	bool isReady = false;
	bool isHit = false;
	bool onShader = false;
	bool isStop = false;
	bool isMoving = false;
	bool isDead = false;
	bool isCameraShaking = false;
	bool stopDeadEffect = false;
	bool isShootBlade = false;

	bool FirstSmoke = false;
	bool SecondSmoke = false;
	bool ThirdSmoke = false;

	float bossHealth;

	int sequenceNum = 1;
	cocos2d::Node* sequenceNode;

private:
	cocos2d::Layer* sceneLayer;
	float spawnX;
	float spawnY;

	cocos2d::PhysicsBody* playerVar;

	cocos2d::DrawNode* _node;

	int bossDir;
	float cameraShakingTimer = 0.0f;
	float explosionTimer = 0.0f;

	cocos2d::GLProgram* boss_glProg;
	cocos2d::GLProgramState* boss_glState;

	std::default_random_engine generator;

	SoundManager* soundManager = new SoundManager();
};