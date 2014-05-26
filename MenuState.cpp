// ================================================ //

#include "MenuState.hpp"
#include "MenuStateImpl.hpp"
#include "App.hpp"

// ================================================ //

MenuState::MenuState(void)
	:	m_pImpl(nullptr)
{
	// Assign this pointer here, due to warning C4355: 'this' : used in base member initializer list
	// The compiler assumes that "this" is not fully constructed in the initialization list and it can result in undefined behavior
	m_pImpl.reset(new MenuStateImpl(this));
}

// ================================================ //

MenuState::~MenuState(void)
{

}

// ================================================ //

void MenuState::enter(void)
{
	return m_pImpl->enter();
}

// ================================================ //

void MenuState::exit(void)
{
	return m_pImpl->exit();
}

// ================================================ //

bool MenuState::pause(void)
{
	return m_pImpl->pause();
}

// ================================================ //

void MenuState::resume(void)
{
	return m_pImpl->resume();
}

// ================================================ //

void MenuState::handleInput(SDL_Event& e)
{
	return m_pImpl->handleInput(e);
}

// ================================================ //

void MenuState::update(double dt)
{
	return m_pImpl->update(dt);
}

// ================================================ //