#include "EndScene.h"

USING_NS_CC;

cocos2d::Scene* EndScene::createScene()
{
	return EndScene::create();
}

bool EndScene::init()
{
	if (!Scene::init()) return false;

	titleSprite = Sprite::create("Megaman/UI/EndScene.png");
	titleSprite->setAnchorPoint(Vec2::ZERO);
	titleSprite->getTexture()->setAliasTexParameters();

	animationSprite = Sprite::create();
	animationSprite->setPosition(Vec2(128.0f, 155.0f));
	animationSprite->setScale(0.8f);

	int allSheetNum = 70;
	std::string sPath = "Megaman/Animation/Megaman_Idle.plist";
	std::string sName = "idle_";
	float frameDelay = 0.10f;

	auto cache = SpriteFrameCache::getInstance();
	cache->addSpriteFramesWithFile(sPath);

	// cocos2d::Vector
	Vector<SpriteFrame*> animFrames;

	for (int i = 0; i < allSheetNum; i++) {

		// StringUtils::format => ������ �������� ���ڿ��� ����
		std::string _frames = StringUtils::format("%s%02d.png", sName.c_str(), i);


		// ������ ���ڿ��� �̿��Ͽ� plist ������ SpriteFrame ������ ������
		SpriteFrame* frame = cache->getSpriteFrameByName(_frames);
		//frame->setAnchorPoint(Vec2(0, 0));
		frame->getTexture()->setAliasTexParameters();
		// ������ SpriteFrame�� ����
		animFrames.pushBack(frame);
	}

	// plist ������� ���� SpriteFrame ������ Ȱ���Ͽ� �ν��Ͻ� ����

	auto animation = Animation::createWithSpriteFrames(animFrames, frameDelay);
	auto animate = Animate::create(animation);
	auto runRep = RepeatForever::create(animate);
	
	animationSprite->runAction(runRep);
	
	this->addChild(titleSprite);
	this->addChild(animationSprite);

	return true;
}
