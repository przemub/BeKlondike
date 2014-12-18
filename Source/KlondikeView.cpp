/*
 * Copyright 2013-14 Przemysław Buczkowski <przemub@przemub.pl>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "KlondikeView.h"

#include <Alert.h>
#include <Application.h>
#include <Catalog.h>
#include <Entry.h>
#include <Path.h>
#include <Roster.h>
#include <String.h>
#include <TranslationUtils.h>

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "BeKlondike"


KlondikeView::KlondikeView()
	:
	BView(BRect(0, 0, STARTING_WINDOW_WIDTH+10, STARTING_WINDOW_HEIGHT+10), "KlondikeView",
		B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_PULSE_NEEDED | B_FULL_UPDATE_ON_RESIZE)
{
	SetViewColor(0, 85, 0);

	windowWidth = STARTING_WINDOW_WIDTH;
	windowHeight = STARTING_WINDOW_HEIGHT;

	_LoadBitmaps();
	
	for(short i = 0; i < CARDS_IN_PLAY; i++)
		fAllCards[i] = NULL;
}


KlondikeView::~KlondikeView()
{
	delete fShuffle;
	delete fFanfare;
	delete fEmpty;
	for(short i = 0; i < CACHED_BACKS; i++)
		delete fBack[i];
	for(short i = 0; i < CARDS_IN_DECK; i++)
		delete fCards[i];
	for(short i = 0; i < CARDS_IN_PLAY; i++)
		delete fAllCards[i];
}


void KlondikeView::AllAttached()
{
	_GenerateBoard();
}


void KlondikeView::Draw(BRect rect)
{
	//clock_t start, end;
	//start = clock();
	SetDrawingMode(B_OP_ALPHA);
	
	int hSpacing = _CardHSpacing();
	
	DrawBitmap(fBack[0], BRect(hSpacing, 15, hSpacing + CARD_WIDTH, 15 + CARD_HEIGHT));
	
	// stock and waste
	if (fIsWasteCardPicked) {
		int lastWasteCard = fWasteCard - 1;
		
		if (lastWasteCard != -1)		
			while (fStock[lastWasteCard]->fRevealed) {
				lastWasteCard--;
				if (lastWasteCard == -1) {
					break;
				}
			}
		
		if (lastWasteCard != -1)
			DrawBitmap(
				fCards[fStock[lastWasteCard]->fColor*
				CARDS_IN_SUIT+fStock[lastWasteCard]->fValue],
				BRect(2*hSpacing + CARD_WIDTH, 15,
				2*hSpacing + 2*CARD_WIDTH, 15 + CARD_HEIGHT));
		else
			DrawBitmap(fEmpty, BRect(2*hSpacing + CARD_WIDTH, 15,
				2*hSpacing + 2*CARD_WIDTH, 15 + CARD_HEIGHT));
	}
	else if (fWasteCard != -1) {
		while (fStock[fWasteCard]->fRevealed) {
			fWasteCard++;
			if (fWasteCard == 25) {
				fWasteCard = -1;
				break;
			}
		}
		
		rect = BRect(2*hSpacing + CARD_WIDTH, 15,
			2*hSpacing + 2*CARD_WIDTH, 15 + CARD_HEIGHT);
		
		if (fWasteCard != -1)
			DrawBitmap(
				fCards[fStock[fWasteCard]->fColor*
				CARDS_IN_SUIT+fStock[fWasteCard]->fValue],
				rect);
		else
			DrawBitmap(fEmpty, rect);
			
	}
	else
		DrawBitmap(fEmpty, BRect(2*hSpacing + CARD_WIDTH, 15,
			2*hSpacing + 2*CARD_WIDTH, 15 + CARD_HEIGHT));
	
	// foundations
	for(short i = 0; i < 4; i++) {
		BRect rect = BRect((i+4)*hSpacing + (i+3)*CARD_WIDTH, 15,
			(i+4)*hSpacing + (i+4)*CARD_WIDTH, 15 + CARD_HEIGHT);
		
		if (fFoundations[i]	== -1) {
			DrawBitmap(fEmpty, rect);
		} else {
			DrawBitmap(fCards[fFoundationsColors[i]*CARDS_IN_SUIT+fFoundations[i]], rect);
		}
		fprintf(stderr, "%d/%d ", fFoundations[i], fFoundationsColors[i]);
	}
	fprintf(stderr, "\n");
	
	for(short i = 0; i < 7; i++) {
		BRect rect(hSpacing + i * (CARD_WIDTH + hSpacing), 146,
				hSpacing + (i + 1) * CARD_WIDTH + i * hSpacing,
				146 + CARD_HEIGHT);
		if(fBoard[i] == NULL)
			DrawBitmap(fEmpty, rect);
		else {
			card* currentCard;
			for(currentCard = fBoard[i]; currentCard != NULL;
					currentCard = currentCard->fNextCard) {
				if(currentCard->fRevealed == false) {
					short numberOfBacks = 0; // 1 back
					while(currentCard->fNextCard != NULL &&
							currentCard->fNextCard->fRevealed == false &&
							numberOfBacks < CACHED_BACKS) {
						currentCard = currentCard->fNextCard;
						numberOfBacks++;
					}
					rect.bottom += numberOfBacks*15;
					DrawBitmap(fBack[numberOfBacks], rect);
					rect.top += 15*numberOfBacks;
				} else if(currentCard->fEffect != E_HIDDEN) {
					DrawBitmap(
						fCards[currentCard->fColor*CARDS_IN_SUIT+currentCard->fValue],
						rect);
					switch(currentCard->fEffect) {
					case E_ALPHA25:
						SetHighColor(0, 85, 0, 63);
						break;
					case E_ALPHA50:
						SetHighColor(0, 85, 0, 127);
						break;
					case E_ALPHA75:
						SetHighColor(0, 85, 0, 190);
						break;
					case E_GREEN:
						SetHighColor(0, 204, 0, 127);
						break;
					case E_RED:
						SetHighColor(255, 0, 0, 127);
						break;
					default:
						SetHighColor(0, 85, 0, 0);
					}
					FillRect(rect);
				}
				rect.top += 15;
				rect.bottom = rect.top + CARD_HEIGHT;
			}
		}
	}

	BString points = BString();
	points << fPoints;
	BString moves = BString();
	moves << fMoves;

	BFont bigFont = BFont();
	bigFont.SetFace(B_BOLD_FACE);
	bigFont.SetSize(18);

	BFont smallFont = BFont();
	smallFont.SetSize(12);

	SetHighColor(255,255,255);

	SetFont(&bigFont);
	DrawString(points, BPoint((windowWidth+10 - bigFont.StringWidth(points)) / 2, windowHeight-50));

	SetFont(&smallFont);
	DrawString(B_TRANSLATE("points"), BPoint((windowWidth+10
		- smallFont.StringWidth(B_TRANSLATE("points"))) / 2, windowHeight-35));

	SetFont(&bigFont);
	DrawString(moves, BPoint((windowWidth+10 - bigFont.StringWidth(points)) / 2, windowHeight-15));

	SetFont(&smallFont);
	DrawString(B_TRANSLATE("moves"), BPoint((windowWidth+10
		- smallFont.StringWidth(B_TRANSLATE("moves"))) / 2, windowHeight));

	if (fNoMoves > -1) {
		SetFont(&bigFont);
		SetHighColor(255, 0, 0);

		DrawString(B_TRANSLATE("there are no constructive moves!"),
			BPoint((windowWidth+10 - bigFont.StringWidth(
			B_TRANSLATE("there are no constructive moves!"))) / 2, windowHeight-75));
	}
	
	//end = clock();
	//clock_t diff = end - start;
	//printf("Time: %.6f\n", diff / (double)CLOCKS_PER_SEC);
}


void KlondikeView::Pulse()
{
	/*if (fDealing > 9) {
		fDealing = -1;
		Window()->SetPulseRate(500000);
		_CheckBoard();
	}

	if (fDealing != -1) { // dealing a new row
		card* lastCard = _FindLastUsed(fDealing);
		
		switch (lastCard->fEffect) {
		case E_ALPHA75:
			lastCard->fEffect = E_ALPHA50;
			break;
		case E_ALPHA50:
			lastCard->fEffect = E_ALPHA25;
			// start next animation
			if(fDealing < 9)
				_FindLastUsed(fDealing+1)->fEffect = E_ALPHA75;
			break;
		case E_ALPHA25:
			lastCard->fEffect = E_NONE;
			fDealing++; // move to next card
			break;
		}

		Invalidate();
	} else if (fStacking != -1) {
		card* lastCard = _FindLastUsed(fStacking);
		
		switch (lastCard->fEffect) {
		case E_ALPHA25:
			lastCard->fEffect = E_ALPHA50;
			break;
		case E_ALPHA50:
			lastCard->fEffect = E_ALPHA75;
			break;
		case E_ALPHA75:
			// start next animation (if next card not stacking,
			//   it will be reset later)
			if(lastCard->fPrevCard != NULL)
				lastCard->fPrevCard->fEffect = E_ALPHA25;
			// detach current card
			_RemoveCardFromPile(fStacking, lastCard);
			// move to next card
			fStackingCard++;
			break;
		}

		Invalidate();

		if (fStackingCard == 14) {
			card* lastCard = _FindLastUsed(fStacking);
			if(lastCard != NULL) {
				lastCard->fRevealed = true;
				lastCard->fEffect = E_NONE;
			}
			if(fStacked == 8)
				(new BAlert("WinAlert", B_TRANSLATE("YOU WON!"),
					B_TRANSLATE_CONTEXT("OK!", "Win alert")))->Go();

			fStacking = -1;
			Window()->SetPulseRate(500000);
		}
	} else if (fIsHintShown > 0)
		fIsHintShown--;
	else if (fIsHintShown == 0) {
		for (card* currentCard = fHints[0]; currentCard != NULL;
				currentCard = currentCard->fNextCard) {
			if(currentCard->fEffect == E_GREEN)
				currentCard->fEffect = E_NONE;
		}

		fHints[1]->fEffect = E_NONE;

		fIsHintShown = -1;

		Invalidate();
	} else if (fNoMoves > -1) {
		fNoMoves--;
		if(fNoMoves == -1)
			Invalidate();
	}*/
}


void KlondikeView::Resize(float newWidth, float newHeight)
{
	ResizeTo(newWidth, newHeight-20);
	windowWidth = (int)newWidth - 20;
	windowHeight = (int)newHeight - 30;
}

#include <iostream>
void KlondikeView::MouseDown(BPoint point)
{
	
	if (fMouseLock)
		return;
	fMouseLock = true;

	int hSpacing = _CardHSpacing();
	
	short stack = (int)((point.x - hSpacing) / (CARD_WIDTH + hSpacing));
	
	// stock
	if (stack == 0 && point.y < 15 + CARD_HEIGHT) {
		if (++fWasteCard == 24)
			fWasteCard = -1;
		
		Invalidate();
		return;
	}
	
	// pick up a card from waste
	if (stack == 1 && point.y < 15 + CARD_HEIGHT) {
		if (fWasteCard == -1)
			return;
		
		card* picked = fStock[fWasteCard];
		fPickedCard = picked;
		fIsWasteCardPicked = true;
		
		BMessage msg(B_SIMPLE_DATA);
		msg.AddPointer("view", this);
		BBitmap* img = new BBitmap(fCards[picked->fColor*CARDS_IN_SUIT+picked->fValue]);
		
		DragMessage(&msg, img, B_OP_BLEND,
			BPoint((int)(point.x - hSpacing) % (CARD_WIDTH + hSpacing),
			point.y - 15));
		
		Invalidate();
		
		return;
	}

	// pick up a stack
	if (stack <= 9 && fBoard[stack] != NULL) {
		// find clicked on card
		int cardNumber = 1;
		card* picked = fBoard[stack];
		while(picked->fNextCard != NULL) {
			if(point.y - 15 * cardNumber - 131 < 15) {
				break;
			}
			picked = picked->fNextCard;
			cardNumber++;
		}
		if(picked->fNextCard == NULL) {
			// on last card, if below than not clicking on card
			if(point.y - 15 * cardNumber - 131 >= CARD_HEIGHT) {
				return;
			}
		}
		
		if(picked->fRevealed == false)
			return;
		
		card* currentCard = picked->fNextCard;
		card* lastCard = picked;
		short pickedHeight = 1;
		for(short i = 1; currentCard != NULL;
				i++) {
			pickedHeight++;
			if(lastCard->fIsColorRed == currentCard->fIsColorRed)
				return;
			lastCard = currentCard;
			currentCard = currentCard->fNextCard;
		}
		
		fPickedCardBoardPos = stack;
		fPickedCard = picked;
		fIsCardPicked = true;

		_RemoveCardFromPile(stack, picked);

		BMessage msg(B_SIMPLE_DATA);
		msg.AddPointer("view", this);
		BBitmap* img;
		if(pickedHeight == 1)
			img = new BBitmap(fCards[picked->fColor*CARDS_IN_SUIT+picked->fValue]);
		else {
			img = new BBitmap(BRect(0, 0, CARD_WIDTH-1, CARD_HEIGHT + (pickedHeight-1) * 15), fBack[0]->ColorSpace(), true);
			BView* imgView = new BView(img->Bounds(), NULL, 0, 0);
			BRect destRect = fBack[0]->Bounds();
			img->AddChild(imgView);
			img->Lock();
			currentCard = picked;
			
			imgView->SetDrawingMode(B_OP_COPY);
			imgView->DrawBitmap(fCards
				[currentCard->fColor*CARDS_IN_SUIT+currentCard->fValue],
				destRect);
			destRect.top = (pickedHeight-1)*15;
			destRect.bottom = destRect.top + CARD_HEIGHT;
			// we don't know the top card yet, so we'll overwrite this
			imgView->DrawBitmap(fBack[0], destRect);
			
			imgView->SetDrawingMode(B_OP_ALPHA);
			for(short j = 0; j < pickedHeight; j++) {
				destRect.top = j*15;
				destRect.bottom = destRect.top + CARD_HEIGHT;
				imgView->DrawBitmap(fCards[currentCard->fColor*CARDS_IN_SUIT+currentCard->fValue], destRect);
				currentCard = currentCard->fNextCard;
			}
			
			imgView->Sync();
			img->Unlock();
			img->RemoveChild(imgView);
			delete imgView;
		}
		DragMessage(&msg, img, B_OP_BLEND,
			BPoint((int)(point.x - hSpacing) % (CARD_WIDTH + hSpacing),
			point.y - cardNumber * 15 - 131));
		
		Invalidate();
	}
}


void KlondikeView::MouseMoved(BPoint point,
	uint32 transit, const BMessage* message)
{
	/*if (transit == B_EXITED_VIEW) {
		MouseUp(point);
		return;
	}*/
}


void KlondikeView::MouseUp(BPoint point)
{
	if (fIsCardPicked) {
		int hSpacing = _CardHSpacing();
		short stack = (int)((point.x - hSpacing) / (CARD_WIDTH + hSpacing));

		if (stack >= 3 && stack < 7 && point.y < 15 + CARD_HEIGHT) {
			short foundation = stack-3;
			
			if (fPickedCard->fValue == 0)
				fFoundationsColors[foundation] = fPickedCard->fColor;
			
			if (fFoundationsColors[foundation] == fPickedCard->fColor &&
				fPickedCard->fValue - fFoundations[foundation] == 1) {
				fFoundations[foundation]++;
				
				if (_FindLastUsed(fPickedCardBoardPos) != NULL)
						_FindLastUsed(fPickedCardBoardPos)->fRevealed = true;
			} else
				_AddCardToPile(fPickedCardBoardPos, fPickedCard);
		}
		

		else if (stack >= 0 && stack < 7 && (_FindLastUsed(stack) == NULL ||
				_FindLastUsed(stack)->fValue - fPickedCard->fValue == 1) &&
				fIsCardPicked) {
			// attach to stack
			_AddCardToPile(stack, fPickedCard);
			
			// reveal last card from pile the cards were from
			if(_FindLastUsed(fPickedCardBoardPos) != NULL)
				_FindLastUsed(fPickedCardBoardPos)->fRevealed = true;
			
			fPoints--;
			fMoves++;
		} else {
			// reattach to old stack
			_AddCardToPile(fPickedCardBoardPos, fPickedCard);
		}

		fIsCardPicked = false;

		_CheckBoard();
		Invalidate();
		
		return;
	}
	
	if (fIsWasteCardPicked) {
		int hSpacing = _CardHSpacing();
		short stack = (int)((point.x - hSpacing) / (CARD_WIDTH + hSpacing));
		
		if (stack >= 3 && stack < 7 && point.y < 15 + CARD_HEIGHT) {
			short foundation = stack-3;
			
			if (fPickedCard->fValue == 0)
				fFoundationsColors[foundation] = fPickedCard->fColor;
			
			if (fFoundationsColors[foundation] == fPickedCard->fColor &&
				fPickedCard->fValue - fFoundations[foundation] == 1) {
				fFoundations[foundation]++;
				fPickedCard->fRevealed = true;
				fWasteCard--;
			}
		}
		
		else if (stack >= 0 && stack < 7 && (_FindLastUsed(stack) == NULL ||
				_FindLastUsed(stack)->fValue - fPickedCard->fValue == 1)) {
			// attach to stack
			_AddCardToPile(stack, fPickedCard);
			
			fPickedCard->fRevealed = true;
			fWasteCard--;
		} else {
			// reattach to old stack
			//_AddCardToPile(fPickedCardBoardPos, fPickedCard);
		}

		fIsWasteCardPicked = false;

		_CheckBoard();
		Invalidate();
	}

	fMouseLock = false;
}


void KlondikeView::NewGame()
{
	_GenerateBoard();
	Invalidate();
}


int KlondikeView::_CardHSpacing()
{
	return((windowWidth - (CARD_WIDTH*7)) / 8);
}



void KlondikeView::Hint()
{
	if (fIsHintShown != -1 /*|| Dealing != -1*/)
		return;

	card* highestCard[10];
	short stocksValues[10];

	for (short i = 0; i != 7; i++) {
		highestCard[i] = _FindLastUsed(i);
		if(highestCard[i] == NULL) {
			stocksValues[i] = -1;
			continue;
		}
		stocksValues[i] = highestCard[i]->fValue;

		for (card* currentCard = highestCard[i]->fPrevCard;
				currentCard != NULL; currentCard = currentCard->fPrevCard) {
			if (currentCard->fRevealed == false || 
					currentCard->fValue - stocksValues[i] != 1)
				break;

			highestCard[i] = currentCard;
			stocksValues[i] = currentCard->fValue;
		}
	}

	
	short status = 0;
	short x = fHintStatus[0];
	short y = fHintStatus[1]+1;
	
	for(;; x = (x+1) % 10) {
		for(; y < 10; y++) {
			if(_FindLastUsed(y) == NULL ||
					_FindLastUsed(y)->fValue - stocksValues[x] == 1) {
				status = 1; // found a match
				break;
			}
			if(x == fHintStatus[0] && y == fHintStatus[1]) {
				status = 2; // didn't find a match
				break;
			}
		}
		if(status != 0) break;
		y = 0;
	}
	
	fHintStatus[0] = x;
	fHintStatus[1] = y;
	
	if(status == 1) {
		fIsHintShown = 2;
		fHints[0] = highestCard[x];
		fHints[1] = _FindLastUsed(y);

		for (card* currentCard = fHints[0];
				currentCard != NULL; currentCard = currentCard->fNextCard)
			currentCard->fEffect = E_GREEN;

		fHints[1]->fEffect = E_RED;
	} else {
		fNoMoves = 4;
	}

	Invalidate();
}


BSimpleGameSound* KlondikeView::_LoadSound(const char* resourceName)
{
	size_t size;
	const void* data = fResources->LoadResource('rSFX', resourceName, &size);
	if(data == NULL) {
		printf("Error loading sound resource: %s\n", resourceName);
		return NULL;
	}
	
	gs_audio_format format;
	format.frame_rate = 44100;
	format.channel_count = 2; // stereo
	format.format = 0x02;
	format.byte_order = 0;
	format.buffer_size = 0; // auto
	
	BSimpleGameSound* sound = new BSimpleGameSound(data, size, &format);
	
	if(sound->InitCheck() != B_OK) {
		printf("Error loading sound resource: %s\n", resourceName);
	}
	return sound;
}


void KlondikeView::_LoadBitmaps()
{
	BString suits[] = {
		"spade",
		"heart",
		"club",
		"diamond"
	};
	// load images
	BString filename;
	for (short i = 0; i < CARDS_IN_SUIT; i++) {
		for(short j = 0; j < 4; j++) {
			filename = "";
			filename << "Artwork/" << i + 1 << "_" << suits[j] << ".png";
			fCards[j*CARDS_IN_SUIT+i] = BTranslationUtils::GetBitmap('rGFX', filename);
		}
	}
	fBack[0] = BTranslationUtils::GetBitmap('rGFX', "Artwork/back.png");
	fEmpty = BTranslationUtils::GetBitmap('rGFX', "Artwork/empty.png");

	// load audio
	fResources = be_app->AppResources();
	fShuffle = _LoadSound("Artwork/shuffle.wav");
	fFanfare = _LoadSound("Artwork/fanfare.wav");
	
	// cache multiple backs in a row
	for(short i = 1; i < CACHED_BACKS; i++) {
		fBack[i] = new BBitmap(BRect(0, 0, CARD_WIDTH-1, CARD_HEIGHT + i*15),
			fBack[0]->ColorSpace(), true);
		
		BView* fBackView = new BView(fBack[i]->Bounds(), NULL, 0, 0);
		BRect destRect = fBack[0]->Bounds();
		fBack[i]->AddChild(fBackView);
		fBack[i]->Lock();
		
		fBackView->SetDrawingMode(B_OP_COPY);
		fBackView->DrawBitmap(fBack[0], destRect);
		destRect.top = i*15;
		destRect.bottom = destRect.top + CARD_HEIGHT;
		fBackView->DrawBitmap(fBack[0], destRect);
		
		fBackView->SetDrawingMode(B_OP_ALPHA);
		for(short j = 0; j < i+1; j++) {
			destRect.top = j*15;
			destRect.bottom = destRect.top + CARD_HEIGHT;
			fBackView->DrawBitmap(fBack[0], destRect);
		}
		fBackView->Sync();
		fBack[i]->Unlock();
	}

	Invalidate();
}


card* KlondikeView::_PickRandomCard()
{
	for(short i = 0; i < CARDS_IN_PLAY; i++) {
		if(fAllCards[i]->fInPlay == false) { // if card not in play
			fAllCards[i]->fInPlay = true; // move card into play
			return fAllCards[i]; // return that card
		}
	}
	printf("Error: ran out of cards to deal!\n");
	return NULL; // no card found
}


void KlondikeView::_AddCardToPile(int pile, card* cardToAdd) {
	if(fBoard[pile] == NULL) {
		fBoard[pile] = cardToAdd;
	} else {
		cardToAdd->fPrevCard = _FindLastUsed(pile);
		cardToAdd->fPrevCard->fNextCard = cardToAdd;
	}
}


void KlondikeView::_RemoveCardFromPile(int pile, card* cardToRemove) {
	if(fBoard[pile] == cardToRemove) { // first in pile
		fBoard[pile] = NULL;
	} else { // second or later in pile
		cardToRemove->fPrevCard->fNextCard = NULL;
		cardToRemove->fPrevCard = NULL;
	}
}


void KlondikeView::_GenerateBoard()
{
	srand(time(NULL));
	
	// create cards
	card* orderedCards[CARDS_IN_PLAY];
	for (short i = 0; i < CARDS_IN_PLAY; i++) {
		orderedCards[i] = new card();
		orderedCards[i]->fValue = i % CARDS_IN_SUIT; // A->K, repeat
		orderedCards[i]->fColor = (i/CARDS_IN_SUIT) % 4;
		
		if (orderedCards[i]->fColor == 1 ||
			orderedCards[i]->fColor == 3)
			orderedCards[i]->fIsColorRed = true;
		else
			orderedCards[i]->fIsColorRed = false;
			
		orderedCards[i]->fRevealed = false;
		orderedCards[i]->fEffect = E_NONE;
		orderedCards[i]->fInPlay = false;
		orderedCards[i]->fNextCard = NULL;
		orderedCards[i]->fPrevCard = NULL;
	}
	
	// randomize order of card array
	for(short cardsLeft = CARDS_IN_PLAY; cardsLeft > 0; cardsLeft--) {
		// random number between 0 and (52, 51, 50, ...)
		short randomCard = rand() % cardsLeft;
		
		// move card to actual deck
		if(fAllCards[cardsLeft-1] != NULL) delete fAllCards[cardsLeft-1];
		fAllCards[cardsLeft-1] = orderedCards[randomCard];
		
		// replace picked card with card at back of deck
		orderedCards[randomCard] = orderedCards[cardsLeft-1];
	}

	fWasteCard = -1;
	fStacking = -1;
	fStackingCard = -1;
	fStacked = 0;
	fIsCardPicked = false;
	fIsWasteCardPicked = false;
	fIsHintShown = -1;
	fNoMoves = -1;
	fMouseLock = false;
	fHintStatus[0] = 0;
	fHintStatus[1] = 0;
	
	for (short i = 0; i < 4; i++) {
		fFoundations[i] = -1;
		fFoundationsColors[i] = -1;
	}

	fPoints = 200;
	fMoves = 0;

	short cardsOnPile = 1;
	for (short i = 0; i != 7; i++) {
		fBoard[i] = NULL; // clear
		short j = cardsOnPile;
		for(; j > 0; j--) { // for each card
			// pick a random next card, and add it
			_AddCardToPile(i, _PickRandomCard());
		}
		cardsOnPile++;
		// at the last card, show it
		card* lastCard = _FindLastUsed(i);
		lastCard->fRevealed = true;
		//lastCard->fEffect = E_HIDDEN;
		/*if(i == 0) { // if the first pile
			// start showing animation
			lastCard->fEffect = E_ALPHA75;
			fDealing = 0;
			Window()->SetPulseRate(50000);
		}*/
	}
	
	for (short i = 0; i != 24; i++) {
		fStock[i] = _PickRandomCard();
	}
	
	

	fShuffle->StartPlaying();
}


void KlondikeView::_CheckBoard()
{
	for (short i = 0; i != 10; i++) {
		short needed = 0;
		bool stacked = true;
		card* currentCard = _FindLastUsed(i);
		if(currentCard == NULL)
			continue;
		short color = currentCard->fColor;

		while(stacked) {
			if(currentCard != NULL
				&& currentCard->fValue == needed
				&& currentCard->fColor == color) {
				needed++;
				currentCard = currentCard->fPrevCard;
			} else
				stacked = false;
		}

		if (needed == CARDS_IN_SUIT)
			stacked = true;

		if (stacked) {
			card* first = _FindLastUsed(i);
			fStackedColor[fStacked] = first->fColor;
			fStacking = i;
			fStackingCard = 1;
			fStacked++;

			first->fEffect = E_ALPHA25;
			Window()->SetPulseRate(50000);

			fPoints += 100;

			Invalidate();

			if (fStacked == 8) {
				fFanfare->StartPlaying();
			} else
				fShuffle->StartPlaying();
		}
	}
}


card* KlondikeView::_FindLastUsed(short stock) {
	card* currentCard = fBoard[stock];
	if(currentCard == NULL)
		return NULL;
	while(currentCard->fNextCard != NULL)
		currentCard = currentCard->fNextCard;

	return currentCard;
}
