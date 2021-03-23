/*
** geoconform.c
**
** Public Domain Mark 1.0
** No Copyright
**
** Depends:
** 1) lm
*/

#include "geoconform.h"

////////////////////////////////////////////////////////////////////////////////

BYTE ByteClamp(int c)
{
    BYTE buff[3] = {(BYTE)c, 255, 0};
    return buff[ (int)(c < 0) + (int)((unsigned)c > 255) ];
}

////////////////////////////////////////////////////////////////////////////////

WORD Byte3Clamp(int c)
{
    WORD buff[3] = {(WORD)c, 765, 0};
    return buff[ (int)(c < 0) + (int)((unsigned)c > 765) ];
}

////////////////////////////////////////////////////////////////////////////////

unsigned IndexClamp(int i, unsigned threshold)
{
    unsigned buff[3] = {(unsigned)i, threshold, 0};
    return buff[ (int)(i < 0) + (int)((unsigned)i > threshold) ];
}

////////////////////////////////////////////////////////////////////////////////

IMTpixel IMTset (BYTE c0, BYTE c1, BYTE c2)
{
    IMTpixel im;

    im.c[0] = (BYTE)c0;
    im.c[1] = (BYTE)c1;
    im.c[2] = (BYTE)c2;
    im.s = (WORD)c0 + (WORD)c1 + (WORD)c2;

    return im;
}

////////////////////////////////////////////////////////////////////////////////

IMTpixel IMTcalcS (IMTpixel im)
{
    unsigned ims, d;

    ims = 0;
    for (d = 0; d < 3; d++)
    {
        ims += (unsigned)im.c[d];
    }
    im.s = (WORD)ims;

    return im;
}

////////////////////////////////////////////////////////////////////////////////

IMTimage IMTalloc (IMTsize size, int bs)
{
    IMTimage im;
    unsigned y;

    im.bits = 0;
    if (bs > 1)
    {
        im.p = (IMTpixel**)malloc (size.height * sizeof(IMTpixel*));
        for (y = 0; y < size.height; y++) {im.p[y] = (IMTpixel*)malloc (size.width * sizeof(IMTpixel));}
        im.size.height = size.height;
        im.size.width = size.width;
        im.bits += 24;
        bs -= 24;
    }
    if (bs > 0)
    {
        im.b = (BYTE**)malloc(size.height * sizeof(BYTE*));
        for (y = 0; y < size.height; y++) {im.b[y] = (BYTE*)malloc(size.width * sizeof(BYTE));}
        im.size.height = size.height;
        im.size.width = size.width;
        im.bits += 1;
    }

    return im;
}

////////////////////////////////////////////////////////////////////////////////

IMTimage IMTfree (IMTimage im)
{
    unsigned y;

    if (im.bits > 1)
    {
        for (y = 0; y < im.size.height; y++) {free(im.p[y]);}
        free(im.p);
        im.bits -= 24;
    }
    if (im.bits > 0)
    {
        for (y = 0; y < im.size.height; y++) {free(im.b[y]);}
        free(im.b);
        im.bits = 0;
    }

    return im;  
}

////////////////////////////////////////////////////////////////////////////////

IMTpixel IMTinterpolation (IMTimage p_im, GCIcoord p)
{
    unsigned d, y1, x1, y2, x2;
    float p11, p21, p12, p22, ky, kx, k11, k21, k12, k22, t;
    IMTpixel res;

    y1 = IndexClamp((int)p.y, (p_im.size.height - 1));
    x1 = IndexClamp((int)p.x, (p_im.size.width - 1));
    y2 = IndexClamp((int)(y1 + 1), (p_im.size.height - 1));
    x2 = IndexClamp((int)(x1 + 1), (p_im.size.width - 1));
    ky = p.y - y1;
    if (ky < 0) {ky = 0.0;}
    if (ky > 1) {ky = 1.0;}
    kx = p.x - x1;
    if (kx < 0) {kx = 0.0;}
    if (kx > 1) {kx = 1.0;}
    k11 = (1.0 - ky) * (1.0 - kx);
    k21 = ky * (1.0 - kx);
    k12 = (1.0 - ky) * kx;
    k22 = ky * kx;
    for (d = 0; d < 3; d++)
    {
        p11 = (float)p_im.p[y1][x1].c[d];
        p21 = (float)p_im.p[y2][x1].c[d];
        p12 = (float)p_im.p[y1][x2].c[d];
        p22 = (float)p_im.p[y2][x2].c[d];
        t = p11 * k11 + p21 * k21 + p12 * k12 + p22 * k22;
        res.c[d] = ByteClamp((int)(t + 0.5));
    }
    res = IMTcalcS (res);

    return res;
}

////////////////////////////////////////////////////////////////////////////////

GCIparams GCIcalcallparams(GCIparams params)
{
    return params;
}

////////////////////////////////////////////////////////////////////////////////

IMTimage IMTFilterGeoConform (IMTimage p_im, IMTimage d_im, GCIparams params)
{
    unsigned y, x;
    for (y = 0; y < p_im.size.height; y++)
        for (x = 0; x < p_im.size.width; x++)
            d_im.p[y][x] = p_im.p[y][x];
            
    return d_im;
}

////////////////////////////////////////////////////////////////////////////////
