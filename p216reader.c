/*

P21X VFW Reader for AviUtl0.99m

Author Oka Motofumi (chikuzen.mo at gmail dot com)


The MIT License

Copyright (c) 2012 Oka Motofumi

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


#ifdef DEBUG
#include <stdio.h>
#include <stdarg.h>
#endif

#include <windows.h>
#include <vfw.h>
#include "input.h"


INPUT_PLUGIN_TABLE input_plugin_table = {
    INPUT_PLUGIN_FLAG_VIDEO,
    "P21x VFW reader",
    "vapoursynth script (*.vpy)\0*.vpy\0"
    "AVI File (*.avi)\0*.avi\0",
    "P21x VFW reader version 0.2.0",
    func_init,
    func_exit,
    func_open,
    func_close,
    func_info_get,
    func_read_video,
    NULL,               //  ignore audio
    NULL,               //  all keyframe
    NULL,               //  no dialog
};


INPUT_PLUGIN_TABLE __declspec(dllexport) * __stdcall GetInputPluginTable( void )
{
    return &input_plugin_table;
}


typedef struct {
    PAVIFILE file;
    PAVISTREAM stream;
    AVIFILEINFO file_info;
    AVISTREAMINFO stream_info;
    BITMAPINFOHEADER dst_format;
    char *buff;
    LONG buff_size;
} p21x_hnd_t;

#ifdef DEBUG
static void debug_msg(char *format, ...)
{
    va_list args;
    char buf[1024];
    va_start(args, format);
    vsprintf(buf, format, args);
    va_end(args);
    MessageBox(NULL, buf, "debug info", MB_OK);
}
#endif


BOOL func_init(void)
{
    AVIFileInit();
    return TRUE;
}


BOOL func_exit(void)
{
    AVIFileExit();
    return TRUE;
}


static void set_dst_format(p21x_hnd_t *h)
{
    int width = h->file_info.dwWidth;
    int height = h->file_info.dwHeight;
    h->dst_format.biSize = sizeof(BITMAPINFOHEADER);
    h->dst_format.biWidth = width;
    h->dst_format.biHeight = height;
    h->dst_format.biPlanes = 1;
    h->dst_format.biBitCount = 48;
    h->dst_format.biCompression = MAKEFOURCC('Y', 'C', '4', '8');
    h->dst_format.biSizeImage = (((width * 3 + 3) >> 2) << 2) * height;
}


static int allocate_buff(p21x_hnd_t *h)
{
    AVIStreamRead(h->stream, 0, 1, NULL, 0, &h->buff_size, NULL);
    h->buff = malloc(h->buff_size);
    return !h->buff;
}


static void close_handler(p21x_hnd_t *h)
{
    if (h->buff) {
        free(h->buff);
    }
    if (h->stream) {
        AVIStreamRelease(h->stream);
    }
    AVIFileRelease(h->file);
    free(h);
}


INPUT_HANDLE func_open(LPSTR file)
{
    p21x_hnd_t *h = (p21x_hnd_t *)calloc(1, sizeof(p21x_hnd_t));
    if (!h) {
        return NULL;
    }

    if (AVIFileOpen(&h->file, file, OF_READ, NULL) != 0) {
        free(h);
        return NULL;
    }

    if (AVIFileInfo(h->file, &h->file_info, sizeof(AVIFILEINFO)) != 0) {
        goto fail;
    }

    if (AVIFileGetStream(h->file, &h->stream, streamtypeVIDEO, 0) != 0 ) {
        goto fail;
    }

    AVIStreamInfo(h->stream, &h->stream_info, sizeof(AVISTREAMINFO));
    if (h->stream_info.fccHandler != MAKEFOURCC('P', '2', '1', '6') &&
        h->stream_info.fccHandler != MAKEFOURCC('P', '2', '1', '0')) {
        goto fail;
    }

    set_dst_format(h);

    if (allocate_buff(h)) {
        goto fail;
    }

    return h;

fail:
    close_handler(h);
    return NULL;
}


BOOL func_close(INPUT_HANDLE ih)
{
    p21x_hnd_t *h = (p21x_hnd_t *)ih;
    if (!h) {
        return TRUE;
    }

    close_handler(h);

    return TRUE;
}


#define INPUT_INFO_FLAG_VIDEO 1

BOOL func_info_get(INPUT_HANDLE ih, INPUT_INFO *iip)
{
    p21x_hnd_t *h = (p21x_hnd_t *)ih;

    iip->flag = INPUT_INFO_FLAG_VIDEO;
    iip->rate = h->stream_info.dwRate;
    iip->scale = h->stream_info.dwScale;
    iip->n = h->stream_info.dwLength;
    iip->format = &h->dst_format;
    iip->format_size = sizeof(BITMAPINFOHEADER);

    return TRUE;
}


typedef struct {
    short y;
    short u;
    short v;
} PIXEL_YC;


typedef struct {
    WORD u;
    WORD v;
} uv_t;


int func_read_video(INPUT_HANDLE ih, int frame, void *buf)
{
    p21x_hnd_t *h = (p21x_hnd_t *)ih;
    LONG size;

    if (AVIStreamRead(h->stream, frame, 1, h->buff,
                      h->buff_size, &size, NULL)) {
        return 0;
    }

    int width = h->dst_format.biWidth;
    int height = h->dst_format.biHeight;
    PIXEL_YC *dst = (PIXEL_YC *)buf;
    WORD *Y = (WORD *)h->buff;
    uv_t *UV = (uv_t *)(Y + width * height);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            dst[x].y = (short)((((int)Y[x] - 4096) * 4789) >> 16);
        }
        int tmp = width >> 1;
        for (int x = 0; x < tmp; x++) {
            dst[x << 1].u = (short)((((int)UV[x].u - 32768) * 4683) >> 16);
            dst[x << 1].v = (short)((((int)UV[x].v - 32768) * 4683) >> 16);
        }
        tmp--;
        for (int x = 0; x < tmp; x++) {
            dst[(x << 1) + 1].u
                = (short)(((int)dst[x << 1].u + dst[(x << 1) + 2].u) >> 1);
            dst[(x << 1) + 1].v
                = (short)(((int)dst[x << 1].v + dst[(x << 1) + 2].v) >> 1);
        }
        dst[width - 1].u = dst[width - 2].u;
        dst[width - 1].v = dst[width - 2].v;
        
        dst += width;
        Y += width;
        UV += width >> 1;
    }

    return (int)h->dst_format.biSizeImage;
}
