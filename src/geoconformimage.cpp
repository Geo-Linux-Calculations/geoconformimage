/*
** geoconform.c
**
** Public Domain Mark 1.0
** No Copyright
**
** Depends:
** 1) lm
** 2) freeimage
*/

#include <unistd.h>
#include <FreeImage.h>
#include "geoconformfreeimage.h"

////////////////////////////////////////////////////////////////////////////////

void GeoConformImageTitle()
{
    printf("GeoConformImage.\n");
    printf("https://github.com/Geo-Linux-Calculations/geoconformimage/.\n\n");
}

////////////////////////////////////////////////////////////////////////////////

void GeoConformImageUsage(const char *progname)
{
    printf("Usage : %s [options] <input_image> <output_image>\n", progname);
    printf("options:\n");
    printf("    -g N    grid set (default = 5)\n");
    printf("    -p str  string confom params: \"A0,B0,A1,B1,[...,A5,B5]\"\n");
    printf("    -r str  string region image: \"Xws,Yws,Xne,Yne\"\n");
    printf("    -h      this help\n");
}

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    // call this ONLY when linking with FreeImage as a static library
#ifdef FREEIMAGE_LIB
    FreeImage_Initialise();
#endif // FREEIMAGE_LIB

    int opt, nk = 0;
    GCIparams params;
    IMTimage imgin, imgout;
    bool fhelp = false;
    while ((opt = getopt(argc, argv, ":g:p:r:h")) != -1)
    {
        switch(opt)
        {
            case 'g':
                params.ng = atoi(optarg);
                break;
            case 'p':
                params.na = sscanf(optarg, "%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf", &params.a[0], &params.a[1], &params.a[2], &params.a[3], &params.a[4], &params.a[5], &params.a[6], &params.a[7], &params.a[8], &params.a[9], &params.a[10], &params.a[11]);
                if (params.na < 4)
                {
                    printf("Error! Bad parameters: %s (%d < %d).\n", optarg, params.na, 4);
                    exit(1);
                }
                params.na /= 2;
                params.na *= 2;
                printf("DEBUG: %s np: %d.\n", optarg, params.na);
                break;
            case 'r':
                nk = sscanf(optarg, "%lf,%lf,%lf,%lf", &params.rect1.p[0].x, &params.rect1.p[0].y, &params.rect1.p[2].x, &params.rect1.p[2].y);
                if (nk < 4)
                {
                    printf("Error! Bad region: %s (%d < %d).\n", optarg, nk, 4);
                    exit(1);
                }
                params.rect1.p[1].x = params.rect1.p[0].x;
                params.rect1.p[1].y = params.rect1.p[2].y;
                params.rect1.p[3].x = params.rect1.p[2].x;
                params.rect1.p[3].y = params.rect1.p[0].y;
                break;
            case 'h':
                fhelp = true;
                break;
            case ':':
                printf("option needs a value\n");
                break;
            case '?':
                printf("unknown option: %c\n", optopt);
                break;
        }
    }

    GeoConformImageTitle();

    if (optind + 2 > argc || params.na < 4 || nk < 4 || fhelp)
    {
        GeoConformImageUsage(argv[0]);
        return 0;
    }
    const char *src_filename = argv[optind];
    const char *output_filename = argv[optind + 1];

    FreeImage_SetOutputMessage(FreeImageErrorHandler);

    printf("Input= %s\n", src_filename);
    FIBITMAP *dib = ImthresholdGenericLoader(src_filename, 0);
    if (dib)
    {
        if (FreeImage_GetImageType(dib) == FIT_BITMAP)
        {
            FIBITMAP* dst_dib;
            imgin.size.width = FreeImage_GetWidth(dib);
            imgin.size.height = FreeImage_GetHeight(dib);
            params.size1 = imgin.size;
            IMTimage p_im = IMTalloc(imgin.size, 24);
            p_im = ImthresholdGetData(dib, p_im);
            FreeImage_Unload(dib);
            params = GCIcalcallparams(params);
            params.size2 = imgin.size;
            IMTimage d_im = IMTalloc(params.size2, 24);

            IMTFilterGeoConform(p_im, d_im, params);
            p_im = IMTfree(p_im);
            dst_dib = FreeImage_Allocate(d_im.size.width, d_im.size.height, 24);
            ImthresholdSetData(dst_dib, d_im);
            d_im = IMTfree(d_im);

            if (dst_dib)
            {
                FREE_IMAGE_FORMAT out_fif = FreeImage_GetFIFFromFilename(output_filename);
                if(out_fif != FIF_UNKNOWN)
                {
                    FreeImage_Save(out_fif, dst_dib, output_filename, 0);
                    printf("Output= %s\n\n", output_filename);
                }
                FreeImage_Unload(dst_dib);
            }
        } else {
            printf("%s\n", "Unsupported format type.");
            FreeImage_Unload(dib);
        }
    }

    // call this ONLY when linking with FreeImage as a static library
#ifdef FREEIMAGE_LIB
    FreeImage_DeInitialise();
#endif // FREEIMAGE_LIB

    return 0;
}
