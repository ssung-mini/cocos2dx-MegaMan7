#include "TitleScene.h"
#include "ReplaceTutorialSceneHelper.h"
#include "AudioEngine.h"

USING_NS_CC;

cocos2d::Scene* TitleScene::createScene()
{
	return TitleScene::create();
}

bool TitleScene::init()
{
	if (!Scene::init()) return false;

	titleSprite = Sprite::create("Megaman/UI/Title.png");
	titleSprite->setAnchorPoint(Vec2::ZERO);
	titleSprite->getTexture()->setAliasTexParameters();

	this->addChild(titleSprite);

	soundManager->initSoundManager(bgmPath);
	soundManager->PlayBGM(bgmPath);

	this->scheduleUpdate();
	return true;
}

void TitleScene::onEnter()
{
	Scene::onEnter();

	_listener = EventListenerKeyboard::create();
	_listener->onKeyPressed = CC_CALLBACK_2(TitleScene::onKeyPressed, this);

	_eventDispatcher->addEventListenerWithSceneGraphPriority(_listener, this);
}

void TitleScene::onExit()
{

	_eventDispatcher->removeEventListener(_listener);
	Scene::onExit();
}

void TitleScene::update(float dt)
{
	if (isReplaceScene)
		soundManager->DecreaseVolume(&nowVolume);
}

void TitleScene::onKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event)
{
	isReplaceScene = true;
	startTutorialScene();
}

void TitleScene::startTutorialScene()
{
	_director->replaceScene(TransitionFade::create(1.0f, ReplaceTutorialSceneHelper::createScene()));
}
