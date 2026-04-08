/*
 * glaux_image.c - Implementations of auxDIBImageLoad and auxRGBImageLoad
 *
 * MinGW's glaux.h declares these but provides no library with their
 * implementations.  We provide our own here.
 *
 * auxDIBImageLoad  - loads a Windows BMP file (any bit-depth)
 * auxRGBImageLoad  - loads an SGI RGB/RGBA file (verbatim or RLE)
 *
 * Both return an AUX_RGBImageRec* where .data is packed RGB, 3 bytes/pixel,
 * row-major, bottom-row first (OpenGL convention).  Caller frees the struct
 * (but NOT .data – ProcessTkTexture hands .data to OpenGL and frees the
 * struct itself).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <GL/gl.h>
#include <GL/glaux.h>

/* =========================================================================
 * BMP loader  (auxDIBImageLoadA / auxDIBImageLoadW)
 * ========================================================================= */

AUX_RGBImageRec * APIENTRY auxDIBImageLoadA(LPCSTR filename)
{
    FILE *f;
    BITMAPFILEHEADER bfh;
    BITMAPINFOHEADER bih;
    AUX_RGBImageRec *image;
    unsigned char *row;
    int i, j, width, height, bpp, stride;
    int flipped;          /* 1 = BMP stored bottom-up (normal) */

    f = fopen(filename, "rb");
    if (!f) return NULL;

    /* --- file header --- */
    if (fread(&bfh, sizeof(BITMAPFILEHEADER), 1, f) != 1) goto fail;
    if (bfh.bfType != 0x4D42) goto fail;   /* "BM" */

    /* --- info header --- */
    if (fread(&bih, sizeof(BITMAPINFOHEADER), 1, f) != 1) goto fail;

    width  = bih.biWidth;
    height = (int)bih.biHeight;
    bpp    = bih.biBitCount;
    flipped = (height > 0);
    if (!flipped) height = -height;

    /* We handle 8, 24, 32 bpp.  8-bpp requires palette lookup. */
    if (bpp != 8 && bpp != 24 && bpp != 32) goto fail;

    /* --- allocate output --- */
    image = (AUX_RGBImageRec *)malloc(sizeof(AUX_RGBImageRec));
    if (!image) goto fail;
    image->sizeX = (GLint)width;
    image->sizeY = (GLint)height;
    image->data  = (unsigned char *)malloc((size_t)(width * height * 3));
    if (!image->data) { free(image); goto fail; }

    /* stride: each BMP row is padded to a 4-byte boundary */
    stride = ((width * (bpp / 8) + 3) & ~3);
    row    = (unsigned char *)malloc((size_t)stride);
    if (!row) { free(image->data); free(image); goto fail; }

    /* --- palette (for 8-bpp) --- */
    RGBQUAD palette[256];
    memset(palette, 0, sizeof(palette));
    if (bpp == 8) {
        int ncolors = bih.biClrUsed ? (int)bih.biClrUsed : 256;
        fread(palette, sizeof(RGBQUAD), (size_t)ncolors, f);
    }

    fseek(f, (long)bfh.bfOffBits, SEEK_SET);

    for (i = 0; i < height; i++) {
        /* OpenGL wants bottom-row first; BMP normal orientation is bottom-up */
        int destRow = flipped ? (height - 1 - i) : i;
        unsigned char *dest = image->data + destRow * width * 3;

        if (fread(row, 1, (size_t)stride, f) != (size_t)stride) break;

        for (j = 0; j < width; j++) {
            if (bpp == 8) {
                RGBQUAD *q = &palette[row[j]];
                dest[j*3 + 0] = q->rgbRed;
                dest[j*3 + 1] = q->rgbGreen;
                dest[j*3 + 2] = q->rgbBlue;
            } else {
                /* 24 or 32-bpp: stored as BGR[A] */
                dest[j*3 + 0] = row[j*(bpp/8) + 2]; /* R */
                dest[j*3 + 1] = row[j*(bpp/8) + 1]; /* G */
                dest[j*3 + 2] = row[j*(bpp/8) + 0]; /* B */
            }
        }
    }

    free(row);
    fclose(f);
    return image;

fail:
    fclose(f);
    return NULL;
}

AUX_RGBImageRec * APIENTRY auxDIBImageLoadW(LPCWSTR filename)
{
    /* Convert wide string to narrow and delegate */
    char narrow[MAX_PATH];
    if (!WideCharToMultiByte(CP_ACP, 0, filename, -1,
                             narrow, MAX_PATH, NULL, NULL))
        return NULL;
    return auxDIBImageLoadA(narrow);
}

/* =========================================================================
 * SGI RGB loader  (auxRGBImageLoadA / auxRGBImageLoadW)
 *
 * Format reference: http://paulbourke.net/dataformats/sgirgb/sgiversion.html
 * ========================================================================= */

#define SGI_MAGIC      0x01DA
#define SGI_VERBATIM   0
#define SGI_RLE        1

/* Big-endian reads (SGI files are big-endian) */
static unsigned short read_be16(FILE *f)
{
    unsigned char b[2];
    fread(b, 1, 2, f);
    return (unsigned short)((b[0] << 8) | b[1]);
}
static unsigned int read_be32(FILE *f)
{
    unsigned char b[4];
    fread(b, 1, 4, f);
    return ((unsigned int)b[0] << 24) | ((unsigned int)b[1] << 16) |
           ((unsigned int)b[2] <<  8) |  (unsigned int)b[3];
}

AUX_RGBImageRec * APIENTRY auxRGBImageLoadA(LPCSTR filename)
{
    FILE *f;
    unsigned short magic, storage, dimension, xsize, ysize, zsize;
    AUX_RGBImageRec *image;
    unsigned char *rowbuf;
    int c, row;
    long dataStart;

    f = fopen(filename, "rb");
    if (!f) return NULL;

    magic   = read_be16(f);
    if (magic != SGI_MAGIC) { fclose(f); return NULL; }
    storage = (unsigned short)fgetc(f);  /* 0=verbatim, 1=RLE */
    fgetc(f);                             /* bytes-per-channel (ignore, assume 1) */
    dimension = read_be16(f);
    xsize     = read_be16(f);
    ysize     = (dimension >= 2) ? read_be16(f) : 1;
    zsize     = (dimension >= 3) ? read_be16(f) : 1;
    (void)read_be32(f);  /* pinmin */
    (void)read_be32(f);  /* pinmax */
    (void)read_be32(f);  /* dummy */
    /* skip imagename (80 bytes) and colormap (4 bytes) and padding (404 bytes) */
    fseek(f, 512, SEEK_SET);
    dataStart = 512;

    image = (AUX_RGBImageRec *)malloc(sizeof(AUX_RGBImageRec));
    if (!image) { fclose(f); return NULL; }
    image->sizeX = (GLint)xsize;
    image->sizeY = (GLint)ysize;
    image->data  = (unsigned char *)calloc((size_t)(xsize * ysize * 3), 1);
    if (!image->data) { free(image); fclose(f); return NULL; }

    rowbuf = (unsigned char *)malloc((size_t)xsize * 2 + 16);
    if (!rowbuf) { free(image->data); free(image); fclose(f); return NULL; }

    if (storage == SGI_VERBATIM) {
        /* channels stored sequentially: all-R rows, then all-G, then all-B */
        for (c = 0; c < 3 && c < (int)zsize; c++) {
            for (row = 0; row < (int)ysize; row++) {
                unsigned char *dest = image->data + row * xsize * 3 + c;
                if (fread(rowbuf, 1, (size_t)xsize, f) != (size_t)xsize) goto done;
                for (int x = 0; x < (int)xsize; x++)
                    dest[x * 3] = rowbuf[x];
            }
        }
    } else {
        /* RLE: offset and length tables come first */
        int nrows  = (int)ysize * (int)zsize;
        long *offsets = (long *)malloc((size_t)nrows * sizeof(long));
        int  *lengths = (int  *)malloc((size_t)nrows * sizeof(int));
        if (!offsets || !lengths) {
            free(offsets); free(lengths);
            goto done;
        }
        for (int r = 0; r < nrows; r++)
            offsets[r] = (long)read_be32(f);
        for (int r = 0; r < nrows; r++)
            lengths[r] = (int)read_be32(f);

        for (c = 0; c < 3 && c < (int)zsize; c++) {
            for (row = 0; row < (int)ysize; row++) {
                int tableIdx = c * (int)ysize + row;
                int rleLen   = lengths[tableIdx];
                unsigned char *dest = image->data + row * xsize * 3 + c;
                int x = 0;

                fseek(f, offsets[tableIdx], SEEK_SET);
                /* read up to rleLen bytes of RLE data */
                unsigned char *rle = (unsigned char *)malloc((size_t)rleLen);
                if (!rle) continue;
                fread(rle, 1, (size_t)rleLen, f);

                int p = 0;
                while (p < rleLen && x < (int)xsize) {
                    unsigned char pkt = rle[p++];
                    int count = pkt & 0x7F;
                    if (!count) break;
                    if (pkt & 0x80) { /* literal */
                        while (count-- && x < (int)xsize && p < rleLen)
                            dest[(x++) * 3] = rle[p++];
                    } else {          /* run */
                        unsigned char val = (p < rleLen) ? rle[p++] : 0;
                        while (count-- && x < (int)xsize)
                            dest[(x++) * 3] = val;
                    }
                }
                free(rle);
            }
        }
        free(offsets);
        free(lengths);
    }

done:
    free(rowbuf);
    fclose(f);
    return image;
}

AUX_RGBImageRec * APIENTRY auxRGBImageLoadW(LPCWSTR filename)
{
    char narrow[MAX_PATH];
    if (!WideCharToMultiByte(CP_ACP, 0, filename, -1,
                             narrow, MAX_PATH, NULL, NULL))
        return NULL;
    return auxRGBImageLoadA(narrow);
}
