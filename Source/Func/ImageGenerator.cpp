#include "ImageGenerator.h"
#ifdef IU_ENABLE_MEDIAINFO
#include "MediaInfoHelper.h"
#endif
#include "Core/ServiceLocator.h"
#include "3rdpart/GdiplusH.h"
#include "Core/Images/Utils.h"
#include "Core/Settings/WtlGuiSettings.h"


ImageGeneratorTask::ImageGeneratorTask(HWND wnd, std::vector<FileItem> files, int maxWidth, int maxHeight, CString mediaFile):
	files_(std::move(files)),
    maxWidth_(maxWidth),
	maxHeight_(maxHeight),
    wnd_(wnd),
    mediaFile_(mediaFile)
{

}

BackgroundTaskResult ImageGeneratorTask::doJob() {
    using namespace Gdiplus;
    WtlGuiSettings& Settings = *ServiceLocator::instance()->settings<WtlGuiSettings>();
    RectF TextRect;
    int infoHeight = 0;
    CString Report, fullInfo;
#ifdef IU_ENABLE_MEDIAINFO
    if (Settings.VideoSettings.ShowMediaInfo)
    {
        onProgress(this, -1, -1, W2U(TR("Getting info about file...")));
        /*bool bMediaInfoResult = */
        MediaInfoHelper::GetMediaFileInfo(mediaFile_, Report, fullInfo, Settings.MediaInfoSettings.EnableLocalization);

        Graphics g1(wnd_);

        auto font = ImageUtils::StringToGdiplusFont(Settings.VideoSettings.Font);

        FontFamily ff;
        font->GetFamily(&ff);
        g1.SetPageUnit(UnitPixel);
        g1.MeasureString(Report, -1, font.get(), PointF(0, 0), &TextRect);
        infoHeight = int(TextRect.Height);
    }
#endif
    int n = files_.size();

    onProgress(this, 0, n-1, W2U(TR("Generating image...")));
	
    int ncols = min(Settings.VideoSettings.Columns, n);
    if (ncols <= 0) {
        ncols = 1;
    }
    int nstrings = n / ncols + ((n % ncols) ? 1 : 0);
    int maxwidth = maxWidth_;
    int maxheight = maxHeight_;
    int gapwidth = Settings.VideoSettings.GapWidth;
    int gapheight = Settings.VideoSettings.GapHeight;
    infoHeight += gapheight;
    int tilewidth = Settings.VideoSettings.TileWidth;
    int tileheight = static_cast<int>(((float)tilewidth) / ((float)maxwidth) * ((float)maxheight));
    int needwidth = gapwidth + ncols * (tilewidth + gapwidth);
    int needheight = gapheight + nstrings * (tileheight + gapheight) + infoHeight;

    Graphics g(wnd_, true);
    Bitmap BackBuffer(needwidth, needheight, &g);
    Graphics gr(&BackBuffer);
    Rect r(0, 0, needwidth, needheight);
    gr.Clear(Color(255, 180, 180, 180));
    LinearGradientBrush br(r, Color(255, 224, 224, 224), Color(255, 243, 243, 243),
        LinearGradientModeBackwardDiagonal);
    gr.FillRectangle(&br, r);
    int x, y;
    Pen Framepen(Color(90, 90, 90));

    Font font(L"Tahoma", 10, FontStyleBold);
    Color ColorText(140, 255, 255, 255);
    Color ColorStroke(120, 0, 0, 0);
	
    for (int i = 0; i < n; i++)
    {
    	if (isCanceled()) {
            return BackgroundTaskResult::Canceled;
    	}
        Image bm(files_[i].fileName);
    	if (bm.GetLastStatus() != Gdiplus::Ok) {
            LOG(ERROR) << "Failed to load image " << files_[i].fileName;
    	}
        x = gapwidth + (i % ncols) * (tilewidth + gapwidth);
        y = infoHeight + (infoHeight ? gapheight : 0) + ((i / ncols)) * (tileheight + gapheight);
        //ThumbsView.GetItemText(i, 0, buf, 256);
        CString buf = files_[i].title;
        gr.DrawImage(&bm, (int)(x /*(tilewidth-newwidth)/2*/), (int)y, (int)tilewidth, (int)tileheight);
        ImageUtils::DrawStrokedText(gr, buf, RectF(float(x), float(y), float(tilewidth),
            float(tileheight)), font, ColorText, ColorStroke, 3, 3);
        gr.DrawRectangle(&Framepen, Rect(x /*(tilewidth-newwidth)/2*/, (int)y, (int)tilewidth, (int)tileheight));

        onProgress(this, i+1, n-1, W2U(TR("Generating image...")));
    }

    if (infoHeight)
    {
        StringFormat format;
        format.SetAlignment(StringAlignmentNear);
        format.SetLineAlignment(StringAlignmentNear);
        auto font = ImageUtils::StringToGdiplusFont(Settings.VideoSettings.Font);

        SolidBrush br(/*Settings.ThumbSettings.ThumbTextColor*/ MYRGB(255, Settings.VideoSettings.TextColor));
        RectF textBounds(float(gapwidth), float(gapheight), float(needwidth - gapwidth), float(infoHeight - gapheight));
        gr.DrawString(Report, -1, font.get(), textBounds, &format, &br);
        // /DrawStrokedText(gr, Report,textBounds,font,ColorText,ColorStroke,3,3);
    }
    if (isCanceled()) {
        return BackgroundTaskResult::Canceled;
    }
	
    onProgress(this, -1, -1, W2U(TR("Saving image...")));

    try {
        ImageUtils::MySaveImage(&BackBuffer, _T("grab_custom"), outFileName_, ImageUtils::sifPNG, 100);
        return BackgroundTaskResult::Success;
    } catch (const std::exception& ex) {
        LOG(ERROR) << ex.what();
    }
    return BackgroundTaskResult::Failed;
}

CString ImageGeneratorTask::outFileName() const {
    return outFileName_;
}
