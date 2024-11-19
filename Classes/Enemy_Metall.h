#pragma once
#include "cocos2d.h"
#include "Core.h"
#include "Utils.h"
#include "SoundManager.h"

enum _metallState {
	METALL_PLACEHOLDER,
	METALL_AWAKE,
	METALL_MOVE,
	METALL_ATTACK,
	METALL_SLEEP
};

class Enemy_Metall : public cocos2d::Node {
public:
	Enemy_Metall::Enemy_Metall(cocos2d::Layer* layer, float posX, float posY);

	static Enemy_Metall* create(cocos2d::Layer* layer, float posX, float posY);

	[[nodiscard]] bool init() override;
	void pause() override;
	void resume() override;

	void setPlayerPos(cocos2d::PhysicsBody* playerBody);
	
	void AnimState(_metallState state);
	void update(float dt) override;
	void checkPlayerPos();
	void shootMetallBullet();
	void awakeMetall();
	void sleepMetall();
	void attackMetall();
	void setOnAwakeBool();
	void setOffAwakeBool();
	void moveMetall();
	void stopMetall();
	void contactDeadEnd();
	void die();
	void removeObject();

	cocos2d::Sprite* metallSprite;
	cocos2d::Sprite* metallSensor;

	cocos2d::PhysicsBody* metallBody;
	cocos2d::PhysicsBody* awakeSensor;

	_metallState metallState = METALL_PLACEHOLDER;
	_metallState metallBeforeState = METALL_PLACEHOLDER;

	bool isAwake = false;
	bool canMove = false;
	bool isHit = false;
	bool onShader = false;
	bool innerSensor = false;
	bool isDeadEnd = false;
	float deadEndTime = 0.0f;
	bool isDead = false;
	

	float metallHealth;

private:
	cocos2d::Layer* sceneLayer;
	float spawnX;
	float spawnY;

	cocos2d::PhysicsBody* playerVar;

	int metallDir;

	cocos2d::Vec2 savePlayerPos;

	cocos2d::GLProgram* metall_glProg;
	cocos2d::GLProgramState* metall_glState;

	SoundManager* soundManager = new SoundManager();
};