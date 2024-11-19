#include "Enemy_Metall.h"

USING_NS_CC;

Enemy_Metall::Enemy_Metall(cocos2d::Layer* layer, float posX, float posY)
{
	this->sceneLayer = layer;
	this->spawnX = posX;
	this->spawnY = posY;
}

Enemy_Metall* Enemy_Metall::create(cocos2d::Layer* layer, float posX, float posY)
{
	auto pRet{ new (std::nothrow) Enemy_Metall(layer, posX, posY) };
	if (pRet && pRet->init()) {
		pRet->autorelease();
	}
	else {
		delete pRet;
		pRet = nullptr;
	}
	return pRet;
}

bool Enemy_Metall::init()
{
	if (!cocos2d::Node::init()) {
		return false;
	}

	log("spawn point : %f, %f", spawnX, spawnY);

	this->metallState = METALL_PLACEHOLDER;
	this->metallBeforeState = METALL_PLACEHOLDER;
	this->metallDir = -1;	// LEFT
	this->isAwake = false;
	this->isDeadEnd = false;
	this->isDead = false;
	this->isHit = false;
	this->onShader = false;

	this->metallHealth = 2.0f;

	metallSprite = Sprite::create("Megaman/Enemies/Metall/metall_init.png");
	metallSprite->setPosition(spawnX, spawnY);
	metallSprite->getTexture()->setAliasTexParameters();

	metallSensor = Sprite::create("Placeholder.png");
	//metallSensor->setPosition(spawnX, spawnY);

	metallSprite->addChild(metallSensor);

	// Main PhysicsBody 생성
	auto material1 = PhysicsMaterial(1.0f, 0.0f, 0.0f);
	metallBody = PhysicsBody::createBox(Size(24, 24), material1, Vec2(0.0f, -8.5f));

	metallBody->setRotationEnable(false);
	metallBody->setCategoryBitmask(Utils::CreateMask(core::CategoryBits::ENEMY));
	metallBody->setCollisionBitmask(Utils::CreateMask(core::CategoryBits::PLATFORM));
	metallBody->setContactTestBitmask(Utils::CreateMask(core::CategoryBits::PLAYER
													, core::CategoryBits::PLAYER_PROJECTILE
													, core::CategoryBits::DEADEND));
	metallBody->setTag(core::TagIndex::METALL);

	// Player 접근 감지 센서 생성
	awakeSensor = PhysicsBody::createBox(Size(95, 24), PHYSICSSHAPE_MATERIAL_DEFAULT, Vec2(32.0f, 23.5f));	// 기존 : 0.0f, -8.5f
	awakeSensor->setCategoryBitmask(Utils::CreateMask(core::CategoryBits::ENEMY_AWAKE_SENSOR));
	awakeSensor->setCollisionBitmask(0);
	awakeSensor->setContactTestBitmask(Utils::CreateMask(core::CategoryBits::PLAYER));
	awakeSensor->setTag(core::TagIndex::AWAKE_SENSOR);

	awakeSensor->setDynamic(false);


	metallSprite->setPhysicsBody(metallBody);
	metallSensor->setPhysicsBody(awakeSensor);

	this->addChild(metallSprite);


	//////////////////////////////////////////////////////////////////////
	// Take Damage Shader
	GLchar* metallFragmentSource = (GLchar*)String::createWithContentsOfFile(FileUtils::getInstance()->fullPathForFilename("enemyBright_Shader.fsh").c_str())->getCString();
	// Create a GLProgram with the shader file.
	metall_glProg = GLProgram::createWithByteArrays(ccPositionTextureColor_noMVP_vert, metallFragmentSource);
	// Create a GLProgramState from te GLProgram. 
	metall_glState = GLProgramState::getOrCreateWithGLProgram(metall_glProg);

	metall_glState->setUniformInt("brightMode", 0);
	//glProgramState->setUniformTexture("samp_replacements", img1); 
	// Apply the shader to the Sprite.
	metallSprite->setGLProgramState(metall_glState);
	//////////////////////////////////////////////////////////////////////

	return true;
}

void Enemy_Metall::pause()
{
	cocos2d::Node::pause();
}

void Enemy_Metall::resume()
{
	cocos2d::Node::resume();
}

void Enemy_Metall::setPlayerPos(cocos2d::PhysicsBody* playerBody)
{
	this->playerVar = playerBody;
}



void Enemy_Metall::AnimState(_metallState state)
{
	int allSheetNum;
	std::string sPath;
	std::string sName;
	float frameDelay = 0.075f;

	if (state != metallBeforeState) {

		if (metallBeforeState != METALL_PLACEHOLDER)
			metallSprite->stopAllActions();

		

		switch (state)
		{
		case METALL_AWAKE:
			allSheetNum = 6;
			sPath = "Megaman/Enemies/Metall/Enemies_Metall_Awake.plist";
			sName = "metall_awake_";
			frameDelay = 0.07f;
			break;

		case METALL_MOVE:
			allSheetNum = 6;
			sPath = "Megaman/Enemies/Metall/Enemies_Metall_Move.plist";
			sName = "metall_move_";
			frameDelay = 0.085f;
			break;

		case METALL_ATTACK:
			allSheetNum = 3;
			sPath = "Megaman/Enemies/Metall/Enemies_Metall_Attack.plist";
			sName = "metall_attack_";
			frameDelay = 0.07f;
			break;

		case METALL_SLEEP:
			allSheetNum = 6;
			sPath = "Megaman/Enemies/Metall/Enemies_Metall_Sleep.plist";
			sName = "metall_sleep_";
			frameDelay = 0.07f;
			break;
		}



		/////////////////////////////////////////////////////////////////////////////////

		// 외부 프로그램을 사용하여 만든 plist 파일 불러오기
		auto cache = SpriteFrameCache::getInstance();
		cache->addSpriteFramesWithFile(sPath);

		// cocos2d::Vector
		Vector<SpriteFrame*> animFrames;

		for (int i = 0; i < allSheetNum; i++) {

			// StringUtils::format => 지정한 형식으로 문자열을 생성
			std::string _frames = StringUtils::format("%s%02d.png", sName.c_str(), i);


			// 생성한 문자열을 이용하여 plist 내부의 SpriteFrame 정보를 가져옴
			SpriteFrame* frame = cache->getSpriteFrameByName(_frames);
			//frame->setAnchorPoint(Vec2(0, 0));
			frame->getTexture()->setAliasTexParameters();
			// 선별한 SpriteFrame을 삽입
			animFrames.pushBack(frame);
		}

		// plist 기반으로 만든 SpriteFrame 정보를 활용하여 인스턴스 생성

		auto animation = Animation::createWithSpriteFrames(animFrames, frameDelay);
		auto animate = Animate::create(animation);
		//auto runRep = RepeatForever::create(animate);
		if (state == METALL_MOVE) {
			auto runRep = RepeatForever::create(animate);
			metallSprite->runAction(runRep);
		}
		else if (state == METALL_ATTACK) {
			auto callback1 = CallFunc::create(this, callfunc_selector(Enemy_Metall::shootMetallBullet));
			auto callback2 = CallFunc::create(this, callfunc_selector(Enemy_Metall::sleepMetall));
			auto seq = Sequence::create(callback1, animate, callback2, nullptr);
			metallSprite->runAction(seq);
		}
		else if (state == METALL_AWAKE) {
			auto callback = CallFunc::create(this, callfunc_selector(Enemy_Metall::setOnAwakeBool));
			auto seq = Sequence::create(animate, callback, nullptr);
			metallSprite->runAction(seq);
		}

		else if (state == METALL_SLEEP) {
			auto callback = CallFunc::create(this, callfunc_selector(Enemy_Metall::setOffAwakeBool));
			auto seq = Sequence::create(animate, callback, DelayTime::create(1.0f), nullptr);
			metallSprite->runAction(seq);
		}
		/////////////////////////////////////////////////////////////////////////////////
	}

	this->metallBeforeState = state;
}

void Enemy_Metall::update(float dt)
{
	if (!onShader && isHit) {
		metall_glState->setUniformInt("brightMode", 1);
		this->onShader = true;
		this->isHit = false;
		soundManager->PlayEnemyHitEffect(soundManager->enemyTakeDamagePath);
	}
	else if (onShader && !isHit) {
		metall_glState->setUniformInt("brightMode", 0);
		this->onShader = false;
	}

	if (metallHealth <= 0 && !isDead) {
		this->die();
		this->isDead = true;
	}

	if (metallHealth > 0 && !isDead) {
		if (!this->isDeadEnd)
			this->checkPlayerPos();

		if (isAwake && canMove) {

			if (metallState != METALL_MOVE) {
				this->metallState = METALL_MOVE;
				this->AnimState(metallState);
			}

			if (this->isDeadEnd) {
				deadEndTime += dt;

				if (deadEndTime > 1.25f) {
					this->isDeadEnd = false;
					deadEndTime = 0.0f;
				}
			}
			this->moveMetall();
		}

		else {
			this->stopMetall();
		}
	}
}

void Enemy_Metall::checkPlayerPos()
{
	auto playerPos = sceneLayer->convertToNodeSpace(playerVar->getPosition());
	auto metallPos = sceneLayer->convertToNodeSpace(this->metallBody->getPosition());
	auto distance = playerPos.distance(metallPos);
	if (!isAwake && !canMove) {
		if (distance <= 95.0f)
			this->awakeMetall();
	}

	else if (isAwake && canMove) {
		if (distance > 95.0f || this->innerSensor)
			this->attackMetall();
	}

	if ((playerPos.x <= metallPos.x) && this->metallDir != -1) {
		this->metallDir = -1;
		this->metallSprite->setFlippedX(false);
	}

	else if ((playerPos.x > metallPos.x) && this->metallDir != 1) {
		this->metallDir = 1;
		this->metallSprite->setFlippedX(true);
	}
		
	
}

void Enemy_Metall::shootMetallBullet()
{
	auto metallPosition = sceneLayer->convertToNodeSpace(this->metallBody->getPosition());
	auto shootPosition = Vec2((metallPosition.x - (metallDir * -10.0f)), (metallPosition.y - 10.0f));

	auto pBullet = Sprite::create("Megaman/Enemies/Metall/metall_bullet.png");
	auto bulletPBody = PhysicsBody::createBox(Size(10, 9), PHYSICSBODY_MATERIAL_DEFAULT);
	bulletPBody->setGravityEnable(false);
	pBullet->setPosition(shootPosition);
	bulletPBody->setTag(core::TagIndex::METALL_BULLET);

	bulletPBody->setCategoryBitmask(Utils::CreateMask(core::CategoryBits::ENEMY_PROJECTILE));
	bulletPBody->setCollisionBitmask(0);
	bulletPBody->setContactTestBitmask(Utils::CreateMask(core::CategoryBits::PLATFORM
														, core::CategoryBits::BOUNDARY
														, core::CategoryBits::PLAYER));

	pBullet->setPhysicsBody(bulletPBody);

	sceneLayer->addChild(pBullet);
	soundManager->PlayEnemyEffect(soundManager->metallBulletPath);

	if (this->metallSprite->isFlippedX() == false) {
		pBullet->setFlippedX(false);
		bulletPBody->setVelocity(Vec2(-120.0f, 0.0f));
	}

	else {
		pBullet->setFlippedX(true);
		bulletPBody->setVelocity(Vec2(120.0f, 0.0f));
	}
}

void Enemy_Metall::awakeMetall()
{
	this->metallState = METALL_AWAKE;
	this->isAwake = true;
	this->AnimState(metallState);
}

void Enemy_Metall::sleepMetall()
{
	this->metallState = METALL_SLEEP;
	//this->isAwake = false;
	this->AnimState(metallState);
}

void Enemy_Metall::attackMetall()
{
	this->canMove = false;
	this->metallState = METALL_ATTACK;
	this->AnimState(metallState);
}

void Enemy_Metall::setOnAwakeBool()
{
	this->canMove = true;
}

void Enemy_Metall::setOffAwakeBool()
{
	this->isAwake = false;
}

void Enemy_Metall::moveMetall()
{
	metallBody->setVelocity(Vec2(this->metallDir * 10.0f, 0.0f));
}

void Enemy_Metall::stopMetall()
{
	metallBody->setVelocity(Vec2::ZERO);
}

void Enemy_Metall::contactDeadEnd()
{
	if (this->metallSprite->isFlippedX() == false) {
		this->metallSprite->setFlippedX(true);
		this->metallDir = this->metallDir * (-1);
	}

	else {
		this->metallSprite->setFlippedX(false);
		this->metallDir = this->metallDir * (-1);
	}
}

void Enemy_Metall::die()
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

	//auto callback = CallFunc::create(this, callfunc_selector(Enemy_Metall::removeObject));
	auto seq = Sequence::create(animate->clone(), RemoveSelf::create(), nullptr);

	auto explosion = Sprite::create();
	auto metallPos = sceneLayer->convertToNodeSpace(this->metallBody->getPosition());
	explosion->setPosition(metallPos.x, metallPos.y - 8.5f);

	soundManager->StopEnemyEffect();

	explosion->runAction(seq);
	sceneLayer->addChild(explosion);
	soundManager->PlayEnemyEffect(soundManager->enemyDeathPath);
	this->removeObject();
}

void Enemy_Metall::removeObject()
{
	this->removeAllChildren();
}



