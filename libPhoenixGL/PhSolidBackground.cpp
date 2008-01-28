/*

Copyright (c) 2007, Jonathan Wayne Parrott.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

#include "PhSolidBackground.h"

using namespace phoenix;

PhSolidBackground::PhSolidBackground(PhSceneManager* s, PhColor c, float d)
	: PhSceneNode(d), color(c), smgr(s)
{
    smgr->addNode((PhSceneNode*)this);
}

PhSolidBackground::~PhSolidBackground()
{
    smgr->removeNode(this);
}

void PhSolidBackground::setColor(PhColor c)
{
    color = c;
}

PhColor PhSolidBackground::getColor()
{
    return color;
}

void PhSolidBackground::onPreRender()
{
    smgr->registerForRendering(this);
}

void PhSolidBackground::onRender()
{
    smgr->getRenderSystem()->drawRectangle( PhRect( smgr->getView()->getX(), smgr->getView()->getY(), smgr->getRenderSystem()->getScreenSize().getX(), smgr->getRenderSystem()->getScreenSize().getY()), depth, color, color, color, color);
}

void PhSolidBackground::onPostRender(){}
