#include "RestartHelper.h"
#include "MainScene.h"

USING_NS_CC;

cocos2d::Scene* RestartHelper::createScene()
{
	return RestartHelper::create();
}

bool RestartHelper::init()
{
	if (!Scene::init()) return false;
	auto wlayer = LayerColor::create(Color4B::BLACK);
	this->addChild(wlayer);

	return true;
}

void RestartHelper::onEnter()
{
	Scene::onEnter();
	makeSequence();
}

void RestartHelper::onExit()
{
	Scene::onExit();
}



void RestartHelper::makeSequence()
{
	auto node = Sprite::create();

	auto callback = CallFunc::create(this, callfunc_selector(RestartHelper::restartFunc));
	auto seq = Sequence::create(DelayTime::create(2.1f), callback, nullptr);

	this->addChild(node);
	node->runAction(seq);
}

void RestartHelper::restartFunc()
{
	_director->replaceScene(TransitionFade::create(1.0f, MainScene::createScene()));
}





