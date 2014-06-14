// ================================================ //

#include "Input.hpp"
#include "Config.hpp"

// ================================================ //

Input::Input(const std::string& bmap)
{
	// Set all button states to false
	std::fill_n(m_buttons, static_cast<int>(Input::NUM_BUTTONS), false);
	std::fill_n(m_keyboardMap, static_cast<int>(Input::NUM_BUTTONS), 0);
	std::fill_n(m_gamepadMap, static_cast<int>(Input::NUM_BUTTONS), 0);

	// Load button map
	this->loadButtonMap(bmap);
}

// ================================================ //

Input::~Input(void)
{

}

// ================================================ //

void Input::loadButtonMap(const std::string& file)
{
	Log::getSingletonPtr()->logMessage("Loading button map from \"" + file + "\"");

	// Parse the bmap file
	Config c(file);
	if (c.isLoaded()){
		// Keyboard
		m_keyboardMap[BUTTON_UP] = c.parseIntValue("keyboard", "up");
		m_keyboardMap[BUTTON_DOWN] = c.parseIntValue("keyboard", "down");
		m_keyboardMap[BUTTON_LEFT] = c.parseIntValue("keyboard", "left");
		m_keyboardMap[BUTTON_RIGHT] = c.parseIntValue("keyboard", "right");

		m_gamepadMap[BUTTON_UP] = c.parseIntValue("gamepad", "up");
		m_gamepadMap[BUTTON_DOWN] = c.parseIntValue("gamepad", "down");
		m_gamepadMap[BUTTON_LEFT] = c.parseIntValue("gamepad", "left");
		m_gamepadMap[BUTTON_RIGHT] = c.parseIntValue("gamepad", "right");
		m_gamepadMap[BUTTON_START] = c.parseIntValue("gamepad", "start");
		m_gamepadMap[BUTTON_SELECT] = c.parseIntValue("gamepad", "select");

		Log::getSingletonPtr()->logMessage("Button map loaded!");
	}
	else{
		Log::getSingletonPtr()->logMessage("ERROR: Failed to open button map file!");
	}
}

// ================================================ //

void Input::resetAllButtons(void)
{
	std::fill_n(m_buttons, static_cast<int>(Input::NUM_BUTTONS), false);
}

// ================================================ //