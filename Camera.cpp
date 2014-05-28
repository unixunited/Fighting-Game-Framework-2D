// ================================================ //

#include "Camera.hpp"

// ================================================ //

template<> Camera* Singleton<Camera>::msSingleton = nullptr;

// ================================================ //

Camera::Camera(void)
	:	moveX(0),
		moveY(0),
		m_locked(false)
{
	
}

// ================================================ //

Camera::~Camera(void)
{

}

// ================================================ //

void Camera::clear(void)
{
	moveX = moveY = 0;
}

// ================================================ //