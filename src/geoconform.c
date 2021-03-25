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
        for (y = 0; y < size.height; y++)
        {
            im.p[y] = (IMTpixel*)malloc (size.width * sizeof(IMTpixel));
        }
        im.size.height = size.height;
        im.size.width = size.width;
        im.bits += 24;
        bs -= 24;
    }
    if (bs > 0)
    {
        im.b = (BYTE**)malloc(size.height * sizeof(BYTE*));
        for (y = 0; y < size.height; y++)
        {
            im.b[y] = (BYTE*)malloc(size.width * sizeof(BYTE));
        }
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
        for (y = 0; y < im.size.height; y++)
        {
            free(im.p[y]);
        }
        free(im.p);
        im.bits -= 24;
    }
    if (im.bits > 0)
    {
        for (y = 0; y < im.size.height; y++)
        {
            free(im.b[y]);
        }
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

    y1 = IndexClamp((int)p.x, (p_im.size.height - 1));
    x1 = IndexClamp((int)p.y, (p_im.size.width - 1));
    y2 = IndexClamp((int)(y1 + 1), (p_im.size.height - 1));
    x2 = IndexClamp((int)(x1 + 1), (p_im.size.width - 1));
    ky = p.x - y1;
    if (ky < 0)
    {
        ky = 0.0;
    }
    if (ky > 1)
    {
        ky = 1.0;
    }
    kx = p.y - x1;
    if (kx < 0)
    {
        kx = 0.0;
    }
    if (kx > 1)
    {
        kx = 1.0;
    }
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

GCIcoord GCIconformaltrans(GCIctrans trans, GCIcoord p)
{
    unsigned i, i0, i1, n;
    GCIcoord c, pq, pqn;
    pqn.x = 1.0f;
    pqn.y = 0.0f;
    n = (trans.na + 1) / 2;
    c.x = 0.0f;
    c.y = 0.0f;
    for (i = 0; i < n; i++)
    {
        pq = pqn;
        i0 = i * 2;
        i1 = i0 + 1;
        c.x += trans.a[i0] * pq.x - trans.a[i1] * pq.y;
        c.y += trans.a[i0] * pq.y + trans.a[i1] * pq.x;
        pqn.x = p.x * pq.x - p.y * pq.y;
        pqn.y = p.x * pq.y + p.y * pq.x;
    }

    return c;
}

////////////////////////////////////////////////////////////////////////////////

GCIcoord GCIconformaltransnewton(GCIctrans trans, GCIcoord p, GCIcoord t)
{
    unsigned i, i0, i1, n;
    float l, ldx, ldy;
    GCIcoord c, cdx, cdy, pq, pqn, dxpq, dxpqn, dypq, dypqn, dt, gt, gx, gy, gl, g;
    pqn.x = 1.0f;
    pqn.y = 0.0f;
    dxpqn.x = 0.0f;
    dxpqn.y = 0.0f;
    dypqn.x = 0.0f;
    dypqn.y = 0.0f;
    n = (trans.na + 1) / 2;
    c.x = 0.0f;
    c.y = 0.0f;
    cdx.x = 0.0f;
    cdx.y = 0.0f;
    cdy.x = 0.0f;
    cdy.y = 0.0f;
    for (i = 0; i < n; i++)
    {
        pq = pqn;
        dxpq = dxpqn;
        dypq = dypqn;
        i0 = i * 2;
        i1 = i0 + 1;
        c.x += trans.a[i0] * pq.x - trans.a[i1] * pq.y;
        c.y += trans.a[i0] * pq.y + trans.a[i1] * pq.x;
        cdx.x += trans.a[i0] * dxpq.x - trans.a[i1] * dxpq.y;
        cdx.y += trans.a[i0] * dxpq.y + trans.a[i1] * dxpq.x;
        cdy.x += trans.a[i0] * dypq.x - trans.a[i1] * dypq.y;
        cdy.y += trans.a[i0] * dypq.y + trans.a[i1] * dypq.x;
        pqn.x = p.x * pq.x - p.y * pq.y;
        pqn.y = p.x * pq.y + p.y * pq.x;
        dxpqn.x = p.x * dxpq.x - p.y * dxpq.y + pq.x;
        dxpqn.y = p.x * dxpq.y + p.y * dxpq.x + pq.y;
        dypqn.x = p.x * dypq.x - p.y * dypq.y - pq.y;
        dypqn.y = p.x * dypq.y + p.y * dypq.x + pq.x;
    }
    dt.x = c.x - t.x;
    dt.y = c.y - t.y;
    l = sqrt(dt.x * dt.x + dt.y * dt.y);
    l = (l > 0.0f) ? l : 1.0f;
    gt.x = dt.x / l;
    gt.y = dt.y / l;
    ldx = sqrt(cdx.x * cdx.x + cdx.y * cdx.y);
    ldx = (ldx > 0.0f) ? ldx : 1.0f;
    gx.x = cdx.x / ldx;
    gx.y = cdx.y / ldx;
    ldy = sqrt(cdy.x * cdy.x + cdy.y * cdy.y);
    ldy = (ldy > 0.0f) ? ldy : 1.0f;
    gy.x = cdy.x / ldy;
    gy.y = cdy.y / ldy;
    gl.x = l / ldx;
    gl.y = l / ldy;
    g.x = gl.x * (gt.x * gx.x + gt.y * gx.y) / 2;
    g.y = gl.y * (gt.x * gy.x + gt.y * gy.y) / 2;
    c.x = p.x - g.x;
    c.y = p.y - g.y;

    return c;
}

////////////////////////////////////////////////////////////////////////////////

GCIparams GCIcalcallparams(GCIparams params)
{
    unsigned i;
    int h, w;
    GCIcoord cmin1, cmax1, cmean1, cd1;
    GCIcoord cmin2, cmax2, cmean2, cd2;
    float areai, areac, aream;

    params.rect1.p[1].x = params.rect1.p[0].x;
    params.rect1.p[1].y = params.rect1.p[2].y;
    params.rect1.p[3].x = params.rect1.p[2].x;
    params.rect1.p[3].y = params.rect1.p[0].y;
    for (i = params.trans.na; i < COUNTA; i++) params.trans.a[i] = 0.0f;
    cmin1 = params.rect1.p[0];
    cmax1 = params.rect1.p[0];
    cmean1.x = 0;
    cmean1.y = 0;
    for (i = 0; i < 4; i++)
    {
        cmean1.x += params.rect1.p[i].x;
        cmean1.y += params.rect1.p[i].y;
        if (params.rect1.p[i].x < cmin1.x) cmin1.x = params.rect1.p[i].x;
        if (params.rect1.p[i].y < cmin1.y) cmin1.y = params.rect1.p[i].y;
        if (params.rect1.p[i].x > cmax1.x) cmax1.x = params.rect1.p[i].x;
        if (params.rect1.p[i].y > cmax1.y) cmax1.y = params.rect1.p[i].y;
    }
    cmean1.x *= 0.25f;
    cmean1.y *= 0.25f;
    cd1.x = cmax1.x - cmin1.x;
    cd1.y = cmax1.y - cmin1.y;
    cd1.x = (cd1.x > 0) ? cd1.x : 1;
    cd1.y = (cd1.y > 0) ? cd1.y : 1;
    params.rect1.mean = cmean1;
    areac = cd1.x * cd1.y;
    areai = params.size1.height * params.size1.width;
    aream = (areac > 0) ? sqrt((float)areai / areac) : 1;
    params.m = aream;
    aream = (areai > 0) ? sqrt(areac / areai) : 1;
    params.mi = aream;
    cmin1.x = cmean1.x - 0.5f * params.size1.height * params.mi;
    cmin1.y = cmean1.y - 0.5f * params.size1.width * params.mi;
    cmax1.x = cmean1.x + 0.5f * params.size1.height * params.mi;
    cmax1.y = cmean1.y + 0.5f * params.size1.width * params.mi;
    cd1.x = cmax1.x - cmin1.x;
    cd1.y = cmax1.y - cmin1.y;
    cd1.x = (cd1.x > 0) ? cd1.x : 1;
    cd1.y = (cd1.y > 0) ? cd1.y : 1;
    params.rect1.min = cmin1;
    params.rect1.max = cmax1;
    params.rect1.p[0] = cmin1;
    params.rect1.p[2] = cmax1;
    params.rect1.p[1].x = params.rect1.p[0].x;
    params.rect1.p[1].y = params.rect1.p[2].y;
    params.rect1.p[3].x = params.rect1.p[2].x;
    params.rect1.p[3].y = params.rect1.p[0].y;
    for (i = 0; i < 4; i++)
    {
        params.rect2.p[i] = GCIconformaltrans(params.trans, params.rect1.p[i]);
    }
    cmin2 = params.rect2.p[0];
    cmax2 = params.rect2.p[0];
    cmean2.x = 0;
    cmean2.y = 0;
    for (i = 0; i < 4; i++)
    {
        cmean2.x += params.rect2.p[i].x;
        cmean2.y += params.rect2.p[i].y;
        if (params.rect2.p[i].x < cmin2.x) cmin2.x = params.rect2.p[i].x;
        if (params.rect2.p[i].y < cmin2.y) cmin2.y = params.rect2.p[i].y;
        if (params.rect2.p[i].x > cmax2.x) cmax2.x = params.rect2.p[i].x;
        if (params.rect2.p[i].y > cmax2.y) cmax2.y = params.rect2.p[i].y;
    }
    cmean2.x *= 0.25f;
    cmean2.y *= 0.25f;
    cd2.x = cmax2.x - cmin2.x;
    cd2.y = cmax2.y - cmin2.y;
    params.rect2.min = cmin2;
    params.rect2.max = cmax2;
    params.rect2.mean = cmean2;
    h = (int)(cd2.x * params.m + 0.5f);
    w = (int)(cd2.y * params.m + 0.5f);
    params.size2.height = (h > 0) ? h : 1;
    params.size2.width = (w > 0) ? w : 1;

    return params;
}

////////////////////////////////////////////////////////////////////////////////

IMTimage IMTFilterGeoConform (IMTimage p_im, IMTimage d_im, GCIparams params)
{
    unsigned i, j, k;
    GCIcoord ct, cf;

    for (i = 0; i < d_im.size.height; i++)
    {
        ct.x = params.rect2.min.x + (0.5f + i) * params.mi;
        for (j = 0; j < d_im.size.width; j++)
        {
            ct.y = params.rect2.min.y + (0.5f + j) * params.mi;
            cf = params.rect1.mean;
            for (k = 0; k < params.iters; k++)
            {
                cf = GCIconformaltransnewton(params.trans, cf, ct);
            }
            cf.x -= params.rect1.min.x;
            cf.y -= params.rect1.min.y;
            cf.x *= params.m;
            cf.y *= params.m;
            if (cf.x < 0 || cf.y < 0 || cf.x >= params.size1.height || cf.y >= params.size1.width)
            {
                d_im.p[i][j] = IMTset(0, 0, 0);
            }
            else
            {
                d_im.p[i][j] = IMTinterpolation(p_im, cf);
            }
        }
    }
    return d_im;
}

////////////////////////////////////////////////////////////////////////////////
