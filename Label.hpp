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
// File: Label.hpp
// Author: Jordan Sparks <unixunited@live.com>
// ================================================ //
// Defines Label class.
// ================================================ //

#ifndef __LABEL_HPP__
#define __LABEL_HPP__

// ================================================ //

#include "stdafx.hpp"

// ================================================ //

// A class for dynamically rendering text. Use getTexturePtr()
// to get the texture containing the text.
class Label
{
public:
	// Sets SDL_Texture to nullptr and offset to zero.
	explicit Label(const bool centered = false);

	// Frees the SDL_Texture.
	virtual ~Label(void);

	// Creates the label texture with the text contained in parameter label.
	// If wrap is greater than zero, the label is wrapped within that width.
	void build(const std::string& label, const int wrap = 0);

	// Getters

	// Returns a pointer to the SDL_Texture containing the text.
	SDL_Texture* getTexturePtr(void) const;

	// Returns the SDL_Color of the label.
	const SDL_Color getColor(void) const;

	// Returns the offset for rendering the text.
	const int getOffset(void) const;

	// Returns true if the text is centered.
	const bool isCentered(void) const;

	// Returns the width of the generated label.
	const int getWidth(void) const;

	// Returns the height of the generated label.
	const int getHeight(void) const;

	// Returns the actual text of the label.
	const std::string getText(void) const;

	// Returns index of Font to use from FontManager.
	const int getFont(void) const;

	// Setters

	// Sets the label's color. If rebuild is true, the Label is rebuilt
	// (it will internally call build()).
	void setColor(const int r, const int g, const int b, const int a, const bool rebuild = true);

	// Sets the label's color. If rebuild is true, the Label is rebuilt
	// (it will internally call build()).
	void setColor(const SDL_Color& color, const bool rebuild = true);

	// Sets the offset for rendering centered text.
	void setOffset(const int offset);

	// Sets whether or not the text is center-oriented.
	void setCentered(const bool centered);

	// Sets index of Font to use from the FontManager.
	void setFont(const int font);

private:
	SDL_Texture* m_pTexture;
	SDL_Color m_color;
	bool m_centered;
	int m_width, m_height;
	std::string m_text;
	int m_wrap;
	// Index of font to use from FontManager.
	int m_font;

	// Space between both left/right sides of Object's dst.
	int m_offset;
};

// ================================================ //

// Getters

inline SDL_Texture* Label::getTexturePtr(void) const{
	return m_pTexture;
}

inline const SDL_Color Label::getColor(void) const{
	return m_color;
}

inline const int Label::getOffset(void) const{
	return m_offset;
}

inline const bool Label::isCentered(void) const{
	return m_centered;
}

inline const int Label::getWidth(void) const{
	return m_width;
}

inline const int Label::getHeight(void) const{
	return m_height;
}

inline const std::string Label::getText(void) const{
	return m_text;
}

inline const int Label::getFont(void) const{
	return m_font;
}

// Setters

inline void Label::setColor(const int r, const int g, const int b, const int a,
	const bool rebuild){
	m_color.r = r; m_color.g = g; m_color.b = b; m_color.a = a;
	if (rebuild){
		this->build(m_text, m_wrap);
	}
}

inline void Label::setColor(const SDL_Color& color, const bool rebuild){
	m_color = color;
	if (rebuild){
		this->build(m_text, m_wrap);
	}
}

inline void Label::setOffset(const int offset){
	m_offset = offset;
}

inline void Label::setCentered(const bool centered){
	m_centered = centered;
}

inline void Label::setFont(const int font){
	m_font = font;
}

// ================================================ //

#endif

// ================================================ //