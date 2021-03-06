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
// File: WidgetTextbox.cpp
// Author: Jordan Sparks <unixunited@live.com>
// ================================================ //
// Implements the WidgetTextbox class, derived from Widget.
// ================================================ //

#include "WidgetTextbox.hpp"
#include "Label.hpp"
#include "Timer.hpp"
#include "Engine.hpp"

// ================================================ //

WidgetTextbox::WidgetTextbox(const int id) :
Widget(id),
m_cursorPos(0),
m_offset(0),
m_text(),
m_pCursorTimer(new Timer()),
m_renderCursor(true)
{
	this->setType(Widget::Type::TEXTBOX);

	m_renderLabel = true;
	m_pLabel.reset(new Label());

	m_pCursorTimer->restart();
}

// ================================================ //

WidgetTextbox::~WidgetTextbox(void)
{

}

// ================================================ //

void WidgetTextbox::handleEditing(const char* text, const bool backspace)
{
	if (backspace){
		// An empty label is one space, so don't operate on it if so.
		if (m_text != " " && !m_text.empty()){
			m_text.pop_back();
			if (m_offset > 0){
				std::string scrolledText = m_text.substr(--m_offset, m_text.length());
				Object::setLabel(scrolledText, m_pLabel->getOffset());
			}
			else{
				Object::setLabel(m_text, m_pLabel->getOffset());
			}
		}
	}
	else{
		// Test for number only text if the style is set.
		if (this->getStyle() & Widget::TB_NUMBER){
			for (unsigned int i = 0; i < std::strlen(text); ++i){
				if (!isdigit(text[i])){
					return;
				}
			}
		}

		m_text += text;
		Object::setLabel(m_text, m_pLabel->getOffset());

		// Prevent text from going outside of widget by scrolling it.
		if (m_pLabel->getWidth() > m_dst.w){
			std::string scrolledText = m_text.substr(++m_offset, m_text.length());
			Object::setLabel(scrolledText, m_pLabel->getOffset());
		}
	}
}

// ================================================ //

void WidgetTextbox::update(double dt)
{
	if (m_pCursorTimer->getTicks() > 500){
		m_renderCursor = !m_renderCursor;
		m_pCursorTimer->restart();
	}
}

// ================================================ //

void WidgetTextbox::render(void)
{
	SDL_RenderCopyEx(Engine::getSingletonPtr()->getRenderer(), m_pTexture, &m_src, &m_dst, 0, nullptr, m_flip);

	// Render label.
	SDL_Rect dst = m_dst;
	dst.x += 5; // TODO: store this offset as a value and parse from .theme file.
	if (m_pLabel->isCentered()){
		dst.x += m_pLabel->getOffset();
		dst.w -= m_pLabel->getOffset() * 2;
	}
	else{
		dst.w = m_pLabel->getWidth();
	}

	SDL_RenderCopyEx(Engine::getSingletonPtr()->getRenderer(), m_pLabel->getTexturePtr(),
		nullptr, &dst, 0, nullptr, m_flip);

	// Render the blinking cursor.
	if (this->isActive() && m_renderCursor){
		SDL_Color color = m_pLabel->getColor();

		SDL_SetRenderDrawBlendMode(Engine::getSingletonPtr()->getRenderer(), SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor(Engine::getSingletonPtr()->getRenderer(), color.r, color.g, color.b, 200);
		
		SDL_Rect rc;
		rc.x = m_dst.x + m_pLabel->getWidth() + 5;
		rc.y = m_dst.y + m_dst.h / 10;
		rc.w = 2;
		rc.h = m_dst.h - (m_dst.h / 5);
		SDL_RenderFillRect(Engine::getSingletonPtr()->getRenderer(), &rc);
	}
}

// ================================================ //

void WidgetTextbox::setLabel(const std::string& label, const int offset)
{
	m_offset = 0;
	m_text = label;

	Object::setLabel(label, offset);
	// Scroll text to end of string if needed.
	while (m_pLabel->getWidth() > m_dst.w){
		std::string scrolledText = m_text.substr(++m_offset, m_text.length());
		Object::setLabel(scrolledText, m_pLabel->getOffset());
	}
}

// ================================================ //

