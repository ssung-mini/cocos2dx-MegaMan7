#include "ReplaceTutorialSceneHelper.h"
#include "TutorialScene.h"

USING_NS_CC;

cocos2d::Scene* ReplaceTutorialSceneHelper::createScene()
{
	return ReplaceTutorialSceneHelper::create();
}

bool ReplaceTutorialSceneHelper::init()
{
	if (!Scene::init()) return false;
	auto wlayer = LayerColor::create(Color4B::BLACK);
	this->addChild(wlayer);

	return true;
}

void ReplaceTutorialSceneHelper::onEnter()
{
	Scene::onEnter();
	makeSequence();
}

void ReplaceTutorialSceneHelper::onExit()
{
	Scene::onExit();
}



void ReplaceTutorialSceneHelper::makeSequence()
{
	auto node = Sprite::create();

	auto callback = CallFunc::create(this, callfunc_selector(ReplaceTutorialSceneHelper::restartFunc));
	auto seq = Sequence::create(DelayTime::create(2.1f), callback, nullptr);

	this->addChild(node);
	node->runAction(seq);
}

void ReplaceTutorialSceneHelper::restartFunc()
{
	_director->replaceScene(TransitionFade::create(1.0f, TutorialScene::createScene()));
}