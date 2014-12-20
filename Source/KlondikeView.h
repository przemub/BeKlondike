/*
 * Copyright 2013-2014 Przemysław Buczkowski <przemub@przemub.pl>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _KLONDIKEVIEW_H_
#define _KLONDIKEVIEW_H_


#include <Bitmap.h>
#include <Entry.h>
#include <FileGameSound.h>
#include <Point.h>
#include <Rect.h>
#include <Resources.h>
#include <SimpleGameSound.h>
#include <View.h>


#define STARTING_WINDOW_WIDTH 900
#define STARTING_WINDOW_HEIGHT 490
#define CARD_WIDTH 80
#define CARD_HEIGHT 116
#define CARDS_IN_DECK 52
#define CARDS_IN_PLAY CARDS_IN_DECK
#define CARDS_IN_SUIT 13
#define CARD_IMAGE_BACK 52
#define CARD_IMAGE_EMPTY 53
#define CACHED_BACKS 6


enum effect { E_NONE, E_ALPHA25, E_ALPHA50, E_ALPHA75,
	E_HIDDEN, E_GREEN, E_RED };


typedef struct card {
	short fValue;
	short fColor;
	bool fIsColorRed;
	bool fRevealed;
	short fEffect;
	bool fInPlay;
	card* fNextCard;
	card* fPrevCard;
} card;


class KlondikeView : public BView {
public:
								KlondikeView();
								~KlondikeView();

	virtual	void				AllAttached();
	virtual	void				Draw(BRect rect);
	virtual	void				Pulse();
	virtual	void				MouseDown(BPoint point);
	virtual	void				MouseUp(BPoint point);

			void				NewGame();
			void				Cheat();
			void				MoveAllToFoundations();
			void				Resize(float newWidth, float newHeight);

private:
			BSimpleGameSound*	_LoadSound(const char* resourceName);
			void				_LoadBitmaps();
			void				_GenerateBoard();
			void				_CheckBoard();
			bool				_MoveWasteToFoundations();
			int					_CardHSpacing();
			card*				_FindLastUsed(short stock);
			card*				_PickRandomCard();
			void				_AddCardToPile(int pile, card* cardToAdd);
			void				_RemoveCardFromPile(int pile,
									card* cardToRemove);

			BBitmap*			fCards[CARDS_IN_DECK];
			BBitmap*			fBack[CACHED_BACKS];
			BBitmap*			fEmpty;

			BResources*			fResources;
			BSimpleGameSound*	fShuffle;
			BSimpleGameSound*	fFanfare;

			int					windowWidth;
			int					windowHeight;

			card*				fBoard[10];
				// first card in each pile
			card*				fStock[24];
			short				fWasteCard;
			bool				fIsWasteCardPicked;
			short				fFoundations[4];
			short				fFoundationsColors[4];
			bool				fIsFoundationCardPicked;
			card* 				fAllCards[CARDS_IN_PLAY];

			bool				fWon;
			short				fPickedCardBoardPos;
				// pile picked card is from
			card*				fPickedCard;
			bool				fIsCardPicked;
			bool				fMouseLock;

			int					fPoints;
};


#endif // _KLONDIKEVIEW_H_
