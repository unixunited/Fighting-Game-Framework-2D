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
// File: GUI.hpp
// Author: Jordan Sparks <unixunited@live.com>
// ================================================ //
// Defines GUI class, GUITheme, and GUILayer class. 
// ================================================ //

#ifndef __GUI_HPP__
#define __GUI_HPP__

// ================================================ //

#include "stdafx.hpp"

// ================================================ //

class GUILayer;
class Widget;
class Config;

// ================================================ //

typedef std::vector<std::shared_ptr<Widget>> WidgetList;
typedef std::vector<std::shared_ptr<GUILayer>> GUILayerList;
typedef std::vector<std::string> StringList;

// ================================================ //

// A class defining a layer within a GUI system. 
// A GUI object holds a list of GUILayers, and only displays one GUILayer at a time. 
// GUILayer holds a list of Widgets which are updated each frame, and rendered if 
// it's the active GUILayer.
class GUILayer
{
public:
	// Initializes ID to zero.
	explicit GUILayer(void);

	// Empty destructor.
	virtual ~GUILayer(void);

	// Adds a new Widget smart pointer to the list of Widgets.
	// Parameters:
	// widget - a derived object of Widget (smart pointer)
	void addWidget(std::shared_ptr<Widget> widget);

	// Parses all Widgets of type T for the GUILayer and adds them to the Widget list.
	// This should be called from a GUI object's constructor.
	template<typename T>
	void parse(Config& c, const int widgetType, const StringList& names);

	// Getters

	// Returns the ID of the GUILayer.
	const int getID(void) const;

	// Returns a Widget pointer for specified widget.
	// Parameters:
	// n - Index of Widget in GUILayer
	Widget* getWidgetPtr(const int n) const;

	// Returns the number of Widgets in GUILayer.
	const int getNumWidgets(void) const;

	// Setters

	// Sets the ID of the GUILayer.
	void setID(const int id);

	// Sets the name of the GUILayer.
	void setLayerName(const std::string& name);

	// --- //

	// Resets all Widget's appearances in GUILayer to Appearance::IDLE.
	void resetAllWidgets(const int cursor);

	// Render every Widget.
	virtual void render(void);

	// Update all Widgets with delta time.
	virtual void update(double dt);

private:
	// Unique ID for a GUILayer.
	int m_id;

	WidgetList m_widgets;

	// Set by derived classes so GUILayer::parse() knows where to look.
	std::string m_layerName;
};

// ================================================ //

inline void GUILayer::addWidget(std::shared_ptr<Widget> widget){
	m_widgets.push_back(widget);
}

// Getters

inline const int GUILayer::getID(void) const{
	return m_id;
}

inline Widget* GUILayer::getWidgetPtr(const int n) const{
	return (static_cast<unsigned int>(n) < m_widgets.size()) ? m_widgets[n].get() :
		m_widgets[0].get();
}

inline const int GUILayer::getNumWidgets(void) const{
	return m_widgets.size();
}

// Setters

inline void GUILayer::setID(const int id){
	m_id = id;
}

inline void GUILayer::setLayerName(const std::string& name){
	m_layerName = name;
}

// ================================================ //
// ================================================ //

// A polymorphic, abstract class representing a single GUI (e.g, main menu, pause menu). 
// Holds a list of GUILayer objects. Assists in navigation of menu widgets using 
// mouse and keyboard or gamepad.
class GUI
{
public:
	// Default initializes all member variables. Navigation mode is MOUSE.
	explicit GUI(void);

	// Empty destructor, used to make this class abstract.
	virtual ~GUI(void) = 0;

	// The type of navigation modes possible. SELECTOR is for keyboards/gamepads.
	enum NavMode{
		MOUSE = 0,
		SELECTOR
	};

	// Navigation actions, used during both navigation modes.
	enum Action{
		NAVIGATE = 0,
		BEGIN_PRESS,
		FINISH_PRESS
	};

	// Adds a GUILayer to the GUI's layer list.
	void addLayer(std::shared_ptr<GUILayer> layer);

	// Sets the currently selected Widget's appearance to IDLE.
	// Useful for switching from SELECTOR to MOUSE navigation.
	void clearSelector(void);

	// Clears the selector, sets the currently selected widget to 0. If navigating
	// in SELECTOR mode, it sets the appearance of Widget 0 to SELECTED. Pushes 
	// layer value n onto the layer stack.
	void pushLayer(const int n);

	// Pops the layer stack and sets current layer to the top of the stack.
	void popLayer(void);

	// Modifies the label of the Widget under control of the cursor.
	void handleTextInput(const char* text, const bool backspace = false);

	// Shows the message box layer if true, and sets the text if specified.
	void showMessageBox(const bool show, const std::string& text = "");

	// Shows the yes/no box if true, and sets the text if specified.
	void showYesNoBox(const bool show, const std::string& text = "");

	// Getters

	// Returns index value of current layer.
	const int getCurrentLayer(void) const;

	// Returns a pointer to the current GUILayer in use.
	GUILayer* getCurrentLayerPtr(void) const;

	// Returns the navigation mode currently in use (e.g., SELECTOR or MOUSE).
	const int getNavigationMode(void) const;

	// Returns the index of the currently selected Widget. Index is determined
	// by the current GUILayer's index for the Widget. 
	const int getSelectedWidget(void) const;

	// Returns the index at which the cursor is active.
	const int getCursor(void) const;

	// Returns true if the selector button is being held.
	const bool getSelectorPressed(void) const;

	// Returns a Widget pointer for the Widget at index n.
	Widget* getWidgetPtr(const int n) const;

	// Returns a Widget pointer for the currently selected Widget. An easier
	// way to say: gui->WidgetPtr(gui->getSelectedWidget());
	Widget* getSelectedWidgetPtr(void) const;

	// Returns true if a Widget has the cursor and is processing text input.
	const bool isEditingText(void) const;

	// Returns true if the message box is being rendered.
	const bool isMessageBoxVisible(void) const;

	// Returns true if the yes/no box is being rendered.
	const bool isYesNoBoxVisible(void) const;
	
	// Setters

	// Changes the navigation mode. Valid arguments are SELECTOR and MOUSE.
	void setNavigationMode(const int mode);

	// Sets the GUI's internal mouse coordinates. These should be retrieved from
	// SDL_Event in an update loop.
	void setMousePos(const int x, const int y);

	// Sets the GUI's internal left mouse down boolean.
	void setLeftMouseDown(const bool down);

	// Sets the GUI's internal right mouse down boolean.
	void setRightMouseDown(const bool down);

	// Sets the GUI's internal selectedPressed boolean. Useful for preventing
	// the user from navigating away from a widget while it's selected.
	void setSelectorPressed(const bool pressed);

	// Sets the currently selected Widget's appearance to IDLE and changes the 
	// value of m_selectedWidget to n. Sets newly selected Widget to appearance
	// SELECTED as long as the left mouse button is not pressed on the Widget.
	void setSelectedWidget(const int n);

	// Sets the value of the cursor for text editing.
	void setEditingText(const int n);

	// Update the GUI and current GUILayer with delta time.
	virtual void update(double dt);

	// Enumerate message box layer as index zero.
	enum{
		MESSAGEBOX = 0,
		YESNOBOX
	};

private:
	// List of all GUILayer's used by this GUI.
	GUILayerList m_layers;

	// Active stack of layers.
	std::stack<int> m_layerStack;

	// The layer directly underneath the message box.
	int m_nextLayer;

	// Index of currently selected Widget from the current GUILayer.
	int m_selectedWidget;

	// Index of widget with the "cursor" for editing text (usually textboxes).
	int m_cursor;

	// Stores navigation mode (e.g., SELECTOR or MOUSE).
	int m_navMode;

	// The mouse's relative X and Y coordinates.
	int m_mouseX, m_mouseY;

	// Stores true if left/right mouse button is pressed.
	bool m_leftMouseDown, m_rightMouseDown;

	// Stores true if RETURN on keyboard, A/Start on gamepad is pressed.
	bool m_selectorPressed;
};

// ================================================ //

class GUILayerMessageBox : public GUILayer
{
public:
	explicit GUILayerMessageBox(void);

	enum{
		BUTTON_OK = 0,
		LISTBOX_TEXT,
	};
};

// ================================================ //

class GUILayerYesNoBox : public GUILayer
{
public:
	explicit GUILayerYesNoBox(void);

	enum{
		BUTTON_YES = 0,
		BUTTON_NO,
		LISTBOX_TEXT
	};
};

// ================================================ //

inline void GUI::addLayer(std::shared_ptr<GUILayer> layer){
	m_layers.push_back(layer);
}

// Getters

inline const int GUI::getCurrentLayer(void) const{
	return m_layerStack.top();
}

inline GUILayer* GUI::getCurrentLayerPtr(void) const{
	return m_layers[m_layerStack.top()].get();
}

inline const int GUI::getNavigationMode(void) const{
	return m_navMode;
}

inline const int GUI::getSelectedWidget(void) const{
	return m_selectedWidget;
}

inline const int GUI::getCursor(void) const{
	return m_cursor;
}

inline const bool GUI::getSelectorPressed(void) const{
	return m_selectorPressed;
}

inline Widget* GUI::getWidgetPtr(const int n) const{
	return m_layers[m_layerStack.top()]->getWidgetPtr(n);
}

inline Widget* GUI::getSelectedWidgetPtr(void) const{
	return m_layers[m_layerStack.top()]->getWidgetPtr(m_selectedWidget);
}

inline const bool GUI::isEditingText(void) const{
	return (m_cursor != -1);
}

inline const bool GUI::isMessageBoxVisible(void) const{
	return (m_layerStack.top() == GUI::MESSAGEBOX);
}

inline const bool GUI::isYesNoBoxVisible(void) const{
	return (m_layerStack.top() == GUI::YESNOBOX);
}

// Setters

inline void GUI::setNavigationMode(const int mode){
	m_navMode = mode;
}

inline void GUI::setMousePos(const int x, const int y){
	m_mouseX = x; m_mouseY = y;
}

inline void GUI::setLeftMouseDown(const bool down){
	m_leftMouseDown = down;
}

inline void GUI::setRightMouseDown(const bool down){
	m_rightMouseDown = down;
}

inline void GUI::setSelectorPressed(const bool pressed){
	m_selectorPressed = pressed;
}

// ================================================ //
// ================================================ //

// Holds all textures and data for the GUI theme in use.
struct GUITheme : public Singleton<GUITheme>
{
	// Initializes all textures to nullptr.
	explicit GUITheme(void);

	// Empty destructor.
	~GUITheme(void);

	// Parses .theme file and allocates textures.
	void load(const std::string& file);

	// SDL Texture pointers for theme.

	std::shared_ptr<SDL_Texture> ButtonTexture[3];
	std::shared_ptr<SDL_Texture> TextboxTexture[3];
	std::shared_ptr<SDL_Texture> TextboxCursor;
	std::shared_ptr<SDL_Texture> ListboxTexture, ListboxBorder;
	std::shared_ptr<SDL_Texture> HealthbarTexture;
};

// ================================================ //

#endif

// ================================================ //