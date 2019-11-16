/*------------------------------------------------------------------------
 *  Copyright 2007-2009 (c) Jeff Brown <spadix@users.sourceforge.net>
 *
 *  This file is part of the ZBar Bar Code Reader.
 *
 *  The ZBar Bar Code Reader is free software; you can redistribute it
 *  and/or modify it under the terms of the GNU Lesser Public License as
 *  published by the Free Software Foundation; either version 2.1 of
 *  the License, or (at your option) any later version.
 *
 *  The ZBar Bar Code Reader is distributed in the hope that it will be
 *  useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 *  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser Public License
 *  along with the ZBar Bar Code Reader; if not, write to the Free
 *  Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 *  Boston, MA  02110-1301  USA
 *
 *  http://sourceforge.net/projects/zbar
 *------------------------------------------------------------------------*/

#include <string.h>
#include <stdio.h>
#include "image.h"

/* pack bit size and location offset of a component into one byte
 */
#define RGB_BITS(off, size) ((((8 - (size)) & 0x7) << 5) | ((off) & 0x1f))

typedef void (conversion_handler_t)(zbar_image_t*,
                                    const zbar_format_def_t*,
                                    const zbar_image_t*,
                                    const zbar_format_def_t*);

typedef struct conversion_def_s {
    int cost;                           /* conversion "badness" */
    conversion_handler_t *func;         /* function that accomplishes it */
} conversion_def_t;


/* NULL terminated list of known formats, in order of preference
 * (NB Cr=V Cb=U)
 */
const uint32_t _zbar_formats[] = {

    /* planar YUV formats */
    fourcc('4','2','2','P'), /* FIXME also YV16? */
    fourcc('I','4','2','0'),
    fourcc('Y','U','1','2'), /* FIXME also IYUV? */
    fourcc('Y','V','1','2'),
    fourcc('4','1','1','P'),

    /* planar Y + packed UV plane */
    fourcc('N','V','1','2'),
    fourcc('N','V','2','1'),

    /* packed YUV formats */
    fourcc('Y','U','Y','V'),
    fourcc('U','Y','V','Y'),
    fourcc('Y','U','Y','2'), /* FIXME add YVYU */
    fourcc('Y','U','V','4'), /* FIXME where is this from? */

    /* packed rgb formats */
    fourcc('R','G','B','3'),
    fourcc( 3 , 0 , 0 , 0 ),
    fourcc('B','G','R','3'),
    fourcc('R','G','B','4'),
    fourcc('B','G','R','4'),

    fourcc('R','G','B','P'),
    fourcc('R','G','B','O'),
    fourcc('R','G','B','R'),
    fourcc('R','G','B','Q'),

    fourcc('Y','U','V','9'),
    fourcc('Y','V','U','9'),

    /* basic grayscale format */
    fourcc('G','R','E','Y'),
    fourcc('Y','8','0','0'),
    fourcc('Y','8',' ',' '),
    fourcc('Y','8', 0 , 0 ),

    /* low quality RGB formats */
    fourcc('R','G','B','1'),
    fourcc('R','4','4','4'),
    fourcc('B','A','8','1'),

    /* unsupported packed YUV formats */
    fourcc('Y','4','1','P'),
    fourcc('Y','4','4','4'),
    fourcc('Y','U','V','O'),
    fourcc('H','M','1','2'),

    /* unsupported packed RGB format */
    fourcc('H','I','2','4'),

    /* unsupported compressed formats */
    fourcc('J','P','E','G'),
    fourcc('M','J','P','G'),
    fourcc('M','P','E','G'),

    /* terminator */
    0
};

const int _zbar_num_formats = sizeof(_zbar_formats) / sizeof(uint32_t);

/* format definitions */
static const zbar_format_def_t format_defs[] = {

    { fourcc('R','G','B','4'), ZBAR_FMT_RGB_PACKED,
        { { 4, RGB_BITS(8, 8), RGB_BITS(16, 8), RGB_BITS(24, 8) } } },
    { fourcc('B','G','R','1'), ZBAR_FMT_RGB_PACKED,
        { { 1, RGB_BITS(0, 3), RGB_BITS(3, 3), RGB_BITS(6, 2) } } },
    { fourcc('4','2','2','P'), ZBAR_FMT_YUV_PLANAR, { { 1, 0, 0 /*UV*/ } } },
    { fourcc('Y','8','0','0'), ZBAR_FMT_GRAY, },
    { fourcc('Y','U','Y','2'), ZBAR_FMT_YUV_PACKED,
        { { 1, 0, 0, /*YUYV*/ } } },
    { fourcc('J','P','E','G'), ZBAR_FMT_JPEG, },
    { fourcc('Y','V','Y','U'), ZBAR_FMT_YUV_PACKED,
        { { 1, 0, 1, /*YVYU*/ } } },
    { fourcc('Y','8', 0 , 0 ), ZBAR_FMT_GRAY, },
    { fourcc('N','V','2','1'), ZBAR_FMT_YUV_NV,     { { 1, 1, 1 /*VU*/ } } },
    { fourcc('N','V','1','2'), ZBAR_FMT_YUV_NV,     { { 1, 1, 0 /*UV*/ } } },
    { fourcc('B','G','R','3'), ZBAR_FMT_RGB_PACKED,
        { { 3, RGB_BITS(16, 8), RGB_BITS(8, 8), RGB_BITS(0, 8) } } },
    { fourcc('Y','V','U','9'), ZBAR_FMT_YUV_PLANAR, { { 2, 2, 1 /*VU*/ } } },
    { fourcc('R','G','B','O'), ZBAR_FMT_RGB_PACKED,
        { { 2, RGB_BITS(10, 5), RGB_BITS(5, 5), RGB_BITS(0, 5) } } },
    { fourcc('R','G','B','Q'), ZBAR_FMT_RGB_PACKED,
        { { 2, RGB_BITS(2, 5), RGB_BITS(13, 5), RGB_BITS(8, 5) } } },
    { fourcc('G','R','E','Y'), ZBAR_FMT_GRAY, },
    { fourcc( 3 , 0 , 0 , 0 ), ZBAR_FMT_RGB_PACKED,
        { { 4, RGB_BITS(16, 8), RGB_BITS(8, 8), RGB_BITS(0, 8) } } },
    { fourcc('Y','8',' ',' '), ZBAR_FMT_GRAY, },
    { fourcc('I','4','2','0'), ZBAR_FMT_YUV_PLANAR, { { 1, 1, 0 /*UV*/ } } },
    { fourcc('R','G','B','1'), ZBAR_FMT_RGB_PACKED,
        { { 1, RGB_BITS(5, 3), RGB_BITS(2, 3), RGB_BITS(0, 2) } } },
    { fourcc('Y','U','1','2'), ZBAR_FMT_YUV_PLANAR, { { 1, 1, 0 /*UV*/ } } },
    { fourcc('Y','V','1','2'), ZBAR_FMT_YUV_PLANAR, { { 1, 1, 1 /*VU*/ } } },
    { fourcc('R','G','B','3'), ZBAR_FMT_RGB_PACKED,
        { { 3, RGB_BITS(0, 8), RGB_BITS(8, 8), RGB_BITS(16, 8) } } },
    { fourcc('R','4','4','4'), ZBAR_FMT_RGB_PACKED,
        { { 2, RGB_BITS(8, 4), RGB_BITS(4, 4), RGB_BITS(0, 4) } } },
    { fourcc('B','G','R','4'), ZBAR_FMT_RGB_PACKED,
        { { 4, RGB_BITS(16, 8), RGB_BITS(8, 8), RGB_BITS(0, 8) } } },
    { fourcc('Y','U','V','9'), ZBAR_FMT_YUV_PLANAR, { { 2, 2, 0 /*UV*/ } } },
    { fourcc('M','J','P','G'), ZBAR_FMT_JPEG, },
    { fourcc('4','1','1','P'), ZBAR_FMT_YUV_PLANAR, { { 2, 0, 0 /*UV*/ } } },
    { fourcc('R','G','B','P'), ZBAR_FMT_RGB_PACKED,
        { { 2, RGB_BITS(11, 5), RGB_BITS(5, 6), RGB_BITS(0, 5) } } },
    { fourcc('R','G','B','R'), ZBAR_FMT_RGB_PACKED,
        { { 2, RGB_BITS(3, 5), RGB_BITS(13, 6), RGB_BITS(8, 5) } } },
    { fourcc('Y','U','Y','V'), ZBAR_FMT_YUV_PACKED,
        { { 1, 0, 0, /*YUYV*/ } } },
    { fourcc('U','Y','V','Y'), ZBAR_FMT_YUV_PACKED,
        { { 1, 0, 2, /*UYVY*/ } } },
};

static const int num_format_defs =
    sizeof(format_defs) / sizeof(zbar_format_def_t);

#ifdef DEBUG_CONVERT
static int intsort (const void *a,
                    const void *b)
{
    return(*(uint32_t*)a - *(uint32_t*)b);
}
#endif


static inline void uv_round (zbar_image_t *img,
                             const zbar_format_def_t *fmt)
{
    img->width >>= fmt->p.yuv.xsub2;
    img->width <<= fmt->p.yuv.xsub2;
    img->height >>= fmt->p.yuv.ysub2;
    img->height <<= fmt->p.yuv.ysub2;
}

static inline void uv_roundup (zbar_image_t *img,
                               const zbar_format_def_t *fmt)
{
    if(fmt->group == ZBAR_FMT_GRAY)
        return;
    unsigned xmask = (1 << fmt->p.yuv.xsub2) - 1;
    if(img->width & xmask)
        img->width = (img->width + xmask) & ~xmask;
    unsigned ymask = (1 << fmt->p.yuv.ysub2) - 1;
    if(img->height & ymask)
        img->height = (img->height + ymask) & ~ymask;
}

static inline unsigned long uvp_size (const zbar_image_t *img,
                                      const zbar_format_def_t *fmt)
{
    if(fmt->group == ZBAR_FMT_GRAY)
        return(0);
    return((img->width >> fmt->p.yuv.xsub2) *
           (img->height >> fmt->p.yuv.ysub2));
}

static inline uint32_t convert_read_rgb (const uint8_t *srcp,
                                         int bpp)
{
    uint32_t p;
    if(bpp == 3) {
        p = *srcp;
        p |= *(srcp + 1) << 8;
        p |= *(srcp + 2) << 16;
    }
    else if(bpp == 4)
        p = *((uint32_t*)(srcp));
    else if(bpp == 2)
        p = *((uint16_t*)(srcp));
    else
        p = *srcp;
    return(p);
}

static inline void convert_write_rgb (uint8_t *dstp,
                                      uint32_t p,
                                      int bpp)
{
    if(bpp == 3) {
        *dstp = p & 0xff;
        *(dstp + 1) = (p >> 8) & 0xff;
        *(dstp + 2) = (p >> 16) & 0xff;
    }
    else if(bpp == 4)
        *((uint32_t*)dstp) = p;
    else if(bpp == 2)
        *((uint16_t*)dstp) = p;
    else
        *dstp = p;
}

/* cleanup linked image by unrefing */
static void cleanup_ref (zbar_image_t *img)
{
    if(img->next)
        _zbar_image_refcnt(img->next, -1);
}

const zbar_format_def_t *_zbar_format_lookup (uint32_t fmt)
{
    const zbar_format_def_t *def = NULL;
    int i = 0;
    while(i < num_format_defs) {
        def = &format_defs[i];
        if(fmt == def->format)
            return(def);
        i = i * 2 + 1;
        if(fmt > def->format)
            i++;
    }
    return(NULL);
}

static inline int has_format (uint32_t fmt,
                              const uint32_t *fmts)
{
    for(; *fmts; fmts++)
        if(*fmts == fmt)
            return(1);
    return(0);
}
