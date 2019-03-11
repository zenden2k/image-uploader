#ifndef ABSTRACTVIDEOFRAME_H
#define ABSTRACTVIDEOFRAME_H

#include <string>

#include "AbstractImage.h"

class AbstractVideoFrame
{
public:
    AbstractVideoFrame();
    virtual ~AbstractVideoFrame();
    virtual bool saveToFile(const std::string& fileName) const = 0;
    virtual int getWidth() const;
    virtual int getHeight() const;
    int64_t getTime() const;
    virtual AbstractImage* toImage() const;
protected:
    int width_;
    int height_;
    int64_t time_;
};

#endif // ABSTRACTVIDEOFRAME_H
