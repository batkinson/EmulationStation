#include "GuiComponent.h"
#include "Renderer.h"
#include <iostream>
#include "Log.h"

std::vector<GuiComponent*> GuiComponent::sComponentVector;

GuiComponent::GuiComponent()
{
	sComponentVector.push_back(this);

	mOffsetX = 0;
	mOffsetY = 0;
	mOpacity = 255;
}

GuiComponent::~GuiComponent()
{
	for(unsigned int i = 0; i < sComponentVector.size(); i++)
	{
		if(sComponentVector.at(i) == this)
		{
			sComponentVector.erase(sComponentVector.begin() + i);
		}
	}
}

void GuiComponent::addChild(GuiComponent* comp)
{
	mChildren.push_back(comp);
}

void GuiComponent::removeChild(GuiComponent* comp)
{
	for(unsigned int i = 0; i < mChildren.size(); i++)
	{
		if(mChildren.at(i) == comp)
		{
			mChildren.erase(mChildren.begin() + i);
			return;
		}
	}

	LOG(LogError) << "Error - tried to remove GuiComponent child, but couldn't find it!";
}

void GuiComponent::clearChildren()
{
	mChildren.clear();
}

void GuiComponent::processTicks(int deltaTime)
{
	for(unsigned int i = 0; i < sComponentVector.size(); i++)
	{
		sComponentVector.at(i)->onTick(deltaTime);
	}
}

void GuiComponent::render()
{
	onRender();

	for(unsigned int i = 0; i < mChildren.size(); i++)
	{
		mChildren.at(i)->render();
	}
}

void GuiComponent::pause()
{
	onPause();

	for(unsigned int i = 0; i < mChildren.size(); i++)
		mChildren.at(i)->pause();
}

void GuiComponent::resume()
{
	onResume();

	for(unsigned int i = 0; i < mChildren.size(); i++)
		mChildren.at(i)->resume();
}

void GuiComponent::init()
{
	onInit();

	for(unsigned int i = 0; i < mChildren.size(); i++)
	{
		mChildren.at(i)->init();
	}
}

void GuiComponent::deinit()
{
	onDeinit();

	for(unsigned int i = 0; i < mChildren.size(); i++)
	{
		mChildren.at(i)->deinit();
	}
}

void GuiComponent::setOffsetX(int val) { mOffsetX = val; }
void GuiComponent::setOffsetY(int val) { mOffsetY = val; }
void GuiComponent::setOffset(int x, int y) { mOffsetX = x; mOffsetY = y; }
int GuiComponent::getOffsetX() { return mOffsetX; }
int GuiComponent::getOffsetY() { return mOffsetY; }

void GuiComponent::setOpacity(unsigned char opacity) { mOpacity = opacity; }
unsigned char GuiComponent::getOpacity() { return mOpacity; }
