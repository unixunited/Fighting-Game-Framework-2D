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
// File: Client.hpp
// Author: Jordan Sparks <unixunited@live.com>
// ================================================ //
// Defines Client singleton class.
// ================================================ //

#ifndef __CLIENT_HPP__
#define __CLIENT_HPP__

// ================================================ //

#include "stdafx.hpp"
#include "Input.hpp"

// ================================================ //

class Timer;

// ================================================ //

// Handles all client-side operations.
class Client : public Singleton<Client>
{
public:
	// Allocates socket and server IP.
	explicit Client(const std::string& server, const int port);

	// Empty destructor.
	~Client(void);

	// Processes packets that affect more than one game state.
	bool update(void);

	// Send-to-server functions.

	// Sends a packet to the server using a BitStream.
	Uint32 send(const RakNet::BitStream& bit, const PacketPriority priority = HIGH_PRIORITY,
		const PacketReliability reliability = UNRELIABLE);

	// Sends a connect request to server.
	Uint32 connect(void);

	// Sends disconnect message to server if connected, returns 0 if not.
	void disconnect(void);

	// Sends a chat message to server.
	Uint32 chat(const std::string& msg);

	// Sends a READY packet to server.
	Uint32 ready(const Uint32 fighter);

	// Sends an input value to the server.
	Uint32 sendInput(const Uint32 input, const bool value, const double dt);

	// Getters

	// Returns pointer to internal RakNet RakPeerInterface.
	RakNet::RakPeerInterface* getPeer(void);

	// Returns pointer to internal RakNet Packet.
	RakNet::Packet* getPacket(void);

	// Returns a string of the first set of data of the last packet (skipping
	// the first byte).
	const char* getPacketStrData(void) const;

	Uint32 m_inputSeq;
	

	// Setters

	// --- //

	typedef struct{
		Uint32 input;
		bool value;
		Uint32 seq;
		double dt;
	} NetInput;

	typedef struct{
		Uint32 input;
		bool value;
		Uint32 seq;
		double dt;
		int32_t xVel;
	} ClientInput;

	typedef struct{
		int shift;
		int playerShift;
		Uint32 seq;
	} StageShift;

	typedef std::list<Client::ClientInput> ClientInputList;
	ClientInputList m_pendingInputs;
	std::list<StageShift> m_pendingStageShifts;

public:
	RakNet::RakPeerInterface* m_peer;
	RakNet::Packet* m_packet;
	RakNet::SystemAddress m_serverAddr;
	std::string m_server;
	unsigned short m_port;
	bool m_connected;
	Uint32 m_stageShiftSeq;
	
	int m_timeout;
};

// ================================================ //

// Getters

inline RakNet::RakPeerInterface* Client::getPeer(void){
	return m_peer;
}

inline RakNet::Packet* Client::getPacket(void){
	return m_packet;
}

// ================================================ //

#endif

// ================================================ //