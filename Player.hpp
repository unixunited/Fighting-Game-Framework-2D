// ========================================================================= //
// Fighting game framework (2D) with online multiplayer.
// Copyright(C) 2014 Jordan Sparks <unixunited@live.com>
//
// This program is free software; you can redistribute it and / or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 3
// of the License, or(at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
// ========================================================================= //
// File: Player.hpp
// Author: Jordan Sparks <unixunited@live.com>
// ================================================ //
// Defines Player class.
// ================================================ //

#ifndef __PLAYER_HPP__
#define __PLAYER_HPP__

// ================================================ //

#include "Object.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "FSM.hpp"

// ================================================ //

class Hitbox;
class Input;
struct Move;
class FighterMetadata;
class Timer;
class Widget;

typedef std::vector<std::shared_ptr<Move>> MoveList;
typedef std::vector<std::shared_ptr<Hitbox>> HitboxList;

// ================================================ //

// A Player is a fighter that can be controlled by human input, AI, or 
// a networked human or AI.
class Player : public Object
{
public:
	// Enumerations

	enum Side{
		LEFT = 0,
		RIGHT
	};

	enum Mode{
		LOCAL = 0,
		AI,
		NET
	};

	enum State{
		IDLE = 0,
		WALKING_FORWARD,
		WALKING_BACK,
		JUMPING,

		CROUCHING,
		CROUCHED,
		UNCROUCHING,

		ATTACK_LP,

		STUNNED_JUMP,
		STUNNED_HIT,
		STUNNED_BLOCK
	};

	// Allocates the Input object, which loads the button map from the specified file.
	// Loads all fighter data and moves. Builds the FSM.
	explicit Player(const std::string& fighterFile, const std::string& buttomMapFile = "", const int mode = Player::Mode::LOCAL);

	// Empty destructor.
	virtual ~Player(void);

	// Adds net input from client to processing queue.
	void enqueueClientInput(const Client::NetInput& input);

	// Determines whether or not to snap player to the new position, depending
	// on the distance, otherwise it graudally slides the player. Used when
	// receiving a network update on the client.
	void updateFromServer(const Server::PlayerUpdate& update);

	// Rewinds player state to last server update and replays unprocessed inputs.
	void serverReconciliation(void);

	// Processes pending input that is being replayed as part of server reconciliation.
	void processReplayedInput(double dt = 0);

	// Processes local input for player, adjusting its state.
	void processInput(double dt = 0);

	// Applies movement to the player based on key presses.
	void applyInput(double dt);

	// Updates the current Move, handles collision.
	virtual void update(double dt);

	// Renders the Player sprite.
	virtual void render(void);

	// Process animation updates for the current move.
	void updateMove(void);

	// Loads textures, moves, etc.
	void loadFighterData(const std::string& file);

	// Applies a hit from a move performed by the other player.
	// Returns true if the player was hit.
	bool takeHit(const Move* pMove);

	// Sets the current HP and adjusts health bar.
	void updateHP(const Uint32 hp);

	// Getters

	// Returns the Input object.
	Input* getInput(void) const;

	// Returns the mode the player is currently in.
	const Uint32 getMode(void) const;

	// Returns current X velocity.
	const int32_t getXVelocity(void) const;

	// Returns current X jump velocity.
	const int32_t getXJumpVelocity(void) const;

	// Returns max X velocity.
	const int32_t getXMaxVelocity(void) const;

	// Returns pointer to player's health bar.
	Widget* getHealthBarPtr(void) const;

	// Returns the side the player is on (e.g., LEFT or RIGHT).
	const int getSide(void) const;

	// Returns current state ID.
	const Uint32 getCurrentState(void) const;

	// Returns HP (hit points).
	const Uint32 getCurrentHP(void) const;

	// Returns hitbox pointer at index n.
	Hitbox* getHitbox(const int n) const;

	// Returns maximum position X at which the player can be.
	const int getMaxXPos(void) const;

	// Returns pointer to current move.
	Move* getCurrentMove(void) const;

	// Returns true if hitboxes are active.
	const bool hitboxesActive(void) const;

	// Returns the difference between the default render width and the current
	// render width * 2. So it will be nonzero if the player's width has been 
	// expanded to allow for a certain frame of animation.
	// This is used in PlayerManager to calculate the correct midpoint between
	// the players, so that the camera doesn't slide with a move animation.
	const int getRenderWidthDiff(void) const;

	// Returns the absolute destination render rect of the player.
	const SDL_Rect& getRenderRect(void) const;

	// Returns midpoint of player.
	const int getMidpoint(void) const;

	// Setters

	// Sets the side of player, e.g., LEFT or RIGHT.
	void setSide(const Uint32 side);

	// Sets the mode of the player, e.g., LOCAL, NET, or AI.
	void setMode(const Uint32 mode);

	// Points the internal Widget pointer to the player's health bar.
	void setHealthBarPtr(Widget* pHealthBar);

	// Sets the side the player is on. Also sets the rendering flip variable 
	// accordingly.
	void setSide(const int side);

	// Sets current state.
	void setCurrentState(const Uint32 state);

	// Sets the Player's HP directly.
	void setCurrentHP(const Uint32 hp);

	// Sets the player object to draw hitboxes if true.
	void setDrawHitboxes(const bool draw);

	// Toggles the state of whether or not the player will draw hitboxes.
	void toggleDrawHitboxes(void);

	// Sets the player to colliding if true.
	void setColliding(const bool colliding);

	// Sets the current stun of the Player.
	void setStun(const Uint32 stun);

	// Sets active status of hitboxes.
	void setHitboxesActive(const bool active);

	// Sets the debug state which overrides player state for net debugging.
	void setDebugState(const int state);

private:
	// Physics.

	int32_t m_xAccel;
	int32_t m_xVel;
	int32_t m_xMax;
	Uint32 m_jumpStrength;
	Uint32 m_jumpSpeed;
	int m_xJumpVel;
	double m_jump;
	// Stores the input sequence number at which the client player jumps.
	Uint32 m_jumpCtr;

	// Render width and height (default dst rect).
	int m_rW, m_rH;
	SDL_Rect m_render;
	// The amount moved each frame.
	int m_translateX, m_translateY;

	// Game.

	// The y position at which player will appear 26 units from bottom of screen.
	int m_floor;
	Uint32 m_side;
	Uint32 m_mode;
	int m_maxHP;
	int m_currentHP;
	Uint32 m_currentStun;
	Widget* m_pHealthBar;
	std::shared_ptr<Input> m_pInput;
	MoveList m_moves;
	HitboxList m_hitboxes;
	std::shared_ptr<Move> m_pCurrentMove;
	std::shared_ptr<Timer> m_pMoveTimer;
	bool m_drawHitboxes;
	int m_maxXPos;
	bool m_colliding;
	// True if the hitboxes of the current move are active and can do damage.
	// This should be set to false once hitboxes of current move have connected.
	bool m_hitboxesActive;

	// Net stuff.
	int m_debugState;

	std::queue<Client::NetInput> m_clientInputs;
	std::queue<Server::PlayerUpdate> m_serverUpdates;
};

// ================================================ //

// Getters

inline Input* Player::getInput(void) const{
	return m_pInput.get();
}

inline const Uint32 Player::getMode(void) const{
	return m_mode;
}

inline const int32_t Player::getXVelocity(void) const{
	return (m_pFSM->getCurrentStateID() == Player::State::JUMPING) 
		? m_xMax : m_xVel;
}

inline const int32_t Player::getXJumpVelocity(void) const{
	return m_xJumpVel;
}

inline const int32_t Player::getXMaxVelocity(void) const{
	return m_xMax;
}

inline Widget* Player::getHealthBarPtr(void) const{
	return m_pHealthBar;
}

inline const int Player::getSide(void) const{
	return m_side;
}

inline const Uint32 Player::getCurrentState(void) const{
	return m_pFSM->getCurrentStateID();
}

inline const Uint32 Player::getCurrentHP(void) const{
	return m_currentHP;
}

inline Hitbox* Player::getHitbox(const int n) const{
	return m_hitboxes[n].get();
}

inline const int Player::getMaxXPos(void) const{
	return m_maxXPos;
}

inline Move* Player::getCurrentMove(void) const{
	return m_pCurrentMove.get();
}

inline const bool Player::hitboxesActive(void) const{
	return m_hitboxesActive;
}

inline const int Player::getRenderWidthDiff(void) const{
	return (m_side == Player::Side::LEFT) ? 0 : (m_dst.w - m_rW) * 2;
}

inline const SDL_Rect& Player::getRenderRect(void) const{
	return m_render;
}

// Setters

inline void Player::setSide(const Uint32 side){
	m_side = side;
}

inline void Player::setMode(const Uint32 mode){
	m_mode = mode;
}

inline void Player::setHealthBarPtr(Widget* pHealthBar){
	m_pHealthBar = pHealthBar;
}

inline void Player::setSide(const int side){
	m_side = side;
	m_flip = (side == Player::Side::LEFT) ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
}

inline void Player::setCurrentState(const Uint32 state){
	m_pFSM->setCurrentState(state);
}

inline void Player::setCurrentHP(const Uint32 hp){
	m_currentHP = hp;
}

inline void Player::setDrawHitboxes(const bool draw){
	m_drawHitboxes = draw;
}

inline void Player::toggleDrawHitboxes(void){
	m_drawHitboxes = !m_drawHitboxes;
}

inline void Player::setColliding(const bool colliding){
	m_colliding = colliding;
}

inline void Player::setStun(const Uint32 stun){
	m_currentStun = stun;
}

inline void Player::setHitboxesActive(const bool active){
	m_hitboxesActive = active;
}

inline void Player::setDebugState(const int state){
	m_debugState = state;
}

// ================================================ //

#endif

// ================================================ //