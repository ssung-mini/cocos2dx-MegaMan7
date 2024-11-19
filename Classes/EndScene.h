#pragma once
#include "cocos2d.h"

class EndScene : public cocos2d::Scene {
public:
	static cocos2d::Scene* createScene();
	virtual bool init();

	CREATE_FUNC(EndScene);

	

	cocos2d::Sprite* titleSprite;
	cocos2d::Sprite* animationSprite;
};