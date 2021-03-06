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
// File: GUI.cpp
// Author: Jordan Sparks <unixunited@live.com>
// ================================================ //
// Implements GUI, GUITheme, and GUILayer classes. 
// ================================================ //

#include "GUI.hpp"
#include "Config.hpp"
#include "Engine.hpp"
#include "WidgetStatic.hpp"
#include "WidgetButton.hpp"
#include "WidgetTextbox.hpp"
#include "WidgetListbox.hpp"
#include "WidgetHealthBar.hpp"
#include "Label.hpp"

// ================================================ //

GUILayer::GUILayer(void) :
m_id(0),
m_layerName(),
m_widgets()
{

}

// ================================================ //

GUILayer::~GUILayer(void)
{

}

// ================================================ //

template<typename T>
void GUILayer::parse(Config& c, const int widgetType, const StringList& names)
{
	std::string widgetName("");
	std::string layer = "layer." + m_layerName;

	switch (widgetType){
	default:
	case Widget::Type::STATIC:
		widgetName = "static.";
		break;

	case Widget::Type::BUTTON:
		widgetName = "button.";
		break;

	case Widget::Type::TEXTBOX:
		widgetName = "textbox.";
		break;

	case Widget::Type::LISTBOX:
		widgetName = "listbox.";
		break;

	case Widget::Type::HEALTHBAR:
		widgetName = "healthbar.";
		break;
	}

	// Parse each setting for this Widget.
	for (unsigned int i = 0; i < names.size(); ++i){
		// Allocate widget with ID i (the names list parameter should be in order).
		std::shared_ptr<Widget> pWidget(new T(i));
		std::string value = widgetName + names[i] + ":";

		pWidget->setAppearance(Widget::Appearance::IDLE);
		switch (widgetType){
		default:
			pWidget->setPosition(c.parseRect(layer, value + "pos"));
			pWidget->getLabel()->setFont(c.parseIntValue(layer, value + "font"));
			pWidget->getLabel()->setColor(c.parseColor(layer, value + "labelcolor"), false);
			pWidget->setLabel(c.parseValue(layer, value + "label", true), c.parseIntValue(layer, value + "labeloffset"));		
			break;

		case Widget::Type::LISTBOX:
			pWidget->setPosition(c.parseRect(layer, value + "pos"));
			// A listbox does not use the default label, so save font for later when 
			// generating labels.
			static_cast<WidgetListbox*>(pWidget.get())->setFont(c.parseIntValue(layer, value + "font"));
			break;

		case Widget::Type::HEALTHBAR:
			pWidget->setPosition(c.parseRect(layer, value + "pos"));
			// Skip parsing label values for healthbars.
			break;
		}
		
		pWidget->parseLinks(c.parseValue(layer, value + "links"));
		pWidget->setStyle(c.parseIntValue(layer, value + "style"));
		pWidget->setEnabled(!c.parseIntValue(layer, value + "disabled"));

		this->addWidget(pWidget);
	}
}

// Explicitly instantiate template functions for each Widget type.
template void GUILayer::parse<WidgetStatic>(Config& c, const int widgetType, const StringList& names);
template void GUILayer::parse<WidgetButton>(Config& c, const int widgetType, const StringList& names);
template void GUILayer::parse<WidgetTextbox>(Config& c, const int widgetType, const StringList& names);
template void GUILayer::parse<WidgetListbox>(Config& c, const int widgetType, const StringList& names);
template void GUILayer::parse<WidgetHealthBar>(Config& c, const int widgetType, const StringList& names);

// ================================================ //

void GUILayer::resetAllWidgets(const int cursor)
{
	for (unsigned int i = 0; i < m_widgets.size(); ++i){
		if (i != cursor){
			m_widgets[i]->setAppearance(Widget::Appearance::IDLE);
		}
	}
}

// ================================================ //

void GUILayer::render(void)
{
	for (WidgetList::iterator itr = m_widgets.begin();
		itr != m_widgets.end();
		++itr){
		(*itr)->render();
	}
}

// ================================================ //

void GUILayer::update(double dt)
{
	for (WidgetList::iterator itr = m_widgets.begin();
		itr != m_widgets.end();
		++itr){
		(*itr)->update(dt);
	}
}

// ================================================ //
// ================================================ //

GUI::GUI(void) :
m_layers(),
m_layerStack(),
m_nextLayer(GUI::MESSAGEBOX),
m_selectedWidget(Widget::NONE),
m_cursor(Widget::NONE),
m_navMode(NavMode::MOUSE),
m_mouseX(0),
m_mouseY(0),
m_leftMouseDown(false),
m_rightMouseDown(false),
m_selectorPressed(false)
{
	// Setup message box.
	Config e(Engine::getSingletonPtr()->getSettingsFile());
	Config theme(Engine::getSingletonPtr()->getDataDirectory() + "/" + e.parseValue("GUI", "theme"));
	std::shared_ptr<GUILayer> messageBox(new GUILayerMessageBox());

	std::shared_ptr<WidgetButton> ok(new WidgetButton(GUILayerMessageBox::BUTTON_OK));
	SDL_Rect rc = theme.parseRect("messagebox", "ok");
	ok->setPosition(rc);
	ok->setAppearance(Widget::Appearance::IDLE);
	ok->setLabel("Ok", 25);
	messageBox->addWidget(ok);

	std::shared_ptr<WidgetListbox> text(new WidgetListbox(GUILayerMessageBox::LISTBOX_TEXT));
	text->setAppearance(Widget::Appearance::IDLE);
	rc = theme.parseRect("messagebox", "text");
	text->setPosition(rc);
	text->setEnabled(false);
	text->addString("Message");
	messageBox->addWidget(text);

	this->addLayer(messageBox);

	// Setup yes/no box.
	std::shared_ptr<GUILayer> yesnoBox(new GUILayerYesNoBox());
	std::shared_ptr<WidgetButton> yes(new WidgetButton(GUILayerYesNoBox::BUTTON_YES));
	rc = theme.parseRect("yesnobox", "yes");
	yes->setPosition(rc);
	yes->setAppearance(Widget::Appearance::IDLE);
	yes->setLabel("Yes", 25);
	yesnoBox->addWidget(yes);

	std::shared_ptr<WidgetButton> no(new WidgetButton(GUILayerYesNoBox::BUTTON_NO));
	rc = theme.parseRect("yesnobox", "no");
	no->setPosition(rc);
	no->setAppearance(Widget::Appearance::IDLE);
	no->setLabel("No", 25);
	yesnoBox->addWidget(no);

	text.reset(new WidgetListbox(GUILayerYesNoBox::LISTBOX_TEXT));
	text->setAppearance(Widget::Appearance::IDLE);
	rc = theme.parseRect("yesnobox", "text");
	text->setPosition(rc);
	text->setEnabled(false);
	text->addString("Message");
	yesnoBox->addWidget(text);

	this->addLayer(yesnoBox);
}

// ================================================ //

GUI::~GUI(void)
{

}

// ================================================ //

void GUI::clearSelector(void)
{
	if (m_selectedWidget != Widget::NONE){
		m_layers[m_layerStack.top()]->getWidgetPtr(m_selectedWidget)->setAppearance(Widget::Appearance::IDLE);
	}

	m_selectedWidget = 0;
	m_cursor = Widget::NONE;
}

// ================================================ //

void GUI::pushLayer(const int n)
{
	this->clearSelector();
	
	m_layerStack.push(n);

	// Set the default selected Widget's appearance to SELECTED.
	if (m_navMode == GUI::NavMode::SELECTOR){
		m_layers[m_layerStack.top()]->getWidgetPtr(m_selectedWidget)->setAppearance(Widget::Appearance::SELECTED);
	}
}

// ================================================ //

void GUI::popLayer(void)
{
	// Only pop if there is more than one layer.
	if (m_layerStack.size() > 1){
		this->clearSelector();

		m_layerStack.pop();

		if (m_navMode == GUI::NavMode::SELECTOR){
			m_layers[m_layerStack.top()]->getWidgetPtr(m_selectedWidget)->setAppearance(Widget::Appearance::SELECTED);
		}
	}
}

// ================================================ //

void GUI::handleTextInput(const char* text, const bool backspace)
{
	if (m_cursor != Widget::NONE){
		WidgetTextbox* pWidget = static_cast<WidgetTextbox*>(this->getWidgetPtr(m_cursor));
		pWidget->handleEditing(text, backspace);
	}
}

// ================================================ //

void GUI::showMessageBox(const bool show, const std::string& text){
	if (show == true && m_layerStack.top() != GUI::MESSAGEBOX){
		// Set text if specified.
		if (text != ""){
			m_layers[GUI::MESSAGEBOX]->getWidgetPtr(
				GUILayerMessageBox::LISTBOX_TEXT)->setEntry(0, text);
		}
		// Display the message box.
		m_nextLayer = m_layerStack.top();
		this->pushLayer(GUI::MESSAGEBOX);
	}
	else if (show == false && this->getCurrentLayer() == GUI::MESSAGEBOX){
		this->popLayer();
	}
}

// ================================================ //

void GUI::showYesNoBox(const bool show, const std::string& text){
	if (show == true && m_layerStack.top() != GUI::YESNOBOX){
		if (text != ""){
			m_layers[GUI::YESNOBOX]->getWidgetPtr(
				GUILayerYesNoBox::LISTBOX_TEXT)->setEntry(0, text);
		}

		m_nextLayer = m_layerStack.top();
		this->pushLayer(GUI::YESNOBOX);
	}
	else if (show == false && this->getCurrentLayer() == GUI::YESNOBOX){
		this->popLayer();
	}
}

// ================================================ //

void GUI::setSelectedWidget(const int n)
{
	if (n != Widget::NONE){
		// Reset currently selected widget to IDLE appearance.
		if (m_selectedWidget != Widget::NONE){
			m_layers[m_layerStack.top()]->getWidgetPtr(m_selectedWidget)->setAppearance(Widget::Appearance::IDLE);
		}

		m_selectedWidget = n;

		// If the mouse is pressed on a button, don't reset the appearance back to SELECTED.
		if (m_navMode == NavMode::MOUSE && m_leftMouseDown){
			return;
		}

		// Set newly selected widget's appearance to SELECTED.
		if (m_cursor != n){
			m_layers[m_layerStack.top()]->getWidgetPtr(m_selectedWidget)->setAppearance(Widget::Appearance::SELECTED);
		}
	}
}

// ================================================ //

void GUI::setEditingText(const int n)
{
	static int lastActiveWidget = Widget::NONE;
	m_cursor = n;
	if (m_cursor == Widget::NONE){
		SDL_StopTextInput();
		if (lastActiveWidget != Widget::NONE){
			this->getWidgetPtr(lastActiveWidget)->setActive(false);
			this->getWidgetPtr(lastActiveWidget)->setAppearance(Widget::Appearance::IDLE);
		}
		lastActiveWidget = Widget::NONE;
	}
	else{
		this->getWidgetPtr(n)->setAppearance(0, GUITheme::getSingletonPtr()->TextboxCursor);
		this->getWidgetPtr(n)->setActive(true);
		SDL_StartTextInput();
		lastActiveWidget = n;
	}
}

// ================================================ //

void GUI::update(double dt)
{
	if (m_navMode == NavMode::MOUSE){
		// Create a pixel to represent the mouse position for collision testing.
		SDL_Rect mouse;
		mouse.x = m_mouseX;
		mouse.y = m_mouseY;
		mouse.w = mouse.h = 1;

		m_selectedWidget = Widget::NONE;

		// Determines if a reset check for all Widgets should take place.
		static bool resetAllWidgets = false;

		// See if mouse is inside bounds of each widget.
		for (int i = 0; i < m_layers[m_layerStack.top()]->getNumWidgets(); ++i){
			if (m_layers[m_layerStack.top()]->getWidgetPtr(i)->isEnabled() &&
				SDL_HasIntersection(&mouse, &m_layers[m_layerStack.top()]->getWidgetPtr(i)->getPosition())){
				if (this->getWidgetPtr(i)->getType() == Widget::Type::BUTTON ||
					this->getWidgetPtr(i)->getType() == Widget::Type::TEXTBOX){
					if (!m_leftMouseDown){
						m_layers[m_layerStack.top()]->resetAllWidgets(m_cursor);
					}
					this->setSelectedWidget(i);
				}

				// Allow a reset check.
				resetAllWidgets = true;
				break;
			}
		}

		// Reset all of the current layer's widgets if nothing is selected.
		if (m_selectedWidget == Widget::NONE && resetAllWidgets){
			if (!m_leftMouseDown){
				// Pass the index of the Widget with the active cursor.
				m_layers[m_layerStack.top()]->resetAllWidgets(m_cursor);

				// Prevent resetAllWidgets() from being called when not necessary.
				resetAllWidgets = false;
			}
		}
	}

	// If the message box is being rendered, render the next layer on the stack
	// along with a blended black rectangle.
	if (m_layerStack.top() == GUI::MESSAGEBOX || m_layerStack.top() == GUI::YESNOBOX){
		m_layers[m_nextLayer]->render();
		SDL_Rect rc;
		rc.x = rc.y = 0;
		rc.w = Engine::getSingletonPtr()->getWindowWidth();
		rc.h = Engine::getSingletonPtr()->getWindowHeight();
		SDL_SetRenderDrawBlendMode(Engine::getSingletonPtr()->getRenderer(), SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor(Engine::getSingletonPtr()->getRenderer(), 0, 0, 0, 150);
		SDL_RenderFillRect(Engine::getSingletonPtr()->getRenderer(), &rc);
	}

	// Update and render the current layer.
	m_layers[m_layerStack.top()]->update(dt);
	m_layers[m_layerStack.top()]->render();
}

// ================================================ //

GUILayerMessageBox::GUILayerMessageBox(void)
{
	this->setID(GUI::MESSAGEBOX);
	this->setLayerName("messagebox");
}

// ================================================ //

GUILayerYesNoBox::GUILayerYesNoBox(void)
{
	this->setID(GUI::YESNOBOX);
	this->setLayerName("yesnobox");
}

// ================================================ //
// ================================================ //

template<> GUITheme* Singleton<GUITheme>::msSingleton = nullptr;

// ================================================ //

GUITheme::GUITheme(void) :
ButtonTexture(),
TextboxTexture(),
TextboxCursor(nullptr),
ListboxTexture(nullptr),
ListboxBorder(nullptr),
HealthbarTexture(nullptr)
{
	std::fill_n(ButtonTexture, 3, nullptr);
	std::fill_n(TextboxTexture, 3, nullptr);
}

// ================================================ //

GUITheme::~GUITheme(void)
{

}

// ================================================ //

void GUITheme::load(const std::string& file)
{
	Config theme(file);
	if (theme.isLoaded()){
		ButtonTexture[Widget::Appearance::IDLE].reset(Engine::getSingletonPtr()->loadTexture(
			Engine::getSingletonPtr()->getDataDirectory() + "/" + theme.parseValue("button", "tex")), SDL_DestroyTexture);
		ButtonTexture[Widget::Appearance::SELECTED].reset(Engine::getSingletonPtr()->loadTexture(
			Engine::getSingletonPtr()->getDataDirectory() + "/" + theme.parseValue("button", "tex.selected")), SDL_DestroyTexture);
		ButtonTexture[Widget::Appearance::PRESSED].reset(Engine::getSingletonPtr()->loadTexture(
			Engine::getSingletonPtr()->getDataDirectory() + "/" + theme.parseValue("button", "tex.pressed")), SDL_DestroyTexture);

		TextboxTexture[Widget::Appearance::IDLE].reset(Engine::getSingletonPtr()->loadTexture(
			Engine::getSingletonPtr()->getDataDirectory() + "/" + theme.parseValue("textbox", "tex")), SDL_DestroyTexture);
		TextboxTexture[Widget::Appearance::SELECTED].reset(Engine::getSingletonPtr()->loadTexture(
			Engine::getSingletonPtr()->getDataDirectory() + "/" + theme.parseValue("textbox", "tex.selected")), SDL_DestroyTexture);
		TextboxTexture[Widget::Appearance::PRESSED].reset(Engine::getSingletonPtr()->loadTexture(
			Engine::getSingletonPtr()->getDataDirectory() + "/" + theme.parseValue("textbox", "tex.pressed")), SDL_DestroyTexture);
		TextboxCursor.reset(Engine::getSingletonPtr()->loadTexture(
			Engine::getSingletonPtr()->getDataDirectory() + "/" + theme.parseValue("textbox", "cursor")), SDL_DestroyTexture);

		ListboxTexture.reset(Engine::getSingletonPtr()->loadTexture(
			Engine::getSingletonPtr()->getDataDirectory() + "/" + theme.parseValue("listbox", "tex")), SDL_DestroyTexture);
		ListboxBorder.reset(Engine::getSingletonPtr()->loadTexture(
			Engine::getSingletonPtr()->getDataDirectory() + "/" + theme.parseValue("listbox", "border")), SDL_DestroyTexture);

		HealthbarTexture.reset(Engine::getSingletonPtr()->loadTexture(
			Engine::getSingletonPtr()->getDataDirectory() + "/" + theme.parseValue("healthbar", "tex")), SDL_DestroyTexture);
		Log::getSingletonPtr()->logMessage("Theme loaded successfully from \"" + file + "\"");
	}
	else{
		Log::getSingletonPtr()->logMessage("Failed to open theme file \"" + file + "\"");
	}
}

// ================================================ //

