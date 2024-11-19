#include "SoundManager.h"
USING_NS_CC;
using namespace experimental;

SoundManager::SoundManager() {

}

void SoundManager::initSoundManager(std::string& filePath)
{
	AudioEngine::preload(filePath);

	AudioEngine::preload(warpUpPath);
	AudioEngine::preload(warpDownPath);
	AudioEngine::preload(jumpPath);
	AudioEngine::preload(landPath);
	AudioEngine::preload(slidingPath);
	AudioEngine::preload(megamanTakeDamagePath);
	AudioEngine::preload(megamanDeathPath);

	AudioEngine::preload(bulletPath);
	AudioEngine::preload(chargeX1Path);
	AudioEngine::preload(chargeX2Path);
	AudioEngine::preload(startChargePath);
	AudioEngine::preload(chargingLoopPath);

	AudioEngine::preload(bunbyMovePath);
	AudioEngine::preload(metallTinkPath);
	AudioEngine::preload(enemyTakeDamagePath);
	AudioEngine::preload(enemyDeathPath);

	AudioEngine::preload(bossMovePath);
	AudioEngine::preload(bossTakeDamagePath);
	AudioEngine::preload(bossDeathPath);

	AudioEngine::preload(metallBulletPath);
	AudioEngine::preload(bossBladePath);
}

void SoundManager::PreloadBGM(std::string& filePath)
{
	AudioEngine::preload(filePath);
}

void SoundManager::PlayBGM(std::string& filePath)
{
	this->bgmID = AudioEngine::play2d(filePath, true, 0.5f);
}

void SoundManager::PlayMovementEffect(std::string& filePath)
{
	if (AudioEngine::getState(movementID) == AudioEngine::AudioState::PLAYING)
		AudioEngine::stop(movementID);
	this->movementID = AudioEngine::play2d(filePath, false, 0.5f);
}

void SoundManager::PlayAttackEffect(std::string& filePath)
{
	if (AudioEngine::getState(attackID) == AudioEngine::AudioState::PLAYING)
		AudioEngine::stop(attackID);

	this->attackID = AudioEngine::play2d(filePath, false, 0.5f);
}

void SoundManager::PlayEnemyEffect(std::string& filePath)
{
	if (AudioEngine::getState(enemyID) == AudioEngine::AudioState::PLAYING)
		AudioEngine::stop(enemyID);

	this->enemyID = AudioEngine::play2d(filePath, false, 0.5f);
}

void SoundManager::PlayEnemyHitEffect(std::string& filePath)
{
	if (AudioEngine::getState(enemyHitID) == AudioEngine::AudioState::PLAYING)
		AudioEngine::stop(enemyHitID);
	this->enemyHitID = AudioEngine::play2d(filePath, false, 0.5f);
}

void SoundManager::PlayTinkEffect(std::string& filePath)
{
	if (AudioEngine::getState(tinkID) == AudioEngine::AudioState::PLAYING)
		AudioEngine::stop(tinkID);
	this->tinkID = AudioEngine::play2d(filePath, false, 0.5f);
}

bool SoundManager::isEnemyEffectRunning()
{
	if (AudioEngine::getState(enemyID) == AudioEngine::AudioState::PLAYING)
		return true;
	else return false;
}

void SoundManager::StopAllSounds()
{
	AudioEngine::stopAll();
}

void SoundManager::DecreaseVolume(float* nowVolume)
{
	*nowVolume -= 0.025f;
	AudioEngine::setVolume(bgmID, *nowVolume);
}

void SoundManager::StopEnemyEffect()
{
	AudioEngine::setLoop(enemyID, false);
	AudioEngine::stop(this->enemyID);
}

SoundManager::~SoundManager()
{
	this->StopAllSounds();
	AudioEngine::uncacheAll();
}