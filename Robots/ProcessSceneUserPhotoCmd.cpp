#include "Robot.h"

bool Robot::doSceneUserPhotoCmd(Cmd::UserCmd* buf, unsigned short len)
{
  if (!buf || !len) return false;
  switch (buf->param)
  {
    default:
      break;
  }
  return false;
}
