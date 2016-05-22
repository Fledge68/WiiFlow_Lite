/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 *
 * gui_element.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui.h"

/**
 * Constructor for the Object class.
 */
GuiElement::GuiElement()
{
	xoffset = 0;
	yoffset = 0;
	xmin = 0;
	xmax = 0;
	ymin = 0;
	ymax = 0;
	width = 0;
	height = 0;
	alpha = 255;
	xscale = 1;
	yscale = 1;
	state = STATE_DEFAULT;
	stateChan = -1;
	trigger[0] = NULL;
	trigger[1] = NULL;
	trigger[2] = NULL;
	parentElement = NULL;
	rumble = true;
	selectable = false;
	clickable = false;
	holdable = false;
	visible = true;
	focus = -1; // cannot be focused
	updateCB = NULL;
	yoffsetDyn = 0;
	xoffsetDyn = 0;
	alphaDyn = -1;
	scaleDyn = 1;
	effects = 0;
	effectAmount = 0;
	effectTarget = 0;
	effectsOver = 0;
	effectAmountOver = 0;
	effectTargetOver = 0;

	// default alignment - align to top left
	alignmentVert = ALIGN_TOP;
	alignmentHor = ALIGN_LEFT;
}

/**
 * Destructor for the GuiElement class.
 */
GuiElement::~GuiElement()
{
}

void GuiElement::SetParent(GuiElement * e)
{
	parentElement = e;
}

GuiElement * GuiElement::GetParent()
{
	return parentElement;
}

int GuiElement::GetLeft()
{
	int x = 0;
	int pWidth = 0;
	int pLeft = 0;

	if(parentElement)
	{
		pWidth = parentElement->GetWidth();
		pLeft = parentElement->GetLeft();
	}

	if(effects & (EFFECT_SLIDE_IN | EFFECT_SLIDE_OUT))
		pLeft += xoffsetDyn;

	switch(alignmentHor)
	{
		case ALIGN_LEFT:
			x = pLeft;
			break;
		case ALIGN_CENTRE:
			x = pLeft + pWidth/2.0 - (width*xscale)/2.0;
			break;
		case ALIGN_RIGHT:
			x = pLeft + pWidth - width*xscale;
			break;
	}
	x += (width*(xscale - 1))/2.0; // correct offset for scaled images
	return x + xoffset;
}

int GuiElement::GetTop()
{
	int y = 0;
	int pHeight = 0;
	int pTop = 0;

	if(parentElement)
	{
		pHeight = parentElement->GetHeight();
		pTop = parentElement->GetTop();
	}

	if(effects & (EFFECT_SLIDE_IN | EFFECT_SLIDE_OUT))
		pTop += yoffsetDyn;

	switch(alignmentVert)
	{
		case ALIGN_TOP:
			y = pTop;
			break;
		case ALIGN_MIDDLE:
			y = pTop + pHeight/2.0 - (height*yscale)/2.0;
			break;
		case ALIGN_BOTTOM:
			y = pTop + pHeight - height*yscale;
			break;
	}
	y += (height*(yscale - 1))/2.0; // correct offset for scaled images
	return y + yoffset;
}

void GuiElement::SetMinX(int x)
{
	xmin = x;
}

int GuiElement::GetMinX()
{
	return xmin;
}

void GuiElement::SetMaxX(int x)
{
	xmax = x;
}

int GuiElement::GetMaxX()
{
	return xmax;
}

void GuiElement::SetMinY(int y)
{
	ymin = y;
}

int GuiElement::GetMinY()
{
	return ymin;
}

void GuiElement::SetMaxY(int y)
{
	ymax = y;
}

int GuiElement::GetMaxY()
{
	return ymax;
}

int GuiElement::GetWidth()
{
	return width;
}

int GuiElement::GetHeight()
{
	return height;
}

void GuiElement::SetSize(int w, int h)
{

	width = w;
	height = h;
}

bool GuiElement::IsVisible()
{
	return visible;
}

void GuiElement::SetVisible(bool v)
{
	visible = v;
}

void GuiElement::SetAlpha(int a)
{
	alpha = a;
}

int GuiElement::GetAlpha()
{
	int a = alpha;

	if(alphaDyn >= 0)
		a = alphaDyn;

	if(parentElement)
		a *= float(parentElement->GetAlpha())/255.0f;

	return a;
}

void GuiElement::SetScale(float s)
{
	xscale = s;
	yscale = s;
}

void GuiElement::SetScaleX(float s)
{
	xscale = s;
}

void GuiElement::SetScaleY(float s)
{
	yscale = s;
}

void GuiElement::SetScale(int mw, int mh)
{
	xscale = 1.0f;
	if(width > mw || height > mh)
	{
		if(width/(height*1.0) > mw/(mh*1.0))
			xscale = mw/(width*1.0);
		else
			xscale = mh/(height*1.0);
	}
	yscale = xscale;
}

float GuiElement::GetScale()
{
	float s = xscale * scaleDyn;

	if(parentElement)
		s *= parentElement->GetScale();

	return s;
}

float GuiElement::GetScaleX()
{
	float s = xscale * scaleDyn;

	if(parentElement)
		s *= parentElement->GetScale();

	return s;
}

float GuiElement::GetScaleY()
{
	float s = yscale * scaleDyn;

	if(parentElement)
		s *= parentElement->GetScaleY();

	return s;
}

int GuiElement::GetState()
{
	return state;
}

int GuiElement::GetStateChan()
{
	return stateChan;
}

void GuiElement::SetState(int s, int c)
{
	state = s;
	stateChan = c;
}

void GuiElement::ResetState()
{
	if(state != STATE_DISABLED)
	{
		state = STATE_DEFAULT;
		stateChan = -1;
	}
}

void GuiElement::SetClickable(bool c)
{
	clickable = c;
}

void GuiElement::SetSelectable(bool s)
{
	selectable = s;
}

void GuiElement::SetHoldable(bool d)
{
	holdable = d;
}

bool GuiElement::IsSelectable()
{
	if(state == STATE_DISABLED || state == STATE_CLICKED)
		return false;
	else
		return selectable;
}

bool GuiElement::IsClickable()
{
	if(state == STATE_DISABLED ||
		state == STATE_CLICKED ||
		state == STATE_HELD)
		return false;
	else
		return clickable;
}

bool GuiElement::IsHoldable()
{
	if(state == STATE_DISABLED)
		return false;
	else
		return holdable;
}

void GuiElement::SetFocus(int f)
{
	focus = f;
}

int GuiElement::IsFocused()
{
	return focus;
}

void GuiElement::SetTrigger(GuiTrigger * t)
{
	if(!trigger[0])
		trigger[0] = t;
	else if(!trigger[1])
		trigger[1] = t;
	else if(!trigger[2])
		trigger[2] = t;
	else // all were assigned, so we'll just overwrite the first one
		trigger[0] = t;
}

void GuiElement::SetTrigger(u8 i, GuiTrigger * t)
{
	trigger[i] = t;
}

bool GuiElement::Rumble()
{
	return rumble;
}

void GuiElement::SetRumble(bool r)
{
	rumble = r;
}

int GuiElement::GetEffect()
{
	return effects;
}

void GuiElement::SetEffect(int eff, int amount, int target)
{
	if(eff & EFFECT_SLIDE_IN)
	{
		// these calculations overcompensate a little
		if(eff & EFFECT_SLIDE_TOP)
			yoffsetDyn = -screenheight;
		else if(eff & EFFECT_SLIDE_LEFT)
			xoffsetDyn = -screenwidth;
		else if(eff & EFFECT_SLIDE_BOTTOM)
			yoffsetDyn = screenheight;
		else if(eff & EFFECT_SLIDE_RIGHT)
			xoffsetDyn = screenwidth;
	}
	if(eff & EFFECT_FADE)
	{
		if(amount > 0)
			alphaDyn = 0;
		else if(amount < 0)
			alphaDyn = alpha;
	}

	effects |= eff;
	effectAmount = amount;
	effectTarget = target;
}

void GuiElement::SetEffectOnOver(int eff, int amount, int target)
{
	effectsOver |= eff;
	effectAmountOver = amount;
	effectTargetOver = target;
}

void GuiElement::SetEffectGrow()
{
	SetEffectOnOver(EFFECT_SCALE, 4, 110);
}

void GuiElement::UpdateEffects()
{
	if(effects & (EFFECT_SLIDE_IN | EFFECT_SLIDE_OUT))
	{
		if(effects & EFFECT_SLIDE_IN)
		{
			if(effects & EFFECT_SLIDE_LEFT)
			{
				xoffsetDyn += effectAmount;

				if(xoffsetDyn >= 0)
				{
					xoffsetDyn = 0;
					effects = 0;
				}
			}
			else if(effects & EFFECT_SLIDE_RIGHT)
			{
				xoffsetDyn -= effectAmount;

				if(xoffsetDyn <= 0)
				{
					xoffsetDyn = 0;
					effects = 0;
				}
			}
			else if(effects & EFFECT_SLIDE_TOP)
			{
				yoffsetDyn += effectAmount;

				if(yoffsetDyn >= 0)
				{
					yoffsetDyn = 0;
					effects = 0;
				}
			}
			else if(effects & EFFECT_SLIDE_BOTTOM)
			{
				yoffsetDyn -= effectAmount;

				if(yoffsetDyn <= 0)
				{
					yoffsetDyn = 0;
					effects = 0;
				}
			}
		}
		else
		{
			if(effects & EFFECT_SLIDE_LEFT)
			{
				xoffsetDyn -= effectAmount;

				if(xoffsetDyn <= -screenwidth)
					effects = 0; // shut off effect
			}
			else if(effects & EFFECT_SLIDE_RIGHT)
			{
				xoffsetDyn += effectAmount;

				if(xoffsetDyn >= screenwidth)
					effects = 0; // shut off effect
			}
			else if(effects & EFFECT_SLIDE_TOP)
			{
				yoffsetDyn -= effectAmount;

				if(yoffsetDyn <= -screenheight)
					effects = 0; // shut off effect
			}
			else if(effects & EFFECT_SLIDE_BOTTOM)
			{
				yoffsetDyn += effectAmount;

				if(yoffsetDyn >= screenheight)
					effects = 0; // shut off effect
			}
		}
	}
	if(effects & EFFECT_FADE)
	{
		alphaDyn += effectAmount;

		if(effectAmount < 0 && alphaDyn <= 0)
		{
			alphaDyn = 0;
			effects = 0; // shut off effect
		}
		else if(effectAmount > 0 && alphaDyn >= alpha)
		{
			alphaDyn = alpha;
			effects = 0; // shut off effect
		}
	}
	if(effects & EFFECT_SCALE)
	{
		scaleDyn += f32(effectAmount)*0.01f;
		f32 effTar100 = f32(effectTarget)*0.01f;

		if((effectAmount < 0 && scaleDyn <= effTar100)
			|| (effectAmount > 0 && scaleDyn >= effTar100))
		{
			scaleDyn = effTar100;
			effects = 0; // shut off effect
		}
	}
}

void GuiElement::Update(GuiTrigger * t)
{
	if(updateCB)
		updateCB(this);
}

void GuiElement::SetUpdateCallback(UpdateCallback u)
{
	updateCB = u;
}

void GuiElement::SetPosition(int xoff, int yoff)
{
	xoffset = xoff;
	yoffset = yoff;
}

void GuiElement::SetAlignment(int hor, int vert)
{
	alignmentHor = hor;
	alignmentVert = vert;
}

int GuiElement::GetSelected()
{
	return -1;
}

void GuiElement::ResetText()
{
}

void GuiElement::Draw()
{
}

void GuiElement::DrawTooltip()
{
}

bool GuiElement::IsInside(int x, int y)
{
	if(unsigned(x - this->GetLeft())  < unsigned(width)
	&& unsigned(y - this->GetTop())  < unsigned(height))
		return true;
	return false;
}
