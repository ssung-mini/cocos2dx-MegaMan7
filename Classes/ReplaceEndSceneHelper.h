#pragma once
#include "cocos2d.h"

class ReplaceEndSceneHelper : public cocos2d::Scene {
public:
	static cocos2d::Scene* createScene();
	virtual bool init();

	CREATE_FUNC(ReplaceEndSceneHelper);

	void onEnter();
	void onExit();
	void makeSequence();
	void restartFunc();
};