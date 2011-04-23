This folder contains classes which provide Image Uploader's basic functionality, 
such as networking, file uploading, converting images. Unfortunately Video Grabber module still depends on GUI.

Most of them contain cross-platform code (they don't have  Windows API or ATL dependencies).

Anyway some of them still aren't (such as ScreenCapture and ImageConverter. They can't be ported to other platfroms
due to Gdiplus dependency)
