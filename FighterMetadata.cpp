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
// File: FighterMetadata.cpp
// Author: Jordan Sparks <unixunited@live.com>
// ================================================ //
// Implements FighterMetadata class.
// ================================================ //

#include "FighterMetadata.hpp"
#include "Engine.hpp"
#include "Move.hpp"
#include "Hitbox.hpp"
#include "Player.hpp"

// ================================================ //

FighterMetadata::FighterMetadata(void) :	
Config(Config::FIGHTER_METADATA),
m_moveBeg()
{

}

// ================================================ //

FighterMetadata::FighterMetadata(const std::string& file) : 
Config(Config::FIGHTER_METADATA),
m_moveBeg()
{
	this->loadFile(file);
}

// ================================================ //

FighterMetadata::~FighterMetadata(void)
{

}

// ================================================ //

std::shared_ptr<Move> FighterMetadata::parseMove(const std::string& name)
{
	this->resetFilePointer();

	while(!m_file.eof()){
		m_file >> m_buffer;

		// Look for moves section.
		if (m_buffer[0] == '[' && m_buffer[m_buffer.size() - 1] == ']'){
			if (m_buffer.compare(1, m_buffer.size() - 2, "moves") == 0){

				// In the moves section, now find the move.
				while(!m_file.eof()){
					m_file >> m_buffer;

					if (m_buffer[0] == '+'){
						// Found a move, see if it's the right one (example m_buffer: "+(IDLE)").
						if (m_buffer.compare(2, m_buffer.size() - 3, name) == 0){

							// Setup Move object for this move.
							std::shared_ptr<Move> pMove;
							pMove.reset(new Move());
							pMove->name = name;

							// Store beginning of this move's data for later.
							m_moveBeg = m_file.tellg();

							// Get the main data for the move.
							pMove->numFrames = this->parseMoveIntValue("core", "numFrames");
							pMove->frames.reserve(pMove->numFrames);
							pMove->frameGap = this->parseMoveIntValue("core", "frameGap");

							// Get the frame data.
							m_buffer = this->parseMoveValue("core", "frameData");
							char c;
							std::istringstream parse(m_buffer);
							parse >> pMove->startupFrames;
							parse >> c;
							parse >> pMove->hitFrames;
							parse >> c;
							parse >> pMove->recoveryFrames;

							// Get other core data.
							pMove->damage = this->parseMoveIntValue("core", "damage");
							pMove->hitstun = this->parseMoveIntValue("core", "hitstun");
							pMove->blockstun = this->parseMoveIntValue("core", "blockstun");

							pMove->knockback = this->parseMoveIntValue("core", "knockback");
							pMove->recoil = this->parseMoveIntValue("core", "recoil");
							pMove->repeat = this->parseMoveBoolValue("core", "repeat");
							if (pMove->repeat)
								pMove->repeatFrame = this->parseMoveIntValue("core", "repeatFrame");
							pMove->reverse = this->parseMoveBoolValue("core", "reverse");
							pMove->transition = this->parseMoveIntValue("core", "transition");

							// Parse any cancels.
							// ...

							// Get locomotion data.
							pMove->xVel = this->parseMoveIntValue("locomotion", "xVel");
							pMove->yVel = this->parseMoveIntValue("locomotion", "yVel");

							// Parse initial frame.
							Frame frame1;
							frame1.x = this->parseMoveIntValue("frame1", "x");
							frame1.y = this->parseMoveIntValue("frame1", "y");
							frame1.w = this->parseMoveIntValue("frame1", "w");
							frame1.h = this->parseMoveIntValue("frame1", "h");							
							if ((frame1.gap = this->parseIntValue("frame1", "gap")) == -1){
								// Inherit global frame gap.
								frame1.gap = pMove->frameGap;
							}

							// Calculate first rw and rh values based on default size.
							frame1.rw = this->parseMoveIntValue("frame1", "rw");
							frame1.rh = this->parseMoveIntValue("frame1", "rh");
							// Only allow expanding of the rendering size on the first frame.
							if (frame1.rw < 0){
								frame1.rw = 0;
							}
							if (frame1.rh < 0){
								frame1.rh = 0;
							}

							pMove->frames.push_back(frame1);
							this->parseHitboxes(pMove, "frame1");

							// Parse the rest of the frames.
							for(int i=2; i<=pMove->numFrames; ++i){
								Frame frame;
								std::string frameSection = "frame" + Engine::toString(i);
								frame.x = this->parseMoveIntValue(frameSection.c_str(), "x");
								frame.y = this->parseMoveIntValue(frameSection.c_str(), "y");
								frame.w = this->parseMoveIntValue(frameSection.c_str(), "w");
								frame.h = this->parseMoveIntValue(frameSection.c_str(), "h");								
								if ((frame.gap = this->parseMoveIntValue(frameSection.c_str(), "gap")) == -1){
									frame.gap = pMove->frameGap;
								}

								// See if any values should be inherited (-1 means inherit from previous frame).
								if (frame.x == -1)
									frame.x = pMove->frames.back().x;
								if (frame.y == -1)
									frame.y = pMove->frames.back().y;
								if (frame.w == -1)
									frame.w = pMove->frames.back().w;
								if (frame.h == -1)
									frame.h = pMove->frames.back().h;

								// Calculate rw and rh values.
								frame.rw = (frame.w - pMove->frames.back().w);
								frame.rh = (frame.h - pMove->frames.back().h);								

								pMove->frames.push_back(frame);
								this->parseHitboxes(pMove, frameSection.c_str());
							}

							return pMove;
						}
					}
				}
			}
		}
	}

	return nullptr;
}

// ================================================ //

std::string FighterMetadata::parseMoveValue(const std::string& section, const std::string& value)
{
	// Reset file pointer to beginning of this move.
	m_file.clear();
	m_file.seekg(m_moveBeg);

	while(!m_file.eof()){
		// Find the section.
		m_file >> m_buffer;

		if (m_buffer[0] == '[' && m_buffer[m_buffer.size() - 2] == ']'){
			
			// Check section name.
			if (m_buffer.compare(1, m_buffer.size() - 3, section) == 0){
				while(m_buffer[0] != '}'){
					m_file >> m_buffer;

					// Skip comments.
					if (m_buffer[0] != '#'){
						size_t assign = m_buffer.find_first_of('=');

						// See if this is the value.
						if (m_buffer.compare(0, assign, value) == 0){
							// Trim the string to only the value.
							m_buffer = m_buffer.substr(assign + 1, m_buffer.size());

							return m_buffer;
						}
					}
				}
			}
		}
		else if (m_buffer[0] == '-'){
			// End of move found.
			break;
		}
	}

	m_file.clear();
	return std::string("");
}

// ================================================ //

const int FighterMetadata::parseMoveIntValue(const std::string& section, const std::string& value)
{
	std::string str = this->parseMoveValue(section, value);
	if (!str.empty()){
		return std::stoi(str);
	}

	return -1;
}

// ================================================ //

const bool FighterMetadata::parseMoveBoolValue(const std::string& section, const std::string& value)
{
	return (this->parseMoveIntValue(section, value) >= 1);
}

// ================================================ //

SDL_Rect FighterMetadata::parseRect(const std::string& str)
{
	SDL_Rect rc;
	memset(&rc, 0, sizeof(rc));
	
	if (str.empty()){
		Log::getSingletonPtr()->logMessage("No hitbox found for \"" + str + "\"");
		return rc;
	}
	
	// str should look like "(0,0,100,100)".
	char c;
	std::istringstream parse(str);
	parse >> c; // eat "(".
	parse >> rc.x;
	parse >> c; // eat ",".
	parse >> rc.y;
	parse >> c;
	parse >> rc.w;
	parse >> c;
	parse >> rc.h;

	return rc;
}

// ================================================ //

void FighterMetadata::parseHitboxes(std::shared_ptr<Move> pMove, const std::string& frame)
{
	// Load all hitbox rects into the frame's hitbox list.

	// Parse normal hitboxes.
	for (int i = Hitbox::HBOX_LOWER, num = 1; i <= Hitbox::HBOX_HEAD; ++i, ++num){
		std::string hbox = this->parseMoveValue(frame, std::string("hbox" + Engine::toString(num)).c_str());
		pMove->frames.back().hitboxes.push_back(this->parseRect(hbox));
	}

	// Parse throw box.
	std::string tbox = this->parseMoveValue(frame, "tbox");
	pMove->frames.back().hitboxes.push_back(this->parseRect(tbox));

	// Parse damage boxes.
	for (int i = Hitbox::DBOX1, num = 1; i <= Hitbox::DBOX2; ++i, ++num){
		std::string dbox = this->parseMoveValue(frame, std::string("dbox" + Engine::toString(num)).c_str());
		pMove->frames.back().hitboxes.push_back(this->parseRect(dbox));
	}

	// Parse counter boxes.
	for (int i = Hitbox::CBOX1, num = 1; i <= Hitbox::CBOX2; ++i, ++num){
		std::string cbox = this->parseMoveValue(frame, std::string("cbox" + Engine::toString(num)).c_str());
		pMove->frames.back().hitboxes.push_back(this->parseRect(cbox));
	}
}

// ================================================ //