#include "GameStarter.h"
#include "TitleScene.h"

USING_NS_CC;

cocos2d::Scene* GameStarter::createScene()
{
	return GameStarter::create();
}

bool GameStarter::init()
{
	if (!Scene::init()) return false;
	auto wlayer = LayerColor::create(Color4B::BLACK);
	this->addChild(wlayer);

	return true;
}

void GameStarter::onEnter()
{
	Scene::onEnter();
	makeSequence();
}

void GameStarter::onExit()
{
	Scene::onExit();
}



void GameStarter::makeSequence()
{
	auto node = Sprite::create();

	auto callback = CallFunc::create(this, callfunc_selector(GameStarter::restartFunc));
	auto seq = Sequence::create(DelayTime::create(1.1f), callback, nullptr);

	this->addChild(node);
	node->runAction(seq);
}

void GameStarter::restartFunc()
{
	_director->replaceScene(TransitionFade::create(1.0f, TitleScene::createScene()));
}