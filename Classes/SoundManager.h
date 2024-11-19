#pragma once
#include "cocos2d.h"
#include "AudioEngine.h"

class SoundManager {
public:
	std::string warpUpPath = "Megaman/SFX/Megaman_Warp_Up.mp3";
	std::string warpDownPath = "Megaman/SFX/Megaman_Warp_Down.mp3";
	std::string jumpPath = "Megaman/SFX/Megaman_Jump.mp3";
	std::string landPath = "Megaman/SFX/Megaman_Land_Ground.mp3";
	std::string slidingPath = "Megaman/SFX/Megaman_Slide.mp3";
	std::string megamanTakeDamagePath = "Megaman/SFX/Megaman_Take_Damage.mp3";
	std::string megamanDeathPath = "Megaman/SFX/Megaman_Death.mp3";

	std::string bulletPath = "Megaman/SFX/Megaman_Common_Shoot.mp3";
	std::string chargeX1Path = "Megaman/SFX/Charge_X1_Shoot.mp3";
	std::string chargeX2Path = "Megaman/SFX/Charge_X2_Shoot.mp3";
	std::string startChargePath = "Megaman/SFX/Charge_Buster_Start.mp3";
	std::string chargingLoopPath = "Megaman/SFX/Charge_Buster_Loop.mp3";
	
	std::string bunbyMovePath = "Megaman/SFX/Bunby_Move.mp3";
	std::string metallTinkPath = "Megaman/SFX/Enemy_Tink.mp3";
	std::string enemyTakeDamagePath = "Megaman/SFX/Enemy_Take_Damage.mp3";
	std::string enemyDeathPath = "Megaman/SFX/Enemy_Death.mp3";
	
	std::string bossMovePath = "Megaman/SFX/Boss_Move.mp3";
	std::string bossTakeDamagePath = "Megaman/SFX/Boss_Take_Damage_Explode.mp3";
	std::string bossDeathPath = "Megaman/SFX/Boss_Death.mp3";

	std::string metallBulletPath = "Megaman/SFX/Metall_Bullet.mp3";
	std::string bossBladePath = "Megaman/SFX/boss_blade_loop.mp3";

	void initSoundManager(std::string& filePath);

	SoundManager::SoundManager();

	void PreloadBGM(std::string& filePath);
	void PlayBGM(std::string &filePath);
	void PlayMovementEffect(std::string& filePath);
	void PlayAttackEffect(std::string& filePath);
	void PlayEnemyEffect(std::string& filePath);
	void PlayEnemyHitEffect(std::string& filePath);
	void PlayTinkEffect(std::string& filePath);
	bool isEnemyEffectRunning();

	void DecreaseVolume(float* nowVolume);
	void StopEnemyEffect();
	void StopAllSounds();

	SoundManager::~SoundManager();

private:
	int bgmID;

	// megaman Sounds
	int movementID;
	int attackID;
	int enemyID;
	int enemyHitID;
	int tinkID;
};