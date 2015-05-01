#include "GdiPlusImage.h"
#include "Core/logging.h"
#include "Core/Utils/CoreUtils.h"
#include "Core/Logging.h"
#include "Core/Utils/StringUtils.h"
#include "Core/Images/Utils.h"
#include <map>

using namespace Gdiplus;

GdiPlusImage::GdiPlusImage() {
    init();
}

GdiPlusImage::GdiPlusImage(Gdiplus::Bitmap* bm, bool takeOwnership)
{
    release_deleter<Gdiplus::Bitmap> deleter;
    if (!takeOwnership) {
        deleter.release();
    }
    bm_.reset(bm, deleter);
    init();
}

void GdiPlusImage::init()
{
    data_ = 0;
    width_ = 0;
    height_ = 0;
}

GdiPlusImage::~GdiPlusImage() {
    delete[] data_;
}


bool GdiPlusImage::saveToFile(const Utf8String& fileName) const
{
    std::map<std::string, std::string> mimeTypes;
    mimeTypes["jpg"] = "image/jpeg";
    mimeTypes["jpeg"] = "image/jpeg";
    mimeTypes["png"] = "image/png";
    std::string ext = IuStringUtils::toLower(IuCoreUtils::ExtractFileExt(fileName));

    std::map<std::string, std::string>::iterator it  = mimeTypes.find(ext);
    if ( it == mimeTypes.end() ) {
        LOG(ERROR) << "Format "<<ext<<" is not supported";
        return false;
    }
    std::string mimeType = it->second;
    CLSID clsidEncoder;
    EncoderParameters eps;
    eps.Count = 1;
    int quality = 85;

    if (mimeType == "image/jpeg") {
        eps.Parameter[0].Guid = EncoderQuality;
        eps.Parameter[0].Type = EncoderParameterValueTypeLong;
        eps.Parameter[0].NumberOfValues = 1;
        eps.Parameter[0].Value = &quality;
    } else if (mimeType == "image/png"){
            eps.Parameter[0].Guid = EncoderCompression;
            eps.Parameter[0].Type = EncoderParameterValueTypeLong;
            eps.Parameter[0].NumberOfValues = 1;
            eps.Parameter[0].Value = &quality;
    } else {
        LOG(ERROR) << "Mime  type  " << mimeType << " is not supported";
        return false;
    }

    if (GetEncoderClsid(IuCoreUtils::Utf8ToWstring(mimeType).c_str(), &clsidEncoder) != -1){
        if ( mimeType == "image/jpeg") {
            return bm_->Save(IuCoreUtils::Utf8ToWstring(fileName).c_str(), &clsidEncoder, &eps) == Ok;
        } else {
            return bm_->Save(IuCoreUtils::Utf8ToWstring(fileName).c_str(), &clsidEncoder) == Ok;
        }
    }
    return false;
}

bool GdiPlusImage::isNull() const
{
    return !bm_ || bm_->GetLastStatus() != Ok;
}


bool GdiPlusImage::loadFromRawData(DataFormat dt, int width, int height, uint8_t* data, size_t dataSize, void* parameter)
{
    if ( dt == AbstractImage::dfRGB888 )  {
        int lineSizeInBytes = reinterpret_cast<int>(parameter);
//        int dataSize = dataSize;
        size_t newLineSize = width * 3;
        newLineSize = ((newLineSize + 3) & ~3);
        size_t newDataSize = /*dataSize*2/**2*//*+100000**/newLineSize * height;
        data_ = new uint8_t[newDataSize];
        for ( int y=height-1; y>=0; y--) {
            memcpy(data_+(height-y-1)*newLineSize, data+y*lineSizeInBytes, width * 3 );
        }
        bool res =  loadFromRgb(/*width*/width, height,data_,  newDataSize);
        return res;

    } else if ( dt == AbstractImage::dfBitmapRgb ) {
        return loadFromRgb(width, height, data+sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER),dataSize);
    } else {
        LOG(ERROR) << "Format not supported "<< dt;
    }
    return false;
}

Gdiplus::Bitmap* GdiPlusImage::getBitmap() const
{
    return bm_.get();
}

int GdiPlusImage::getWidth() const
{
    return bm_->GetWidth();
}

int GdiPlusImage::getHeight() const
{
    return bm_->GetHeight();
}

bool GdiPlusImage::loadFromRgb(int width, int height, uint8_t* data, size_t dataSize)
{
    BITMAPINFOHEADER bih;
    memset( &bih, 0, sizeof(bih) );
    bih.biSize = sizeof(bih);
    bih.biWidth = width;
    bih.biHeight = height;
    bih.biPlanes = 1;
    bih.biBitCount = 24;

    BITMAPINFO bi;
    memset( &bi, 0, sizeof(bi) );
    bi.bmiHeader = bih;

    bm_.reset(new Gdiplus::Bitmap(&bi, data));

    if ( bm_->GetLastStatus() == Ok ) {
        width_ = width;
        height_ = height;
        return true;
    }

    return false;
}
