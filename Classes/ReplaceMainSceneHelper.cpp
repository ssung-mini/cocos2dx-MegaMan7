#include "ReplaceMainSceneHelper.h"
#include "MainScene.h"

USING_NS_CC;

cocos2d::Scene* ReplaceMainSceneHelper::createScene()
{
	return ReplaceMainSceneHelper::create();
}

bool ReplaceMainSceneHelper::init()
{
	if (!Scene::init()) return false;
	auto wlayer = LayerColor::create(Color4B::BLACK);
	this->addChild(wlayer);

	return true;
}

void ReplaceMainSceneHelper::onEnter()
{
	Scene::onEnter();
	makeSequence();
}

void ReplaceMainSceneHelper::onExit()
{
	Scene::onExit();
}



void ReplaceMainSceneHelper::makeSequence()
{
	auto node = Sprite::create();

	auto callback = CallFunc::create(this, callfunc_selector(ReplaceMainSceneHelper::restartFunc));
	auto seq = Sequence::create(DelayTime::create(2.1f), callback, nullptr);

	this->addChild(node);
	node->runAction(seq);
}

void ReplaceMainSceneHelper::restartFunc()
{
	_director->replaceScene(TransitionFade::create(1.0f, MainScene::createScene()));
}