// ================================================ //
// Extreme Metal Fighter
// Copyright (C) 2014 Jordan Sparks. All Rights Reserved.
// Unauthorized copying of this file, via any medium is strictly prohibited
// Proprietary and confidential
// Written by Jordan Sparks <unixunited@live.com> June 2014
// ================================================ //
// File: Singleton.hpp
// Author: Jordan Sparks <unixunited@live.com>
// ================================================ //
// Defines and implements Singleton class.
// ================================================ //

#ifndef __SINGLETON_HPP__
#define __SINGLETON_HPP__

// ================================================ //

#include "stdafx.hpp"

// ================================================ //

// Disable warnings for this singleton class.
#pragma warning (disable : 4311)
#pragma warning (disable : 4312)

// ================================================ //

// A basic singleton utility.
// Derive a class from Singleton and allocate it with new ClassName();,
// free it with delete ClassName::getSingletonPtr();.
template <typename T>
class Singleton
{
public:
	// Assign singleton's location in memory.
	explicit Singleton(void)
	{
		assert(!msSingleton);

#if defined ( _MSC_VER ) && _MSC_VER < 1200
		int offset = (int)(T*)1 - (int)(Singleton <T>*)(T*)1;
		msSingleton = (T*)((int)this + offset);
#else
		msSingleton = static_cast<T*>(this);
#endif
	}

	// Set singleton's memory location to null.
	~Singleton(void)
	{
		assert(msSingleton);
		msSingleton = nullptr;
	}

	// Getters

	// Returns the singleton object.
	static T* getSingletonPtr(void)
	{ return msSingleton; }

protected:
	static T* msSingleton;

private:
	/** \brief Explicit private copy constructor. This is a forbidden operation.*/
	Singleton(const Singleton<T> &);

	/** \brief Private operator= . This is a forbidden operation. */
	Singleton& operator=(const Singleton<T> &);
};

// ================================================ //

#endif 

// ================================================ //