/*
** geoconform.c
**
** Public Domain Mark 1.0
** No Copyright
**
** Depends:
** 1) lm
*/

#include <inttypes.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <memory.h>

#ifndef GEOCONFORM_H
#define GEOCONFORM_H

#ifdef __cplusplus
extern "C" {
#endif

#define MAXVAL 256
#ifndef PI
#define PI  ((float)3.14159265358979323846264338327950288419716939937510)
#endif
#ifndef RADGRD
#define RADGRD ((float)57.295779513082320876798154814105)
#endif
#ifndef ABS
#define ABS(a)    ((a) < 0 ? (-(a)) : (a))
#endif
#ifndef MIN
#define MIN(a,b)  ((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a,b)  ((a) > (b) ? (a) : (b))
#endif
#ifndef TRIM
#define TRIM(x,a,b) (MIN(FA_MAX((x),(a)),(b)))
#endif
#define COUNTA 20
#define COUNTC 3
#define COUNTG 10
#define COUNTM 0

    typedef uint8_t BYTE;
    typedef uint16_t WORD;

    typedef struct
    {
        BYTE c[COUNTC];
        WORD s;
    }
    IMTpixel;

    typedef struct
    {
        unsigned width, height;
    }
    IMTsize;

    typedef struct
    {
        unsigned bits;
        IMTsize size;
        IMTpixel **p;
        BYTE **b;
    }
    IMTimage;

    typedef struct
    {
        float x, y;
    }
    GCIcoord;

    typedef struct
    {
        GCIcoord p[4];
        GCIcoord min, max, mean;
    }
    GCIrect;

    typedef struct
    {
        unsigned na;
        double a[COUNTA];
    }
    GCIctrans;

    typedef struct
    {
        IMTsize size1, size2;
        GCIctrans trans;
        GCIrect rect1, rect2;
        float m, mi;
        int iters;
        int margin;
    }
    GCIparams;

////////////////////////////////////////////////////////////////////////////////

    BYTE ByteClamp(int);
    WORD Byte3Clamp(int);
    unsigned IndexClamp(int, unsigned);
    IMTpixel IMTset (BYTE, BYTE, BYTE);
    IMTpixel IMTcalcS (IMTpixel);
    IMTimage IMTalloc (IMTsize, int);
    IMTimage IMTfree (IMTimage);
    IMTpixel IMTInterpolateBiCubic (IMTimage, GCIcoord);
    IMTpixel IMTInterpolateBiLine (IMTimage, GCIcoord);
    GCIcoord GCIconformaltrans(GCIctrans, GCIcoord);
    GCIparams GCIcalcallparams(GCIparams);
    IMTimage IMTFilterGeoConform (IMTimage, IMTimage, GCIparams);

#ifdef __cplusplus
}
#endif

#endif // GEOCONFORM_H
