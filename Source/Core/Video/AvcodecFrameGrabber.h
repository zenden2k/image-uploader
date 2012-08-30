#ifndef FRAMEGRABBER_H
#define FRAMEGRABBER_H

#include <atlheaders.h>
#include "Gui/Dialogs/videograbber.h"

class CSampleGrabberCB;
int av_grab_frames(int numOfFrames, CString fname, CSampleGrabberCB * cb, HWND progressBar, bool& NeedStop, bool Jump=true, bool SeekToKeyFrame = true);


#endif // FRAMEGRABBER_H
