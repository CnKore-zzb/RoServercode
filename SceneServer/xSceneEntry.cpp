#include "xSceneEntry.h"
#include "Scene.h"

xSceneEntry::xSceneEntry()
{
  posI = 0;
  oldposI = 0;
}

xSceneEntry::~xSceneEntry()
{
}

void xSceneEntry::setPos(xPos p)
{
  if (!getScene()) return;
  oldpos = pos;
  oldposI = posI;
  pos = p;
  posI = getScene()->xPos2xPosI(p);
}
