/********************************************************************************
    Copyright 2004 Sjaak Priester    

    This file is part of Tinter.

    Tinter is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Tinter is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Tinter; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
********************************************************************************/

// QColorQuantizer
//
// Version 1.0 (c) 2004, Sjaak Priester, Amsterdam.
// mailto:sjaak@sjaakpriester.nl
#include "atlheaders.h"
#include <cassert>
#include "3rdpart/GdiplusH.h"
#include "QColorQuantizer.h"
using namespace Gdiplus;
QColorQuantizer::QColorQuantizer(void)
:
 m_Message(0)
, m_bStop(false)
, m_pSource(NULL)
, m_nMaxColors(0)
{
}

QColorQuantizer::~QColorQuantizer(void)
{
}

void QColorQuantizer::MakeQuantized(Image * pSource, Mode mode, UINT nMaxColors)
{
    Stop();
    m_pSource = pSource;
    m_Mode = mode;
    m_nMaxColors = nMaxColors;

//    m_Message = message;
ThreadProc(this);
    //m_pThread = ::AfxBeginThread(ThreadProc, this);
}

void QColorQuantizer::Stop(void)
{
    /*if (m_pThread)
    {
        m_bStop = true;
        ::WaitForSingleObject(m_pThread->m_hThread, INFINITE);
    }*/
}

/* static */
UINT QColorQuantizer::ThreadProc(LPVOID pParam)
{
    QColorQuantizer * pThis = (QColorQuantizer *) pParam;

    
    /*Bitmap * pResult = */pThis->Calculate();
    


    //pThis->Notify(pResult);
    return 0;
}

// called by ThreadProc
Bitmap * QColorQuantizer::Calculate(void)
{
    m_bStop = false;
    return GetQuantized(m_pSource, m_Mode, m_nMaxColors);
}

// called by ThreadProc
void QColorQuantizer::Notify(Bitmap * pResult)
{
    //if (pResult && m_pMsgWnd) m_pMsgWnd->PostMessage(m_Message, 0, (LPARAM) pResult);
}

Bitmap * QColorQuantizer::GetQuantized(Image * pSource, Mode mode, UINT nMaxColors)
{
    UINT w = pSource->GetWidth();
    UINT h = pSource->GetHeight();

    Bitmap * pSrcBitmap = NULL;

    // If necessary, convert source to 24 bpp RGB bitmap
    if (pSource->GetPixelFormat() == PixelFormat24bppRGB) pSrcBitmap = (Bitmap *) pSource;
    else
    {
        pSrcBitmap = new Bitmap(w, h, PixelFormat24bppRGB);

        Graphics g(pSrcBitmap);
        if (g.DrawImage(pSource, 0, 0, w, h) != Ok)
        {
            delete pSrcBitmap;
            return NULL;
        }
    }

    // Create 8 bpp indexed bitmap of the same size
    Bitmap * pResult = new Bitmap(w, h, PixelFormat8bppIndexed);

    bool bSucceeded = false;

    switch (mode)
    {
    case HalfTone:
        bSucceeded = QuantizeWebSafe(pSrcBitmap, pResult);
        break;
    case Octree:
        bSucceeded = QuantizeOctree(pSrcBitmap, pResult, nMaxColors);
        break;
    default:
        break;
    }

    if (m_bStop || ! bSucceeded)
    {
        delete pResult;
        pResult = NULL;
    }
    
    if (pSource != pSrcBitmap) delete pSrcBitmap;

    return pResult;
}

bool QColorQuantizer::QuantizeWebSafe(Bitmap * pSource , Bitmap * pDest)
{
    // It would be nice if we could simply draw an Image to an 8bpp Bitmap,
    // but GDI+ doesn't support it. Apparently, when saving a GIF, the conversion
    // is done by the encoder.
    //
    // Websafe color reduction simply uses the standard GDI/GDI+ halftone palette.
    // Entries 40...255 are the websafe colors, arranged like this:
    //
    //    40 ff000000
    //    41 ff000033
    //    42 ff000066
    //    43 ff000099
    //    44 ff0000cc
    //    45 ff0000ff
    //    46 ff003300
    //    47 ff003333
    //    ...
    //    252 ffffff66
    //    253 ffffff99
    //    254 ffffffcc
    //    255 ffffffff
    //
    // For the 216 (= 6^3) websafe colors, the color scale in each dimension
    // is divided in six parts.
    // Knowing that 6 * 43 = 258 = just a little bit more than 256, we derive the
    // following formula to calculate the color index from a 3x8 RGB color:
    //    index = 40 + 6 * (6 * ((R + 1) / 43) + ((G + 1) / 43)) + ((B + 1) / 43)

    bool r = true;

    UINT w = pSource->GetWidth();
    UINT h = pSource->GetHeight();
    Rect rc(0, 0, w, h);

    BitmapData dataSource;

    // Lock bits on 3x8 source bitmap
    if (pSource->LockBits(& rc, ImageLockModeRead, PixelFormat24bppRGB, & dataSource) == Ok)
    {
        BitmapData dataDest;

        // Lock bits on indexed destination bitmap
        if (pDest->LockBits(& rc, ImageLockModeWrite, PixelFormat8bppIndexed, & dataDest) == Ok)
        {
            BYTE * pRowSource = (BYTE *) dataSource.Scan0;
            UINT strideSource;

            if (dataSource.Stride > 0) strideSource = dataSource.Stride;
            else
            {
                // Compensate for possible negative stride
                pRowSource += h * dataSource.Stride;
                strideSource = - dataSource.Stride;
            }

            BYTE * pRowDest = (BYTE *) dataDest.Scan0;
            UINT strideDest;

            if (dataDest.Stride > 0) strideDest = dataDest.Stride;
            else
            {
                pRowDest += h * dataDest.Stride;
                strideDest = - dataDest.Stride;
            }

            for (UINT y = 0; y < h; y++)    // For each row...
            {
                BYTE * pPixelSource = pRowSource;
                BYTE * pPixelDest = pRowDest;

                for (UINT x = 0; x < w; x++)    // ...for each pixel...
                {
                    // ...gather color information from 3x8 source bitmap...
                    UINT b = * pPixelSource++;
                    UINT g = * pPixelSource++;
                    UINT r = * pPixelSource++;

                    // ...calculate index in standard halftone palette...
                    UINT index = 40 + 6 * (6 * ((r + 1) / 43) + ((g + 1) / 43)) + ((b + 1) / 43);

                    // ...and put result in indexed bitmap.
                    * pPixelDest++ = (BYTE) index;
                }

                if (m_bStop) break;

                pRowSource += strideSource;
                pRowDest += strideDest;
            }
            pDest->UnlockBits(& dataDest);
        }
        else r = false;

        pSource->UnlockBits(& dataSource);
    }
    else r = false;

    return r;
}


//====================
// Octree Color quantizing


// The work horse for Octree color quantizing.
bool QColorQuantizer::QuantizeOctree(Bitmap * pSource, Bitmap * pDest, UINT nMaxColors)
{
    if (nMaxColors > 256 || nMaxColors < 8) return false;

    bool r = true;
    UINT nLeafs = 0;    // counter of leaf nodes in octree

    OctreeNode head(1, NULL, nLeafs);    // head of list with reducible non-leaf nodes (dummy)
    head.m_nLevel = 0;
    OctreeNode root(7, NULL, nLeafs);    // root of octree

    // Put tree root in reducible list. Root will always be the last node in the list,
    // and also functions as list sentinel.
    head.m_pNext = & root;
    root.m_pPrev = & head;

    UINT w = pSource->GetWidth();
    UINT h = pSource->GetHeight();
    Rect rc(0, 0, w, h);

    BitmapData dataSource;

    // Lock bits on 3x8 source bitmap
    if (pSource->LockBits(& rc, ImageLockModeRead, PixelFormat24bppRGB, & dataSource) == Ok)
    {
        BYTE * pScan0Source = (BYTE *) dataSource.Scan0;
        UINT strideSource;

        if (dataSource.Stride > 0) strideSource = dataSource.Stride;
        else
        {
            // Compensate for possible negative stride
            // (not needed for first loop, but we have to do it
            // for second loop anyway)
            pScan0Source += h * dataSource.Stride;
            strideSource = - dataSource.Stride;
        }

        BYTE * pRowSource = pScan0Source;

        // First loop: gather color information, put it in tree leaves
        for (UINT y = 0; y < h; y++)    // For each row...
        {
            BYTE * pPixelSource = pRowSource;

            for (UINT x = 0; x < w; x++)    // ...for each pixel...
            {
                UINT b = * pPixelSource++;
                UINT g = * pPixelSource++;
                UINT r = * pPixelSource++;

                root.SetColor(r, g, b, & head, nLeafs);
                                    // ...add color to tree, make new leaf if necessary.

                while (nLeafs > nMaxColors)    // If in the process too many leaves are created...
                {
                    // ... then, one by one, pick a non-leaf node...
                    OctreeNode * pReducible = head.m_pNext;
                    assert(pReducible);

                    if (m_bStop || ! pReducible->m_pNext) break;
                    pReducible->Remove();    // ...remove it from the list of reducibles...
                    pReducible->GetQuantized(nLeafs);    // ...and reduce it to a leaf.
                }
                if (m_bStop) break;

            }
            if (m_bStop) break;

            pRowSource += strideSource;
        }

        if (! m_bStop)
        {
            // We now have the important colors in the pic. Put them in a palette
            BYTE * pPaletteBytes = new BYTE[sizeof(ColorPalette) + (nLeafs - 1) * sizeof(ARGB)];
            {
                ColorPalette * pPalette = (ColorPalette *) pPaletteBytes;
                pPalette->Flags = PaletteFlagsHasAlpha;
                pPalette->Count = nLeafs;

                UINT index = 0;
                root.FillPalette(pPalette, index);    // Let octree fill the palette

                assert(index == nLeafs);

                pDest->SetPalette(pPalette);

                BitmapData dataDest;

                // Lock bits on indexed destination bitmap
                if (pDest->LockBits(& rc, ImageLockModeWrite, PixelFormat8bppIndexed, & dataDest) == Ok)
                {
                    BYTE * pRowSource = pScan0Source;

                    BYTE * pRowDest = (BYTE *) dataDest.Scan0;
                    UINT strideDest;

                    // Compensate for possible negative stride
                    if (dataDest.Stride > 0) strideDest = dataDest.Stride;
                    else
                    {
                        pRowDest += h * dataDest.Stride;
                        strideDest = - dataDest.Stride;
                    }

                    // Second loop: fill indexed bitmap
                    for (UINT y = 0; y < h; y++)    // For each row...
                    {
                        BYTE * pPixelSource = pRowSource;
                        BYTE * pPixelDest = pRowDest;

                        for (UINT x = 0; x < w; x++)    // ...for each pixel...
                        {
                            // ...gather color information from source bitmap...
                            UINT b = * pPixelSource++;
                            UINT g = * pPixelSource++;
                            UINT r = * pPixelSource++;

                            // ...let octree calculate index...
                            BYTE index = (BYTE) root.GetPaletteIndex(r, g, b);

                            // ...and put index in the destination bitmap.
                            * pPixelDest++ = index;
                        }

                        pRowSource += strideSource;
                        pRowDest += strideDest;

                        if (m_bStop) break;
                    }
                    pDest->UnlockBits(& dataDest);
                }
                else r = false;

                delete[] pPaletteBytes;
            }
            //else r = false;
        }
        pSource->UnlockBits(& dataSource);
    }
    else r = false;
    if (m_bStop) r = false;

    return r;
}

//===============
// OctreeNode helper class

QColorQuantizer::OctreeNode::OctreeNode(UINT level, OctreeNode * pHead, UINT& nLeafs)
: m_nPixels(0)
, m_totalR(0)
, m_totalG(0)
, m_totalB(0)
, m_nLevel(level)
, m_bLeaf(level == 0)
, m_pNext(NULL)
, m_pPrev(NULL)
{
    for (UINT i = 0; i < 8; i++) m_pChild[i] = NULL;

    if (m_bLeaf) ++nLeafs;    // If we are a leaf, increment counter
    else if (pHead)
    {
        // Otherwise, put us at front of list of reducible non-leaf nodes...
        InsertAfter(pHead);

        // ...and determine our priority when it comes to reducing.
        UpdatePosition();
    }
}

QColorQuantizer::OctreeNode::~OctreeNode()
{
    for (UINT i = 0; i < 8; i++) delete m_pChild[i];
}

void QColorQuantizer::OctreeNode::UpdatePosition()
{
    // Determine our priority, in other words our position in the list.
    assert(m_pNext);
    assert(m_pPrev);

    OctreeNode * pNext = m_pNext;

    // Increment as long as pNext should be reduced before we are.
    while (pNext && ! ReduceBefore(* pNext)) pNext = pNext->m_pNext;
    assert(pNext);

    if (pNext != m_pNext)
    {
        Remove();                // Remove us from old position in list...
        InsertBefore(pNext);    // ...and insert us at new position.
    }
}

void QColorQuantizer::OctreeNode::Remove()
{
    assert(m_pNext);
    assert(m_pPrev);

    m_pPrev->m_pNext = m_pNext;
    m_pNext->m_pPrev = m_pPrev;
}

void QColorQuantizer::OctreeNode::InsertBefore(OctreeNode * pNext)
{
    assert(pNext);

    m_pPrev = pNext->m_pPrev;
    m_pNext = pNext;

    m_pPrev->m_pNext = this;
    m_pNext->m_pPrev = this;
}

UINT QColorQuantizer::OctreeNode::ChildIndex(UINT r, UINT g, UINT b) const
{
    // The octree trick: calculate child index based on the m_Level'th bits
    // of the color bytes.
    UINT rBit = (r >> m_nLevel) & 1;
    UINT gBit = (g >> m_nLevel) & 1;
    UINT bBit = (b >> m_nLevel) & 1;

    rBit <<= 2;
    rBit += gBit << 1;
    rBit += bBit;

    return rBit;
}

void QColorQuantizer::OctreeNode::SetColor(UINT r, UINT g, UINT b, OctreeNode * pHead, UINT& nLeafs)
{
    m_nPixels++;
    if (m_bLeaf)
    {
        // If we are a leaf node, accumulate color bytes in totals.
        m_totalR += r;
        m_totalG += g;
        m_totalB += b;
    }
    else
    {
        if (m_pNext) UpdatePosition();    // Perhaps we should move further down the list,
                                        // because we represent more pixels.

        // We are not a leaf node, so delegate to one of our children.
        UINT index = ChildIndex(r, g, b);

        // If we don't have a matching child, create it.
        if (m_pChild[index] == NULL)
            m_pChild[index] = new OctreeNode(m_nLevel - 1, pHead, nLeafs);

        m_pChild[index]->SetColor(r, g, b, pHead, nLeafs);    // recursion
    }
}

UINT QColorQuantizer::OctreeNode::GetPaletteIndex(UINT r, UINT g, UINT b) const
{
    // If we are a leaf node, the palette index is simply stored.
    if (m_bLeaf) return m_iPaletteIndex;
    else
    {
        // Otherwise, the matching child node must know more about it.
        UINT index = ChildIndex(r, g, b);
        assert(m_pChild[index] != NULL);
        return m_pChild[index]->GetPaletteIndex(r, g, b);    // recursion
    }
}

void QColorQuantizer::OctreeNode::GetQuantized(UINT& nLeafs)
{
    // GetQuantized a non-leaf node to a leaf
    assert(! m_bLeaf);

    UINT r = 0;
    UINT g = 0;
    UINT b = 0;

    for (UINT i = 0; i < 8; i++)
    {
        OctreeNode * pChild = m_pChild[i];
        if (pChild == NULL) continue;

        assert(pChild->m_bLeaf);

        // Accumulate color totals of our children...
        r += pChild->m_totalR;
        g += pChild->m_totalG;
        b += pChild->m_totalB;

        // ...and delete them.
        delete pChild;
        m_pChild[i] = NULL;

        --nLeafs;
    }
    m_totalR = r;
    m_totalG = g;
    m_totalB = b;

    // Now we have become a leaf node ourselves.
    m_bLeaf = true;
    ++nLeafs;
}

void QColorQuantizer::OctreeNode::FillPalette(ColorPalette * pPal, UINT& index)
{
    if (m_bLeaf)
    {
        // If we are a leaf, calculate the color by dividing the color totals
        // through the number of pixels we represent...
        Color col(255,
            (BYTE)(m_totalR / m_nPixels),
            (BYTE)(m_totalG / m_nPixels),
            (BYTE)(m_totalB / m_nPixels));

        // ...and put it in the palette.
        pPal->Entries[index] = col.GetValue();
        m_iPaletteIndex = index++;
    }
    else
    {
        // Otherwise, ask our child nodes.
        for (UINT i = 0; i < 8; i++)
            if (m_pChild[i] != NULL) m_pChild[i]->FillPalette(pPal, index); // recursion
    }
}
