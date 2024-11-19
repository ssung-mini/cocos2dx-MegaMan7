#pragma once
#include "cocos2d.h"

class ReplaceMainSceneHelper : public cocos2d::Scene {
public:
	static cocos2d::Scene* createScene();
	virtual bool init();

	CREATE_FUNC(ReplaceMainSceneHelper);

	void onEnter();
	void onExit();
	void makeSequence();
	void restartFunc();
};