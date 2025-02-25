[![Build Image Uploader](https://github.com/zenden2k/image-uploader/actions/workflows/main.yml/badge.svg?branch=master)](https://github.com/zenden2k/image-uploader/actions/workflows/main.yml)

# Image Uploader

Image Uploader is a free program for Windows for uploading images, screenshots and other file formats to over 30 different file hosting services. 
Official website: https://svistunov.dev/imageuploader

![Image Uploader main window](./Pics/main_window.png)

[![Download Image Uploader](./Pics/download_btn.png)](https://svistunov.dev/imageuploader_downloads)

Supported operating systems: Windows 7/8/10/11 or newer

Image Uploader [version 1.3.2](https://github.com/zenden2k/image-uploader/releases/tag/1.3.2-release) was the last supported release for Windows XP and Windows Vista.

## Main Features

### Screen capturing and annotating
Image Uploader can make screenshots of entire screen or selected screen region.

![Screen capturing with Image Uploader](./Pics/image_editor.png)

### Uploading files to image hostings and file hostings
It can upload to more than [20 image hostings sites](https://svistunov.dev/imageuploader_servers). After uploading, Image Uploader generates HTML code, BBCode for forums and Markdown code.

It is also possible to specify your own output format template.

### Grabbing frames from a video clip
Image Uploader can grab frames from various types of video formats, e.g. AVI, MPG, VOB, WMV, MKV. Image Uploader can utilize codecs installed on the system (it is recommended to use K-lite Codec Pack). If you do not have codecs installed or prefer not to clutter the system with them, the program can use the FFmpeg library, which is included with the program (in the portable version, or when using the installer, you need to check the corresponding option).

The extracted frames can be individually uploaded to the selected hosting service or combined into a single mosaic image.

Image Uploader is available in the following languages: English, Russian, Swedish, Romanian, Turkish, Serbian, Hrvatski, Farsi

## Console utility

![Image Uploader CLI running on Ubuntu](./Pics/cli_terminal.png)

Available options:

https://svistunov.dev/imageuploader-cli

## Documentation

[User Guide (Russian)](https://zenden2k.github.io/image-uploader/)

[Scripting API](https://zenden2k.github.io/image-uploader/api/html/index.html)

## Building from source

See [COMPILING.MD](COMPILING.MD)
