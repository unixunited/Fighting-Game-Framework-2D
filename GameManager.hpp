// ================================================ //
// Extreme Metal Fighter
// Copyright (C) 2014 Jordan Sparks. All Rights Reserved.
// Unauthorized copying of this file, via any medium is strictly prohibited
// Proprietary and confidential
// Written by Jordan Sparks <unixunited@live.com> June 2014
// ================================================ //
// File: GameManager.hpp
// Author: Jordan Sparks <unixunited@live.com>
// ================================================ //
// Defines GameManager singleton class.
// ================================================ //

// ================================================ //

#ifndef __GAMEMANAGER_HPP__
#define __GAMEMANAGER_HPP__

// ================================================ //

#include "stdafx.hpp"

// ================================================ //

// A singleton class to hold information for use between
// game states (e.g., should the lobby state start a server
// or a client). 
class GameManager : public Singleton<GameManager>
{
public:
	// Initializes mode to IDLE.
	explicit GameManager(void);

	// Empty destructor.
	~GameManager(void);

	// Game modes.
	enum{
		IDLE = 0,
		CAMPAIGN,
		SERVER,
		CLIENT
	};

	// Getters

	// Returns the game mode.
	const int getMode(void) const;

	// Returns the username.
	const std::string getUsername(void) const;

	// Setters

	// Sets the game mode.
	void setMode(const int mode);

	// Sets the username.
	void setUsername(const std::string& username);

private:
	int m_mode;
	std::string m_username;
};

// ================================================ //

// Getters

inline const int GameManager::getMode(void) const{
	return m_mode;
}

inline const std::string GameManager::getUsername(void) const{
	return m_username;
}

// Setters

inline void GameManager::setMode(const int mode){
	m_mode = mode;
}

inline void GameManager::setUsername(const std::string& username){
	m_username = username;
}

// ================================================ //

#endif

// ================================================ //

