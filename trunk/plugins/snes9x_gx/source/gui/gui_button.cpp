/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 *
 * gui_button.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui.h"
/**
 * Constructor for the GuiButton class.
 */

GuiButton::GuiButton(int w, int h)
{
	width = w;
	height = h;
	image = NULL;
	imageOver = NULL;
	imageHold = NULL;
	imageClick = NULL;
	icon = NULL;
	iconOver = NULL;
	iconHold = NULL;
	iconClick = NULL;

	for(int i=0; i < 3; i++)
	{
		label[i] = NULL;
		labelOver[i] = NULL;
		labelHold[i] = NULL;
		labelClick[i] = NULL;
	}

	soundOver = NULL;
	soundHold = NULL;
	soundClick = NULL;
	tooltip = NULL;
	selectable = true;
	holdable = false;
	clickable = true;
}

/**
 * Destructor for the GuiButton class.
 */
GuiButton::~GuiButton()
{
}

void GuiButton::SetImage(GuiImage* img)
{
	image = img;
	if(img) img->SetParent(this);
}
void GuiButton::SetImageOver(GuiImage* img)
{
	imageOver = img;
	if(img) img->SetParent(this);
}
void GuiButton::SetImageHold(GuiImage* img)
{
	imageHold = img;
	if(img) img->SetParent(this);
}
void GuiButton::SetImageClick(GuiImage* img)
{
	imageClick = img;
	if(img) img->SetParent(this);
}
void GuiButton::SetIcon(GuiImage* img)
{
	icon = img;
	if(img) img->SetParent(this);
}
void GuiButton::SetIconOver(GuiImage* img)
{
	iconOver = img;
	if(img) img->SetParent(this);
}
void GuiButton::SetIconHold(GuiImage* img)
{
	iconHold = img;
	if(img) img->SetParent(this);
}
void GuiButton::SetIconClick(GuiImage* img)
{
	iconClick = img;
	if(img) img->SetParent(this);
}
void GuiButton::SetLabel(GuiText* txt, int n)
{
	label[n] = txt;
	if(txt) txt->SetParent(this);
}
void GuiButton::SetLabelOver(GuiText* txt, int n)
{
	labelOver[n] = txt;
	if(txt) txt->SetParent(this);
}
void GuiButton::SetLabelHold(GuiText* txt, int n)
{
	labelHold[n] = txt;
	if(txt) txt->SetParent(this);
}
void GuiButton::SetLabelClick(GuiText* txt, int n)
{
	labelClick[n] = txt;
	if(txt) txt->SetParent(this);
}
void GuiButton::SetSoundOver(GuiSound * snd)
{
	soundOver = snd;
}
void GuiButton::SetSoundHold(GuiSound * snd)
{
	soundHold = snd;
}
void GuiButton::SetSoundClick(GuiSound * snd)
{
	soundClick = snd;
}
void GuiButton::SetTooltip(GuiTooltip* t)
{
	tooltip = t;
	if(t)
		tooltip->SetParent(this);
}

/**
 * Draw the button on screen
 */
void GuiButton::Draw()
{
	if(!this->IsVisible())
		return;

	if(state == STATE_SELECTED || state == STATE_HELD)
	{
		if(imageOver)
			imageOver->Draw();
		else if(image) // draw image
			image->Draw();

		if(iconOver)
			iconOver->Draw();
		else if(icon) // draw icon
			icon->Draw();

		// draw text
		if(labelOver[0])
			labelOver[0]->Draw();
		else if(label[0])
			label[0]->Draw();
			
		if(labelOver[1])
			
			labelOver[1]->Draw();	
		else if(label[1])
			label[1]->Draw();
			
		if(labelOver[2])
			labelOver[2]->Draw();
		else if(label[2])
			label[2]->Draw();
	}
	else
	{
		if(image) // draw image
			image->Draw();
		if(icon) // draw icon
			icon->Draw();

		// draw text
		if(label[0])
			label[0]->Draw();
		if(label[1])
			label[1]->Draw();
		if(label[2])
			label[2]->Draw();
	}

	this->UpdateEffects();
}

void GuiButton::DrawTooltip()
{
	if(tooltip)
		tooltip->DrawTooltip();
}

void GuiButton::ResetText()
{
	for(int i=0; i<3; i++)
	{
		if(label[i])
			label[i]->ResetText();
		if(labelOver[i])
			labelOver[i]->ResetText();
	}
	if(tooltip)
		tooltip->ResetText();
}

void GuiButton::Update(GuiTrigger * t)
{
	if(state == STATE_CLICKED || state == STATE_DISABLED || !t)
		return;
	else if(parentElement && parentElement->GetState() == STATE_DISABLED)
		return;

	#ifdef HW_RVL
	// cursor
	if(t->wpad->ir.valid && t->chan >= 0)
	{
		if(this->IsInside(t->wpad->ir.x, t->wpad->ir.y))
		{
			if(state == STATE_DEFAULT) // we weren't on the button before!
			{
				this->SetState(STATE_SELECTED, t->chan);

				if(this->Rumble())
					rumbleRequest[t->chan] = 1;

				if(soundOver)
					soundOver->Play();

				if(effectsOver && !effects)
				{
					// initiate effects
					effects = effectsOver;
					effectAmount = effectAmountOver;
					effectTarget = effectTargetOver;
				}
			}
		}
		else
		{
			if(state == STATE_SELECTED && (stateChan == t->chan || stateChan == -1))
				this->ResetState();

			if(effectTarget == effectTargetOver && effectAmount == effectAmountOver)
			{
				// initiate effects (in reverse)
				effects = effectsOver;
				effectAmount = -effectAmountOver;
				effectTarget = 100;
			}
		}
	}
	#endif

	// button triggers
	if(this->IsClickable())
	{
		s32 wm_btns, wm_btns_trig, cc_btns, cc_btns_trig, wupc_btns, wupc_btns_trig;
		for(int i=0; i<3; i++)
		{
			if(trigger[i] && (trigger[i]->chan == -1 || trigger[i]->chan == t->chan))
			{
				// higher 16 bits only (wiimote)
				wm_btns = t->wpad->btns_d << 16;
				wm_btns_trig = trigger[i]->wpad->btns_d << 16;

				// lower 16 bits only (classic controller)
				cc_btns = t->wpad->btns_d >> 16;
				cc_btns_trig = trigger[i]->wpad->btns_d >> 16;

				// lower 16 bits only (WiiU Pro controller)
				wupc_btns = t->wupcdata.btns_d >> 16;
				wupc_btns_trig = trigger[i]->wupcdata.btns_d >> 16;

				if(
					(t->wpad->btns_d > 0 &&
					(wm_btns == wm_btns_trig ||
					(cc_btns == cc_btns_trig && t->wpad->exp.type == EXP_CLASSIC))) ||
					(t->pad.btns_d == trigger[i]->pad.btns_d && t->pad.btns_d > 0) ||
					(wupc_btns == wupc_btns_trig && wupc_btns_trig > 0))
					
				{
					if(t->chan == stateChan || stateChan == -1)
					{
						if(state == STATE_SELECTED)
						{
							if(!t->wpad->ir.valid ||	this->IsInside(t->wpad->ir.x, t->wpad->ir.y))
							{
								this->SetState(STATE_CLICKED, t->chan);

								if(soundClick)
									soundClick->Play();
							}
						}
						else if(trigger[i]->type == TRIGGER_BUTTON_ONLY)
						{
							this->SetState(STATE_CLICKED, t->chan);
						}
						else if(trigger[i]->type == TRIGGER_BUTTON_ONLY_IN_FOCUS &&
								parentElement->IsFocused())
						{
							this->SetState(STATE_CLICKED, t->chan);
						}
					}
				}
			}
		}
	}

	if(this->IsHoldable())
	{
		bool held = false;
		s32 wm_btns, wm_btns_h, wm_btns_trig, cc_btns, cc_btns_h, cc_btns_trig, wupc_btns, wupc_btns_h, wupc_btns_trig;

		for(int i=0; i<3; i++)
		{
			if(trigger[i] && (trigger[i]->chan == -1 || trigger[i]->chan == t->chan))
			{
				// higher 16 bits only (wiimote)
				wm_btns = t->wpad->btns_d << 16;
				wm_btns_h = t->wpad->btns_h << 16;
				wm_btns_trig = trigger[i]->wpad->btns_h << 16;

				// lower 16 bits only (classic controller)
				cc_btns = t->wpad->btns_d >> 16;
				cc_btns_h = t->wpad->btns_h >> 16;
				cc_btns_trig = trigger[i]->wpad->btns_h >> 16;

				// lower 16 bits only (WiiU Pro controller)
				wupc_btns = t->wpad->btns_d >> 16;
				wupc_btns_h = t->wpad->btns_h >> 16;
				wupc_btns_trig = trigger[i]->wpad->btns_h >> 16;				
				
				if(
					(t->wpad->btns_d > 0 &&
					(wm_btns == wm_btns_trig ||
					(cc_btns == cc_btns_trig && t->wpad->exp.type == EXP_CLASSIC))) ||
					(t->pad.btns_d == trigger[i]->pad.btns_h && t->pad.btns_d > 0) ||
					(wupc_btns == wupc_btns_trig && wupc_btns > 0))					
				{
					if(trigger[i]->type == TRIGGER_HELD && state == STATE_SELECTED &&
						(t->chan == stateChan || stateChan == -1))
						this->SetState(STATE_CLICKED, t->chan);
				}

				if(
					(t->wpad->btns_h > 0 &&
					(wm_btns_h == wm_btns_trig ||
					(cc_btns_h == cc_btns_trig && t->wpad->exp.type == EXP_CLASSIC))) ||
					(t->pad.btns_h == trigger[i]->pad.btns_h && t->pad.btns_h > 0) ||
					(wupc_btns_h == wupc_btns_trig && wupc_btns_h > 0))
					
				{
					if(trigger[i]->type == TRIGGER_HELD)
						held = true;
				}

				if(!held && state == STATE_HELD && stateChan == t->chan)
				{
					this->ResetState();
				}
				else if(held && state == STATE_CLICKED && stateChan == t->chan)
				{
					this->SetState(STATE_HELD, t->chan);
				}
			}
		}
	}

	if(updateCB)
		updateCB(this);
}
