#include "Enemy_Boss.h"
#include "MainScene.h"

USING_NS_CC;

Enemy_Boss::Enemy_Boss(cocos2d::Layer* layer, float posX, float posY)
{
	this->sceneLayer = layer;
	this->spawnX = posX;
	this->spawnY = posY;
}

Enemy_Boss* Enemy_Boss::create(cocos2d::Layer* layer, float posX, float posY)
{
	auto pRet{ new (std::nothrow) Enemy_Boss(layer, posX, posY) };
	if (pRet && pRet->init()) {
		pRet->autorelease();
	}
	else {
		delete pRet;
		pRet = nullptr;
	}
	return pRet;
}

bool Enemy_Boss::init()
{
	if (!cocos2d::Node::init()) {
		return false;
	}

	this->bossState = BOSS_IDLE;
	this->isMoving = false;
	this->isStop = true;

	this->isHit = false;
	this->onShader = false;

	this->bossHealth = 28.0f;

	bossSprite = Sprite::create("Megaman/Enemies/Mad_Grinder/subboss_Init.png");
	bossSprite->setPosition(spawnX, spawnY);
	bossSprite->getTexture()->setAliasTexParameters();

	bossHitboxSprite = Sprite::create("Placeholder.png");

	bossSprite->addChild(bossHitboxSprite);
	

	// Main PhysicsBody (Mad Ground) 생성
	auto material1 = PhysicsMaterial(1.0f, 0.0f, 0.0f);
	bossBody = PhysicsBody::createBox(Size(60.5, 54), material1, Vec2(-30.25f, -27.5f));

	bossBody->setRotationEnable(false);
	bossBody->setCategoryBitmask(Utils::CreateMask(core::CategoryBits::ENEMY));
	bossBody->setCollisionBitmask(Utils::CreateMask(core::CategoryBits::PLATFORM));
	bossBody->setContactTestBitmask(Utils::CreateMask(core::CategoryBits::PLAYER
													, core::CategoryBits::PLAYER_PROJECTILE
													, core::CategoryBits::BOSS_MOVE_POINT));
	bossBody->setTag(core::TagIndex::BOSS_SHIELD);

	// Mad Ground Hitbox 생성
	bossHitbox = PhysicsBody::createBox(Size(84, 47), material1, Vec2(60.5f, 82.5f));
	bossHitbox->setDynamic(false);
	bossHitbox->setRotationEnable(false);

	bossHitbox->setCategoryBitmask(Utils::CreateMask(core::CategoryBits::ENEMY));
	bossHitbox->setCollisionBitmask(Utils::CreateMask(0));
	bossHitbox->setContactTestBitmask(Utils::CreateMask(core::CategoryBits::PLAYER
		, core::CategoryBits::PLAYER_PROJECTILE));
	bossHitbox->setTag(core::TagIndex::BOSS);

	// Tank를 메인으로 자식 노드 설정
	bossSprite->setPhysicsBody(bossBody);
	bossHitboxSprite->setPhysicsBody(bossHitbox);

	this->addChild(bossSprite);

	bossSmoke = Sprite::create();
	bossSprite->addChild(bossSmoke);
	bossSmoke->setPosition(bossBody->getPosition().x + 113.0f, bossBody->getPosition().y + 81.0f);
	bossSmoke->setScale(1.1f);

	bossSmoke_1 = Sprite::create();
	bossSprite->addChild(bossSmoke_1);
	bossSmoke_1->setPosition(bossBody->getPosition().x + 60.0f, bossBody->getPosition().y + 111.0f);
	bossSmoke_1->setScale(1.1f);

	bossSmoke_2 = Sprite::create();
	bossSprite->addChild(bossSmoke_2);
	bossSmoke_2->setPosition(bossBody->getPosition().x + 67.0f, bossBody->getPosition().y + 85.0f);
	bossSmoke_2->setScale(1.1f);

	bossSmoke_3 = Sprite::create();
	bossSprite->addChild(bossSmoke_3);
	bossSmoke_3->setPosition(bossBody->getPosition().x + 44.0f, bossBody->getPosition().y + 100.0f);
	bossSmoke_3->setScale(1.1f);

	bossSmoke_4 = Sprite::create();
	bossSprite->addChild(bossSmoke_4);
	bossSmoke_4->setPosition(bossBody->getPosition().x + 88.0f, bossBody->getPosition().y + 50.0f);
	bossSmoke_4->setScale(1.1f);
	
	this->createSmoke(bossSmoke);

	//////////////////////////////////////////////////////////////////////
	// Take Damage Shader
	GLchar* boss_fragmentSource = (GLchar*)String::createWithContentsOfFile(FileUtils::getInstance()->fullPathForFilename("enemyBright_Shader.fsh").c_str())->getCString();
	// Create a GLProgram with the shader file.
	boss_glProg = GLProgram::createWithByteArrays(ccPositionTextureColor_noMVP_vert, boss_fragmentSource);
	// Create a GLProgramState from te GLProgram. 
	boss_glState = GLProgramState::getOrCreateWithGLProgram(boss_glProg);

	boss_glState->setUniformInt("brightMode", 0);
	//glProgramState->setUniformTexture("samp_replacements", img1); 
	// Apply the shader to the Sprite.
	bossSprite->setGLProgramState(boss_glState);
	//////////////////////////////////////////////////////////////////////

	return true;
}

void Enemy_Boss::pause()
{
	cocos2d::Node::pause();
}

void Enemy_Boss::resume()
{
	cocos2d::Node::resume();
}

void Enemy_Boss::setPlayerPos(cocos2d::PhysicsBody* playerBody)
{
	this->playerVar = playerBody;
}

void Enemy_Boss::AnimState(_bossState state)
{
	int allSheetNum;
	std::string sPath;
	std::string sName;
	float frameDelay = 0.075f;

	if (state != bossBeforeState) {

		if (bossBeforeState != BOSS_IDLE || bossBeforeState != BOSS_DEAD)
			bossSprite->stopAllActions();

		switch (state)
		{
		case BOSS_IDLE:
			sPath = "Megaman/Enemies/Mad_Grinder/subboss_Init.png";
			break;

		case BOSS_FORWARD:
			allSheetNum = 20;
			sPath = "Megaman/Enemies/Mad_Grinder/Subboss_Move.plist";
			sName = "subboss_move_";
			frameDelay = 0.05f;
			break;

		case BOSS_BACKWARD:
			allSheetNum = 20;
			sPath = "Megaman/Enemies/Mad_Grinder/Subboss_Move.plist";
			sName = "subboss_move_";
			frameDelay = 0.05f;
			break;

		case BOSS_ATTACK:
			allSheetNum = 11;
			sPath = "Megaman/Enemies/Mad_Grinder/Subboss_Attack.plist";
			sName = "subboss_attack_";
			frameDelay = 0.07f;
			break;

		case BOSS_DEAD:
			sPath = "Megaman/Enemies/Mad_Grinder/Subboss_Dead.png";
			break;
		}

		if (state == BOSS_IDLE || state == BOSS_DEAD) {
			bossSprite->setTexture(sPath);
			bossSprite->getTexture()->setAliasTexParameters();
		}

		else {
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
			if (state == BOSS_FORWARD) {
				//auto callback_1 = CallFunc::create(this, callfunc_selector(Enemy_Boss::moveForward));
				//auto callback_2 = CallFunc::create(this, callfunc_selector(Enemy_Boss::stopBoss));
				//auto seq = Sequence::create(callback_1, animate, callback_2, nullptr);
				auto rep = RepeatForever::create(animate);
				bossSprite->runAction(rep);
			}
			else if (state == BOSS_BACKWARD) {
				/*auto callback_1 = CallFunc::create(this, callfunc_selector(Enemy_Boss::moveBackward));
				auto callback_2 = CallFunc::create(this, callfunc_selector(Enemy_Boss::stopBoss));
				auto seq = Sequence::create(callback_1, animate->reverse(), callback_2, nullptr);*/
				auto rep = RepeatForever::create(animate->reverse());
				bossSprite->runAction(rep);
			}
			else if (state == BOSS_ATTACK) {
				auto callback = CallFunc::create(this, callfunc_selector(Enemy_Boss::createBlade));
				auto seq = Sequence::create(animate, callback, nullptr);
				bossSprite->runAction(seq);
			}
		}
		/////////////////////////////////////////////////////////////////////////////////
	}

	this->bossBeforeState = state;
}

void Enemy_Boss::update(float dt)
{
	//log("SceneLayer Position : %f, %f", sceneLayer->getPosition().x, sceneLayer->getPosition().y);
	if (isCameraShaking) {
		cameraShakingTimer += dt;
		this->cameraShaking();
	}
		
	else {
		sceneLayer->setPosition(Vec2(sceneLayer->getPosition().x, 0));
		cameraShakingTimer = 0.0f;
	}

	if (this->isDead && !stopDeadEffect) {
		explosionTimer += dt;
		//log("explosionTimer : %f", explosionTimer);
		if (fmod(explosionTimer, 0.25f) > 0.2f) {
			soundManager->PlayEnemyHitEffect(soundManager->bossTakeDamagePath);
			this->createDeadExplosion();
		}
			

		if (explosionTimer >= 1.5f) {
			if (fmod(explosionTimer, 0.05f) < 0.025f) {
				bossSprite->setOpacity(100);
			}
			else bossSprite->setOpacity(255);
				
		}

		if (explosionTimer >= 3.5f) {
			boss_glState->setUniformInt("brightMode", 2);
			stopDeadEffect = true;
			isCameraShaking = false;
			bossSprite->setOpacity(255);
			explosionTimer = 0;
			log("Stop Explosion");
			soundManager->PlayEnemyEffect(soundManager->bossDeathPath);
		}
	}

	if ((bossHealth <= 0) && !(this->isDead)) {
		this->isDead = true;
		ThirdSmoke = true;
		this->die();
	}

	else {
		if (!onShader && isHit) {
			boss_glState->setUniformInt("brightMode", 1);
			this->onShader = true;
			this->isHit = false;
			soundManager->PlayEnemyHitEffect(soundManager->bossTakeDamagePath);
			log("Boss Health : %f", bossHealth);

			if (bossHealth <= 20 && !FirstSmoke) {
				createSmoke(bossSmoke_1);
				FirstSmoke = true;
			}

			else if (bossHealth <= 10 && !SecondSmoke) {
				createSmoke(bossSmoke_2);
				SecondSmoke = true;
			}
		}
		else if (onShader && !isHit) {
			boss_glState->setUniformInt("brightMode", 0);
			this->onShader = false;
		}

		if (!isReady) {
			if (isMoving) {
				bossBody->setVelocity(Vec2(this->bossDir * 25.0f, 0.0f));
			}

			if (!isMoving) {
				bossBody->setVelocity(Vec2::ZERO);
			}
		}

		else {
			
			if (isMoving && !isDead) {
				bossBody->setVelocity(Vec2(this->bossDir * 42.5f, 0.0f));
			}

			if (!isMoving) {
				bossBody->setVelocity(Vec2::ZERO);
			}
		}

		if (isShootBlade) {
			if (!soundManager->isEnemyEffectRunning())
				soundManager->PlayEnemyEffect(soundManager->bossBladePath);
		}
	}
}

void Enemy_Boss::appearBoss()
{
	this->isCameraShaking = true;
	soundManager->PlayEnemyEffect(soundManager->bossMovePath);
	auto _node = Node::create();
	auto callback = CallFunc::create(this, callfunc_selector(Enemy_Boss::moveForward));
	auto seq = Sequence::create(DelayTime::create(1.0f), callback, RemoveSelf::create(), nullptr);
	
	this->addChild(_node);
	_node->runAction(seq);
}

void Enemy_Boss::setReadySequence()
{
	auto _node = Node::create();
	this->stopBoss();
	auto callback = CallFunc::create(this, callfunc_selector(Enemy_Boss::setReadyBool));
	auto seq = Sequence::create(DelayTime::create(1.0f), callback, RemoveSelf::create(), nullptr);
	
	this->addChild(_node);
	_node->runAction(seq);
}

void Enemy_Boss::setReadyBool()
{
	this->isReady = true;
	this->bossSequence();
}

void Enemy_Boss::bossSequence()
{
	if (sequenceNum == 1) {
		this->stopBoss();
		sequenceNode = Node::create();
		auto callback = CallFunc::create(this, callfunc_selector(Enemy_Boss::moveForward));
		auto seq = Sequence::create(DelayTime::create(0.75f), callback, nullptr);

		this->addChild(sequenceNode);
		sequenceNode->runAction(seq);
	}

	else if (sequenceNum == 2) {
		this->stopBoss();
		sequenceNode = Node::create();
		auto callback = CallFunc::create(this, callfunc_selector(Enemy_Boss::attackBoss));
		auto seq = Sequence::create(DelayTime::create(0.75f), callback, nullptr);

		this->addChild(sequenceNode);
		sequenceNode->runAction(seq);
	}

	else if (sequenceNum == 3) {
		this->stopBoss();
		sequenceNode = Node::create();
		auto callback = CallFunc::create(this, callfunc_selector(Enemy_Boss::moveBackward));
		auto seq = Sequence::create(DelayTime::create(0.75f), callback, nullptr);

		this->addChild(sequenceNode);
		sequenceNode->runAction(seq);
	}

	else if (sequenceNum == 4) {
		this->stopBoss();
		sequenceNode = Node::create();
		auto callback = CallFunc::create(this, callfunc_selector(Enemy_Boss::attackBoss));
		auto seq = Sequence::create(DelayTime::create(0.75f), callback, nullptr);

		this->addChild(sequenceNode);
		sequenceNode->runAction(seq);
	}
}

void Enemy_Boss::moveForward()
{
	this->isMoving = true;
	this->isStop = false;
	this->bossDir = -1;
	this->isCameraShaking = true;
	bossState = BOSS_FORWARD;
	AnimState(bossState);
	soundManager->PlayEnemyEffect(soundManager->bossMovePath);
}

void Enemy_Boss::moveBackward()
{
	this->isMoving = true;
	this->isStop = false;
	this->bossDir = 1;
	this->isCameraShaking = true;
	bossState = BOSS_BACKWARD;
	AnimState(bossState);
	soundManager->PlayEnemyEffect(soundManager->bossMovePath);
}

void Enemy_Boss::attackBoss()
{
	this->isMoving = false;
	this->isStop = true;
	this->isCameraShaking = false;
	bossState = BOSS_ATTACK;
	AnimState(bossState);
}

void Enemy_Boss::stopBoss()
{
	this->isMoving = false;
	this->isStop = true;
	this->isCameraShaking = false;
	bossState = BOSS_IDLE;
	AnimState(bossState);
	soundManager->StopEnemyEffect();
}

void Enemy_Boss::createBlade()
{
	auto bossHeadPosition = sceneLayer->convertToNodeSpace(this->bossBody->getPosition());
	auto shootPosition = Vec2((bossHeadPosition.x - 30.0f), (bossHeadPosition.y + 35.5f));

	auto material = PhysicsMaterial(1.0f, 0.0f, 0.0f);

	auto pBlade = Sprite::create("Megaman/Enemies/Mad_Grinder/subboss_blade_init.png");
	auto bladePBody = PhysicsBody::createCircle(24.0f, material);
	
	pBlade->setPosition(shootPosition);
	bladePBody->setTag(core::TagIndex::BOSS_BLADE);

	bladePBody->setCategoryBitmask(Utils::CreateMask(core::CategoryBits::ENEMY_PROJECTILE));
	bladePBody->setCollisionBitmask(Utils::CreateMask(core::CategoryBits::PLATFORM));
													//, core::CategoryBits::BOUNDARY));
	bladePBody->setContactTestBitmask(Utils::CreateMask(core::CategoryBits::PLATFORM
													, core::CategoryBits::BLADE_BOUNDARY
													, core::CategoryBits::PLAYER));

	pBlade->setPhysicsBody(bladePBody);

	int allSheetNum = 4;
	std::string sPath = "Megaman/Enemies/Mad_Grinder/Subboss_Blade.plist";
	std::string sName = "boss_blade_";
	float frameDelay = 0.075f;

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
	auto rep = RepeatForever::create(animate);

	pBlade->runAction(rep);

	sceneLayer->addChild(pBlade);

	bladePBody->setVelocity(Vec2(- 120.0f, 85.0f));
	isShootBlade = true;
	/*auto test = Sprite::create();
	auto callback = CallFunc::create(this, callfunc_selector(Enemy_Boss::die));
	auto seq = Sequence::create(DelayTime::create(5.0f), callback, nullptr);
	test->runAction(seq);

	sceneLayer->addChild(test);
	log("afjksldfjlks");*/
}

void Enemy_Boss::returnBlade(cocos2d::PhysicsBody* bladeBody)
{
	
	ccBezierConfig _bezierConfig;

	auto bladePosition = sceneLayer->convertToNodeSpace(bladeBody->getPosition());
	auto bossHeadPosition = sceneLayer->convertToNodeSpace(this->bossBody->getPosition());
	auto shootPosition = Vec2((bossHeadPosition.x - 30.0f), (bossHeadPosition.y + 35.5f));

	_bezierConfig.controlPoint_1 = Vec2((bladePosition.x + shootPosition.x)/2, 224);
	_bezierConfig.controlPoint_2 = Vec2((bladePosition.x + shootPosition.x) / 2, 224);
	_bezierConfig.endPosition = shootPosition;

	auto bezierAct = BezierTo::create(1.0f, _bezierConfig);
	auto callback = CallFunc::create(this, callfunc_selector(Enemy_Boss::nextSequence));
	auto seq = Sequence::create(bezierAct, RemoveSelf::create(), callback, nullptr);

	bladeBody->setEnabled(false);
	bladeBody->getNode()->runAction(seq);
}

void Enemy_Boss::createSmoke(cocos2d::Sprite* smokeSprite)
{
	int allSheetNum = 10;
	std::string sPath = "Megaman/Particle/Particle_Boss_Smoke.plist";
	std::string sName = "enemy_smoke_";
	float frameDelay = 0.065f;

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
	auto seq = Sequence::create(animate, DelayTime::create(0.5f), nullptr);
	auto rep = RepeatForever::create(seq);

	smokeSprite->runAction(rep);
}

void Enemy_Boss::nextSequence()
{
	if (!isDead) {
		if (!(this->sequenceNum == 4))
			this->sequenceNum += 1;
		else
			this->sequenceNum = 1;

		this->bossSequence();

		if (isShootBlade)
			isShootBlade = false;

		auto colliderNodeRoot = static_cast<MainScene*>(sceneLayer->getParent());

		if (this->sequenceNum == 1 || this->sequenceNum == 4) {
			colliderNodeRoot->_BossMoveCollider[2]->getPhysicsBody()->setEnabled(false);
			colliderNodeRoot->_BossMoveCollider[1]->getPhysicsBody()->setEnabled(true);
			colliderNodeRoot->_BladeBoundaryCollider[1]->getPhysicsBody()->setEnabled(false);
			colliderNodeRoot->_BladeBoundaryCollider[0]->getPhysicsBody()->setEnabled(true);
		}

		else if (this->sequenceNum == 2 || this->sequenceNum == 3) {
			colliderNodeRoot->_BossMoveCollider[2]->getPhysicsBody()->setEnabled(true);
			colliderNodeRoot->_BossMoveCollider[1]->getPhysicsBody()->setEnabled(false);
			colliderNodeRoot->_BladeBoundaryCollider[1]->getPhysicsBody()->setEnabled(true);
			colliderNodeRoot->_BladeBoundaryCollider[0]->getPhysicsBody()->setEnabled(false);
		}
	}
	
}

void Enemy_Boss::cameraShaking()
{
	if (fmod(cameraShakingTimer, 0.05f) < 0.025f)
		sceneLayer->setPosition(Vec2(sceneLayer->getPosition().x, -2));
	else sceneLayer->setPosition(Vec2(sceneLayer->getPosition().x, 0));
}

void Enemy_Boss::die()
{
	if (sequenceNode->isRunning())
		sequenceNode->stopAllActions();
	this->isMoving = false;
	this->isStop = true;
	this->isCameraShaking = true;
	this->bossBody->setEnabled(false);
	this->bossHitbox->setEnabled(false);
	createSmoke(bossSmoke_3);
	createSmoke(bossSmoke_4);
	bossState = BOSS_DEAD;
	AnimState(bossState);
}

void Enemy_Boss::removeObject()
{
	

}

void Enemy_Boss::createDeadExplosion()
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

	//std::default_random_engine generator;
	std::uniform_int_distribution<int> distributionX(-70, 70);
	std::uniform_int_distribution<int> distributionY(-60, 60);

	int randomX = distributionX(generator);
	int randomY = distributionY(generator);

	//log("%d, %d", randomX, randomY);

	auto explosion = Sprite::create();
	auto metallPos = sceneLayer->convertToNodeSpace(this->bossBody->getPosition());
	explosion->setPosition(metallPos.x + randomX, metallPos.y + randomY);

	auto seq = Sequence::create(animate, RemoveSelf::create(), nullptr);

	explosion->runAction(seq);
	sceneLayer->addChild(explosion);
}
