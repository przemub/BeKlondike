/*
 * Copyright 2013 Przemysław Buczkowski <przemub@przemub.pl>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _KLONDIKEAPPLICATION_H_
#define _KLONDIKEAPPLICATION_H_


#include <Application.h>


class KlondikeWindow;


class KlondikeApplication : public BApplication {
public:
	KlondikeApplication();

	virtual void AboutRequested();

private:
	KlondikeWindow* fWindow;
};


#endif	// _KLONDIKEAPPLICATION_H_
