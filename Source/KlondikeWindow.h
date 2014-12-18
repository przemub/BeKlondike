/*
 * Copyright 2013 Przemysław Buczkowski <przemub@przemub.pl>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _KLONDIKEWINDOW_H_
#define _KLONDIKEWINDOW_H__

#include "KlondikeView.h"

#include <Invoker.h>
#include <Window.h>


const int32 kNewGameMessage = 'NewG';
const int32 kDifficultyMessage = 'Diff';
const int32 kDiffChosenMessage = 'DiCh';
const int32 kHintMessage = 'Hint';


class KlondikeWindow : public BWindow {
public:
	KlondikeWindow(BRect frame, const char* title);

	virtual void MessageReceived(BMessage* message);
	virtual void FrameResized(float newWidth, float newHeight);

private:
	BMenuBar* _CreateMenuBar();

	KlondikeView* fView;
	BInvoker* fDiffSet;
};

#endif	// _KLONDIKEWINDOW_H_