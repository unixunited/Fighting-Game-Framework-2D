// ================================================ //
// Extreme Metal Fighter
// Copyright (C) 2014 Jordan Sparks. All Rights Reserved.
// Unauthorized copying of this file, via any medium is strictly prohibited
// Proprietary and confidential
// Written by Jordan Sparks <unixunited@live.com> June 2014
// ================================================ //
// File: PlayerManager.cpp
// Author: Jordan Sparks <unixunited@live.com>
// ================================================ //
// Implements PlayerManager singleton class.
// ================================================ //

#include "PlayerManager.hpp"
#include "StageManager.hpp"
#include "Stage.hpp"
#include "Hitbox.hpp"
#include "Config.hpp"
#include "Input.hpp"
#include "Engine.hpp"
#include "GamepadManager.hpp"
#include "Game.hpp"
#include "Move.hpp"

// ================================================ //

template<> PlayerManager* Singleton<PlayerManager>::msSingleton = nullptr;

// ================================================ //

PlayerManager::PlayerManager(void) :	
m_pRedPlayer(nullptr),
m_pBluePlayer(nullptr),
m_redFighter(0),
m_blueFighter(0),
m_redMax(0),
m_blueMax(0),
m_fighters()
{
	Log::getSingletonPtr()->logMessage("Initializing PlayerManager...");

	Config c(Engine::getSingletonPtr()->getDataDirectory() + "/Fighters/fighters.cfg");
	const int numFighters = c.parseIntValue("core", "numFighters");

	for(int i=1; i<=numFighters; ++i){
		FighterEntry fighter;
		std::string fighterName = "fighter" + Engine::toString(i);

		fighter.name = c.parseValue(fighterName.c_str(), "name", true);
		fighter.file = c.parseValue(fighterName.c_str(), "file", true);

		// TODO: load portrait texture.

		m_fighters.push_back(fighter);

		Log::getSingletonPtr()->logMessage("Fighter entry loaded => Name: " + fighter.name + "\tFile: " + fighter.file);
	}
}

// ================================================ //

PlayerManager::~PlayerManager(void)
{
	 
}

// ================================================ //

bool PlayerManager::load(const std::string& redFighterFile, const std::string& blueFighterFile)
{
	Config c(Engine::getSingletonPtr()->getSettingsFile());

	// Free any previously allocated Players and allocate new ones.
	m_pRedPlayer.reset(new Player(redFighterFile, 
		Engine::getSingletonPtr()->getDataDirectory() + "/" + c.parseValue("controls", "red")));
	m_pBluePlayer.reset(new Player(blueFighterFile, 
		Engine::getSingletonPtr()->getDataDirectory() + "/" + c.parseValue("controls", "blue")));

	// Set default player gamepads.
	if (GamepadManager::getSingletonPtr()->getPad(1) == nullptr){
		m_pRedPlayer->getInput()->setPad(GamepadManager::getSingletonPtr()->getPad(0));
		m_pBluePlayer->getInput()->setPad(nullptr);
	}
	else{
		m_pRedPlayer->getInput()->setPad(GamepadManager::getSingletonPtr()->getPad(1));
		m_pBluePlayer->getInput()->setPad(GamepadManager::getSingletonPtr()->getPad(0));
	}

	// Set default starting sides and positions.
	m_pRedPlayer->setSide(Player::Side::LEFT);
	m_pBluePlayer->setSide(Player::Side::RIGHT);

	// startingOffset is the virtual pixel width each player starts from the edge of the viewport.
	const int startingOffset = 40;
	m_pRedPlayer->setPosition(startingOffset, m_pRedPlayer->getPosition().y);
	m_pBluePlayer->setPosition(Engine::getSingletonPtr()->getLogicalWindowWidth() - m_pBluePlayer->getPosition().w - startingOffset, 
		m_pBluePlayer->getPosition().y);

	return (m_pRedPlayer.get() != nullptr) && (m_pBluePlayer.get() != nullptr);
}

// ================================================ //

bool PlayerManager::load(const Uint32 redFighter, const Uint32 blueFighter)
{
	Log::getSingletonPtr()->logMessage("Loading fighter files from \"" +
		Engine::getSingletonPtr()->getDataDirectory() + "/Fighters\"");
	bool ret = this->load(Engine::getSingletonPtr()->getDataDirectory() + 
		"/Fighters/" + m_fighters[redFighter].file, 
		Engine::getSingletonPtr()->getDataDirectory() + 
		"/Fighters/" + m_fighters[blueFighter].file);
	if (ret){
		Log::getSingletonPtr()->logMessage("Fighters loaded!");
	}
	else{
		Log::getSingletonPtr()->logMessage("ERROR: Failed to load fighters");
	}

	m_redFighter = redFighter;
	m_blueFighter = blueFighter;

	return ret;
}

// ================================================ //

bool PlayerManager::reload(void)
{
	return this->load(m_redFighter, m_blueFighter);
}

// ================================================ //

bool PlayerManager::reset(void)
{
	Config c(Engine::getSingletonPtr()->getSettingsFile());

	if (c.isLoaded()){
		m_pRedPlayer.reset(new Player("", Engine::getSingletonPtr()->getDataDirectory() + 
			"/" + c.parseValue("controls", "red")));
		m_pBluePlayer.reset(new Player("", Engine::getSingletonPtr()->getDataDirectory() + 
			"/" + c.parseValue("controls", "blue")));

		// Set default player gamepads.
		if (GamepadManager::getSingletonPtr()->getPad(1) == nullptr){
			m_pRedPlayer->getInput()->setPad(GamepadManager::getSingletonPtr()->getPad(0));
			m_pBluePlayer->getInput()->setPad(nullptr);
		}
		else{
			m_pRedPlayer->getInput()->setPad(GamepadManager::getSingletonPtr()->getPad(1));
			m_pBluePlayer->getInput()->setPad(GamepadManager::getSingletonPtr()->getPad(0));
		}

		return true;
	}

	return false;
}

// ================================================ //

void PlayerManager::update(double dt)
{
	// Perform game mode specific operations.
	switch (Game::getSingletonPtr()->getMode()){
	default:
		break;

	// Perform server-side calculations.
	case Game::SERVER:
	case Game::LOCAL:
		m_pRedPlayer->update(dt);
		m_pBluePlayer->update(dt);

		// Test hitbox collisions (this is predicted on the client).
		for (int i = Hitbox::DBOX1; i <= Hitbox::DBOX2; ++i){
			for (int j = Hitbox::HBOX_LOWER; j <= Hitbox::HBOX_HEAD; ++j){
				if (m_pRedPlayer->getHitbox(i)->intersects(m_pBluePlayer->getHitbox(j))){
					if (m_pRedPlayer->hitboxesActive()){
						m_pRedPlayer->setHitboxesActive(false);
						Move* pMove = m_pRedPlayer->getCurrentMove();
						bool hit = m_pBluePlayer->takeHit(pMove);

						if (Game::getSingletonPtr()->getMode() == Game::SERVER){
							// Send damage notification to client.
							if (hit){
								Server::getSingletonPtr()->broadcastHit(Game::Playing::PLAYING_BLUE,
																		pMove->damage, pMove->hitstun);
							}
							else{
								Server::getSingletonPtr()->broadcastHitBlock(Game::Playing::PLAYING_BLUE,
																			 pMove->blockstun);
							}
						}
					}
				}
				if (m_pBluePlayer->getHitbox(i)->intersects(m_pRedPlayer->getHitbox(j))){
					if (m_pBluePlayer->hitboxesActive()){
						m_pBluePlayer->setHitboxesActive(false);
						Move* pMove = m_pBluePlayer->getCurrentMove();
						bool hit = m_pRedPlayer->takeHit(pMove);

						if (Game::getSingletonPtr()->getMode() == Game::SERVER){
							if (hit){
								Server::getSingletonPtr()->broadcastHit(Game::Playing::PLAYING_RED,
																		pMove->damage, pMove->hitstun);
							}
							else{
								Server::getSingletonPtr()->broadcastHitBlock(Game::Playing::PLAYING_RED,
																			 pMove->blockstun);
							}
						}
					}
				}
			}
		}
		break;

	case Game::CLIENT:
		if (Game::getSingletonPtr()->getPlaying() == Game::PLAYING_RED){
			m_pRedPlayer->update(dt);
			m_pRedPlayer->serverReconciliation();			
		}
		else if (Game::getSingletonPtr()->getPlaying() == Game::PLAYING_BLUE){
			m_pBluePlayer->update(dt);
			m_pBluePlayer->serverReconciliation();			
		}
		break;
	}

	// Update stage shift and player sides.
	const double shiftMultiplier = 0.50;
	int shift = 0;
	// How much the other player shifts for adjustment.
	int playerShift = 0;
	SDL_Rect redPos, bluePos;
	redPos = m_pRedPlayer->getPosition();
	bluePos = m_pBluePlayer->getPosition();

	printf("red: %d/%d\tblue: %d/%d\n", redPos.x, m_pRedPlayer->getSide(), bluePos.x, m_pBluePlayer->getSide());

	// Red player.
	if (m_pRedPlayer->getSide() == Player::Side::LEFT){
		if (redPos.x < 0){
			m_pRedPlayer->setPosition(0, redPos.y);
			if (bluePos.x < m_pBluePlayer->getMaxXPos()){
				shift = static_cast<int>(m_pRedPlayer->getXVelocity() * dt * shiftMultiplier);

				StageManager::getSingletonPtr()->getStage()->shift(shift);
				if (StageManager::getSingletonPtr()->getStage()->getShift() > 0){
					// Adjust blue player to compensate for stage shift.
					playerShift = -(shift * 2);
					m_pBluePlayer->setPosition(bluePos.x + playerShift, bluePos.y);
				}
			}
		}
	}
	else{
		if (redPos.x > m_pRedPlayer->getMaxXPos()){
			m_pRedPlayer->setPosition(m_pRedPlayer->getMaxXPos(), redPos.y);
			if (bluePos.x > 0){
				shift = static_cast<int>(m_pRedPlayer->getXVelocity() * dt * shiftMultiplier);

				StageManager::getSingletonPtr()->getStage()->shift(shift);
				if (StageManager::getSingletonPtr()->getStage()->getShift() < StageManager::getSingletonPtr()->getStage()->getRightEdge()){
					playerShift = -(shift * 2);
					m_pBluePlayer->setPosition(bluePos.x + playerShift, bluePos.y);
				}
			}
		}
	}
	// Blue Player.
	if (m_pBluePlayer->getSide() == Player::Side::LEFT){
		if (bluePos.x < 0){
			m_pBluePlayer->setPosition(0, bluePos.y);
			if (redPos.x < m_pRedPlayer->getMaxXPos()){
				shift = static_cast<int>(m_pBluePlayer->getXVelocity() * dt * shiftMultiplier);

				StageManager::getSingletonPtr()->getStage()->shift(shift);
				if (StageManager::getSingletonPtr()->getStage()->getShift() > 0){
					playerShift = -(shift * 2);
					m_pRedPlayer->setPosition(redPos.x + playerShift, redPos.y);
				}
			}
		}
	}
	else{
		if (bluePos.x > m_pBluePlayer->getMaxXPos()){
			m_pBluePlayer->setPosition(m_pBluePlayer->getMaxXPos(), bluePos.y);
			if (redPos.x > 0){
				shift = static_cast<int>(m_pBluePlayer->getXVelocity() * dt * shiftMultiplier);

				StageManager::getSingletonPtr()->getStage()->shift(shift);
				if (StageManager::getSingletonPtr()->getStage()->getShift() < StageManager::getSingletonPtr()->getStage()->getRightEdge()){
					playerShift = -(shift * 2);
					m_pRedPlayer->setPosition(redPos.x + playerShift, redPos.y);
				}
			}
		}
	}
	// If the stage was shifted, apply client/server operations.
	if (shift != 0){
		if (Game::getSingletonPtr()->getMode() == Game::SERVER){
			Server::getSingletonPtr()->stageShift(StageManager::getSingletonPtr()->getStage()->getShift());
		}
		else if (Game::getSingletonPtr()->getMode() == Game::CLIENT){
			Client::StageShift pendingShift;
			pendingShift.shift = shift;
			pendingShift.playerShift = playerShift;
			pendingShift.seq = Client::getSingletonPtr()->m_stageShiftSeq++;
			Client::getSingletonPtr()->m_pendingStageShifts.push_back(pendingShift);
		}
	}

	printf("Stage shift: %d\n", StageManager::getSingletonPtr()->getStage()->getShift());

	// Test normal hitbox collision (hitboxes 0 through 3).
	for(int i=0; i<4; ++i){
		for(int j=0; j<4; ++j){
			if ((m_pRedPlayer->getHitbox(i)->intersects(m_pBluePlayer->getHitbox(j)))){
				const int diff = (m_pRedPlayer->getSide() == Player::Side::LEFT) ?
					std::abs(m_pBluePlayer->getHitbox(j)->getRect().x -
					m_pRedPlayer->getHitbox(i)->getRect().x -
					m_pRedPlayer->getHitbox(i)->getRect().w) :
					std::abs(m_pRedPlayer->getHitbox(i)->getRect().x -
					m_pBluePlayer->getHitbox(j)->getRect().x -
					m_pBluePlayer->getHitbox(j)->getRect().w);

				// Apply collision handling on a case-by-case basis.
				StateID redState = m_pRedPlayer->getCurrentState();
				StateID blueState = m_pBluePlayer->getCurrentState();
				if ((redState != Player::State::JUMPING && blueState != Player::State::JUMPING) ||
					(redState == Player::State::JUMPING && blueState == Player::State::JUMPING)){
					redPos.x += (m_pRedPlayer->getSide() == Player::Side::LEFT) ? -diff : diff;
					m_pRedPlayer->setPosition(redPos);
					bluePos.x += (m_pRedPlayer->getSide() == Player::Side::LEFT) ? diff : -diff;
					m_pBluePlayer->setPosition(bluePos);
				}
				else if (redState == Player::State::JUMPING && blueState != Player::State::JUMPING){
					redPos.x += (m_pRedPlayer->getSide() == Player::Side::LEFT) ? -diff : diff;
					m_pRedPlayer->setPosition(redPos);
				}
				else if (blueState == Player::State::JUMPING && redState != Player::State::JUMPING){
					bluePos.x += (m_pRedPlayer->getSide() == Player::Side::LEFT) ? diff : -diff;
					m_pBluePlayer->setPosition(bluePos);
				}
			}
		}
	}

	// Switch player sides if necessary.
	if (redPos.x > (bluePos.x)){
		if (m_pRedPlayer->getSide() != Player::Side::RIGHT){
			m_pRedPlayer->setSide(Player::Side::RIGHT);
			m_pBluePlayer->setSide(Player::Side::LEFT);
		}
	}
	else if(bluePos.x > (redPos.x)){
		if (m_pBluePlayer->getSide() != Player::Side::RIGHT){
			m_pBluePlayer->setSide(Player::Side::RIGHT);
			m_pRedPlayer->setSide(Player::Side::LEFT);
		}
	}

	// Render the players.
	m_pRedPlayer->render();
	m_pBluePlayer->render();
}

// ================================================ //