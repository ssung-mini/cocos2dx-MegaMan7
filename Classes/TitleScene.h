#pragma once
#include "cocos2d.h"
#include "SoundManager.h"

class TitleScene : public cocos2d::Scene {
public:
	static cocos2d::Scene* createScene();
	virtual bool init();

	CREATE_FUNC(TitleScene);

	void onEnter();
	void onExit();

	void update(float dt) override;

	void onKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);

	cocos2d::Sprite* titleSprite;

	void startTutorialScene();

	std::string bgmPath = "Megaman/BGM/Title.mp3";
	float nowVolume = 0.5f;
	bool isReplaceScene = false;

private:
	cocos2d::EventListenerKeyboard* _listener;
	SoundManager* soundManager = new SoundManager();
	
};