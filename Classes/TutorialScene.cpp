#include "TutorialScene.h"

USING_NS_CC;

cocos2d::Scene* TutorialScene::createScene()
{
	return TutorialScene::create();
}

bool TutorialScene::init()
{
	if (Scene::initWithPhysics())
		createMyPhysicsWorld();
	else return false;

	// Health Bar �̴ϼȶ���¡
	playerHealth = 28.0f; // Default : 28

	EmptyHealthBar = Sprite::create("Megaman/UI/HealthBar_Empty.png");
	HealthBar = Sprite::create("Megaman/UI/HealthBar_Divide.png");	// Size = (11 x 56), Health = 28

	EmptyHealthBar->getTexture()->setAliasTexParameters();
	HealthBar->getTexture()->setAliasTexParameters();

	EmptyHealthBar->setPosition(15, 178);
	HealthBar->setPosition(15, 182);

	this->addChild(EmptyHealthBar, 40);
	this->addChild(HealthBar, 41);

	isInited = false;

	moveInputState = MS_INIT;
	attackInputState = DEFAULT_POSE;
	moveBeforeState = MS_PLACEHOLDER;
	attackBeforeState = ATK_PLACEHOLDER;
	chargeState = COMMON_BULLET;


	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Ÿ�ϸ� �ҷ�����

	tmap = TMXTiledMap::create("Tutorial_Tileset/Tutorial_Tileset.tmx");
	background = tmap->getLayer("Background");
	background->getTexture()->setAliasTexParameters();
	portal = tmap->getLayer("Portal");
	portal->getTexture()->setAliasTexParameters();
	block_1 = tmap->getLayer("Block_1");
	block_1->getTexture()->setAliasTexParameters();
	block_2 = tmap->getLayer("Block_2");
	block_2->getTexture()->setAliasTexParameters();

	this->addChild(tmap);

	// ZOrder�� -1�� �ϸ� ȭ�� ����� �ȵ�
	//wlayer->addChild(tmap, 2, 11);

	//wlayer->addChild(parallax, 1, 12);

	// Ÿ�ϸʿ��� Objects��� �Է��� �������� ���̾ ���� ��ü�� ��������
	auto spawnObjects = tmap->getObjectGroup("SpawnPoint");

	// �츮�� �������� Object Layer���� SpawnPoint��� ������ �Ӽ� ������ �б�
	ValueMap spawnPoint = spawnObjects->getObject("SpawnPoint");
	Value objectval = Value(spawnPoint);
	//log("%s", objectval.getDescription());

	float x = spawnPoint["x"].asFloat();
	float y = spawnPoint["y"].asFloat();

	// ĳ���͸� SpwnPoint ��ġ�� ������ �� �ֵ��� ��ǥ �غ� 
	//spawnPosition = Vec2(x, y+ (y/2));
	spawnPosition = Vec2(x, y +(y/2)-12.0f);

	auto colliderObjects = tmap->getObjectGroup("Collider");

	auto colliders = colliderObjects->getObjects();


	Vector<Sprite*> _colliderSprite;

	for (int i = 0; i < colliders.size(); i++) {

		auto size = Size(colliders.data()[i].asValueMap()["width"].asFloat(), colliders.data()[i].asValueMap()["height"].asFloat());
		auto pos = Vec2(colliders.data()[i].asValueMap()["x"].asFloat(), colliders.data()[i].asValueMap()["y"].asFloat());

		auto backCollider = PhysicsBody::createBox(size, PHYSICSBODY_MATERIAL_DEFAULT);
		backCollider->setDynamic(false);
		backCollider->setCategoryBitmask(Utils::CreateMask(core::CategoryBits::PLATFORM));

		backCollider->setCollisionBitmask(Utils::CreateMask(core::CategoryBits::PLAYER					// ���� �浹
			, core::CategoryBits::ENEMY
			, core::CategoryBits::ENEMY_PROJECTILE));

		backCollider->setContactTestBitmask(Utils::CreateMask(core::CategoryBits::PLAYER_PROJECTILE
			, core::CategoryBits::ENEMY_PROJECTILE));  // �ݸ��� �̺�Ʈ

		backCollider->setTag(core::TagIndex::PLATFORM);

		auto setcol = Sprite::create();
		setcol->setContentSize(size);
		setcol->setPosition(pos);
		setcol->setAnchorPoint(Vec2(0, 0));
		setcol->setPhysicsBody(backCollider);

		_colliderSprite.pushBack(setcol);

		this->addChild(_colliderSprite.at(i));
	}

	auto portalObjects = tmap->getObjectGroup("PortalCollider");

	auto portalCol = portalObjects->getObjects();

	auto portalColSize = Size(portalCol.data()[0].asValueMap()["width"].asFloat(), portalCol.data()[0].asValueMap()["height"].asFloat());
	auto portalColPos = Vec2(portalCol.data()[0].asValueMap()["x"].asFloat(), portalCol.data()[0].asValueMap()["y"].asFloat());

	auto portalBody = PhysicsBody::createBox(portalColSize, PHYSICSBODY_MATERIAL_DEFAULT);
	portalBody->setDynamic(false);
	portalBody->setCategoryBitmask(Utils::CreateMask(core::CategoryBits::TRIGGER_POINT));

	portalBody->setCollisionBitmask(Utils::CreateMask(0));

	portalBody->setContactTestBitmask(Utils::CreateMask(core::CategoryBits::PLAYER));  // �ݸ��� �̺�Ʈ

	portalBody->setTag(core::TagIndex::TRIGGER_POINT);

	auto portalSprite = Sprite::create();
	portalSprite->setContentSize(portalColSize);
	portalSprite->setPosition(portalColPos);
	portalSprite->setAnchorPoint(Vec2(0, 0));
	portalSprite->setPhysicsBody(portalBody);

	this->addChild(portalSprite);

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Character �θ� ��� ����
	charLayer = Sprite::create("Placeholder.png");
	charLayer->setScale(1.0f);
	//charLayer->setPosition(Vec2(128, 0));
	charLayer->setPosition(spawnPosition);
	charLayer->setAnchorPoint(Vec2(0.5, 0.25));
	charLayer->getTexture()->setAliasTexParameters();


	// Movement ��� Sprite
	character = Sprite::create("test_megaman.png");
	character->setAnchorPoint(Vec2::ZERO);
	character->getTexture()->setAliasTexParameters();

	// Attack ��� Sprite
	attackChar = Sprite::create("test_megaman.png");
	attackChar->setAnchorPoint(Vec2::ZERO);
	attackChar->getTexture()->setAliasTexParameters();



	//jumpCheckBox = Sprite::create();

	//jumpCheckBox->setAnchorPoint(Vec2::ZERO);

	auto material1 = PhysicsMaterial(1.0f, 0.0f, 0.0f);
	// auto body = PhysicsBody::createCircle(character->getContentSize().width / 2, material2);
	//characterBody = PhysicsBody::createBox(Size(character->getContentSize().width, character->getContentSize().height-2.0f), material1, Vec2(0.5f, 0.5f));
	characterBody = PhysicsBody::createBox(Size(32, 40), material1, Vec2(0.5f, 0.5f));

	characterBody->setMass(1.0f);
	//characterBody->setGravityEnable(false);
	//characterBody->setDynamic(false);
	characterBody->setRotationEnable(false);
	characterBody->setCategoryBitmask(Utils::CreateMask(core::CategoryBits::PLAYER));

	characterBody->setCollisionBitmask(Utils::CreateMask(core::CategoryBits::PLATFORM
														, core::CategoryBits::BOUNDARY));

	characterBody->setContactTestBitmask(Utils::CreateMask(core::CategoryBits::ENEMY
		, core::CategoryBits::ENEMY_PROJECTILE
		, core::CategoryBits::ENEMY_AWAKE_SENSOR
		, core::CategoryBits::TRIGGER_POINT));

	characterBody->setTag(core::TagIndex::PLAYER);
	characterBody->setGravityEnable(true);


	fall = false;
	movingLeft = false;
	movingRight = false;
	movingUp = false;
	movingSliding = false;
	inputShoot = false;
	beforeRunShoot = false;
	isInited = true;
	isChangeMoveSprite = false;

	shootPosition = Vec2(characterBody->getPosition().x - (atk_stop_pos[0][0] + 3), characterBody->getPosition().y + atk_stop_pos[0][1]);

	//character->setPhysicsBody(characterBody);
	charLayer->setPhysicsBody(characterBody);
	charLayer->addChild(character);
	charLayer->addChild(attackChar);
	GLProgram* p = new GLProgram();

	attackChar->setVisible(false);

	this->addChild(charLayer, 3);


	////////////////////////////////////////////////////////

	// �浹 �̺�Ʈ ������
	auto contact = EventListenerPhysicsContact::create();

	// �浹 �̺�Ʈ �ݹ� �Լ� ���
	/*
		1) onContactBegin
			- �������� ��

		2) onContactPresolve
			- �����ϱ� ����

		3) onContactPostSolve
			- �������� ����

		4) onContactSeparate
			- ������ ��
	*/
	contact->onContactBegin = [this](PhysicsContact& contact) -> bool {
		// Node Ŭ������ ��������
		/*
			contact : �浹 ������ �����ϴ� PhysicsContact �ν��Ͻ�
			getShapeA&B : �浹�� PhysicsShape ��������
			getBody : PhysicsShape���� PhysicsBody ��������
			getNode : PhysicsBody���� �� Rigidbody�� ����� Node�� ��������
		*/

		// nodeA : ���浹 / nodeB : �浹
		auto nodeA = contact.getShapeA()->getBody()->getTag();
		auto nodeB = contact.getShapeB()->getBody()->getTag();

		///////////////////////////////////////////////////////////////////////////////////////////////////////////
		if ((nodeA == core::TagIndex::COMMON_BULLET || nodeA == core::TagIndex::CHARGE_X1 || nodeA == core::TagIndex::CHARGE_X2 || nodeA == core::TagIndex::METALL_BULLET)
			&& nodeB != core::TagIndex::BOUNDARY && contact.getShapeB()->getBody()->getNode() != nullptr && contact.getShapeA()->getBody()->getNode() != nullptr)
		{
			//contact.getShapeA()->getBody()->getNode()->removeFromParentAndCleanup(true);
			//log("Begin nodeA Deleted");

			if (nodeA == core::TagIndex::COMMON_BULLET) {
				contact.getShapeA()->getBody()->getNode()->removeFromParentAndCleanup(true);
				auto posX = contact.getContactData()->points->x;
				auto posY = contact.getShapeA()->getBody()->getPosition().y;

				auto world = this->convertToNodeSpace(Vec2(posX, posY));
				//log("World Pos : %f, %f", world.x, world.y);
				bulletParticleAnimState(COMMON_BULLET, world.x, world.y);
			}

			else if (nodeA == core::TagIndex::CHARGE_X1) {
				contact.getShapeA()->getBody()->getNode()->removeFromParentAndCleanup(true);

				auto posX = contact.getContactData()->points->x;
				auto posY = contact.getShapeA()->getBody()->getPosition().y;

				auto world = this->convertToNodeSpace(Vec2(posX, posY));
				//log("World Pos : %f, %f", world.x, world.y);
				bulletParticleAnimState(CHARGE_X1, world.x, world.y);
			}

			else if (nodeA == core::TagIndex::CHARGE_X2) {
				contact.getShapeA()->getBody()->getNode()->removeFromParentAndCleanup(true);

				auto posX = contact.getContactData()->points->x;
				auto posY = contact.getShapeA()->getBody()->getPosition().y;

				auto world = this->convertToNodeSpace(Vec2(posX, posY));
				//log("World Pos : %f, %f", world.x, world.y);
				bulletParticleAnimState(CHARGE_X2, world.x, world.y);
			}
		}
		else if ((nodeB == core::TagIndex::COMMON_BULLET || nodeB == core::TagIndex::CHARGE_X1 || nodeB == core::TagIndex::CHARGE_X2 || nodeB == core::TagIndex::METALL_BULLET)
			&& nodeA != core::TagIndex::BOUNDARY && contact.getShapeA()->getBody()->getNode() != nullptr && contact.getShapeB()->getBody()->getNode() != nullptr)
		{
			//contact.getShapeB()->getBody()->getNode()->removeFromParentAndCleanup(true);
			//log("Begin nodeB Deleted");

			if (nodeB == core::TagIndex::COMMON_BULLET) {
				contact.getShapeB()->getBody()->getNode()->removeFromParentAndCleanup(true);
				auto posX = contact.getContactData()->points->x;
				auto posY = contact.getShapeB()->getBody()->getPosition().y;

				auto world = this->convertToNodeSpace(Vec2(posX, posY));
				//log("World Pos : %f, %f", world.x, world.y);
				bulletParticleAnimState(COMMON_BULLET, world.x, world.y);
			}

			else if (nodeB == core::TagIndex::CHARGE_X1) {
				contact.getShapeB()->getBody()->getNode()->removeFromParentAndCleanup(true);

				auto posX = contact.getContactData()->points->x;
				auto posY = contact.getShapeB()->getBody()->getPosition().y;

				auto world = this->convertToNodeSpace(Vec2(posX, posY));
				//log("World Pos : %f, %f", world.x, world.y);
				bulletParticleAnimState(CHARGE_X1, world.x, world.y);
			}

			else if (nodeB == core::TagIndex::CHARGE_X2) {
				contact.getShapeB()->getBody()->getNode()->removeFromParentAndCleanup(true);

				auto posX = contact.getContactData()->points->x;
				auto posY = contact.getShapeB()->getBody()->getPosition().y;

				auto world = this->convertToNodeSpace(Vec2(posX, posY));
				//log("World Pos : %f, %f", world.x, world.y);
				bulletParticleAnimState(CHARGE_X2, world.x, world.y);
			}
		}

		if (nodeA == core::TagIndex::TRIGGER_POINT) {
			this->canTeleport = true;
		}

		else if (nodeB == core::TagIndex::TRIGGER_POINT) {
			this->canTeleport = true;
		}

		return true;
		};


	contact->onContactSeparate = [this](PhysicsContact& contact) -> bool {
		// Node Ŭ������ ��������
		/*
			contact : �浹 ������ �����ϴ� PhysicsContact �ν��Ͻ�
			getShapeA&B : �浹�� PhysicsShape ��������
			getBody : PhysicsShape���� PhysicsBody ��������
			getNode : PhysicsBody���� �� Rigidbody�� ����� Node�� ��������
		*/

		// nodeA : ���浹 / nodeB : �浹

		auto nodeA = contact.getShapeA()->getBody()->getTag();
		auto nodeB = contact.getShapeB()->getBody()->getTag();

		//if ((nodeA == 10 || nodeA == 11) && contact.getShapeB()->getBody()->getNode() != nullptr)
		if ((nodeA == core::TagIndex::COMMON_BULLET || nodeA == core::TagIndex::CHARGE_X1 || nodeA == core::TagIndex::CHARGE_X2)
			&& contact.getShapeA()->getBody()->getNode() != nullptr && nodeB == core::TagIndex::BOUNDARY && contact.getShapeB()->getBody()->getNode() != nullptr)
		{
			contact.getShapeA()->getBody()->getNode()->removeFromParentAndCleanup(true);
			log("Separate NodeA Deleted");
		}

		else if ((nodeB == core::TagIndex::COMMON_BULLET || nodeB == core::TagIndex::CHARGE_X1 || nodeB == core::TagIndex::CHARGE_X2)
			&& nodeA == core::TagIndex::BOUNDARY && contact.getShapeB()->getBody()->getNode() != nullptr && contact.getShapeB()->getBody()->getNode() != nullptr)
		{
			contact.getShapeB()->getBody()->getNode()->removeFromParentAndCleanup(true);
			log("Separate nodeB Deleted");
		}

		if (nodeA == core::TagIndex::TRIGGER_POINT) {
			this->canTeleport = false;
		}

		else if (nodeB == core::TagIndex::TRIGGER_POINT) {
			this->canTeleport = false;
		}

		return true;
		};
		

	////////////////////////////////////////////////////////

	// Create and apply the shader
	// Load the external shader file into the application as a CString cast as a GLchar*. Note that the ChannelSwap.fsh shader file can be found in the Resources/shaders folder of our project. 
	//std::string contents = FileUtils::getInstance()->getStringFromFile("ChargeMode_Shader.fsh");
	GLchar* fragmentSource = (GLchar*)String::createWithContentsOfFile(FileUtils::getInstance()->fullPathForFilename("ChargeMode_Shader.fsh").c_str())->getCString();
	// Create a GLProgram with the shader file.
	glProg = GLProgram::createWithByteArrays(ccPositionTextureColor_noMVP_vert, fragmentSource);
	// Create a GLProgramState from te GLProgram. 
	glProgramState = GLProgramState::getOrCreateWithGLProgram(glProg);

	glProgramState->setUniformInt("chargeMode", 0);
	//glProgramState->setUniformTexture("samp_replacements", img1); 
	// Apply the shader to the Sprite.
	attackChar->setGLProgramState(glProgramState);

	soundManager->initSoundManager(bgmPath);
	soundManager->PlayBGM(bgmPath);

	_eventDispatcher->addEventListenerWithSceneGraphPriority(contact, this);
	this->scheduleUpdate();

	return true;
}

void TutorialScene::onEnter()
{
	Scene::onEnter();

	// �̺�Ʈ ������
	_listener = EventListenerKeyboard::create();
	_listener->onKeyPressed = CC_CALLBACK_2(TutorialScene::onKeyPressed, this);
	_listener->onKeyReleased = CC_CALLBACK_2(TutorialScene::onKeyReleased, this);

	_eventDispatcher->addEventListenerWithSceneGraphPriority(_listener, this);
}

void TutorialScene::onExit()
{
	_eventDispatcher->removeEventListener(_listener);
	Scene::onExit();
}

void TutorialScene::update(float dt)
{
	

	if (!isTeleport) {

		if (inputShoot) {
			atkChargeTime += dt;
			if (0.85f < atkChargeTime && atkChargeTime <= 2.0f) {
				if (chargeState != CHARGE_X1) {
					chargeState = CHARGE_X1;
					log("Charge_X1 Ready");

					if (!x1Ready) {
						soundManager->PlayAttackEffect(startChargePath);
						x1Ready = true;
					}
				}

				if (fmod(atkChargeTime, 0.15f) < 0.1f)
					glProgramState->setUniformInt("chargeMode", 1);
				else glProgramState->setUniformInt("chargeMode", 0);
			}

			if (2.0f < atkChargeTime) {
				if (chargeState != CHARGE_X2) {
					chargeState = CHARGE_X2;
					log("Charge_X2 Ready");
					if (!x2Ready) {
						soundManager->PlayAttackEffect(chargingLoopPath);
						x2Ready = true;
					}
				}

				if (fmod(atkChargeTime, 0.1f) < 0.05f)
					glProgramState->setUniformInt("chargeMode", 2);
				else glProgramState->setUniformInt("chargeMode", 3);
			}
		}

		else if (!inputShoot && !isChangeMoveSprite) {
			changeTime += dt;

			if (!inputShoot && (changeTime > 0.18f)) {
				turnOnMoveSprite();
				changeTime = 0;
				isChangeMoveSprite = true;
			}

		}

		// �ٴ� üũ (���� ����, �ӽ� �ڵ�)

		if (characterBody->getVelocity().y < 0.0001f && characterBody->getVelocity().y > -0.0001f) {
			fall = false;
		}

		else fall = true;
			

		// ���� ����
		//log("%f", jumpChargeTime);
		if (movingUp && !movingSliding) {
			jumpChargeTime += dt;
			if (jumpChargeTime > 0.2f) {

				isJumping = false;
				movingUp = false;
			}

		}

		if (movingUp && !fall && jumpChargeTime < 0.2f) {
			//characterBody->applyForce(Vec2(0, 31000));
			characterBody->setVelocity(Vec2(0, 50));
			isJumping = true;
			moveInputState = MS_JUMP;
			attackInputState = JUMP_SHOOT;
			attackAnimState(attackInputState);
			moveAnimState(moveInputState);
		}

		if (isJumping == true && jumpChargeTime < 0.2f) {
			characterBody->setVelocity(Vec2(0, 110));
			//jumpChargeTime -= dt;
		}



		if (fall && inputShoot) {
			turnOffMoveSprite();
		}

		//log("Megaman's Velocity : (%f, %f)", characterBody->getVelocity().x, characterBody->getVelocity().y);
		if (fall && characterBody->getVelocity().y < -20.0f) {
			moveInputState = MS_FALL;
			attackInputState = FALL_SHOOT;
			characterBody->setVelocity(Vec2(characterBody->getVelocity().x, (characterBody->getVelocity().y) * 1.05f));
			attackAnimState(attackInputState);
			moveAnimState(moveInputState);
			movingSliding = false;

		}

		// ���� ���� ����
		if (movingLeft && !movingRight) {

			if (movingDir == -1 && !character->isFlippedX()) {
				flippedCharacter(true);

				movingSliding = false;
			}

			else characterBody->setVelocity(Vec2(-50, characterBody->getVelocity().y));

			if (!movingUp && !fall && !movingSliding) {
				moveInputState = MS_RUN;
				attackInputState = RUN_SHOOT;
				attackAnimState(attackInputState);
				moveAnimState(moveInputState);

				if (inputShoot)
					turnOffMoveSprite();
			}
		}

		// ������ ���� ����
		if (movingRight && !movingLeft) {

			if (movingDir == 1 && character->isFlippedX()) {
				flippedCharacter(false);

				movingSliding = false;
			}

			else characterBody->setVelocity(Vec2(50, characterBody->getVelocity().y));

			if (!movingUp && !fall & !movingSliding) {
				moveInputState = MS_RUN;
				attackInputState = RUN_SHOOT;
				attackAnimState(attackInputState);
				moveAnimState(moveInputState);

				if (inputShoot)
					turnOffMoveSprite();
			}
		}

		// ���ÿ� ������ �� or ������ �� �ӵ� ���� (�̲��� ����)
		if ((!movingLeft && !movingRight && !movingSliding) || (movingLeft && movingRight)) {
			characterBody->setVelocity(Vec2(0, characterBody->getVelocity().y));
			movingSliding = false;
			if (!movingUp && !fall) {
				moveInputState = MS_STOP;
				moveAnimState(moveInputState);
				attackInputState = DEFAULT_POSE;
				attackAnimState(attackInputState);

				if (inputShoot) {
					attackInputState = STOP_SHOOT;
					turnOffMoveSprite();
					attackAnimState(attackInputState);
				}
			}
		}

		// �����̵� ����
		if (movingSliding && !movingUp && !fall) {
			//slidingClock += dt;

			if (character->isFlippedX())
				characterBody->setVelocity(Vec2(-85, characterBody->getVelocity().y));
			else characterBody->setVelocity(Vec2(85, characterBody->getVelocity().y));

			if (!movingUp && !fall) {
				moveInputState = MS_SLIDING;
				moveAnimState(moveInputState);
			}

		}

		auto frameNum = (atkAnimate->getCurrentFrameIndex());

		//this->removeChild(_node);

		//_node = DrawNode::create();

		auto nodePosition = this->convertToNodeSpace(characterBody->getPosition());

		if (character->isFlippedX()) {
			if (attackInputState == STOP_SHOOT || attackInputState == DEFAULT_POSE) {
				shootPosition = Vec2(nodePosition.x - (atk_stop_pos[0][0] + 3), characterBody->getPosition().y + atk_stop_pos[0][1]);
				//_node->drawDot(shootPosition, 2, Color4F::MAGENTA);
			}

			else if (attackInputState == JUMP_SHOOT) {
				shootPosition = Vec2(nodePosition.x - (atk_jump_pos[frameNum][0] + 3), nodePosition.y + atk_jump_pos[frameNum][1]);
				//_node->drawDot(shootPosition, 2, Color4F::MAGENTA);
			}

			else if (attackInputState == FALL_SHOOT) {
				shootPosition = Vec2(nodePosition.x - (atk_fall_pos[frameNum][0] + 3), nodePosition.y + atk_fall_pos[frameNum][1]);
				//_node->drawDot(shootPosition, 2, Color4F::MAGENTA);
			}

			else if (attackInputState == RUN_SHOOT) {
				shootPosition = Vec2(nodePosition.x - (atk_run_pos[frameNum][0] + 3), nodePosition.y + atk_run_pos[frameNum][1]);
				//_node->drawDot(shootPosition, 2, Color4F::MAGENTA);
			}
		}

		else {
			if (attackInputState == STOP_SHOOT || attackInputState == DEFAULT_POSE) {
				shootPosition = Vec2(nodePosition.x + (atk_stop_pos[0][0] + 3), nodePosition.y + atk_stop_pos[0][1]);
				//_node->drawDot(shootPosition, 2, Color4F::MAGENTA);
			}

			else if (attackInputState == JUMP_SHOOT) {
				shootPosition = Vec2(nodePosition.x + (atk_jump_pos[frameNum][0] + 3), nodePosition.y + atk_jump_pos[frameNum][1]);
				//_node->drawDot(shootPosition, 2, Color4F::MAGENTA);
			}

			else if (attackInputState == FALL_SHOOT) {
				shootPosition = Vec2(nodePosition.x + (atk_fall_pos[frameNum][0] + 3), nodePosition.y + atk_fall_pos[frameNum][1]);
				//_node->drawDot(shootPosition, 2, Color4F::MAGENTA);
			}

			else if (attackInputState == RUN_SHOOT) {
				shootPosition = Vec2(nodePosition.x + (atk_run_pos[frameNum][0] + 3), nodePosition.y + atk_run_pos[frameNum][1]);
				//_node->drawDot(shootPosition, 2, Color4F::MAGENTA);
			}
		}

		//this->addChild(_node);
	}

	else if (isReplaceScene) {
		soundManager->DecreaseVolume(&nowVolume);
	}

}

void TutorialScene::onKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event)
{

	if (!isTeleport) {
		switch (keyCode)
		{
		case EventKeyboard::KeyCode::KEY_SPACE:
			// _keyCode = _keyCode | 1;
			// _keyCode |= 1;
			if (!readySlide) {
				if (jumpChargeTime <= 0.3f) {
					movingUp = true;
					movingSliding = false;
				}
			}
			else if (readySlide && !fall && !inputShoot) {
				movingSliding = true;
			}



			break;

		case EventKeyboard::KeyCode::KEY_DOWN_ARROW:
			//_keyCode |= 2;
			readySlide = true;
			break;

		case EventKeyboard::KeyCode::KEY_LEFT_ARROW:
			//_keyCode |= 4;
			movingLeft = true;
			movingDir = -1;
			if (movingDir != -1)
				movingSliding = false;

			break;

		case EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
			//_keyCode |= 8;
			movingRight = true;
			movingDir = 1;
			if (movingDir != 1)
				movingSliding = false;

			break;

			/*case EventKeyboard::KeyCode::KEY_C:
				if (movingLeft && !movingRight)
					inputState = MS_LEFT_SLIDING;
				else if (!movingLeft && movingRight)
					inputState = MS_RIGHT_SLIDING;
				break;*/

		case EventKeyboard::KeyCode::KEY_X:
			if (!isTeleport) {
				inputShoot = true;
				if (!movingSliding) {
					createBullet(COMMON_BULLET);
				}
			}

			break;

		case EventKeyboard::KeyCode::KEY_UP_ARROW:
			if (canTeleport) {
				this->teleportNextScene();
			}
			break;
		}
	}
}

void TutorialScene::onKeyReleased(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event)
{
	if (!isTeleport) {
		// Ű���� �Է��� �����ϱ� ���ؼ��� XOR ������(^)�� �����
		switch (keyCode)
		{
		case EventKeyboard::KeyCode::KEY_SPACE:
			// _keyCode = _keyCode ^ 1;
			//_keyCode ^= 1;
			movingUp = false;
			isJumping = false;
			jumpChargeTime = 0;

			break;
		case EventKeyboard::KeyCode::KEY_DOWN_ARROW:
			//_keyCode ^= 2;
			readySlide = false;
			break;

		case EventKeyboard::KeyCode::KEY_LEFT_ARROW:
			//_keyCode ^= 4;
			movingLeft = false;
			moveInputState = MS_STOP;

			break;
		case EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
			//_keyCode ^= 8;
			movingRight = false;
			moveInputState = MS_STOP;

			break;

		case EventKeyboard::KeyCode::KEY_X:
			if (chargeState != COMMON_BULLET) {
				createBullet(chargeState);
			}
			glProgramState->setUniformInt("chargeMode", 0);
			chargeState = COMMON_BULLET;
			inputShoot = false;
			isChangeMoveSprite = false;
			changeTime = 0;
			atkChargeTime = 0;
			x1Ready = false;
			x2Ready = false;
			break;
		}

		
	}
}

void TutorialScene::moveAnimState(_TmoveState state)
{
	int allSheetNum;
	std::string sPath;
	std::string sName;
	float frameDelay = 0.075f;

	if (state != moveBeforeState) {

		if (isInited)
			character->stopAllActions();

		switch (state)
		{
		case MS_INIT:
			allSheetNum = 6;
			sPath = "Megaman/Animation/Megaman_Teleport_Init.plist";
			sName = "teleport_";
			frameDelay = 0.05f;

			break;
		case MS_TELEPORT:
			allSheetNum = 7;
			sPath = "Megaman/Animation/Megaman_Teleport.plist";
			sName = "teleport_";
			frameDelay = 0.05f;

			break;
		case MS_STOP:
			allSheetNum = 70;
			sPath = "Megaman/Animation/Megaman_Idle.plist";
			sName = "idle_";
			frameDelay = 0.12f;

			break;
		case MS_RUN:
			allSheetNum = 10;
			sPath = "Megaman/Animation/Megaman_Run.plist";
			sName = "run_";
			frameDelay = 0.07f;
			break;
		case MS_JUMP:
			allSheetNum = 5;
			sPath = "Megaman/Animation/Megaman_Jump.plist";
			sName = "jump_";
			frameDelay = 0.05f;
			break;
		case MS_SLIDING:
			allSheetNum = 3;
			sPath = "Megaman/Animation/Megaman_Sliding.plist";
			sName = "sliding_";
			break;
		case MS_FALL:
			allSheetNum = 5;
			sPath = "Megaman/Animation/Megaman_Fall.plist";
			sName = "fall_";
			frameDelay = 0.1f;
			break;
		case MS_HURT:
			allSheetNum = 5;
			sPath = "Megaman/Animation/Megaman_Hurt.plist";
			sName = "hurt_";
			frameDelay = 0.05f;
		}



		/////////////////////////////////////////////////////////////////////////////////
		// 3. plist ������ ����Ͽ� �ִϸ��̼� ����

		// �ܺ� ���α׷��� ����Ͽ� ���� plist ���� �ҷ�����
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
		//auto runRep = RepeatForever::create(animate);
		if (state == MS_STOP || state == MS_RUN) {
			auto runRep = RepeatForever::create(animate);
			character->runAction(runRep);
		}

		else if (state == MS_TELEPORT) {
			log("teleporting...");
			animate->retain();
			auto callback_1 = CallFunc::create(this, callfunc_selector(TutorialScene::setTeleportVelocity));
			auto callback_2 = CallFunc::create(this, callfunc_selector(TutorialScene::replaceMainScene));
			auto seq = Sequence::create(animate, callback_1, DelayTime::create(1.5f), callback_2, nullptr);
			character->runAction(seq);
		}

		else {
			character->runAction(animate);
		}

		if (moveBeforeState == MS_FALL)
			soundManager->PlayMovementEffect(landPath);
		if (state == MS_JUMP)
			soundManager->PlayMovementEffect(jumpPath);
		if (state == MS_TELEPORT)
			soundManager->PlayMovementEffect(warpUpPath);
		if (state == MS_INIT)
			soundManager->PlayMovementEffect(warpDownPath);
		if (state == MS_SLIDING)
			soundManager->PlayMovementEffect(slidingPath);
		/////////////////////////////////////////////////////////////////////////////////
	}

	moveBeforeState = state;
}

void TutorialScene::attackAnimState(_TattackState state)
{
	int allSheetNum;
	std::string sPath;
	std::string sName;
	float frameDelay;

	if (state != attackBeforeState || state == STOP_SHOOT) {
		if (isInited)
			attackChar->stopAllActions();

		switch (state)
		{
		case DEFAULT_POSE:
			//allSheetNum = 1;
			//sPath = "Megaman/Animation/Megaman_Shoot_Pose.plist";
			//sName = "pose_";
			//frameDelay = 1000.0f;


		case STOP_SHOOT:
			allSheetNum = 3;
			sPath = "Megaman/Animation/Megaman_Shoot.plist";
			sName = "shoot_";
			frameDelay = 0.0333f;

			break;

		case RUN_SHOOT:
			allSheetNum = 10;
			sPath = "Megaman/Animation/Megaman_Run_Shoot.plist";
			sName = "run_shoot_";
			frameDelay = 0.07f;

			break;
		case JUMP_SHOOT:
			allSheetNum = 5;
			sPath = "Megaman/Animation/Megaman_Jump_Shoot.plist";
			sName = "jump_shoot_";
			frameDelay = 0.05f;
			break;
		case FALL_SHOOT:
			allSheetNum = 5;
			sPath = "Megaman/Animation/Megaman_Fall_Shoot.plist";
			sName = "fall_shoot_";
			frameDelay = 0.1f;
			break;
		}

		//log("Now Aniamtion : %d", state);
		//log("sPath : %s", sPath.c_str());
		//log("sName : %s", sName.c_str());
		//log("\n");

		/////////////////////////////////////////////////////////////////////////////////
			// 3. plist ������ ����Ͽ� �ִϸ��̼� ����



		// �ܺ� ���α׷��� ����Ͽ� ���� plist ���� �ҷ�����
		auto cache = SpriteFrameCache::getInstance();
		cache->addSpriteFramesWithFile(sPath);

		// cocos2d::Vector
		//Vector<AnimationFrame*> animFrames;
		Vector<SpriteFrame*> animFrames;
		//ValueMap myValueMap;

		for (int i = 0; i < allSheetNum; i++) {

			// StringUtils::format => ������ �������� ���ڿ��� ����
			std::string _frames = StringUtils::format("%s%02d.png", sName.c_str(), i);
			//myValueMap.insert(std::pair<std::string, Value>("position", ValueVector(Value(1.0f), Value(2.0f))));

			// ������ ���ڿ��� �̿��Ͽ� plist ������ SpriteFrame ������ ������
			SpriteFrame* frame = cache->getSpriteFrameByName(_frames);
			//frame->setAnchorPoint(Vec2(0, 0));
			frame->getTexture()->setAliasTexParameters();
			// ������ SpriteFrame�� ����
			//animFrames.pushBack(AnimationFrame::create(cache->getSpriteFrameByName(_frames), 1, myValueMap));
			animFrames.pushBack(frame);
		}

		// plist ������� ���� SpriteFrame ������ Ȱ���Ͽ� �ν��Ͻ� ����
		auto atkAnimation = Animation::createWithSpriteFrames(animFrames, frameDelay);
		//auto atkAnimation = Animation::createWithSpriteFrames(animFrames, frameDelay);
		atkAnimate = Animate::create(atkAnimation);
		atkAnimate->retain();


		//auto runRep = RepeatForever::create(animate);

		if (state == STOP_SHOOT) {
			auto callback = CallFunc::create(this, callfunc_selector(TutorialScene::turnOnMoveSprite));
			auto seq = Sequence::create(atkAnimate, callback, nullptr);
			attackChar->runAction(seq);
		}

		else if (state == RUN_SHOOT) {
			auto runRep = RepeatForever::create(atkAnimate);

			attackChar->runAction(runRep);
		}

		else if (state == JUMP_SHOOT || state == FALL_SHOOT || state == DEFAULT_POSE) {
			attackChar->runAction(atkAnimate);
		}

		attackBeforeState = state;
	}




	/////////////////////////////////////////////////////////////////////////////////
}

void TutorialScene::bulletParticleAnimState(_Tbullets state, float x, float y)
{
	int allSheetNum;
	std::string sPath;
	std::string sName;
	float frameDelay = 0.05f;
	float particleScale = 1.0f;

	switch (state)
	{
	case COMMON_BULLET:
		allSheetNum = 4;
		sPath = "Megaman/Particle/Particle_Explosion_Small.plist";
		sName = "explosion_small_";
		particleScale = 0.9f;
		break;

	case CHARGE_X1:
		allSheetNum = 5;
		sPath = "Megaman/Particle/Particle_Charge_Affect.plist";
		sName = "charge_affect_";
		particleScale = 0.5f;
		break;
	case CHARGE_X2:
		allSheetNum = 5;
		sPath = "Megaman/Particle/Particle_Charge_Affect.plist";
		sName = "charge_affect_";
		break;
	}

	// �ܺ� ���α׷��� ����Ͽ� ���� plist ���� �ҷ�����
	auto cache = SpriteFrameCache::getInstance();
	cache->addSpriteFramesWithFile(sPath);

	// cocos2d::Vector
	//Vector<AnimationFrame*> animFrames;
	Vector<SpriteFrame*> animFrames;
	//ValueMap myValueMap;

	for (int i = 0; i < allSheetNum; i++) {

		// StringUtils::format => ������ �������� ���ڿ��� ����
		std::string _frames = StringUtils::format("%s%02d.png", sName.c_str(), i);
		//myValueMap.insert(std::pair<std::string, Value>("position", ValueVector(Value(1.0f), Value(2.0f))));

		// ������ ���ڿ��� �̿��Ͽ� plist ������ SpriteFrame ������ ������
		SpriteFrame* frame = cache->getSpriteFrameByName(_frames);
		//frame->setAnchorPoint(Vec2(0, 0));
		frame->getTexture()->setAliasTexParameters();
		// ������ SpriteFrame�� ����
		//animFrames.pushBack(AnimationFrame::create(cache->getSpriteFrameByName(_frames), 1, myValueMap));
		animFrames.pushBack(frame);
	}

	// plist ������� ���� SpriteFrame ������ Ȱ���Ͽ� �ν��Ͻ� ����
	auto animation = Animation::createWithSpriteFrames(animFrames, frameDelay);
	//auto atkAnimation = Animation::createWithSpriteFrames(animFrames, frameDelay);
	auto animate = Animate::create(animation);



	auto particleBullet = Sprite::create();
	particleBullet->setPosition(x, y);
	//log("%f, %f", x, y);
	//particleBullet->setAnchorPoint(Vec2(0.75f, 0.5f));
	particleBullet->setScale(particleScale);
	auto seq = Sequence::create(animate, RemoveSelf::create(), nullptr);
	this->addChild(particleBullet);



	//auto callback = CallFuncN::create(std::bind(MainScene::removeParticleSprite, this, particleBullet));
	//auto callback = CallFunc::create(this, callfunc_selector(MainScene::removeParticleSprite));





	particleBullet->runAction(seq);




	//auto runRep = RepeatForever::create(animate);

	/*if (state == STOP_SHOOT) {

	}*/
}

void TutorialScene::createMyPhysicsWorld()
{
	// ��ũ�� �ٿ���� (Screen Boundary)�� ���� (���⼭�� ȭ�� ��ü�� ����)
	auto visibleSize = _director->getVisibleSize();

	// �߷��� ����Ǵ� ������ ���⸦ ����
	//Vect gravity = Vect(0.0f, -128.0f);
	Vect gravity = Vect(0.0f, -300.0f);

	// ���� Scene�� PhysicsWorld ������ �޾ƿ���
	m_world = this->getPhysicsWorld();

	// setDebugDrawMask() : ��ü�� ���� �ڽ��� ǥ��
	//m_world->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);

	// setGravity() : �߷��� ����� ���⸦ ����
	// �߷��� �������� ���� ��� �⺻���� Vect(0.0f, -98.0f)
	m_world->setGravity(gravity);

	// setSpeed() : ���Ͽ� ���� �߷°��ӵ��� ���� (�⺻���� 1.0f)
	m_world->setSpeed(2.0f);

	// setSubsteps() : ������ ���� �ǽ��ϴ� ���� ������ Ƚ���� ����
	// �⺻���� 1
	m_world->setSubsteps(60);

	////////////////////////////////////////////////////////

	// ��ü (Rigidbody) ���� (���� ������ ������ ��ü ����)
	// PhysicsBody::createEdgeBox
	// ��1�μ� : ���� �ùķ��̼��� ����� ������ ����
	// ��2�μ� : ��ü�� �е�, �ݹ߰��, ������� ����
	// ��3�μ� : setDebugDrawMask�� �׸��� ���� �β��� ���� (�� �β��� ������ ��� ����)
	auto body = PhysicsBody::createEdgeBox(visibleSize, PHYSICSBODY_MATERIAL_DEFAULT, 1);
	body->setCategoryBitmask(Utils::CreateMask(core::CategoryBits::BOUNDARY));
	body->setCollisionBitmask(Utils::CreateMask(core::CategoryBits::PLAYER
		, core::CategoryBits::ENEMY
		, core::CategoryBits::ENEMY_PROJECTILE));
	body->setContactTestBitmask(Utils::CreateMask(core::CategoryBits::PLAYER_PROJECTILE
		, core::CategoryBits::ENEMY_PROJECTILE));
	body->setTag(core::TagIndex::BOUNDARY);

	// ���� �ùķ��̼� ������ ���� �ֻ��� Ŭ������ Node�� �ν��Ͻ� ����
	auto edgeNode = Node::create();

	// Physics ���� ��ġ ���� (�߰��� �������� �Ű���)
	edgeNode->setPosition(Vec2(visibleSize) * 0.5f);

	// ��ü ����
	edgeNode->setPhysicsBody(body);
	this->addChild(edgeNode);
}

void TutorialScene::turnOnMoveSprite()
{
	if (!(character->isVisible())) {
		character->setVisible(true);
		attackChar->setVisible(false);
	}
}

void TutorialScene::turnOffMoveSprite()
{
	if (character->isVisible()) {
		character->setVisible(false);
		attackChar->setVisible(true);
	}
}

void TutorialScene::flippedCharacter(bool state)
{
	character->setFlippedX(state);
	attackChar->setFlippedX(state);
}

void TutorialScene::teleportNextScene()
{
	isTeleport = true;
	moveInputState = MS_TELEPORT;
	characterBody->setVelocity(Vec2::ZERO);
	characterBody->setCollisionBitmask(Utils::CreateMask(core::CategoryBits::PLATFORM));
	characterBody->setGravityEnable(false);
	moveAnimState(moveInputState);

}

void TutorialScene::setTeleportVelocity()
{
	characterBody->setVelocity(Vec2(0, 125.0f));
}

void TutorialScene::replaceMainScene()
{
	isReplaceScene = true;
	_director->replaceScene(TransitionFade::create(2.0f, ReplaceMainSceneHelper::createScene()));
}

void TutorialScene::createBullet(_Tbullets state)
{
	int allSheetNum;
	std::string sPath;
	std::string sName;
	Size size;
	float frameDelay;

	switch (state)
	{
	case COMMON_BULLET:
		allSheetNum = 3;
		sPath = "Megaman/Weapon/Weapon_Bullet.png";
		sName = "shooting_";
		size = Size(10.0f, 9.0f);
		break;

	case CHARGE_X1:
		allSheetNum = 4;
		sPath = "Megaman/Weapon/Charge_X1_Bullet.plist";
		sName = "charge_x1_bullet_";
		frameDelay = 0.05f;
		size = Size(22.0f, 15.0f);
		break;

	case CHARGE_X2:
		allSheetNum = 6;
		sPath = "Megaman/Weapon/Charge_X2_Bullet.plist";
		sName = "charge_x2_bullet_";
		frameDelay = 0.05f;
		size = Size(33.0f, 27.0f);

		break;

	}

	// źȯ ����
	auto pBullet = new Sprite();
	auto bulletPBody = PhysicsBody::createBox(size, PHYSICSBODY_MATERIAL_DEFAULT);
	bulletPBody->setGravityEnable(false);

	if (state == COMMON_BULLET) {
		pBullet = Sprite::create(sPath);
		pBullet->setPosition(shootPosition);
		bulletPBody->setTag(core::TagIndex::COMMON_BULLET);
		/*bulletPBody = PhysicsBody::createBox(size, PHYSICSBODY_MATERIAL_DEFAULT);
		bulletPBody->setGravityEnable(false);
		bulletPBody->setCategoryBitmask(3);
		bulletPBody->setContactTestBitmask(1 || 4);
		bulletPBody->setCollisionBitmask(1);
		bulletPBody->setGroup(-1);
		bulletPBody->setTag(10);
		pBullet->setPhysicsBody(bulletPBody);*/
		soundManager->PlayAttackEffect(bulletPath);
	}

	else if (state == CHARGE_X1 || state == CHARGE_X2) {
		// �ܺ� ���α׷��� ����Ͽ� ���� plist ���� �ҷ�����
		auto cache = SpriteFrameCache::getInstance();
		cache->addSpriteFramesWithFile(sPath);

		// cocos2d::Vector
		//Vector<AnimationFrame*> animFrames;
		Vector<SpriteFrame*> animFrames;
		//ValueMap myValueMap;

		for (int i = 0; i < allSheetNum; i++) {

			// StringUtils::format => ������ �������� ���ڿ��� ����
			std::string _frames = StringUtils::format("%s%02d.png", sName.c_str(), i);
			//myValueMap.insert(std::pair<std::string, Value>("position", ValueVector(Value(1.0f), Value(2.0f))));

			// ������ ���ڿ��� �̿��Ͽ� plist ������ SpriteFrame ������ ������
			SpriteFrame* frame = cache->getSpriteFrameByName(_frames);
			//frame->setAnchorPoint(Vec2(0.0f, 0.0f));
			frame->getTexture()->setAliasTexParameters();
			// ������ SpriteFrame�� ����
			//animFrames.pushBack(AnimationFrame::create(cache->getSpriteFrameByName(_frames), 1, myValueMap));
			animFrames.pushBack(frame);
		}

		// plist ������� ���� SpriteFrame ������ Ȱ���Ͽ� �ν��Ͻ� ����
		auto Animation = Animation::createWithSpriteFrames(animFrames, frameDelay);
		//auto atkAnimation = Animation::createWithSpriteFrames(animFrames, frameDelay);
		auto bulletAnim = Animate::create(Animation);
		bulletAnim->retain();



		pBullet = Sprite::create("Placeholder.png");
		pBullet->setPosition(shootPosition);

		if (state == CHARGE_X1) {
			bulletPBody->setTag(core::TagIndex::CHARGE_X1);
			soundManager->PlayAttackEffect(chargeX1Path);
		}	

		else if (state == CHARGE_X2) {
			bulletPBody->setTag(core::TagIndex::CHARGE_X2);
			soundManager->PlayAttackEffect(chargeX2Path);
		}

		/*bulletPBody = PhysicsBody::createBox(size, PHYSICSBODY_MATERIAL_DEFAULT);
		bulletPBody->setGravityEnable(false);
		bulletPBody->setCategoryBitmask(3);
		bulletPBody->setContactTestBitmask(1 || 2 || 3);
		bulletPBody->setGroup(-1);
		bulletPBody->setCollisionBitmask(1);
		bulletPBody->setTag(10);

		pBullet->setPhysicsBody(bulletPBody);*/
		pBullet->runAction(bulletAnim);
	}

	bulletPBody->setCategoryBitmask(Utils::CreateMask(core::CategoryBits::PLAYER_PROJECTILE));
	bulletPBody->setCollisionBitmask(0);
	bulletPBody->setContactTestBitmask(Utils::CreateMask(core::CategoryBits::BOUNDARY
		//, core::CategoryBits::PLATFORM
		, core::CategoryBits::ENEMY));

	pBullet->setPhysicsBody(bulletPBody);

	//bulletSprite->setPosition(shootPosition);

	/*auto callback = CallFunc::create([&]() {
			bulletSprite->setVisible(false);
			bulletSprite->removeAllChildrenWithCleanup(true);
		});
	auto seq = Sequence::create(fireParticle, callback, nullptr);
	bulletSprite->runAction(seq);
	this->addChild(bulletSprite);*/

	this->addChild(pBullet);

	// �׼� ����
	if (character->isFlippedX()) {
		pBullet->setFlippedX(true);
		//auto forward = MoveBy::create(0.8f, Vec2(-256, 0));
		bulletPBody->setVelocity(Vec2(-175.0f, 0.0f));
		//pBullet->runAction(forward);
	}

	else {
		pBullet->setFlippedX(false);
		//auto forward = MoveBy::create(0.8f, Vec2(256, 0));
		bulletPBody->setVelocity(Vec2(175.0f, 0.0f));
		//pBullet->runAction(forward);
	}
}