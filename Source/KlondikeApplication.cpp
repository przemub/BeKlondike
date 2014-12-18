/*
 * Copyright 2013 Przemysław Buczkowski <przemub@przemub.pl>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "KlondikeApplication.h"
#include "KlondikeView.h"
#include "KlondikeWindow.h"

#include <Alert.h>
#include <Catalog.h>
#include <String.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "BeKlondike"


KlondikeApplication::KlondikeApplication()
	:
	BApplication("application/x-vnd.przemub.BeKlondike")
{
	fWindow = new KlondikeWindow(BRect(150, 150, 150+STARTING_WINDOW_WIDTH+20, 150+STARTING_WINDOW_HEIGHT+30), "BeKlondike");

	fWindow->Lock();
	fWindow->Show();
	fWindow->Unlock();
}


void KlondikeApplication::AboutRequested()
{
	BString aboutText = "";
	aboutText << "BeKlondike v0.1" << "\n"
	<< B_TRANSLATE("A klondike solitaire clone for Haiku.")<<"\n\n"
	<< B_TRANSLATE("BeKlondike (c) 2013-14 Przemysław Buczkowski <przemub@przemub.pl>")<<"\n"
	<< B_TRANSLATE("Distributed under the terms of the MIT license.")<<"\n\n"
	<< B_TRANSLATE("SVG-Z-cards (c) 2008 David Bellot, Brandon Ardiente.")<<"\n"
	<< B_TRANSLATE("Licensed under GNU Lesser General Public License 2.1.")<<"\n\n"
	<< B_TRANSLATE("Fanfare sound (c) 2009 Anthony Batiste.")<<"\n"
	<< B_TRANSLATE("Licensed under Creative Commons Attribution 3.0.");
	
	(new BAlert("AboutAlert", aboutText, B_TRANSLATE_CONTEXT("OK", "About alert")))->Go();
}


int main(int argc, char** argv)
{
	KlondikeApplication app;

	app.Run();

	return 0;
}
