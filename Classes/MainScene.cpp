#include "MainScene.h"
#include "ReplaceEndSceneHelper.h"

USING_NS_CC;

cocos2d::Scene* MainScene::createScene()
{
	return MainScene::create();
}

bool MainScene::init()
{
	if (Scene::initWithPhysics())
		createMyPhysicsWorld();
	else return false;

	soundManager->StopAllSounds();

	// Main Scene (Ruined Street Map) Layer (2304x224)
	wlayer = LayerColor::create(Color4B(0, 148, 255, 255));	// Skyblue Color
	wlayer->setContentSize(Size(2304, 224));				// Main Map Sprite Size
	this->addChild(wlayer);


	// Health Bar Initializing
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

	// Init enum states
	moveInputState = MS_INIT;
	attackInputState = DEFAULT_POSE;
	moveBeforeState = MS_PLACEHOLDER;
	attackBeforeState = ATK_PLACEHOLDER;
	chargeState = COMMON_BULLET;


	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Load Tile Map (Tiled)
	tmap = TMXTiledMap::create("MainStage_Tileset/ruined_street_tileset.tmx");
	background = tmap->getLayer("Background");
	background->getTexture()->setAliasTexParameters();

	// Init Background Parallax
	auto skyImage = Sprite::create("MainStage_Tileset/Ruined_Street_Background.png");
	skyImage->setPosition(0, 0);
	skyImage->setAnchorPoint(Vec2(0.0f, 0.0f));
	skyImage->getTexture()->setAliasTexParameters();

	auto parallax = ParallaxNode::create();
	parallax->addChild(skyImage, 1, Vec2(0.86f, 1.0f), Vec2::ZERO);
	parallax->addChild(tmap, 2, Vec2(1.0f, 1.0f), Vec2::ZERO);

	wlayer->addChild(parallax, 0);

	// Init Player's Spawn Point
	auto spawnObjects = tmap->getObjectGroup("SpawnPoint");

	// 우리가 만들어놓은 Object Layer에서 SpawnPoint라고 설정한 속성 정보를 읽기
	ValueMap spawnPoint = spawnObjects->getObject("SpawnPoint");
	Value objectval = Value(spawnPoint);

	float x = spawnPoint["x"].asFloat();
	float y = spawnPoint["y"].asFloat();

	// 캐릭터를 SpwnPoint 위치에 생성할 수 있도록 좌표 준비 
	spawnPosition = Vec2(x, y + 150.0f);


	// Init Map Collider (Ground)
	auto colliderObjects = tmap->getObjectGroup("Collider");

	auto colliders = colliderObjects->getObjects();


	Vector<Sprite*> _colliderSprite;

	for (int i = 0; i < colliders.size(); i++) {

		auto size = Size(colliders.data()[i].asValueMap()["width"].asFloat(), colliders.data()[i].asValueMap()["height"].asFloat());
		auto pos = Vec2(colliders.data()[i].asValueMap()["x"].asFloat(), colliders.data()[i].asValueMap()["y"].asFloat());

		auto backCollider = PhysicsBody::createBox(size, PHYSICSBODY_MATERIAL_DEFAULT);
		backCollider->setDynamic(false);
		backCollider->setCategoryBitmask(Utils::CreateMask(core::CategoryBits::PLATFORM));

		backCollider->setCollisionBitmask(Utils::CreateMask(core::CategoryBits::PLAYER					// 물리 충돌
			, core::CategoryBits::ENEMY
			, core::CategoryBits::ENEMY_PROJECTILE));

		backCollider->setContactTestBitmask(Utils::CreateMask(core::CategoryBits::PLAYER_PROJECTILE
			, core::CategoryBits::ENEMY_PROJECTILE));  // 콜리전 이벤트

		backCollider->setTag(core::TagIndex::PLATFORM);

		auto setcol = Sprite::create();
		setcol->setContentSize(size);
		setcol->setPosition(pos);
		setcol->setAnchorPoint(Vec2(0, 0));
		setcol->setPhysicsBody(backCollider);

		_colliderSprite.pushBack(setcol);

		wlayer->addChild(_colliderSprite.at(i));
	}

	// Init Ground's DeadEnd (for Enemies)
	auto deadEndObjects = tmap->getObjectGroup("DeadEnd");
	auto deadEnds = deadEndObjects->getObjects();

	Vector<Sprite*> _deadEndSprite;

	for (int i = 0; i < deadEnds.size(); i++) {

		auto size = Size(deadEnds.data()[i].asValueMap()["width"].asFloat(), deadEnds.data()[i].asValueMap()["height"].asFloat());
		auto pos = Vec2(deadEnds.data()[i].asValueMap()["x"].asFloat(), deadEnds.data()[i].asValueMap()["y"].asFloat());

		auto deadEndCollider = PhysicsBody::createBox(size, PHYSICSBODY_MATERIAL_DEFAULT);
		deadEndCollider->setDynamic(false);
		deadEndCollider->setCategoryBitmask(Utils::CreateMask(core::CategoryBits::DEADEND));

		deadEndCollider->setCollisionBitmask(0);

		deadEndCollider->setContactTestBitmask(Utils::CreateMask(core::CategoryBits::ENEMY));

		deadEndCollider->setTag(core::TagIndex::DEADEND);
		//backCollider->setGravityEnable(false);
		auto settingdeadEnd = Sprite::create();
		settingdeadEnd->setContentSize(size);
		settingdeadEnd->setPosition(pos);
		settingdeadEnd->setAnchorPoint(Vec2(0, 0));
		settingdeadEnd->setPhysicsBody(deadEndCollider);

		_deadEndSprite.pushBack(settingdeadEnd);

		wlayer->addChild(_deadEndSprite.at(i));
	}

	// Trigger Event for Player (Enter Boss Stage)
	auto TriggerPointObjects = tmap->getObjectGroup("TriggerPoint");
	auto triggerPoints = TriggerPointObjects->getObjects();

	Vector<Sprite*> _TriggerPointCollider;

	for (int i = 0; i < triggerPoints.size(); i++) {

		auto size = Size(triggerPoints.data()[i].asValueMap()["width"].asFloat(), triggerPoints.data()[i].asValueMap()["height"].asFloat());
		auto pos = Vec2(triggerPoints.data()[i].asValueMap()["x"].asFloat(), triggerPoints.data()[i].asValueMap()["y"].asFloat());

		auto triggerCollider = PhysicsBody::createBox(size, PHYSICSBODY_MATERIAL_DEFAULT);
		triggerCollider->setDynamic(false);
		triggerCollider->setCategoryBitmask(Utils::CreateMask(core::CategoryBits::TRIGGER_POINT));

		triggerCollider->setCollisionBitmask(Utils::CreateMask(0));

		triggerCollider->setContactTestBitmask(Utils::CreateMask(core::CategoryBits::PLAYER));  // 콜리전 이벤트

		triggerCollider->setTag(core::TagIndex::TRIGGER_POINT);

		auto setcol = Sprite::create();
		setcol->setContentSize(size);
		setcol->setPosition(pos);
		setcol->setAnchorPoint(Vec2(0, 0));
		setcol->setPhysicsBody(triggerCollider);

		_TriggerPointCollider.pushBack(setcol);

		wlayer->addChild(_TriggerPointCollider.at(i));
	}

	// Boss's Move Point Trigger Point
	auto bossMoveObjects = tmap->getObjectGroup("BossMovePoint");
	auto bossMovePoints = bossMoveObjects->getObjects();
	_BossMoveCollider.reserve(10);
	for (int i = 0; i < bossMovePoints.size(); i++) {

		auto size = Size(bossMovePoints.data()[i].asValueMap()["width"].asFloat(), bossMovePoints.data()[i].asValueMap()["height"].asFloat());
		auto pos = Vec2(bossMovePoints.data()[i].asValueMap()["x"].asFloat(), bossMovePoints.data()[i].asValueMap()["y"].asFloat());

		auto bossMoveCollider = PhysicsBody::createBox(size, PHYSICSBODY_MATERIAL_DEFAULT);
		bossMoveCollider->setDynamic(false);
		bossMoveCollider->setCategoryBitmask(Utils::CreateMask(core::CategoryBits::BOSS_MOVE_POINT));

		bossMoveCollider->setCollisionBitmask(Utils::CreateMask(0));

		bossMoveCollider->setContactTestBitmask(Utils::CreateMask(core::CategoryBits::ENEMY));  // 콜리전 이벤트

		bossMoveCollider->setTag(core::TagIndex::BOSS_MOVE_POINT);

		auto setcol = Sprite::create();
		setcol->setContentSize(size);
		setcol->setPosition(pos);
		setcol->setAnchorPoint(Vec2(0, 0));
		setcol->setPhysicsBody(bossMoveCollider);

		_BossMoveCollider.emplace(i, setcol);

		wlayer->addChild(_BossMoveCollider[i]);
	}

	_BossMoveCollider[1]->getPhysicsBody()->setEnabled(false);
	_BossMoveCollider[2]->getPhysicsBody()->setEnabled(false);
	

	// Return Boss's Shooting Blade Boundary
	auto bladeBoundaryObjects = tmap->getObjectGroup("BladeWall");
	ValueVector bladeBoundarys = bladeBoundaryObjects->getObjects();
	_BladeBoundaryCollider.reserve(10);
	for (int i = 0; i < bladeBoundarys.size(); i++) {

		auto boundary_size = Size(bladeBoundarys.data()[i].asValueMap()["width"].asFloat(), bladeBoundarys.data()[i].asValueMap()["height"].asFloat());
		auto boundary_pos = Vec2(bladeBoundarys.data()[i].asValueMap()["x"].asFloat(), bladeBoundarys.data()[i].asValueMap()["y"].asFloat());

		auto bladeBoundaryCollider = PhysicsBody::createBox(boundary_size, PHYSICSBODY_MATERIAL_DEFAULT);
		bladeBoundaryCollider->setDynamic(false);
		bladeBoundaryCollider->setCategoryBitmask(Utils::CreateMask(core::CategoryBits::BLADE_BOUNDARY));

		bladeBoundaryCollider->setCollisionBitmask(Utils::CreateMask(0));

		bladeBoundaryCollider->setContactTestBitmask(Utils::CreateMask(core::CategoryBits::ENEMY_PROJECTILE));  // 콜리전 이벤트

		bladeBoundaryCollider->setTag(core::TagIndex::BLADE_BOUNDARY);

		auto setcol = Sprite::create();
		setcol->setContentSize(boundary_size);
		setcol->setPosition(boundary_pos);
		setcol->setAnchorPoint(Vec2(0, 0));
		setcol->setPhysicsBody(bladeBoundaryCollider);

		_BladeBoundaryCollider.emplace(i, setcol);

		wlayer->addChild(_BladeBoundaryCollider[i]);
	}


	_BladeBoundaryCollider[1]->getPhysicsBody()->setEnabled(false);

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Character Parent
	charLayer = Sprite::create("Placeholder.png");
	charLayer->setScale(1.0f);
	charLayer->setPosition(spawnPosition);
	charLayer->setAnchorPoint(Vec2(0.5, 0.25));
	charLayer->getTexture()->setAliasTexParameters();
	

	// Movement Sprite
	character = Sprite::create("init_teleport.png");
	character->setAnchorPoint(Vec2::ZERO);
	character->getTexture()->setAliasTexParameters();

	// Attack Sprite
	attackChar = Sprite::create("test_megaman.png");
	attackChar->setAnchorPoint(Vec2::ZERO);
	attackChar->getTexture()->setAliasTexParameters();

	// Player's Physics
	auto material1 = PhysicsMaterial(1.0f, 0.0f, 0.0f);
	characterBody = PhysicsBody::createBox(Size(32, 40), material1, Vec2(0.5f, 0.5f));

	characterBody->setMass(1.0f);
	//characterBody->setGravityEnable(false);
	//characterBody->setDynamic(false);
	characterBody->setRotationEnable(false);
	characterBody->setCategoryBitmask(Utils::CreateMask(core::CategoryBits::PLAYER));

	characterBody->setCollisionBitmask(Utils::CreateMask(core::CategoryBits::PLATFORM));

	characterBody->setContactTestBitmask(Utils::CreateMask(core::CategoryBits::ENEMY
														, core::CategoryBits::ENEMY_PROJECTILE
														, core::CategoryBits::ENEMY_AWAKE_SENSOR
														, core::CategoryBits::TRIGGER_POINT));
	
	characterBody->setTag(core::TagIndex::PLAYER);
	characterBody->setGravityEnable(false);

	// Init bools
	fall = false;
	movingLeft = false;
	movingRight = false;
	movingUp = false;
	movingSliding = false;
	inputShoot = false;
	beforeRunShoot = false;
	isInited = true;
	isChangeMoveSprite = false;
	isStuck = false;
	isDead = false;
	_node = nullptr;
	startPauseTimer = false;

	// Init Player's Shooting Bullet Position
	shootPosition = Vec2(characterBody->getPosition().x - (atk_stop_pos[0][0] + 3), characterBody->getPosition().y + atk_stop_pos[0][1]);

	charLayer->setPhysicsBody(characterBody);
	charLayer->addChild(character);
	charLayer->addChild(attackChar);
	
	attackChar->setVisible(false);
	
	wlayer->addChild(charLayer, 3);

	// Init Enemies's Array size
	bunbys.reserve(10);
	metalls.reserve(10);

	// Spawn Enemies 1 (Metall)
	auto metallSpawnObjects = tmap->getObjectGroup("MetallSpawn");
	auto metallSpawnPoints = metallSpawnObjects->getObjects();
	metallsNum = metallSpawnPoints.size();

	for (int i = 0; i < metallsNum; i++) {
		auto pos = Vec2(metallSpawnPoints.data()[i].asValueMap()["x"].asFloat(), metallSpawnPoints.data()[i].asValueMap()["y"].asFloat());

		auto metall{ Enemy_Metall::create(wlayer, pos.x, pos.y) };
		metall->setPlayerPos(characterBody);
		metalls.emplace(i, metall);

		wlayer->addChild(metalls[i]);
	}
	log("metalls Size : %d", metalls.size());

	// Spawn Enemies 2 (Bunby Tank)
	auto bunbySpawnObjects = tmap->getObjectGroup("BunbySpawn");
	auto bunbySpawnPoints = bunbySpawnObjects->getObjects();
	bunbysNum = bunbySpawnPoints.size();
	for (int i = 0; i < bunbysNum; i++) {
		auto pos = Vec2(bunbySpawnPoints.data()[i].asValueMap()["x"].asFloat(), bunbySpawnPoints.data()[i].asValueMap()["y"].asFloat());

		auto bunby{ Enemy_Bunby::create(wlayer, pos.x, pos.y) };
		bunby->setPlayerPos(characterBody);
		bunbys.emplace(i, bunby);

		wlayer->addChild(bunbys[i]);
	}
	log("bunbys Size : %d", bunbys.size());

	// Spawn Boss (Mad Grinder)
	auto bossSpawnObjects = tmap->getObjectGroup("BossSpawn");
	ValueMap bossSpawnPoint = bossSpawnObjects->getObject("BossSpawn");
	Value objectval_2 = Value(bossSpawnPoint);

	float bossX = bossSpawnPoint["x"].asFloat();
	float bossY = bossSpawnPoint["y"].asFloat();

	madGrinder = Enemy_Boss::create(wlayer, bossX, bossY);

	wlayer->addChild(madGrinder);

	////////////////////////////////////////////////////////
	
	// Init Physics Contact(Collision) Event
	auto contact = EventListenerPhysicsContact::create();

	// 충돌 이벤트 콜백 함수 목록
	/*
		1) onContactBegin
			- 접촉했을 때

		2) onContactPresolve
			- 접촉하기 직전

		3) onContactPostSolve
			- 떨어지기 직전

		4) onContactSeparate
			- 떨어질 때
	*/
	contact->onContactBegin = [this](PhysicsContact& contact) -> bool {
		// Node 클래스를 가져오기
		/*
			contact : 충돌 정보를 관리하는 PhysicsContact 인스턴스
			getShapeA&B : 충돌한 PhysicsShape 가져오기
			getBody : PhysicsShape에서 PhysicsBody 가져오기
			getNode : PhysicsBody에서 이 Rigidbody가 접목된 Node를 가져오기
		*/

		// nodeA : 피충돌 / nodeB : 충돌
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

				auto world = wlayer->convertToNodeSpace(Vec2(posX, posY));
				//log("World Pos : %f, %f", world.x, world.y);
				bulletParticleAnimState(COMMON_BULLET, world.x, world.y);

				if (nodeB == core::TagIndex::METALL) {
					if ((static_cast<Enemy_Metall*>(contact.getShapeB()->getBody()->getNode()->getParent())->isAwake)) {
						static_cast<Enemy_Metall*>(contact.getShapeB()->getBody()->getNode()->getParent())->isHit = true;
						static_cast<Enemy_Metall*>(contact.getShapeB()->getBody()->getNode()->getParent())->metallHealth -= 1.0f;
					}

					else soundManager->PlayTinkEffect(soundManager->metallTinkPath);
				}

				if (nodeB == core::TagIndex::BUNBY_HEAD) {
					static_cast<Enemy_Bunby*>(contact.getShapeB()->getBody()->getNode()->getParent()->getParent())->isHit = true;
					static_cast<Enemy_Bunby*>(contact.getShapeB()->getBody()->getNode()->getParent()->getParent())->bunbyHeadHealth -= 1.0f;
				}

				if (nodeB == core::TagIndex::BUNBY_TANK) {
					static_cast<Enemy_Bunby*>(contact.getShapeB()->getBody()->getNode()->getParent())->isHit = true;
					static_cast<Enemy_Bunby*>(contact.getShapeB()->getBody()->getNode()->getParent())->bunbyTankHealth -= 1.0f;
				}

				if (nodeB == core::TagIndex::BOSS) {
					madGrinder->isHit = true;
					madGrinder->bossHealth -= 1.0f;
				}

				if (nodeB == core::TagIndex::BOSS_SHIELD) {
					soundManager->PlayTinkEffect(soundManager->metallTinkPath);
				}
			}

			else if (nodeA == core::TagIndex::CHARGE_X1) {
				float targetHealth = 1;
				if (nodeB == core::TagIndex::METALL) {
					if ((static_cast<Enemy_Metall*>(contact.getShapeB()->getBody()->getNode()->getParent())->isAwake)) {
						static_cast<Enemy_Metall*>(contact.getShapeB()->getBody()->getNode()->getParent())->isHit = true;
						static_cast<Enemy_Metall*>(contact.getShapeB()->getBody()->getNode()->getParent())->metallHealth -= 2.0f;
					}
					else soundManager->PlayTinkEffect(soundManager->metallTinkPath);

					targetHealth = static_cast<Enemy_Metall*>(contact.getShapeB()->getBody()->getNode()->getParent())->metallHealth;
				}

				if (nodeB == core::TagIndex::BUNBY_HEAD) {
					static_cast<Enemy_Bunby*>(contact.getShapeB()->getBody()->getNode()->getParent()->getParent())->isHit = true;
					static_cast<Enemy_Bunby*>(contact.getShapeB()->getBody()->getNode()->getParent()->getParent())->bunbyHeadHealth -= 2.0f;
					targetHealth = static_cast<Enemy_Bunby*>(contact.getShapeB()->getBody()->getNode()->getParent()->getParent())->bunbyHeadHealth;
					//log("Bunby Head Health : %f", static_cast<Enemy_Bunby*>(contact.getShapeB()->getBody()->getNode()->getParent()->getParent())->bunbyHeadHealth);
				}

				if (nodeB == core::TagIndex::BUNBY_TANK) {
					static_cast<Enemy_Bunby*>(contact.getShapeB()->getBody()->getNode()->getParent())->isHit = true;
					static_cast<Enemy_Bunby*>(contact.getShapeB()->getBody()->getNode()->getParent())->bunbyTankHealth -= 2.0f;
					targetHealth = static_cast<Enemy_Bunby*>(contact.getShapeB()->getBody()->getNode()->getParent())->bunbyTankHealth;
					//log("Bunby Body Health : %f", static_cast<Enemy_Bunby*>(contact.getShapeB()->getBody()->getNode()->getParent())->bunbyTankHealth);
				}

				if (nodeB == core::TagIndex::BOSS) {
					madGrinder->isHit = true;
					madGrinder->bossHealth -= 2.0f;
				}

				if (nodeB == core::TagIndex::BOSS_SHIELD) {
					soundManager->PlayTinkEffect(soundManager->metallTinkPath);
				}

				if (targetHealth > 0) {
					contact.getShapeA()->getBody()->getNode()->removeFromParentAndCleanup(true);

					auto posX = contact.getContactData()->points->x;
					auto posY = contact.getShapeA()->getBody()->getPosition().y;

					auto world = wlayer->convertToNodeSpace(Vec2(posX, posY));
					//log("World Pos : %f, %f", world.x, world.y);
					bulletParticleAnimState(CHARGE_X1, world.x, world.y);
				}
			}

			else if (nodeA == core::TagIndex::CHARGE_X2) {
				float targetHealth = 1;
				if (nodeB == core::TagIndex::METALL) {
					if ((static_cast<Enemy_Metall*>(contact.getShapeB()->getBody()->getNode()->getParent())->isAwake)) {
						static_cast<Enemy_Metall*>(contact.getShapeB()->getBody()->getNode()->getParent())->isHit = true;
						static_cast<Enemy_Metall*>(contact.getShapeB()->getBody()->getNode()->getParent())->metallHealth -= 4.0f;
					}	

					else soundManager->PlayTinkEffect(soundManager->metallTinkPath);
					targetHealth = static_cast<Enemy_Metall*>(contact.getShapeB()->getBody()->getNode()->getParent())->metallHealth;
				}

				if (nodeB == core::TagIndex::BUNBY_HEAD) {
					static_cast<Enemy_Bunby*>(contact.getShapeB()->getBody()->getNode()->getParent()->getParent())->isHit = true;
					static_cast<Enemy_Bunby*>(contact.getShapeB()->getBody()->getNode()->getParent()->getParent())->bunbyHeadHealth -= 4.0f;
					targetHealth = static_cast<Enemy_Bunby*>(contact.getShapeB()->getBody()->getNode()->getParent()->getParent())->bunbyHeadHealth;
					log("Bunby Head Health : %f", static_cast<Enemy_Bunby*>(contact.getShapeB()->getBody()->getNode()->getParent()->getParent())->bunbyHeadHealth);
				}

				if (nodeB == core::TagIndex::BUNBY_TANK) {
					static_cast<Enemy_Bunby*>(contact.getShapeB()->getBody()->getNode()->getParent())->isHit = true;
					static_cast<Enemy_Bunby*>(contact.getShapeB()->getBody()->getNode()->getParent())->bunbyTankHealth -= 4.0f;
					targetHealth = static_cast<Enemy_Bunby*>(contact.getShapeB()->getBody()->getNode()->getParent())->bunbyTankHealth;
					log("Bunby Body Health : %f", static_cast<Enemy_Bunby*>(contact.getShapeB()->getBody()->getNode()->getParent())->bunbyTankHealth);
				}

				if (nodeB == core::TagIndex::BOSS) {
					madGrinder->isHit = true;
					madGrinder->bossHealth -= 4.0f;
				}

				if (nodeB == core::TagIndex::BOSS_SHIELD) {
					soundManager->PlayTinkEffect(soundManager->metallTinkPath);
				}

				if (targetHealth > 0) {
					contact.getShapeA()->getBody()->getNode()->removeFromParentAndCleanup(true);

					auto posX = contact.getContactData()->points->x;
					auto posY = contact.getShapeA()->getBody()->getPosition().y;

					auto world = wlayer->convertToNodeSpace(Vec2(posX, posY));
					//log("World Pos : %f, %f", world.x, world.y);
					bulletParticleAnimState(CHARGE_X2, world.x, world.y);
				}
			}

			else if (nodeA == core::TagIndex::METALL_BULLET) {
				contact.getShapeA()->getBody()->getNode()->removeFromParentAndCleanup(true);
				auto posX = contact.getContactData()->points->x;
				auto posY = contact.getShapeA()->getBody()->getPosition().y;

				auto world = wlayer->convertToNodeSpace(Vec2(posX, posY));
				//log("World Pos : %f, %f", world.x, world.y);
				bulletParticleAnimState(COMMON_BULLET, world.x, world.y);
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
				
				auto world = wlayer->convertToNodeSpace(Vec2(posX, posY));
				//log("World Pos : %f, %f", world.x, world.y);
				bulletParticleAnimState(COMMON_BULLET, world.x, world.y);

				if (nodeA == core::TagIndex::METALL) {
					if ((static_cast<Enemy_Metall*>(contact.getShapeA()->getBody()->getNode()->getParent())->isAwake)) {
						static_cast<Enemy_Metall*>(contact.getShapeA()->getBody()->getNode()->getParent())->isHit = true;
						static_cast<Enemy_Metall*>(contact.getShapeA()->getBody()->getNode()->getParent())->metallHealth -= 1.0f;
					}
					else soundManager->PlayTinkEffect(soundManager->metallTinkPath);
				}

				if (nodeA == core::TagIndex::BUNBY_HEAD) {
					static_cast<Enemy_Bunby*>(contact.getShapeA()->getBody()->getNode()->getParent()->getParent())->isHit = true;
					static_cast<Enemy_Bunby*>(contact.getShapeA()->getBody()->getNode()->getParent()->getParent())->bunbyHeadHealth -= 1.0f;
					//log("Bunby Head Health : %f", static_cast<Enemy_Bunby*>(contact.getShapeA()->getBody()->getNode()->getParent()->getParent())->bunbyHeadHealth);
				}

				if (nodeA == core::TagIndex::BUNBY_TANK) {
					static_cast<Enemy_Bunby*>(contact.getShapeA()->getBody()->getNode()->getParent())->isHit = true;
					static_cast<Enemy_Bunby*>(contact.getShapeA()->getBody()->getNode()->getParent())->bunbyTankHealth -= 1.0f;
					log("Bunby Tank Health : %f", static_cast<Enemy_Bunby*>(contact.getShapeA()->getBody()->getNode()->getParent())->bunbyTankHealth);
				}

				if (nodeA == core::TagIndex::BOSS) {
					madGrinder->isHit = true;
					madGrinder->bossHealth -= 1.0f;
				}

				if (nodeA == core::TagIndex::BOSS_SHIELD) {
					soundManager->PlayTinkEffect(soundManager->metallTinkPath);
				}
			}

			else if (nodeB == core::TagIndex::CHARGE_X1) {
				float targetHealth = 1;
				if (nodeA == core::TagIndex::METALL) {
					if ((static_cast<Enemy_Metall*>(contact.getShapeA()->getBody()->getNode()->getParent())->isAwake)) {
						static_cast<Enemy_Metall*>(contact.getShapeA()->getBody()->getNode()->getParent())->isHit = true;
						static_cast<Enemy_Metall*>(contact.getShapeA()->getBody()->getNode()->getParent())->metallHealth -= 2.0f;
					}
					else soundManager->PlayTinkEffect(soundManager->metallTinkPath);
					targetHealth = static_cast<Enemy_Metall*>(contact.getShapeA()->getBody()->getNode()->getParent())->metallHealth;
				}

				if (nodeA == core::TagIndex::BUNBY_HEAD) {
					static_cast<Enemy_Bunby*>(contact.getShapeA()->getBody()->getNode()->getParent()->getParent())->isHit = true;
					static_cast<Enemy_Bunby*>(contact.getShapeA()->getBody()->getNode()->getParent()->getParent())->bunbyHeadHealth -= 2.0f;
					targetHealth = static_cast<Enemy_Bunby*>(contact.getShapeA()->getBody()->getNode()->getParent()->getParent())->bunbyHeadHealth;
					//log("Bunby Head Health : %f", static_cast<Enemy_Bunby*>(contact.getShapeA()->getBody()->getNode()->getParent()->getParent())->bunbyHeadHealth);
				}

				if (nodeA == core::TagIndex::BUNBY_TANK) {
					static_cast<Enemy_Bunby*>(contact.getShapeA()->getBody()->getNode()->getParent())->isHit = true;
					static_cast<Enemy_Bunby*>(contact.getShapeA()->getBody()->getNode()->getParent())->bunbyTankHealth -= 2.0f;
					targetHealth = static_cast<Enemy_Bunby*>(contact.getShapeA()->getBody()->getNode()->getParent())->bunbyTankHealth;
					//log("Bunby Tank Health : %f", static_cast<Enemy_Bunby*>(contact.getShapeA()->getBody()->getNode()->getParent())->bunbyTankHealth);
				}

				if (nodeA == core::TagIndex::BOSS) {
					madGrinder->isHit = true;
					madGrinder->bossHealth -= 2.0f;
				}

				if (nodeA == core::TagIndex::BOSS_SHIELD) {
					soundManager->PlayTinkEffect(soundManager->metallTinkPath);
				}
				
				if (targetHealth > 0) {
					contact.getShapeB()->getBody()->getNode()->removeFromParentAndCleanup(true);

					auto posX = contact.getContactData()->points->x;
					auto posY = contact.getShapeB()->getBody()->getPosition().y;

					auto world = wlayer->convertToNodeSpace(Vec2(posX, posY));
					//log("World Pos : %f, %f", world.x, world.y);
					bulletParticleAnimState(CHARGE_X1, world.x, world.y);
				}
			}

			else if (nodeB == core::TagIndex::CHARGE_X2) {
				float targetHealth = 1;
				if (nodeA == core::TagIndex::METALL) {
					if ((static_cast<Enemy_Metall*>(contact.getShapeA()->getBody()->getNode()->getParent())->isAwake)) {
						static_cast<Enemy_Metall*>(contact.getShapeA()->getBody()->getNode()->getParent())->isHit = true;
						static_cast<Enemy_Metall*>(contact.getShapeA()->getBody()->getNode()->getParent())->metallHealth -= 4.0f;
					}
					else soundManager->PlayTinkEffect(soundManager->metallTinkPath);
					targetHealth = static_cast<Enemy_Metall*>(contact.getShapeA()->getBody()->getNode()->getParent())->metallHealth;
				}

				if (nodeA == core::TagIndex::BUNBY_HEAD) {
					static_cast<Enemy_Bunby*>(contact.getShapeA()->getBody()->getNode()->getParent()->getParent())->isHit = true;
					static_cast<Enemy_Bunby*>(contact.getShapeA()->getBody()->getNode()->getParent()->getParent())->bunbyHeadHealth -= 4.0f;
					targetHealth = static_cast<Enemy_Bunby*>(contact.getShapeA()->getBody()->getNode()->getParent()->getParent())->bunbyHeadHealth;
					log("Bunby Head Health : %f", static_cast<Enemy_Bunby*>(contact.getShapeA()->getBody()->getNode()->getParent()->getParent())->bunbyHeadHealth);
				}

				if (nodeA == core::TagIndex::BUNBY_TANK) {
					static_cast<Enemy_Bunby*>(contact.getShapeA()->getBody()->getNode()->getParent())->isHit = true;
					static_cast<Enemy_Bunby*>(contact.getShapeA()->getBody()->getNode()->getParent())->bunbyTankHealth -= 4.0f;
					targetHealth = static_cast<Enemy_Bunby*>(contact.getShapeA()->getBody()->getNode()->getParent())->bunbyTankHealth;
					log("Bunby Tank Health : %f", static_cast<Enemy_Bunby*>(contact.getShapeA()->getBody()->getNode()->getParent())->bunbyTankHealth);
				}

				if (nodeA == core::TagIndex::BOSS) {
					madGrinder->isHit = true;
					madGrinder->bossHealth -= 4.0f;
				}

				if (nodeA == core::TagIndex::BOSS_SHIELD) {
					soundManager->PlayTinkEffect(soundManager->metallTinkPath);
				}

				if (targetHealth > 0) {
					contact.getShapeB()->getBody()->getNode()->removeFromParentAndCleanup(true);

					auto posX = contact.getContactData()->points->x;
					auto posY = contact.getShapeB()->getBody()->getPosition().y;

					auto world = wlayer->convertToNodeSpace(Vec2(posX, posY));
					//log("World Pos : %f, %f", world.x, world.y);
					bulletParticleAnimState(CHARGE_X2, world.x, world.y);
				}
			}

			else if (nodeB == core::TagIndex::METALL_BULLET) {
				contact.getShapeB()->getBody()->getNode()->removeFromParentAndCleanup(true);
				auto posX = contact.getContactData()->points->x;
				auto posY = contact.getShapeB()->getBody()->getPosition().y;

				auto world = wlayer->convertToNodeSpace(Vec2(posX, posY));
				//log("World Pos : %f, %f", world.x, world.y);
				bulletParticleAnimState(COMMON_BULLET, world.x, world.y);
			}
		}
		///////////////////////////////////////////////////////////////////////////////////////////////////////////	

		if (nodeA == core::TagIndex::AWAKE_SENSOR) {
			auto a = static_cast<Enemy_Metall*>(contact.getShapeA()->getBody()->getNode()->getParent()->getParent());
			a->innerSensor = true;
		}

		else if (nodeB == core::TagIndex::AWAKE_SENSOR) {
			auto b = static_cast<Enemy_Metall*>(contact.getShapeB()->getBody()->getNode()->getParent()->getParent());
			b->innerSensor = true;
		}

		if (nodeA == core::TagIndex::DEADEND) {
			switch (nodeB)
			{
			case core::TagIndex::METALL:
				log("Contact DeadEnd");
				static_cast<Enemy_Metall*>(contact.getShapeB()->getBody()->getNode()->getParent())->isDeadEnd = true;
				static_cast<Enemy_Metall*>(contact.getShapeB()->getBody()->getNode()->getParent())->contactDeadEnd();
				break;

			case core::TagIndex::BUNBY_TANK:
				static_cast<Enemy_Bunby*>(contact.getShapeB()->getBody()->getNode()->getParent())->isDeadEnd = true;
				static_cast<Enemy_Bunby*>(contact.getShapeB()->getBody()->getNode()->getParent())->contactDeadEnd();
				break;
			}
		}

		if (nodeB == core::TagIndex::DEADEND) {
			switch (nodeA)
			{
			case core::TagIndex::METALL:
				log("Contact DeadEnd");
				static_cast<Enemy_Metall*>(contact.getShapeA()->getBody()->getNode()->getParent())->isDeadEnd = true;
				static_cast<Enemy_Metall*>(contact.getShapeA()->getBody()->getNode()->getParent())->contactDeadEnd();
				break;

			case core::TagIndex::BUNBY_TANK:
				static_cast<Enemy_Bunby*>(contact.getShapeA()->getBody()->getNode()->getParent())->isDeadEnd = true;
				static_cast<Enemy_Bunby*>(contact.getShapeA()->getBody()->getNode()->getParent())->contactDeadEnd();
				break;
			}
		}

		if (nodeA == core::TagIndex::PLAYER
			&& (nodeB == core::TagIndex::METALL || nodeB == core::TagIndex::METALL_BULLET || nodeB == core::TagIndex::BUNBY_HEAD || nodeB == core::TagIndex::BUNBY_TANK || nodeB == core::TagIndex::BOSS || nodeB == core::TagIndex::BOSS_SHIELD || nodeB == core::TagIndex::BOSS_BLADE))
		{
			switch (nodeB)
			{
			case core::TagIndex::METALL :
				CollideEnemy = true;
				contactingDamage = core::EnemyDamage::METALL_DMG;
				break;

			case core::TagIndex::METALL_BULLET :
				CollideEnemyBullet = true;
				contactingDamage = core::EnemyDamage::METALL_BULLET_DMG;
				break;

			case core::TagIndex::BUNBY_HEAD :
				CollideEnemy = true;
				contactingDamage = core::EnemyDamage::BUNBY_DMG;
				break;

			case core::TagIndex::BUNBY_TANK:
				CollideEnemy = true;
				contactingDamage = core::EnemyDamage::BUNBY_DMG;
				break;

			case core::TagIndex::BOSS:
				CollideEnemy = true;
				contactingDamage = core::EnemyDamage::BOSS_DMG;
				break;

			case core::TagIndex::BOSS_SHIELD:
				CollideEnemy = true;
				contactingDamage = core::EnemyDamage::BOSS_DMG;
				break;

			case core::TagIndex::BOSS_BLADE:
				CollideEnemyBullet = true;
				contactingDamage = core::EnemyDamage::BOSS_BLADE_DMG;
				break;
			}
		}

		if (nodeB == core::TagIndex::PLAYER
			&& (nodeA == core::TagIndex::METALL || nodeA == core::TagIndex::METALL_BULLET || nodeA == core::TagIndex::BUNBY_HEAD || nodeA == core::TagIndex::BUNBY_TANK || nodeA == core::TagIndex::BOSS || nodeA == core::TagIndex::BOSS_SHIELD || nodeA == core::TagIndex::BOSS_BLADE))
		{
			switch (nodeA)
			{
			case core::TagIndex::METALL:
				CollideEnemy = true;
				contactingDamage = core::EnemyDamage::METALL_DMG;
				break;

			case core::TagIndex::METALL_BULLET:
				CollideEnemyBullet = true;
				contactingDamage = core::EnemyDamage::METALL_BULLET_DMG;
				break;

			case core::TagIndex::BUNBY_HEAD:
				CollideEnemy = true;
				contactingDamage = core::EnemyDamage::BUNBY_DMG;
				break;

			case core::TagIndex::BUNBY_TANK:
				CollideEnemy = true;
				contactingDamage = core::EnemyDamage::BUNBY_DMG;
				break;

			case core::TagIndex::BOSS:
				CollideEnemy = true;
				contactingDamage = core::EnemyDamage::BOSS_DMG;
				break;

			case core::TagIndex::BOSS_SHIELD:
				CollideEnemy = true;
				contactingDamage = core::EnemyDamage::BOSS_DMG;
				break;

			case core::TagIndex::BOSS_BLADE:
				CollideEnemyBullet = true;
				contactingDamage = core::EnemyDamage::BOSS_BLADE_DMG;
				break;
			}
		}

		if (nodeA == core::TagIndex::TRIGGER_POINT) {
			if (!isTriggedFirst) {
				followWidth = 256.01f;
				followX = 768.0f;
				isTriggingFirst = true;
				isTriggedFirst = true;
				contact.getShapeA()->getBody()->getNode()->removeFromParentAndCleanup(true);
			}

			else if (isTriggedFirst && !isTriggedSecond) {
				cutsceneBoss = true;
				isTriggingSecond = true;
				isTriggedSecond = true;
				madGrinder->appearBoss();
				contact.getShapeA()->getBody()->getNode()->removeFromParentAndCleanup(true);
			}
		}

		if (nodeB == core::TagIndex::TRIGGER_POINT) {
			if (!isTriggedFirst) {
				followWidth = 256.01f;
				followX = 768.0f;
				isTriggingFirst = true;
				isTriggedFirst = true;
				contact.getShapeB()->getBody()->getNode()->removeFromParentAndCleanup(true);
			}

			else if (isTriggedFirst && !isTriggedSecond) {
				cutsceneBoss = true;
				isTriggingSecond = true;
				isTriggedSecond = true;
				madGrinder->appearBoss();
				contact.getShapeB()->getBody()->getNode()->removeFromParentAndCleanup(true);
			}
		}

		if (nodeA == core::TagIndex::BOSS_MOVE_POINT) {
			
			if (!isTriggedThird) {
				
				madGrinder->setReadySequence();

				auto _node = Node::create();
				auto callback = CallFunc::create(this, callfunc_selector(MainScene::startBossStage));
				auto seq = Sequence::create(DelayTime::create(1.0f), callback, RemoveSelf::create(), nullptr);

				this->addChild(_node);
				_node->runAction(seq);

				_BossMoveCollider[0]->getPhysicsBody()->setEnabled(false);

				if (madGrinder->sequenceNum == 1 || madGrinder->sequenceNum == 4) {
					_BossMoveCollider[2]->getPhysicsBody()->setEnabled(false);
					_BossMoveCollider[1]->getPhysicsBody()->setEnabled(true);
					_BladeBoundaryCollider[1]->getPhysicsBody()->setEnabled(false);
					_BladeBoundaryCollider[0]->getPhysicsBody()->setEnabled(true);
				}

				else if (madGrinder->sequenceNum == 2 || madGrinder->sequenceNum == 3) {
					_BossMoveCollider[2]->getPhysicsBody()->setEnabled(true);
					_BossMoveCollider[1]->getPhysicsBody()->setEnabled(false);
					_BladeBoundaryCollider[1]->getPhysicsBody()->setEnabled(true);
					_BladeBoundaryCollider[0]->getPhysicsBody()->setEnabled(false);
				}
			}

			else {
				if (!(madGrinder->isDead)) {
					madGrinder->nextSequence();
				}
			}
			
		}

		if (nodeB == core::TagIndex::BOSS_MOVE_POINT) {
			
			if (!isTriggedThird) {

				madGrinder->setReadySequence();

				
				auto _node = Node::create();
				auto callback = CallFunc::create(this, callfunc_selector(MainScene::startBossStage));
				auto seq = Sequence::create(DelayTime::create(1.0f), callback, RemoveSelf::create(), nullptr);

				this->addChild(_node);
				_node->runAction(seq);

				_BossMoveCollider[0]->getPhysicsBody()->setEnabled(false);

				if (madGrinder->sequenceNum == 1 || madGrinder->sequenceNum == 4) {
					_BossMoveCollider[2]->getPhysicsBody()->setEnabled(false);
					_BossMoveCollider[1]->getPhysicsBody()->setEnabled(true);
					_BladeBoundaryCollider[1]->getPhysicsBody()->setEnabled(false);
					_BladeBoundaryCollider[0]->getPhysicsBody()->setEnabled(true);
				}

				else if (madGrinder->sequenceNum == 2 || madGrinder->sequenceNum == 3) {
					_BossMoveCollider[2]->getPhysicsBody()->setEnabled(true);
					_BossMoveCollider[1]->getPhysicsBody()->setEnabled(false);
					_BladeBoundaryCollider[1]->getPhysicsBody()->setEnabled(true);
					_BladeBoundaryCollider[0]->getPhysicsBody()->setEnabled(false);
				}
			}

			else {
				if (!(madGrinder->isDead)) {
					madGrinder->nextSequence();
				}
			}
		}

		if (nodeA == core::TagIndex::BLADE_BOUNDARY) {
			madGrinder->returnBlade(contact.getShapeB()->getBody());
		}

		if (nodeB == core::TagIndex::BLADE_BOUNDARY) {
			madGrinder->returnBlade(contact.getShapeA()->getBody());
		}

		return true;
		};

	// Contact bullets and Screen Boundary Event
	contact->onContactSeparate = [this](PhysicsContact& contact) -> bool {

		auto nodeA = contact.getShapeA()->getBody()->getTag();
		auto nodeB = contact.getShapeB()->getBody()->getTag();

		//if ((nodeA == 10 || nodeA == 11) && contact.getShapeB()->getBody()->getNode() != nullptr)
		if ((nodeA == core::TagIndex::COMMON_BULLET || nodeA == core::TagIndex::CHARGE_X1 || nodeA == core::TagIndex::CHARGE_X2 || nodeA == core::TagIndex::METALL_BULLET) 
			&& contact.getShapeA()->getBody()->getNode() != nullptr && nodeB == core::TagIndex::BOUNDARY && contact.getShapeB()->getBody()->getNode() != nullptr) 
		{
			contact.getShapeA()->getBody()->getNode()->removeFromParentAndCleanup(true);
			log("Separate NodeA Deleted");
		}
			


		else if ((nodeB == core::TagIndex::COMMON_BULLET || nodeB == core::TagIndex::CHARGE_X1 || nodeB == core::TagIndex::CHARGE_X2 || nodeB == core::TagIndex::METALL_BULLET) 
			&& nodeA == core::TagIndex::BOUNDARY && contact.getShapeB()->getBody()->getNode() != nullptr && contact.getShapeB()->getBody()->getNode() != nullptr) 
		{
			contact.getShapeB()->getBody()->getNode()->removeFromParentAndCleanup(true);
			log("Separate nodeB Deleted");
		}

		if (nodeA == core::TagIndex::AWAKE_SENSOR) {
			auto a = static_cast<Enemy_Metall*>(contact.getShapeA()->getBody()->getNode()->getParent()->getParent());
			a->innerSensor = false;
		}

		else if (nodeB == core::TagIndex::AWAKE_SENSOR) {
			auto b = static_cast<Enemy_Metall*>(contact.getShapeB()->getBody()->getNode()->getParent()->getParent());
			b->innerSensor = false;
		}
		

		if (nodeA == core::TagIndex::PLAYER
			&& (nodeB == core::TagIndex::METALL || nodeB == core::TagIndex::METALL_BULLET || nodeB == core::TagIndex::BUNBY_HEAD || nodeB == core::TagIndex::BUNBY_TANK || nodeB == core::TagIndex::BOSS || nodeB == core::TagIndex::BOSS_SHIELD || nodeB == core::TagIndex::BOSS_BLADE))
		{
			if (nodeB == core::TagIndex::METALL || nodeB == core::TagIndex::BUNBY_HEAD || nodeB == core::TagIndex::BUNBY_TANK || nodeB == core::TagIndex::BOSS || nodeB == core::TagIndex::BOSS_SHIELD) {
				CollideEnemy = false;
				contactingDamage = 0;
			}
				
			else if (nodeB == core::TagIndex::METALL_BULLET || nodeB == core::TagIndex::BOSS_BLADE) {
				CollideEnemyBullet = false;
				contactingDamage = 0;
			}
				
		}

		if (nodeB == core::TagIndex::PLAYER
			&& (nodeA == core::TagIndex::METALL || nodeA == core::TagIndex::METALL_BULLET || nodeA == core::TagIndex::BUNBY_HEAD || nodeA == core::TagIndex::BUNBY_TANK || nodeA == core::TagIndex::BOSS || nodeA == core::TagIndex::BOSS_SHIELD || nodeA == core::TagIndex::BOSS_BLADE))
		{
			if (nodeA == core::TagIndex::METALL || nodeA == core::TagIndex::BUNBY_HEAD || nodeA == core::TagIndex::BUNBY_TANK || nodeA == core::TagIndex::BOSS || nodeA == core::TagIndex::BOSS_SHIELD) {
				CollideEnemy = false;
				contactingDamage = 0;
			}
				
			else if (nodeA == core::TagIndex::METALL_BULLET || nodeA == core::TagIndex::BOSS_BLADE) {
				CollideEnemyBullet = false;
				contactingDamage = 0;
			}
				
		}
		return true;
		};

	_eventDispatcher->addEventListenerWithSceneGraphPriority(contact, wlayer);

	////////////////////////////////////////////////////////

	// Player's Charging Effect Shader
	
	GLchar* fragmentSource = (GLchar*)String::createWithContentsOfFile(FileUtils::getInstance()->fullPathForFilename("ChargeMode_Shader.fsh").c_str())->getCString();
	glProg = GLProgram::createWithByteArrays(ccPositionTextureColor_noMVP_vert, fragmentSource);
	glProgramState = GLProgramState::getOrCreateWithGLProgram(glProg);
	
	glProgramState->setUniformInt("chargeMode", 0);
	attackChar->setGLProgramState(glProgramState);

	soundManager->PreloadBGM(bgmPath);
	soundManager->PlayBGM(bgmPath);

	this->scheduleUpdate();
	
	return true;
}

void MainScene::onEnter()
{
	Scene::onEnter();

	// 이벤트 리스너
	_listener = EventListenerKeyboard::create();
	_listener->onKeyPressed = CC_CALLBACK_2(MainScene::onKeyPressed, this);
	_listener->onKeyReleased = CC_CALLBACK_2(MainScene::onKeyReleased, this);

	_eventDispatcher->addEventListenerWithSceneGraphPriority(_listener, this);

	readyLogoAnim();
}

void MainScene::onExit()
{
	_eventDispatcher->removeEventListener(_listener);
	
	Scene::onExit();
}

void MainScene::update(float dt)
{
	HealthBar->setTextureRect(Rect(0, (2 * (28 - playerHealth)), 11, (56 - (2 * (28 - playerHealth)))));
	HealthBar->setPosition(15, 182 + (playerHealth - 28));

	if (hideLogo && !readyGame) {
		changeTime += dt;
		if (changeTime >= .02f && characterBody->getVelocity().y < 0.0001f && characterBody->getVelocity().y > -0.0001f) {
			moveAnimState(moveInputState);
			attackAnimState(attackInputState);
			characterBody->setCollisionBitmask(Utils::CreateMask(core::CategoryBits::PLATFORM
																, core::CategoryBits::BOUNDARY));
			changeTime = 0;
		}
	}

	else if (hideLogo && readyGame) {

		if (playerHealth <= 0 && !isDead) {

			isDead = true;
			characterBody->setGravityEnable(false);
			characterBody->setVelocity(Vec2::ZERO);
			startPauseTimer = true;
		}

		if (startPauseTimer) {
			pauseTime += dt;

			if (pauseTime >= .15f) {
				Sleep(1500.0);
				playerDie();
				
				startPauseTimer = false;
			}
		}

		if (!isTeleport && madGrinder->isDead && madGrinder->stopDeadEffect) {
			this->endGame();
		}

		if (CollideEnemy && !CollideEnemyBullet && !startStuckTimer || (!CollideEnemy && CollideEnemyBullet && !startStuckTimer)) {
			moveInputState = MS_HURT;
			moveAnimState(moveInputState);
			characterBody->setVelocity(Vec2(-1 * movingDir * 10, characterBody->getVelocity().y + 10));
			
		}

		if (startStuckTimer) {
			stuckTime += dt;

			if (fmod(stuckTime, 0.1f) < 0.05f) {
				character->setOpacity(100);
				attackChar->setOpacity(100);
			}
			else {
				character->setOpacity(255);
				attackChar->setOpacity(255);
			}
				

			if (stuckTime >= 1.5f) {
				stuckTime = 0;
				startStuckTimer = false;
				character->setOpacity(255);
				attackChar->setOpacity(255);
			}
		}

		if (isStuck) {
			startStuckTimer = true;
		}

		if (inputShoot) {
			atkChargeTime += dt;
			if (0.85f < atkChargeTime && atkChargeTime <= 2.0f) {
				if (chargeState != CHARGE_X1) {
					chargeState = CHARGE_X1;
					log("Charge_X1 Ready");

					if (!x1Ready) {
						soundManager->PlayAttackEffect(soundManager->startChargePath);
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
						soundManager->PlayAttackEffect(soundManager->chargingLoopPath);
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

		if (cutsceneBoss) {
			characterBody->setVelocity(Vec2(0, characterBody->getVelocity().y));
			moveInputState = MS_STOP;
			moveAnimState(moveInputState);
			attackInputState = DEFAULT_POSE;
			attackAnimState(attackInputState);
		}

		if (!isStuck && !isDead && !cutsceneBoss && !isTeleport) {

			// Ground Check
			if (characterBody->getVelocity().y < 0.0001f && characterBody->getVelocity().y > -0.0001f)
				fall = false;
			else fall = true;

			// Jump
			//log("%f", jumpChargeTime);
			if (movingUp && !movingSliding) {
				jumpChargeTime += dt;
				if (jumpChargeTime > 0.2f) {

					isJumping = false;
					movingUp = false;
				}

			}

			if (movingUp && !fall && jumpChargeTime < 0.2f) {
				characterBody->setVelocity(Vec2(0, 50));
				isJumping = true;
				moveInputState = MS_JUMP;
				attackInputState = JUMP_SHOOT;
				attackAnimState(attackInputState);
				moveAnimState(moveInputState);
			}

			if (isJumping == true && jumpChargeTime < 0.2f) {
				characterBody->setVelocity(Vec2(0, 110));
			}



			if (fall && inputShoot) {
				turnOffMoveSprite();
			}

			// Fall
			if (fall && characterBody->getVelocity().y < -20.0f) {
				moveInputState = MS_FALL;
				attackInputState = FALL_SHOOT;
				characterBody->setVelocity(Vec2(characterBody->getVelocity().x, (characterBody->getVelocity().y) * 1.05f));
				attackAnimState(attackInputState);
				moveAnimState(moveInputState);
				movingSliding = false;

			}

			// Left Move
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

			// Right Move
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

			// Stop
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

			// Sliding
			if (movingSliding && !movingUp && !fall) {

				if (character->isFlippedX())
					characterBody->setVelocity(Vec2(-85, characterBody->getVelocity().y));
				else characterBody->setVelocity(Vec2(85, characterBody->getVelocity().y));

				if (!movingUp && !fall) {
					moveInputState = MS_SLIDING;
					moveAnimState(moveInputState);
				}

			}

			// Shooting Position by each frames
			auto frameNum = (atkAnimate->getCurrentFrameIndex());

			auto nodePosition = wlayer->convertToNodeSpace(characterBody->getPosition());

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
		}


		// Follow Camera Setting
		if (isTriggingFirst) {
			if (followX < 992.0f) {
				followX += 4.0f;
			}

			if (followX >= 992.0f && followWidth < 1280) {
				followWidth += 4.0f;
			}	
			
			if (followX >= 992.0f && followWidth >= 1280) {
				isTriggingFirst = false;
			}
		}

		if (isTriggingSecond) {
			if (followX < 1152.0f) {
				followX += 4.0f;
			}

			if (followX >= 1152.0f)
				isTriggingSecond = false;
		}

		if (isTriggingThird) {
			if (followX > 1024.0f) {
				followX -= 4.0f;
			}

			if (followX <= 1152.0f)
				isTriggingSecond = false;
		}

		if (isTriggedThird && madGrinder->isReady && !readyBossStage) {
			cutsceneBoss = false;
			readyBossStage = true;
		}

		auto follow = Follow::create(charLayer, Rect(followX, 0, followWidth, 224));
		wlayer->runAction(follow);

		// Update Enemies
		for (int i = 0; i < metallsNum; i++) {
			metalls[i]->update(dt);
		}

		for (int i = 0; i < bunbysNum; i++) {
			bunbys[i]->update(dt);
		}
		madGrinder->update(dt);
	}

	if (isRestartScene) {
		soundManager->DecreaseVolume(&nowVolume);
	}
	
}

void MainScene::onKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event)
{
	
	if (hideLogo && readyGame && !madGrinder->isDead) {
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

		case EventKeyboard::KeyCode::KEY_X:
			if (!isStuck && !isDead && !cutsceneBoss) {
				inputShoot = true;
				if (!movingSliding)
					createBullet(COMMON_BULLET);
			}
			
			break;
		}
	}
}

void MainScene::onKeyReleased(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event)
{
	if (hideLogo && readyGame && !isTeleport) {
		// 키보드 입력을 해제하기 위해서는 XOR 연산자(^)를 사용함
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
			if (chargeState != COMMON_BULLET && !isStuck && !isDead) {
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

void MainScene::moveAnimState(_moveState state)
{
	//log("now animation : %d", state);

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
			break;
		}

		/////////////////////////////////////////////////////////////////////////////////

		// 외부 프로그램을 사용하여 만든 plist 파일 불러오기
		auto cache = SpriteFrameCache::getInstance();
		cache->addSpriteFramesWithFile(sPath);

	
		Vector<SpriteFrame*> animFrames;

		for (int i = 0; i < allSheetNum; i++) {
			
			// StringUtils::format => 지정한 형식으로 문자열을 생성
			std::string _frames = StringUtils::format("%s%02d.png",sName.c_str(), i);
			

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
		if (state == MS_STOP || state == MS_RUN) { 
			auto runRep = RepeatForever::create(animate);
			character->runAction(runRep);
		}

		else if (state == MS_INIT) {
			
			auto callback = CallFunc::create(this, callfunc_selector(MainScene::setReadyGame));
			auto seq = Sequence::create(animate->reverse(), callback, nullptr);
			character->runAction(seq);
		}

		else if (state == MS_HURT) {
			isStuck = true;
			turnOnMoveSprite();
			playerHealth -= (float)contactingDamage;
			auto callback = CallFunc::create(this, callfunc_selector(MainScene::setOffStuck));
			auto seq = Sequence::create(animate, DelayTime::create(0.1f), callback, nullptr);
			character->runAction(seq);
		}

		else if (state == MS_TELEPORT) {
			log("teleporting...");
			animate->retain();
			auto callback_1 = CallFunc::create(this, callfunc_selector(MainScene::playTeleportSound));
			auto callback_2 = CallFunc::create(this, callfunc_selector(MainScene::setTeleportVelocity));
			auto callback_3 = CallFunc::create(this, callfunc_selector(MainScene::replaceEndScene));
			auto seq = Sequence::create(DelayTime::create(1.5f), callback_1, animate, callback_2, DelayTime::create(1.5f), callback_3, nullptr);
			character->runAction(seq);
		}

		else {
			character->runAction(animate);
		}

		if (moveBeforeState == MS_FALL)
			soundManager->PlayMovementEffect(soundManager->landPath);
		if (state == MS_JUMP)
			soundManager->PlayMovementEffect(soundManager->jumpPath);
		/*if (state == MS_TELEPORT)
			soundManager->PlayMovementEffect(soundManager->warpUpPath);*/
		/*if (state == MS_INIT)
			soundManager->PlayMovementEffect(soundManager->warpDownPath);*/
		if (state == MS_SLIDING)
			soundManager->PlayMovementEffect(soundManager->slidingPath);
		if (state == MS_HURT)
			soundManager->PlayMovementEffect(soundManager->megamanTakeDamagePath);
		/////////////////////////////////////////////////////////////////////////////////
	}

	moveBeforeState = state;
}

void MainScene::attackAnimState(_attackState state)
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

		
		/////////////////////////////////////////////////////////////////////////////////
		// 외부 프로그램을 사용하여 만든 plist 파일 불러오기
		auto cache = SpriteFrameCache::getInstance();
		cache->addSpriteFramesWithFile(sPath);

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
		auto atkAnimation = Animation::createWithSpriteFrames(animFrames, frameDelay);
		//auto atkAnimation = Animation::createWithSpriteFrames(animFrames, frameDelay);
		atkAnimate = Animate::create(atkAnimation);
		atkAnimate->retain();
		
		
		//auto runRep = RepeatForever::create(animate);

		if (state == STOP_SHOOT) {
			auto callback = CallFunc::create(this, callfunc_selector(MainScene::turnOnMoveSprite));
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

void MainScene::bulletParticleAnimState(_bullets state, float x, float y)
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

	// 외부 프로그램을 사용하여 만든 plist 파일 불러오기
	auto cache = SpriteFrameCache::getInstance();
	cache->addSpriteFramesWithFile(sPath);

	Vector<SpriteFrame*> animFrames;

	for (int i = 0; i < allSheetNum; i++) {

		// StringUtils::format => 지정한 형식으로 문자열을 생성
		std::string _frames = StringUtils::format("%s%02d.png", sName.c_str(), i);

		// 생성한 문자열을 이용하여 plist 내부의 SpriteFrame 정보를 가져옴
		SpriteFrame* frame = cache->getSpriteFrameByName(_frames);
		frame->getTexture()->setAliasTexParameters();
		// 선별한 SpriteFrame을 삽입
		animFrames.pushBack(frame);
	}

	// plist 기반으로 만든 SpriteFrame 정보를 활용하여 인스턴스 생성
	auto animation = Animation::createWithSpriteFrames(animFrames, frameDelay);
	//auto atkAnimation = Animation::createWithSpriteFrames(animFrames, frameDelay);
	auto animate = Animate::create(animation);

	

	auto particleBullet = Sprite::create();
	particleBullet->setPosition(x, y);
	//log("%f, %f", x, y);
	//particleBullet->setAnchorPoint(Vec2(0.75f, 0.5f));
	particleBullet->setScale(particleScale);
	auto seq = Sequence::create(animate, RemoveSelf::create(), nullptr);
	wlayer->addChild(particleBullet);

	particleBullet->runAction(seq);
}

void MainScene::readyLogoAnim()
{
	// 외부 프로그램을 사용하여 만든 plist 파일 불러오기
	auto cache = SpriteFrameCache::getInstance();
	cache->addSpriteFramesWithFile("Megaman/UI/Ready_Logo.plist");

	// cocos2d::Vector
	//Vector<AnimationFrame*> animFrames;
	Vector<SpriteFrame*> animFrames;
	//ValueMap myValueMap;

	for (int i = 0; i < 27; i++) {

		// StringUtils::format => 지정한 형식으로 문자열을 생성
		std::string _frames = StringUtils::format("ready_%02d.png", i);

		// 생성한 문자열을 이용하여 plist 내부의 SpriteFrame 정보를 가져옴
		SpriteFrame* frame = cache->getSpriteFrameByName(_frames);
		//frame->setAnchorPoint(Vec2(0, 0));
		frame->getTexture()->setAliasTexParameters();
		// 선별한 SpriteFrame을 삽입
		
		animFrames.pushBack(frame);
	}

	// plist 기반으로 만든 SpriteFrame 정보를 활용하여 인스턴스 생성
	auto animation = Animation::createWithSpriteFrames(animFrames, 0.075);
	
	auto animate = Animate::create(animation);
	animate->retain();

	auto callback = CallFunc::create(this, callfunc_selector(MainScene::hideReadyLogo));
	auto seq = Sequence::create(DelayTime::create(1.0f),animate, callback, nullptr);

	readyLogoSprite = Sprite::create();
	readyLogoSprite->setAnchorPoint(Vec2::ZERO);
	this->addChild(readyLogoSprite, 10);
	readyLogoSprite->runAction(seq);

	

}

cocos2d::Vec2 MainScene::getPlayerWorldPos()
{
	auto nodePosition = wlayer->convertToNodeSpace(characterBody->getPosition());
	return nodePosition;
}

void MainScene::createMyPhysicsWorld()
{
	// 스크린 바운더리 (Screen Boundary)를 설정 (여기서는 화면 전체로 설정)
	auto visibleSize = _director->getVisibleSize();

	// 중력이 적용되는 범위의 세기를 설정
	//Vect gravity = Vect(0.0f, -128.0f);
	Vect gravity = Vect(0.0f, -300.0f);

	// 현재 Scene의 PhysicsWorld 정보를 받아오기
	m_world = this->getPhysicsWorld();

	// setDebugDrawMask() : 객체의 판정 박스를 표시
	//m_world->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);

	// setGravity() : 중력의 방향과 세기를 설정
	// 중력을 설정하지 않을 경우 기본값은 Vect(0.0f, -98.0f)
	m_world->setGravity(gravity);

	// setSpeed() : 낙하에 따른 중력가속도를 설정 (기본값은 1.0f)
	m_world->setSpeed(2.0f);

	// setSubsteps() : 프레임 간에 실시하는 물리 연산의 횟수를 조정
	// 기본값은 1
	m_world->setSubsteps(60);
	
	////////////////////////////////////////////////////////

	// 강체 (Rigidbody) 생성 (물리 연산이 가능한 객체 생성)
	// PhysicsBody::createEdgeBox
	// 제1인수 : 물리 시뮬레이션이 적용될 범위를 설정
	// 제2인수 : 물체의 밀도, 반발계수, 마찰계수 설정
	// 제3인수 : setDebugDrawMask가 그리는 선의 두께를 설정 (선 두께와 판정은 상관 없음)
	auto body = PhysicsBody::createEdgeBox(visibleSize, PHYSICSBODY_MATERIAL_DEFAULT, 1);
	body->setCategoryBitmask(Utils::CreateMask(core::CategoryBits::BOUNDARY));
	body->setCollisionBitmask(Utils::CreateMask(core::CategoryBits::PLAYER
											, core::CategoryBits::ENEMY
											, core::CategoryBits::ENEMY_PROJECTILE));
	body->setContactTestBitmask(Utils::CreateMask(core::CategoryBits::PLAYER_PROJECTILE
											, core::CategoryBits::ENEMY_PROJECTILE));
	body->setTag(core::TagIndex::BOUNDARY);

	// 물리 시뮬레이션 적용을 위해 최상위 클래스인 Node로 인스턴스 생성
	auto edgeNode = Node::create();

	// Physics 적용 위치 조정 (중간을 기준으로 옮겨줌)
	edgeNode->setPosition(Vec2(visibleSize) * 0.5f);

	// 강체 적용
	edgeNode->setPhysicsBody(body);
	this->addChild(edgeNode);
}

void MainScene::turnOnMoveSprite()
{
	if (!(character->isVisible())) {
		character->setVisible(true);
		attackChar->setVisible(false);
	}
}

void MainScene::turnOffMoveSprite()
{
	if (character->isVisible()) {
		character->setVisible(false);
		attackChar->setVisible(true);
	}
}

void MainScene::setReadyGame()
{
	readyGame = true;
	moveInputState = MS_STOP;
}

void MainScene::hideReadyLogo()
{
	readyLogoSprite->setVisible(false);
	characterBody->setGravityEnable(true);
	soundManager->PlayMovementEffect(soundManager->warpDownPath);
	hideLogo = true;
}

void MainScene::setOffStuck()
{
	isStuck = false;
}

void MainScene::playTeleportSound()
{
	soundManager->PlayMovementEffect(soundManager->warpUpPath);
}

void MainScene::playerDie()
{
	int allSheetNum = 9;
	std::string sPath = "Megaman/Particle/Particle_Explosion.plist";
	std::string sName = "explosion_";
	float frameDelay = 0.07f;

	Vector<Sprite*> explosions;

	for (int i = 0; i < 16; i++) {
		auto explosion = Sprite::create("Megaman/Particle/explosion_init.png");
		explosions.pushBack(explosion);
	}

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
	auto runRep = RepeatForever::create(animate);
	
	for (int i = 0; i < 16; i++) {
		explosions.at(i)->runAction(runRep->clone());
		
		explosions.at(i)->runAction(MoveBy::create(5.0f, Vec2(1.5 * explosionPos[i][0], 1.5 * explosionPos[i][1])));

		explosions.at(i)->setPosition(wlayer->convertToNodeSpace(characterBody->getPosition()));

		wlayer->addChild(explosions.at(i));
	}

	charLayer->setVisible(false);
	
	auto restart = Node::create();

	auto callback = CallFunc::create(this, callfunc_selector(MainScene::restartGame));
	auto seq = Sequence::create(DelayTime::create(2.0f), callback, nullptr);

	this->addChild(restart);
	soundManager->PlayMovementEffect(soundManager->megamanDeathPath);
	restart->runAction(seq);
}

void MainScene::pauseGame()
{
	_director->pause();
}

void MainScene::resumeGame()
{
	_director->resume();
}

void MainScene::restartGame()
{
	isRestartScene = true;
	
	_director->replaceScene(TransitionFade::create(2.0f, RestartHelper::createScene()));
}

void MainScene::flippedCharacter(bool state)
{
	character->setFlippedX(state);
	attackChar->setFlippedX(state);
}

void MainScene::startBossStage()
{
	isTriggingThird = true;
	isTriggedThird = true;
}

void MainScene::endGame()
{
	CollideEnemy = false;
	CollideEnemyBullet = false;
	contactingDamage = 0;
	isTeleport = true;
	moveInputState = MS_TELEPORT;
	characterBody->setVelocity(Vec2::ZERO);
	characterBody->setCollisionBitmask(Utils::CreateMask(core::CategoryBits::PLATFORM));
	characterBody->setGravityEnable(false);
	moveAnimState(moveInputState);

}

void MainScene::setTeleportVelocity()
{
	characterBody->setVelocity(Vec2(0, 125.0f));
}

void MainScene::replaceEndScene()
{
	_director->replaceScene(TransitionFade::create(2.0f, ReplaceEndSceneHelper::createScene()));
}

void MainScene::createBullet(_bullets state)
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

	// 탄환 생성
	auto pBullet = new Sprite();
	auto bulletPBody = PhysicsBody::createBox(size, PHYSICSBODY_MATERIAL_DEFAULT);
	bulletPBody->setGravityEnable(false);

	if (state == COMMON_BULLET) {
		pBullet = Sprite::create(sPath);
		pBullet->setPosition(shootPosition);
		bulletPBody->setTag(core::TagIndex::COMMON_BULLET);
		soundManager->PlayAttackEffect(soundManager->bulletPath);
	}

	else if (state == CHARGE_X1 || state == CHARGE_X2) {
		// 외부 프로그램을 사용하여 만든 plist 파일 불러오기
		auto cache = SpriteFrameCache::getInstance();
		cache->addSpriteFramesWithFile(sPath);

		Vector<SpriteFrame*> animFrames;

		for (int i = 0; i < allSheetNum; i++) {

			// StringUtils::format => 지정한 형식으로 문자열을 생성
			std::string _frames = StringUtils::format("%s%02d.png", sName.c_str(), i);
			//myValueMap.insert(std::pair<std::string, Value>("position", ValueVector(Value(1.0f), Value(2.0f))));

			// 생성한 문자열을 이용하여 plist 내부의 SpriteFrame 정보를 가져옴
			SpriteFrame* frame = cache->getSpriteFrameByName(_frames);
			//frame->setAnchorPoint(Vec2(0.0f, 0.0f));
			frame->getTexture()->setAliasTexParameters();
			// 선별한 SpriteFrame을 삽입
			//animFrames.pushBack(AnimationFrame::create(cache->getSpriteFrameByName(_frames), 1, myValueMap));
			animFrames.pushBack(frame);
		}

		// plist 기반으로 만든 SpriteFrame 정보를 활용하여 인스턴스 생성
		auto Animation = Animation::createWithSpriteFrames(animFrames, frameDelay);
		//auto atkAnimation = Animation::createWithSpriteFrames(animFrames, frameDelay);
		auto bulletAnim = Animate::create(Animation);
		bulletAnim->retain();

		

		pBullet = Sprite::create("Placeholder.png");
		pBullet->setPosition(shootPosition);
		
		if (state == CHARGE_X1) {
			bulletPBody->setTag(core::TagIndex::CHARGE_X1);
			soundManager->PlayAttackEffect(soundManager->chargeX1Path);
		}

		else if (state == CHARGE_X2) {
			bulletPBody->setTag(core::TagIndex::CHARGE_X2);
			soundManager->PlayAttackEffect(soundManager->chargeX2Path);
		}
			
		pBullet->runAction(bulletAnim);
	}
	
	bulletPBody->setCategoryBitmask(Utils::CreateMask(core::CategoryBits::PLAYER_PROJECTILE));
	bulletPBody->setCollisionBitmask(0);
	bulletPBody->setContactTestBitmask(Utils::CreateMask(core::CategoryBits::BOUNDARY
														//, core::CategoryBits::PLATFORM
														, core::CategoryBits::ENEMY));

	pBullet->setPhysicsBody(bulletPBody);
	
	wlayer->addChild(pBullet);

	// 액션 적용
	if (character->isFlippedX()) {
		pBullet->setFlippedX(true);
		bulletPBody->setVelocity(Vec2(-175.0f, 0.0f));
	}
		
	else {
		pBullet->setFlippedX(false);
		bulletPBody->setVelocity(Vec2(175.0f, 0.0f));
	}
}






