#include "Enemy_Bunby.h"

USING_NS_CC;

Enemy_Bunby::Enemy_Bunby(cocos2d::Layer* layer, float posX, float posY)
{
	this->sceneLayer = layer;
	this->spawnX = posX;
	this->spawnY = posY;
}

Enemy_Bunby* Enemy_Bunby::create(cocos2d::Layer* layer, float posX, float posY)
{
	auto pRet{ new (std::nothrow) Enemy_Bunby(layer, posX, posY) };
	if (pRet && pRet->init()) {
		pRet->autorelease();
	}
	else {
		delete pRet;
		pRet = nullptr;
	}
	return pRet;
}

bool Enemy_Bunby::init()
{
	if (!cocos2d::Node::init()) {
		return false;
	}

	bunbyState = BUNBY_IDLE;
	
	bunbyDir = -1;	// LEFT
	isDeadEnd = false;
	this->isHit = false;
	this->onShader = false;

	bunbyHeadHealth = 4.0f;
	bunbyTankHealth = 8.0f;

	bunbyTankSprite = Sprite::create("Megaman/Enemies/Bunby/bunby_idle.png");
	bunbyTankSprite->setPosition(spawnX, spawnY);
	bunbyTankSprite->getTexture()->setAliasTexParameters();

	bunbyHeadSprite = Sprite::create("Placeholder.png");
	bunbyTankSprite->addChild(bunbyHeadSprite);

	// Main PhysicsBody (Bunby Tank 몸체) 생성
	auto material1 = PhysicsMaterial(1.0f, 0.0f, 0.0f);
	bunbyBody = PhysicsBody::createBox(Size(31, 24), material1, Vec2(0.0f, -6.5f));

	bunbyBody->setRotationEnable(false);
	bunbyBody->setCategoryBitmask(Utils::CreateMask(core::CategoryBits::ENEMY));
	bunbyBody->setCollisionBitmask(Utils::CreateMask(core::CategoryBits::PLATFORM));
	bunbyBody->setContactTestBitmask(Utils::CreateMask(core::CategoryBits::PLAYER
													, core::CategoryBits::PLAYER_PROJECTILE
													, core::CategoryBits::DEADEND));
	bunbyBody->setTag(core::TagIndex::BUNBY_TANK);

	// Bunby Head 생성
	bunbyHead = PhysicsBody::createBox(Size(23, 22), material1, Vec2(33.0f, 48.5f));
	bunbyHead->setDynamic(false);
	bunbyHead->setRotationEnable(false);

	bunbyHead->setCategoryBitmask(Utils::CreateMask(core::CategoryBits::ENEMY));
	bunbyHead->setCollisionBitmask(Utils::CreateMask(0));
	bunbyHead->setContactTestBitmask(Utils::CreateMask(core::CategoryBits::PLAYER
													, core::CategoryBits::PLAYER_PROJECTILE));
	bunbyHead->setTag(core::TagIndex::BUNBY_HEAD);
	
	// Tank를 메인으로 자식 노드 설정
	bunbyTankSprite->setPhysicsBody(bunbyBody);
	bunbyHeadSprite->setPhysicsBody(bunbyHead);

	this->addChild(bunbyTankSprite);

	//////////////////////////////////////////////////////////////////////
	// Take Damage Shader
	GLchar* fragmentSource = (GLchar*)String::createWithContentsOfFile(FileUtils::getInstance()->fullPathForFilename("enemyBright_Shader.fsh").c_str())->getCString();
	// Create a GLProgram with the shader file.
	bunby_glProg = GLProgram::createWithByteArrays(ccPositionTextureColor_noMVP_vert, fragmentSource);
	// Create a GLProgramState from te GLProgram. 
	bunby_glState = GLProgramState::getOrCreateWithGLProgram(bunby_glProg);

	bunby_glState->setUniformInt("brightMode", 0);
	//glProgramState->setUniformTexture("samp_replacements", img1); 
	// Apply the shader to the Sprite.
	bunbyTankSprite->setGLProgramState(bunby_glState);
	//////////////////////////////////////////////////////////////////////

	return true;
}

void Enemy_Bunby::pause()
{
	cocos2d::Node::pause();
}

void Enemy_Bunby::resume()
{
	cocos2d::Node::resume();
}

void Enemy_Bunby::setPlayerPos(cocos2d::PhysicsBody* playerBody)
{
	this->playerVar = playerBody;
}

void Enemy_Bunby::moveAnim()
{
	int allSheetNum = 3;
	std::string sPath = "Megaman/Enemies/Bunby/Enemies_Bunby_Move.plist";
	std::string sName = "bunby_move_";
	float frameDelay = 0.07f;

	auto cache = SpriteFrameCache::getInstance();
	cache->addSpriteFramesWithFile(sPath);

	// cocos2d::Vector
	Vector<SpriteFrame*> animFrames;

	for (int i = 0; i < allSheetNum; i++) {

		// StringUtils::format => 지정한 형식으로 문자열을 생성
		std::string _frames = StringUtils::format("%s%02d.png", sName.c_str(), i);

		// 생성한 문자열을 이용하여 plist 내부의 SpriteFrame 정보를 가져옴
		SpriteFrame* frame = cache->getSpriteFrameByName(_frames);
		frame->getTexture()->setAliasTexParameters();

		animFrames.pushBack(frame);
	}

	// plist 기반으로 만든 SpriteFrame 정보를 활용하여 인스턴스 생성

	auto animation = Animation::createWithSpriteFrames(animFrames, frameDelay);
	auto animate = Animate::create(animation);
	animate->retain();
	auto runRep = RepeatForever::create(animate);
	bunbyTankSprite->runAction(runRep);
}

void Enemy_Bunby::update(float dt)
{
	if (!onShader && isHit) {
		bunby_glState->setUniformInt("brightMode", 1);
		this->onShader = true;
		this->isHit = false;
		soundManager->PlayEnemyHitEffect(soundManager->enemyTakeDamagePath);
	}
	else if (onShader && !isHit) {
		bunby_glState->setUniformInt("brightMode", 0);
		this->onShader = false;
	}

	if ((bunbyHeadHealth <= 0 || bunbyTankHealth <= 0) && !(this->isDead)) {
		this->isDead = true;
		this->bunbyState = BUNBY_IDLE;
		this->die();
	}

	if ((bunbyHeadHealth > 0 && bunbyTankHealth > 0) && !(this->isDead)) {
		if (!this->isDeadEnd)
			this->checkPlayerPos();

		if (this->bunbyState == BUNBY_MOVE) {
			this->bunbyBody->setVelocity(Vec2(this->bunbyDir * 30.0f, 0.0f));
		}

		else if (this->bunbyState == BUNBY_IDLE) {
			this->bunbyBody->setVelocity(Vec2::ZERO);
		}

		if (this->isDeadEnd) {
			deadEndTime += dt;

			if (deadEndTime > 0.75f) {
				this->isDeadEnd = false;
				deadEndTime = 0.0f;
			}
		}
	}
}

void Enemy_Bunby::checkPlayerPos()
{
	auto playerPos = sceneLayer->convertToNodeSpace(playerVar->getPosition());
	auto bunbyPos = sceneLayer->convertToNodeSpace(this->bunbyBody->getPosition());
	auto distance = playerPos.distance(bunbyPos);
	auto distanceX = abs(playerPos.x - bunbyPos.x);

	if (distance <= 130.0f)
		this->moveBunby();

	else if (distance > 130.0f)
		this->stopBunby();

	if (distanceX > 20.0f) {
		if ((playerPos.x <= bunbyPos.x) && this->bunbyDir != -1) {
			this->bunbyDir = -1;
			this->bunbyTankSprite->setFlippedX(false);
		}

		else if ((playerPos.x > bunbyPos.x) && this->bunbyDir != 1) {
			this->bunbyDir = 1;
			this->bunbyTankSprite->setFlippedX(true);
		}
	}
}

void Enemy_Bunby::moveBunby()
{
	if (bunbyState != BUNBY_MOVE) {
		this->bunbyState = BUNBY_MOVE;
		this->moveAnim();
	}
	if (!soundManager->isEnemyEffectRunning())
		soundManager->PlayEnemyEffect(soundManager->bunbyMovePath);
}

void Enemy_Bunby::stopBunby()
{
	if (bunbyState != BUNBY_IDLE) {
		this->bunbyState = BUNBY_IDLE;

		if (this->bunbyTankSprite->isRunning())
			this->bunbyTankSprite->stopAllActions();

		this->bunbyBody->setVelocity(Vec2::ZERO);
		soundManager->StopEnemyEffect();
	}
}

void Enemy_Bunby::contactDeadEnd()
{
	if (this->bunbyTankSprite->isFlippedX() == false) {
		this->bunbyTankSprite->setFlippedX(true);
		this->bunbyDir = this->bunbyDir * (-1);
		//soundManager->PlayEnemyEffect(soundManager->bunbyMovePath);
	}

	else {
		this->bunbyTankSprite->setFlippedX(false);
		this->bunbyDir = this->bunbyDir * (-1);
		//soundManager->PlayEnemyEffect(soundManager->bunbyMovePath);
	}
}

void Enemy_Bunby::die()
{
	int allSheetNum = 6;
	std::string sPath = "Megaman/Particle/Particle_Enemy_Explosion.plist";
	std::string sName = "enemy_explosion_";
	float frameDelay = 0.07f;

	auto cache = SpriteFrameCache::getInstance();
	cache->addSpriteFramesWithFile(sPath);

	Vector<SpriteFrame*> animFrames;

	for (int i = 0; i < allSheetNum; i++) {

		// StringUtils::format => 지정한 형식으로 문자열을 생성
		std::string _frames = StringUtils::format("%s%02d.png", sName.c_str(), i);

		// 생성한 문자열을 이용하여 plist 내부의 SpriteFrame 정보를 가져옴
		SpriteFrame* frame = cache->getSpriteFrameByName(_frames);
		frame->getTexture()->setAliasTexParameters();

		animFrames.pushBack(frame);
	}

	// plist 기반으로 만든 SpriteFrame 정보를 활용하여 인스턴스 생성

	auto animation = Animation::createWithSpriteFrames(animFrames, frameDelay);
	auto animate = Animate::create(animation);

	//auto callback = CallFunc::create(this, callfunc_selector(Enemy_Bunby::removeObject));
	auto seq_body = Sequence::create(animate->clone(), RemoveSelf::create(), nullptr);
	auto seq_head = Sequence::create(animate->clone(), RemoveSelf::create(), nullptr);

	auto explosion_1 = Sprite::create();
	auto explosion_2 = Sprite::create();
	auto bunbyBodyPos = sceneLayer->convertToNodeSpace(this->bunbyBody->getPosition());
	auto bunbyHeadPos = sceneLayer->convertToNodeSpace(this->bunbyHead->getPosition());
	explosion_1->setPosition(bunbyBodyPos.x , bunbyBodyPos.y - 6.5f);
	explosion_2->setPosition(bunbyHeadPos.x + 32.0f, bunbyHeadPos.y + 48.5f);

	explosion_1->runAction(seq_body);
	explosion_2->runAction(seq_head);

	soundManager->StopEnemyEffect();

	sceneLayer->addChild(explosion_1);
	sceneLayer->addChild(explosion_2);
	soundManager->PlayEnemyEffect(soundManager->enemyDeathPath);
	this->removeObject();
}

void Enemy_Bunby::removeObject()
{
	this->removeAllChildren();
}

