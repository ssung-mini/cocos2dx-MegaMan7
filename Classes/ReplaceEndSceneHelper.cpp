#include "ReplaceEndSceneHelper.h"
#include "EndScene.h"

USING_NS_CC;

cocos2d::Scene* ReplaceEndSceneHelper::createScene()
{
	return ReplaceEndSceneHelper::create();
}

bool ReplaceEndSceneHelper::init()
{
	if (!Scene::init()) return false;
	auto wlayer = LayerColor::create(Color4B::BLACK);
	this->addChild(wlayer);

	return true;
}

void ReplaceEndSceneHelper::onEnter()
{
	Scene::onEnter();
	makeSequence();
}

void ReplaceEndSceneHelper::onExit()
{
	Scene::onExit();
}



void ReplaceEndSceneHelper::makeSequence()
{
	auto node = Sprite::create();

	auto callback = CallFunc::create(this, callfunc_selector(ReplaceEndSceneHelper::restartFunc));
	auto seq = Sequence::create(DelayTime::create(2.1f), callback, nullptr);

	this->addChild(node);
	node->runAction(seq);
}

void ReplaceEndSceneHelper::restartFunc()
{
	_director->replaceScene(TransitionFade::create(1.0f, EndScene::createScene()));
}