// ================================================ //
// Extreme Metal Fighter
// Copyright (C) 2014 Jordan Sparks. All Rights Reserved.
// Unauthorized copying of this file, via any medium is strictly prohibited
// Proprietary and confidential
// Written by Jordan Sparks <unixunited@live.com> June 2014
// ================================================ //
// File: Server.cpp
// Author: Jordan Sparks <unixunited@live.com>
// ================================================ //
// Implements Server singleton class.
// ================================================ //

#include "Server.hpp"
#include "NetMessage.hpp"
#include "Engine.hpp"
#include "Config.hpp"
#include "Timer.hpp"
#include "GameManager.hpp"
#include "PlayerManager.hpp"
#include "Input.hpp"
#include "Log.hpp"

// ================================================ //

template<> Server* Singleton<Server>::msSingleton = nullptr;

// ================================================ //

Server::Server(const int port) :
m_peer(RakNet::RakPeerInterface::GetInstance()),
m_packet(nullptr),
m_buffer(),
m_clients(),
m_redAddr(),
m_blueAddr(),
m_redInputSeq(0),
m_blueInputSeq(0),
m_readyQueue(),
m_pUpdateTimer(new Timer())
{
	Log::getSingletonPtr()->logMessage("Initializing Server...");

	RakNet::SocketDescriptor sd(port, 0);
	m_peer->Startup(Server::MaxClients, &sd, 1);
	m_peer->SetMaximumIncomingConnections(Server::MaxClients);

	//m_peer->ApplyNetworkSimulator(0.05f, 50, 0);

	Log::getSingletonPtr()->logMessage("Server initialized!");
}

// ================================================ //

Server::~Server(void)
{
	RakNet::RakPeerInterface::DestroyInstance(m_peer);
}

// ================================================ //

Uint32 Server::send(const RakNet::BitStream& bit, const RakNet::SystemAddress& addr,
	const PacketPriority priority, const PacketReliability reliability)
{
	return m_peer->Send(&bit, priority, reliability, 0, addr, false);
}

// ================================================ //

Uint32 Server::broadcast(const RakNet::Packet* packet, const PacketPriority priority,
	const PacketReliability reliability, const RakNet::SystemAddress& exclude)
{
	RakNet::BitStream bit(packet->data, packet->length, false);
	
	return m_peer->Send(&bit, priority, reliability, 0, exclude, true);
}

// ================================================ //

Uint32 Server::broadcast(const RakNet::BitStream& bit, const PacketPriority priority, 
	const PacketReliability reliability, const RakNet::SystemAddress& exclude)
{
	return m_peer->Send(&bit, priority, reliability, 0, exclude, true);
}

// ================================================ //

Uint32 Server::chat(const std::string& msg)
{
	RakNet::BitStream bit;
	bit.Write(static_cast<RakNet::MessageID>(NetMessage::CHAT));
	bit.Write(msg.c_str());

	return this->broadcast(bit, HIGH_PRIORITY, RELIABLE_ORDERED);
}

// ================================================ //

Uint32 Server::ready(const Uint32 fighter)
{
	if (this->addToReadyQueue(GameManager::getSingletonPtr()->getUsername(), fighter)){
		RakNet::BitStream bit;
		bit.Write(static_cast<RakNet::MessageID>(NetMessage::READY));
		bit.Write(GameManager::getSingletonPtr()->getUsername().c_str());

		return this->broadcast(bit, HIGH_PRIORITY, RELIABLE_ORDERED);
	}

	return 0;
}

// ================================================ //

Uint32 Server::startGame(void)
{
	// First let's tell every client that the game is starting.
	RakNet::BitStream bit;
	bit.Write(static_cast<RakNet::MessageID>(NetMessage::SERVER_STARTING_GAME));
	// Write red and blue fighters in order.
	bit.Write(static_cast<Uint32>(this->getNextRedPlayer().fighter));
	bit.Write(static_cast<Uint32>(this->getNextBluePlayer().fighter));
	this->broadcast(bit, HIGH_PRIORITY, RELIABLE_ORDERED);

	// Then tell any clients that are playing that they are actually playing.
	// Check red player.
	if (GameManager::getSingletonPtr()->getState() != GameManager::PLAYING_RED){
		ReadyClient red = this->getNextRedPlayer();
		ClientConnection redConnection = m_clients[this->getClient(red.username)];
		m_redAddr = redConnection.addr;

		RakNet::BitStream play;
		play.Write(static_cast<RakNet::MessageID>(NetMessage::PLAYING_RED));
		this->send(play, m_redAddr, IMMEDIATE_PRIORITY, RELIABLE_ORDERED);
	}

	// Check blue player.
	if (GameManager::getSingletonPtr()->getState() != GameManager::PLAYING_BLUE){
		ReadyClient blue = this->getNextBluePlayer();
		ClientConnection blueConnection = m_clients[this->getClient(blue.username)];
		m_blueAddr = blueConnection.addr;

		RakNet::BitStream play;
		play.Write(static_cast<RakNet::MessageID>(NetMessage::PLAYING_BLUE));
		this->send(play, m_blueAddr, IMMEDIATE_PRIORITY, RELIABLE_ORDERED);
	}

	m_pUpdateTimer->restart();

	return 1;
}

// ================================================ //

Uint32 Server::sendPlayerList(const RakNet::SystemAddress& addr)
{
	RakNet::BitStream bit;
	bit.Write(static_cast<RakNet::MessageID>(NetMessage::PLAYER_LIST));

	// Write number of connected clients, plus the server.
	bit.Write(static_cast<Uint32>(m_clients.size() + 1));

	// Write server username.
	bit.Write(GameManager::getSingletonPtr()->getUsername().c_str());

	// Write each client username.
	for (ClientList::iterator itr = m_clients.begin();
		itr != m_clients.end();
		++itr){
		bit.Write(itr->username.c_str());
	}

	return this->send(bit, addr, HIGH_PRIORITY, RELIABLE);
}

// ================================================ //

Uint32 Server::updatePlayers(void)
{
	RakNet::BitStream bit;
	bit.Write(static_cast<RakNet::MessageID>(NetMessage::UPDATE_PLAYERS));

	// Write timestamp.
	bit.Write(RakNet::GetTime());

	// Write red player data.
	PlayerUpdate redPlayer;
	Player* red = PlayerManager::getSingletonPtr()->getRedPlayer();
	redPlayer.inputSeq = m_redInputSeq;
	redPlayer.x = red->getPosition().x;
	redPlayer.y = red->getPosition().y;
	redPlayer.xVel = red->m_xVel;
	redPlayer.xAccel = red->m_xAccel;
	bit.Write(redPlayer);

	// Write blue player data.
	PlayerUpdate bluePlayer;
	Player* blue = PlayerManager::getSingletonPtr()->getBluePlayer();
	bluePlayer.inputSeq = m_blueInputSeq;
	bluePlayer.x = blue->getPosition().x;
	bluePlayer.y = blue->getPosition().y;
	bluePlayer.xVel = blue->m_xVel;
	bluePlayer.xAccel = blue->m_xAccel;
	bit.Write(bluePlayer);

	return this->broadcast(bit, IMMEDIATE_PRIORITY, UNRELIABLE_SEQUENCED);
}

// ================================================ //

Uint32 Server::updateRedPlayer(const Uint32 inputSeq)
{
	Player* red = PlayerManager::getSingletonPtr()->getRedPlayer();
	PlayerUpdate update;
	update.inputSeq = inputSeq;
	update.x = red->getPosition().x;
	update.y = red->getPosition().y;
	update.xVel = red->m_xVel;
	update.xAccel = red->m_xAccel;

	RakNet::BitStream bit;
	bit.Write(static_cast<RakNet::MessageID>(NetMessage::UPDATE_RED_PLAYER));
	bit.Write(update);

	return this->send(bit, m_redAddr, IMMEDIATE_PRIORITY);
}

// ================================================ //

Uint32 Server::updateBluePlayer(const Uint32 inputSeq)
{

	return 0;
}

// ================================================ //

int Server::update(double dt)
{
	// Update players.
	this->updatePlayers();
	/*if (m_pUpdateTimer->getTicks() > 100){
		this->updatePlayers();

		m_pUpdateTimer->restart();
	}*/

	// Process any incoming packets.
	for (m_packet = m_peer->Receive(); 
		m_packet; 
		m_peer->DeallocatePacket(m_packet), m_packet = m_peer->Receive()){
		switch (m_packet->data[0]){
		default:

			break;

		case ID_NEW_INCOMING_CONNECTION:
			// Wait for SET_USERNAME message.
			break;

		case ID_DISCONNECTION_NOTIFICATION:
			if (this->isClientConnected(m_packet->systemAddress)){
				m_buffer = m_clients[this->getClient(m_packet->systemAddress)].username;
				Log::getSingletonPtr()->logMessage("SERVER: Removing client [" +
					std::string(m_packet->systemAddress.ToString()) + "]");
				this->removeFromReadyQueue(m_buffer);
				this->removeClient(m_packet->systemAddress);

				// Tell all other clients.
				{
					RakNet::BitStream bit;
					bit.Write(static_cast<RakNet::MessageID>(NetMessage::CLIENT_DISCONNECTED));
					bit.Write(m_buffer.c_str());
					this->broadcast(bit, HIGH_PRIORITY, RELIABLE);
				}
			}
			else{
				return 0;
			}
			break;

		case ID_CONNECTION_LOST:
			m_buffer = m_clients[this->getClient(m_packet->systemAddress)].username;
			Log::getSingletonPtr()->logMessage("SERVER: Removing client [" +
				std::string(m_packet->systemAddress.ToString()) + "]");
			this->removeFromReadyQueue(m_buffer);
			this->removeClient(m_packet->systemAddress);

			{
				RakNet::BitStream bit;
				bit.Write(static_cast<RakNet::MessageID>(NetMessage::CLIENT_LOST_CONNECTION));
				bit.Write(m_buffer.c_str());
				this->broadcast(bit, HIGH_PRIORITY, RELIABLE);
			}
			break;

		case NetMessage::SYSTEM_MESSAGE:
			{
				RakNet::BitStream bit(m_packet->data, m_packet->length, false);
				RakNet::RakString rs;
				bit.IgnoreBytes(sizeof(RakNet::MessageID));
				bit.Read(rs);
				printf("Client says: %s\n", rs.C_String());
			}
			break;

		case NetMessage::SET_USERNAME:
			{
				RakNet::BitStream bit(m_packet->data, m_packet->length, false);
				RakNet::RakString rs;
				bit.IgnoreBytes(sizeof(RakNet::MessageID));
				bit.Read(rs);

				if (this->isUsernameInUse(rs.C_String())){
					RakNet::BitStream reject;
					reject.Write(static_cast<RakNet::MessageID>(NetMessage::USERNAME_IN_USE));

					this->send(reject, m_packet->systemAddress, HIGH_PRIORITY, RELIABLE);
					return 0;
				}
				else{
					Log::getSingletonPtr()->logMessage("SERVER: Client [" + std::string(m_packet->systemAddress.ToString()) +
						"] connected with username \"" + rs.C_String() + "\"");

					// Send a list of players to newly connected client.
					this->sendPlayerList(m_packet->systemAddress);

					// Add them to the client list.
					this->registerClient(rs.C_String(), m_packet->systemAddress);

					this->broadcast(m_packet, HIGH_PRIORITY, RELIABLE, m_packet->systemAddress);
				}
			}
			break;

		case NetMessage::CHAT:
			this->broadcast(m_packet, HIGH_PRIORITY, RELIABLE_ORDERED, m_packet->systemAddress);
			break;

		case NetMessage::READY:
			// Add username to ready queue and broadcast.
			if (this->isClientConnected(m_packet->systemAddress)){
				int client = this->getClient(m_packet->systemAddress);
				RakNet::BitStream data(m_packet->data, m_packet->length, false);
				data.IgnoreBytes(sizeof(RakNet::MessageID));
				Uint32 fighter;
				data.Read(fighter);
				if (this->addToReadyQueue(m_clients[client].username, fighter)){
					m_buffer = m_clients[client].username;

					RakNet::BitStream bit;
					bit.Write(static_cast<RakNet::MessageID>(NetMessage::READY));
					bit.Write(m_buffer.c_str());
					bit.Write(fighter);

					this->broadcast(bit, HIGH_PRIORITY, RELIABLE_ORDERED, m_packet->systemAddress);
					return m_packet->data[0];
				}
			}
			return 0;

		case NetMessage::CLIENT_INPUT:
			if (m_packet->systemAddress == m_redAddr || m_packet->systemAddress == m_blueAddr){
				RakNet::BitStream bit(m_packet->data, m_packet->length, false);
				bit.IgnoreBytes(sizeof(RakNet::MessageID));
				Uint32 button = 0, seq = 0;
				bool value = false;
				bit.Read(seq);
				bit.Read(button);
				bit.Read(value);
				if (m_packet->systemAddress == m_redAddr){
					PlayerManager::getSingletonPtr()->getRedPlayerInput()->setButton(button, value);
					m_redInputSeq = seq;
					//this->updateRedPlayer(m_redInputSeq);
				}
				else{
					PlayerManager::getSingletonPtr()->getBluePlayerInput()->setButton(button, value);
					m_blueInputSeq = seq;
				}
			}
			break;
		}

		return m_packet->data[0];
	}

	return 0;
}

// ================================================ //

void Server::registerClient(const char* username, const RakNet::SystemAddress& addr)
{
	ClientConnection client;
	client.username = username;
	client.addr = addr;
	m_clients.push_back(client);
}

// ================================================ //

void Server::removeClient(const RakNet::SystemAddress& addr)
{
	m_clients.erase(m_clients.begin() + this->getClient(addr));
}

// ================================================ //

bool Server::isUsernameInUse(const std::string& username)
{
	if (username.compare(GameManager::getSingletonPtr()->getUsername()) == 0){
		return true;
	}
	for (ClientList::iterator itr = m_clients.begin();
		itr != m_clients.end();
		++itr){
		if (username.compare(itr->username) == 0){
			return true;
		}
	}

	return false;
}

// ================================================ //

void Server::dbgPrintAllConnectedClients(void)
{
	if (m_clients.size() == 0){
		printf("NO CONNECTED CLIENTS.\n");
	}
	else{
		printf("CURRENTLY CONNECTED CLIENTS (FROM CLIENT LIST):\n");
		for (ClientList::iterator itr = m_clients.begin();
			itr != m_clients.end();
			++itr){
			printf("-----------\nUsername: %s\nAddress: %s\n",
				itr->username.c_str(), itr->addr.ToString());
		}
		printf("\nCONNECTED SYSTEMS:\n");
		unsigned short num = Server::MaxClients;
		RakNet::SystemAddress* addrs = new RakNet::SystemAddress[num];
		m_peer->GetConnectionList(addrs, &num);
		for (USHORT u = 0; u < num; ++u){
			printf("%s\n", addrs[u].ToString());
		}
		printf("DONE.\n");
	}
}

// ================================================ //

void Server::dbgPrintReadyQueue(void)
{
	if (m_readyQueue.size() == 0){
		printf("EMPTY READY QUEUE.\n");
	}
	else{
		printf("READY QUEUE:\n");
		Uint32 i = 1;
		for (ReadyQueue::iterator itr = m_readyQueue.begin();
			itr != m_readyQueue.end();
			++itr, ++i){
			printf("%d - %s / %d\n", i, itr->username.c_str(), itr->fighter);
		}
	}
}

// ================================================ //

const char* Server::getLastPacketStrData(void) const
{
	RakNet::BitStream bit(m_packet->data, m_packet->length, false);
	RakNet::RakString rs;
	bit.IgnoreBytes(sizeof(RakNet::MessageID));
	bit.Read(rs);
	return rs.C_String();
}

// ================================================ //