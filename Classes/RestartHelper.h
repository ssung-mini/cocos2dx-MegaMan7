#pragma once
#include "cocos2d.h"

class RestartHelper : public cocos2d::Scene {
public:
	static cocos2d::Scene* createScene();
	virtual bool init();

	CREATE_FUNC(RestartHelper);

	void onEnter();
	void onExit();
	void makeSequence();
	void restartFunc();
};