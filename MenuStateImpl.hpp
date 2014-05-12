// ================================================ //

#ifndef __MENUSTATEIMPL_HPP__
#define __MENUSTATEIMPL_HPP__

// ================================================ //

#include "stdafx.hpp"
#include "ObjectManager.hpp"

class Stage;

// ================================================ //

class MenuStateImpl
{
public:
	explicit MenuStateImpl(void);
	~MenuStateImpl(void);

	void enter(void);
	void exit(void);
	bool pause(void);
	void resume(void);
	void handleInput(SDL_Event& e);
	void update(double dt);

	const bool shouldQuit(void) const;

private:
	bool	m_bQuit;

	std::tr1::shared_ptr<ObjectManager> m_pObjectManager;
	Stage*	m_pStage;
};

// ================================================ //

inline const bool MenuStateImpl::shouldQuit(void) const
{ return m_bQuit; }

// ================================================ //

#endif

// ================================================ //