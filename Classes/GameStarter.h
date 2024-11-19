#pragma once
#include "cocos2d.h"

class GameStarter : public cocos2d::Scene {
public:
	static cocos2d::Scene* createScene();
	virtual bool init();

	CREATE_FUNC(GameStarter);

	void onEnter();
	void onExit();
	void makeSequence();
	void restartFunc();
};