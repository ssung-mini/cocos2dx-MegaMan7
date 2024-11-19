#pragma once
#include "cocos2d.h"
#include "Core.h"
#include "Utils.h"
#include "SoundManager.h"

enum _bunbyState {
	BUNBY_PLACEHOLDER,
	BUNBY_MOVE,
	BUNBY_IDLE
};

class Enemy_Bunby : public cocos2d::Node {
public:
	Enemy_Bunby::Enemy_Bunby(cocos2d::Layer* layer, float posX, float posY);

	static Enemy_Bunby* create(cocos2d::Layer* layer, float posX, float posY);

	[[nodiscard]] bool init() override;
	void pause() override;
	void resume() override;

	void setPlayerPos(cocos2d::PhysicsBody* playerBody);

	void moveAnim();
	void update(float dt) override;
	void checkPlayerPos();
	void moveBunby();
	void stopBunby();
	void contactDeadEnd();
	void die();
	void removeObject();

	cocos2d::Sprite* bunbyTankSprite;
	cocos2d::Sprite* bunbyHeadSprite;
	cocos2d::Sprite* bunbySensor;

	cocos2d::PhysicsBody* bunbyBody;
	cocos2d::PhysicsBody* bunbyHead;

	_bunbyState bunbyState = BUNBY_IDLE;
	//_bunbyState bunbyBeforeState = BUNBY_PLACEHOLDER;

	bool isAwake = false;
	bool canMove = false;
	bool isHit = false;
	bool onShader = false;
	bool innerSensor = false;
	bool isDeadEnd = false;
	float deadEndTime = 0.0f;
	bool isDead = false;

	float bunbyHeadHealth;
	float bunbyTankHealth;

private:
	cocos2d::Layer* sceneLayer;
	float spawnX;
	float spawnY;

	cocos2d::PhysicsBody* playerVar;

	int bunbyDir;
	cocos2d::Vec2 savePlayerPos;

	cocos2d::GLProgram* bunby_glProg;
	cocos2d::GLProgramState* bunby_glState;

	SoundManager* soundManager = new SoundManager();
};