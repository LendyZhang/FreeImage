/* -*- C++ -*-
 * Copyright 2019-2020 LibRaw LLC (info@libraw.org)
 *
 LibRaw uses code from dcraw.c -- Dave Coffin's raw photo decoder,
 dcraw.c is copyright 1997-2018 by Dave Coffin, dcoffin a cybercom o net.
 LibRaw do not use RESTRICTED code from dcraw.c

 LibRaw is free software; you can redistribute it and/or modify
 it under the terms of the one of two licenses as you choose:

1. GNU LESSER GENERAL PUBLIC LICENSE version 2.1
   (See file LICENSE.LGPL provided in LibRaw distribution archive for details).

2. COMMON DEVELOPMENT AND DISTRIBUTION LICENSE (CDDL) Version 1.0
   (See file LICENSE.CDDL provided in LibRaw distribution archive for details).

 */

#include "../../internal/dcraw_defs.h"

void LibRaw::parseSigmaMakernote (int base, int uptag, unsigned dng_writer) {
unsigned wb_table1 [] = {
  LIBRAW_WBI_Auto, LIBRAW_WBI_Daylight, LIBRAW_WBI_Shade, LIBRAW_WBI_Cloudy,
  LIBRAW_WBI_Tungsten, LIBRAW_WBI_Fluorescent, LIBRAW_WBI_Flash,
  LIBRAW_WBI_Custom, LIBRAW_WBI_Custom1, LIBRAW_WBI_Custom2
};

  unsigned entries, tag, type, len, save;
  unsigned i;

  entries = get2();
  if (entries > 1000)
    return;
  while (entries--) {
    tiff_get(base, &tag, &type, &len, &save);
    if (tag == 0x0027) {
      ilm.LensID = get2();
    } else if (tag == 0x002a) {
      ilm.MinFocal = getreal(type);
      ilm.MaxFocal = getreal(type);
    } else if (tag == 0x002b) {
      ilm.MaxAp4MinFocal = getreal(type);
      ilm.MaxAp4MaxFocal = getreal(type);
    } else if (tag == 0x0120) {
      if ((len >= (sizeof wb_table1 / sizeof wb_table1[0])) && (len%3 == 0)) {
        for (i=0; i<(len/3); i++) {
          icWBC[wb_table1[i]][0] = (int)(getreal(type)*10000.0);
          icWBC[wb_table1[i]][1] = icWBC[wb_table1[i]][3] = (int)(getreal(type)*10000.0);
          icWBC[wb_table1[i]][2] = (int)(getreal(type)*10000.0);
        }
      }
    }
    fseek(ifp, save, SEEK_SET);
  }

  return;
}

void LibRaw::parse_makernote_0xc634(int base, int uptag, unsigned dng_writer)
{

  if (imgdata.params.raw_processing_options & LIBRAW_PROCESSING_SKIP_MAKERNOTES)
    return;

  if (metadata_blocks++ > LIBRAW_MAX_METADATA_BLOCKS)
    throw LIBRAW_EXCEPTION_IO_CORRUPT;

  if (!strncmp(make, "NIKON", 5))
  {
    parseNikonMakernote(base, uptag, AdobeDNG);
    return;
  }
  else if (!strncasecmp(make, "LEICA", 5))
  {
    parseLeicaMakernote(base, uptag, is_0xc634);
    return;
  }

  short morder, sorder = order;
  char buf[10];
  INT64 fsize = ifp->size();

  fread(buf, 1, 10, ifp);

  if (!strcmp(buf, "EPSON"))
  {
    parseEpsonMakernote(base, uptag, AdobeDNG);
    return;
  }
  else if (!strcmp(buf, "SIGMA"))
  {
    parseSigmaMakernote(base, uptag, AdobeDNG);
    return;
  }

  unsigned entries, tag, type, len, save, c;

  uchar *CanonCameraInfo;
  unsigned lenCanonCameraInfo = 0;
  unsigned typeCanonCameraInfo = 0;

  uchar *table_buf_0x0116;
  ushort table_buf_0x0116_len = 0;
  uchar *table_buf_0x2010;
  ushort table_buf_0x2010_len = 0;
  uchar *table_buf_0x9050;
  ushort table_buf_0x9050_len = 0;
  uchar *table_buf_0x9400;
  ushort table_buf_0x9400_len = 0;
  uchar *table_buf_0x9402;
  ushort table_buf_0x9402_len = 0;
  uchar *table_buf_0x9403;
  ushort table_buf_0x9403_len = 0;
  uchar *table_buf_0x9406;
  ushort table_buf_0x9406_len = 0;
  uchar *table_buf_0x940c;
  ushort table_buf_0x940c_len = 0;
  uchar *table_buf_0x940e;
  ushort table_buf_0x940e_len = 0;

  if (!strcmp(buf, "OLYMPUS") || !strcmp(buf, "PENTAX ") ||
      (!strncmp(make, "SAMSUNG", 7) && (dng_writer == CameraDNG)))
  {
    base = ftell(ifp) - 10;
    fseek(ifp, -2, SEEK_CUR);
    order = get2();
    if (buf[0] == 'O')
      get2();
  }
  else if (is_PentaxRicohMakernotes && (dng_writer == CameraDNG))
  {
    base = ftell(ifp) - 10;
    fseek(ifp, -4, SEEK_CUR);
    order = get2();
    is_PentaxRicohMakernotes = 1;
  }
  else if (!strncmp(buf, "SONY", 4) ||
           !strcmp(buf, "Panasonic"))
  {
    order = 0x4949;
    fseek(ifp, 2, SEEK_CUR);
  }
  else if (!strncmp(buf, "FUJIFILM", 8))
  {
    base = ftell(ifp) - 10;
    order = 0x4949;
    fseek(ifp, 2, SEEK_CUR);
  }
  else if (!strcmp(buf, "OLYMP") ||
           !strcmp(buf, "Ricoh"))
  {
    fseek(ifp, -2, SEEK_CUR);
  }
  else if (!strcmp(buf, "AOC") || !strcmp(buf, "QVC"))
  {
    fseek(ifp, -4, SEEK_CUR);
  }
  else
  {
    fseek(ifp, -10, SEEK_CUR);
    if ((!strncmp(make, "SAMSUNG", 7) && (dng_writer == AdobeDNG)))
      base = ftell(ifp);
  }

  entries = get2();
  if (entries > 1000)
    return;

  if (!strncasecmp(make, "SONY", 4) ||
      !strncasecmp(make, "Konica", 6) ||
      !strncasecmp(make, "Minolta", 7) ||
      (!strncasecmp(make, "Hasselblad", 10) &&
       (!strncasecmp(model, "Stellar", 7) ||
        !strncasecmp(model, "Lunar", 5) ||
        !strncasecmp(model, "Lusso", 5) ||
        !strncasecmp(model, "HV", 2))))
    is_Sony = 1;

  morder = order;
  while (entries--)
  {
    order = morder;

    tiff_get(base, &tag, &type, &len, &save);

    INT64 pos = ifp->tell();
    if (len > 8 && pos + len > 2 * fsize)
    {
      fseek(ifp, save, SEEK_SET); // Recover tiff-read position!!
      continue;
    }
    tag |= uptag << 16;
    if (len > 100 * 1024 * 1024)
      goto next; // 100Mb tag? No!

    if (!strncmp(make, "Canon", 5))
    {
      if (tag == 0x000d && len < 256000)
      { // camera info
        if (!tagtypeIs(LIBRAW_EXIFTAG_TYPE_LONG))
        {
          CanonCameraInfo = (uchar *)malloc(MAX(16, len));
          fread(CanonCameraInfo, len, 1, ifp);
        }
        else
        {
          CanonCameraInfo = (uchar *)malloc(MAX(16, len * 4));
          fread(CanonCameraInfo, len, 4, ifp);
        }
        lenCanonCameraInfo = len;
        typeCanonCameraInfo = type;
      }

      else if (tag == 0x0010)
      { // Canon ModelID
        unique_id = get4();
        setCanonBodyFeatures(unique_id);
        if (lenCanonCameraInfo)
        {
          processCanonCameraInfo(unique_id, CanonCameraInfo, lenCanonCameraInfo,
                                 typeCanonCameraInfo, AdobeDNG);
          free(CanonCameraInfo);
          CanonCameraInfo = 0;
          lenCanonCameraInfo = 0;
        }
      }

      else
        parseCanonMakernotes(tag, type, len, AdobeDNG);
    }

    else if (!strncmp(make, "FUJI", 4)) {
      parseFujiMakernotes(tag, type, len, AdobeDNG);

    } else if (!strncasecmp(make, "Hasselblad", 10) && !is_Sony) {
      if (tag == 0x0011) {
        imHassy.SensorCode = getint(type);
      } else if ((tag == 0x0015) && tagtypeIs(LIBRAW_EXIFTAG_TYPE_ASCII)) {
        stmread (imHassy.SensorUnitConnector, len, ifp);
      } else if (tag == 0x0016) {
        imHassy.CoatingCode = getint(type);
      } else if ((tag == 0x002a) &&
                 tagtypeIs(LIBRAW_EXIFTAG_TYPE_SRATIONAL) &&
                 (len == 12)) {
        FORC4 for (int i = 0; i < 3; i++)
                imHassy.mnColorMatrix[c][i] = getreal(type);

      } else if (tag == 0x0031) {
        imHassy.RecommendedCrop[0] = getint(type);
        imHassy.RecommendedCrop[1] = getint(type);
      }

    } else if (!strncmp(make, "OLYMPUS", 7) ||
             (!strncasecmp(make, "CLAUSS", 6) &&
              !strncasecmp(model, "piX 5oo", 7)))
    {

      int SubDirOffsetValid =
          strncmp(model, "E-300", 5) && strncmp(model, "E-330", 5) &&
          strncmp(model, "E-400", 5) && strncmp(model, "E-500", 5) &&
          strncmp(model, "E-1", 3);

      if ((tag == 0x2010) || (tag == 0x2020) || (tag == 0x2030) ||
          (tag == 0x2031) || (tag == 0x2040) || (tag == 0x2050) ||
          (tag == 0x3000))
      {
        fseek(ifp, save - 4, SEEK_SET);
        fseek(ifp, base + get4(), SEEK_SET);
        parse_makernote_0xc634(base, tag, dng_writer);
      }

      if (!SubDirOffsetValid &&
          ((len > 4) ||
           ((tagtypeIs(LIBRAW_EXIFTAG_TYPE_SHORT) ||
            tagtypeIs(LIBRAW_EXIFTAG_TYPE_SSHORT)) && (len > 2)) ||
           ((tagtypeIs(LIBRAW_EXIFTAG_TYPE_LONG) ||
             tagtypeIs(LIBRAW_EXIFTAG_TYPE_SLONG)) && (len > 1)) ||
           tagtypeIs(LIBRAW_EXIFTAG_TYPE_RATIONAL) ||
           (type > LIBRAW_EXIFTAG_TYPE_SLONG)))
        goto skip_Oly_broken_tags;

      else if ((tag >= 0x20100000) && (tag <= 0x2010ffff))
        parseOlympus_Equipment((tag & 0x0000ffff), type, len, AdobeDNG);

      else if ((tag >= 0x20200000) && (tag <= 0x2020ffff))
        parseOlympus_CameraSettings(base, (tag & 0x0000ffff), type, len,
                                    AdobeDNG);

      else if ((tag == 0x20300108) || (tag == 0x20310109)) {
        imOly.ColorSpace = get2();
        switch (imOly.ColorSpace) {
        case 0:
          imCommon.ColorSpace = LIBRAW_COLORSPACE_sRGB;
          break;
        case 1:
          imCommon.ColorSpace = LIBRAW_COLORSPACE_AdobeRGB;
          break;
        case 2:
          imCommon.ColorSpace = LIBRAW_COLORSPACE_ProPhotoRGB;
          break;
        default:
          imCommon.ColorSpace = LIBRAW_COLORSPACE_Unknown;
          break;
        }

      } else if ((tag >= 0x20400000) && (tag <= 0x2040ffff))
        parseOlympus_ImageProcessing((tag & 0x0000ffff), type, len, AdobeDNG);

      else if ((tag >= 0x30000000) && (tag <= 0x3000ffff))
        parseOlympus_RawInfo((tag & 0x0000ffff), type, len, AdobeDNG);

      else
        switch (tag)
        {
        case 0x0207:
          getOlympus_CameraType2();
          break;
        case 0x1002:
          ilm.CurAp = libraw_powf64l(2.0f, getreal(type) / 2);
          break;
        case 0x1007:
          imCommon.SensorTemperature = (float)get2();
          break;
        case 0x1008:
          imCommon.LensTemperature = (float)get2();
          break;
        case 0x20501500:
          getOlympus_SensorTemperature(len);
          break;
        }

    skip_Oly_broken_tags:;
    }

    else if (!strncmp(make, "PENTAX", 6) || !strncmp(model, "PENTAX", 6) ||
             is_PentaxRicohMakernotes)
    {
      parsePentaxMakernotes(base, tag, type, len, dng_writer);
    }
    else if (!strncmp(make, "SAMSUNG", 7))
    {
      if (dng_writer == AdobeDNG)
        parseSamsungMakernotes(base, tag, type, len, dng_writer);
      else
        parsePentaxMakernotes(base, tag, type, len, dng_writer);
    }
    else if (is_Sony)
    {
      parseSonyMakernotes(
          base, tag, type, len, AdobeDNG, table_buf_0x0116,
          table_buf_0x0116_len, table_buf_0x2010, table_buf_0x2010_len,
          table_buf_0x9050, table_buf_0x9050_len, table_buf_0x9400,
          table_buf_0x9400_len, table_buf_0x9402, table_buf_0x9402_len,
          table_buf_0x9403, table_buf_0x9403_len, table_buf_0x9406,
          table_buf_0x9406_len, table_buf_0x940c, table_buf_0x940c_len,
          table_buf_0x940e, table_buf_0x940e_len);
    }
  next:
    fseek(ifp, save, SEEK_SET);
  }

  order = sorder;
}

void LibRaw::parse_makernote(int base, int uptag)
{

  if (imgdata.params.raw_processing_options & LIBRAW_PROCESSING_SKIP_MAKERNOTES)
    return;

  if (metadata_blocks++ > LIBRAW_MAX_METADATA_BLOCKS)
    throw LIBRAW_EXCEPTION_IO_CORRUPT;

  if (!strncmp(make, "NIKON", 5))
  {
    parseNikonMakernote(base, uptag, nonDNG);
    return;
  }
  else if (!strncasecmp(make, "LEICA", 5))
  {
    parseLeicaMakernote(base, uptag, is_0x927c);
    return;
  }

  if (!strncmp(make, "Nokia", 5))
    return;

  char buf[10];
  char another_buf[128];

  fseek(ifp, -12, SEEK_CUR);
  fread (another_buf, 1, 12, ifp);
  if (!strncmp(another_buf, "SONY", 4) ||
      !strncmp(another_buf, "VHAB", 4)) { // Sony branded as Hasselblad
    is_Sony = 1;
  }

  fread(buf, 1, 10, ifp);

  if (!strncmp(buf, "KDK", 3)  || /* these aren't TIFF tables */
      !strncmp(buf, "VER", 3)  ||
      !strncmp(buf, "IIII", 4) ||
      !strncmp(buf, "MMMM", 4))
    return;

  if (!strcmp(buf, "EPSON"))
  {
    parseEpsonMakernote(base, uptag, nonDNG);
    return;
  }
  else if (!strcmp(buf, "SIGMA"))
  {
    parseSigmaMakernote(base, uptag, CameraDNG);
    return;
  }


  unsigned entries, tag, type, len, save, c;
  unsigned i, wb[4] = {0, 0, 0, 0};
  short morder, sorder = order;

  uchar *CanonCameraInfo;
  unsigned lenCanonCameraInfo = 0;
  unsigned typeCanonCameraInfo = 0;
  imCanon.wbi = 0;

  uchar *table_buf_0x0116;
  ushort table_buf_0x0116_len = 0;
  uchar *table_buf_0x2010;
  ushort table_buf_0x2010_len = 0;
  uchar *table_buf_0x9050;
  ushort table_buf_0x9050_len = 0;
  uchar *table_buf_0x9400;
  ushort table_buf_0x9400_len = 0;
  uchar *table_buf_0x9402;
  ushort table_buf_0x9402_len = 0;
  uchar *table_buf_0x9403;
  ushort table_buf_0x9403_len = 0;
  uchar *table_buf_0x9406;
  ushort table_buf_0x9406_len = 0;
  uchar *table_buf_0x940c;
  ushort table_buf_0x940c_len = 0;
  uchar *table_buf_0x940e;
  ushort table_buf_0x940e_len = 0;

  INT64 fsize = ifp->size();

  /*
       The MakerNote might have its own TIFF header (possibly with
       its own byte-order!), or it might just be a table.
  */

  if (!strncmp(buf, "KC", 2) || /* Konica KD-400Z, KD-510Z */
      !strncmp(buf, "MLY", 3))  /* Minolta DiMAGE G series */
  {
    order = 0x4d4d;
    while ((i = ftell(ifp)) < data_offset && i < 16384)
    {
      wb[0] = wb[2];
      wb[2] = wb[1];
      wb[1] = wb[3];
      wb[3] = get2();
      if (wb[1] == 256 && wb[3] == 256 && wb[0] > 256 && wb[0] < 640 &&
          wb[2] > 256 && wb[2] < 640)
        FORC4 cam_mul[c] = wb[c];
    }
    goto quit;
  }

  if (!strcmp(buf, "OLYMPUS") ||
      !strcmp(buf, "PENTAX "))
  {
    base = ftell(ifp) - 10;
    fseek(ifp, -2, SEEK_CUR);
    order = get2();
    if (buf[0] == 'O')
      get2();
  }
  else if (!strncmp(buf, "SONY", 4) || // DSLR-A100
           !strcmp(buf, "Panasonic")) {
    if (buf[0] == 'S')
      is_Sony = 1;
    goto nf;
  }
  else if (!strncmp(buf, "FUJIFILM", 8))
  {
    base = ftell(ifp) - 10;
  nf:
    order = 0x4949;
    fseek(ifp, 2, SEEK_CUR);
  }
  else if (!strcmp (buf, "OLYMP")    ||
           !strncmp(buf, "LEICA", 5) ||
           !strcmp (buf, "Ricoh"))
  {
    fseek(ifp, -2, SEEK_CUR);
  }
  else if (!strcmp(buf, "AOC") || // Pentax, tribute to Asahi Optical Co.
           !strcmp(buf, "QVC"))   // Casio, from "QV-Camera"
  {
    fseek(ifp, -4, SEEK_CUR);
  }
  else if (!strncmp(buf, "CMT3", 4))
  {
    order = sget2((uchar *)(buf + 4));
    fseek(ifp, 2L, SEEK_CUR);
  }
  else if (libraw_internal_data.unpacker_data.CR3_CTMDtag)
  {
    order = sget2((uchar *)buf);
    fseek(ifp, -2L, SEEK_CUR);
  }
  else
  {
    fseek(ifp, -10, SEEK_CUR);
    if (!strncmp(make, "SAMSUNG", 7))
      base = ftell(ifp);
  }

  if (!is_Sony &&
      (!strncasecmp(make, "SONY", 4) ||
       !strncasecmp(make, "Konica", 6) ||
       !strncasecmp(make, "Minolta", 7) ||
       (!strncasecmp(make, "Hasselblad", 10) &&
        (!strncasecmp(model, "Stellar", 7) ||
         !strncasecmp(model, "Lunar", 5) ||
         !strncasecmp(model, "Lusso", 5) ||
         !strncasecmp(model, "HV", 2))))) {
    is_Sony = 1;
  }

  if (strcasestr(make, "Kodak") &&
      (sget2((uchar *)buf) > 1) && // check number of entries
      (sget2((uchar *)buf) < 128) &&
      (sget2((uchar *)(buf + 4)) > 0) && // check type
      (sget2((uchar *)(buf + 4)) < 13) &&
      (sget4((uchar *)(buf + 6)) < 256) // check count
  )
    imKodak.MakerNoteKodak8a = 1; // Kodak P712 / P850 / P880

  entries = get2();
  if (entries > 1000)
    return;

  morder = order;
  while (entries--)
  {
    order = morder;
    tiff_get(base, &tag, &type, &len, &save);
    tag |= uptag << 16;

    INT64 _pos = ftell(ifp);
    INT64 _pos2;
    if (len > 100 * 1024 * 1024)
	goto next; // 100Mb tag? No!
    if (len > 8 && _pos + len > 2 * fsize)
    {
      fseek(ifp, save, SEEK_SET); // Recover tiff-read position!!
      continue;
    }
    if (imKodak.MakerNoteKodak8a)
    {
      if ((tag == 0xff00) && tagtypeIs(LIBRAW_EXIFTAG_TYPE_LONG) && (len == 1))
      {
        INT64 _pos1 = get4();
        if ((_pos1 < fsize) && (_pos1 > 0))
        {
          fseek(ifp, _pos1, SEEK_SET);
          parse_makernote(base, tag);
        }
      }
      else if (tag == 0xff00f90b)
      {
        imKodak.clipBlack = get2();
      }
      else if (tag == 0xff00f90c)
      {
        imKodak.clipWhite = imgdata.color.linear_max[0] =
            imgdata.color.linear_max[1] = imgdata.color.linear_max[2] =
                imgdata.color.linear_max[3] = get2();
      }
    }
    else if (!strncmp(make, "Canon", 5))
    {
      if (tag == 0x000d && len < 256000) // camera info
      {
        if (!tagtypeIs(LIBRAW_EXIFTAG_TYPE_LONG))
        {
          CanonCameraInfo = (uchar *)malloc(MAX(16, len));
          fread(CanonCameraInfo, len, 1, ifp);
        }
        else
        {
          CanonCameraInfo = (uchar *)malloc(MAX(16, len * 4));
          fread(CanonCameraInfo, len, 4, ifp);
        }
        lenCanonCameraInfo = len;
        typeCanonCameraInfo = type;
      }

      else if (tag == 0x0010) // Canon ModelID
      {
        unique_id = get4();
        setCanonBodyFeatures(unique_id);
        if (lenCanonCameraInfo)
        {
          processCanonCameraInfo(unique_id, CanonCameraInfo, lenCanonCameraInfo,
                                 typeCanonCameraInfo, nonDNG);
          free(CanonCameraInfo);
          CanonCameraInfo = 0;
          lenCanonCameraInfo = 0;
        }
      }

      else
        parseCanonMakernotes(tag, type, len, nonDNG);
    }

    else if (!strncmp(make, "FUJI", 4))
      parseFujiMakernotes(tag, type, len, nonDNG);

    else if (!strncasecmp(model, "Hasselblad X1D", 14) ||
             !strncasecmp(model, "Hasselblad H6D", 14) ||
             !strncasecmp(model, "Hasselblad A6D", 14))
    {
      if (tag == 0x0045)
      {
        imHassy.BaseISO = get4();
      }
      else if (tag == 0x0046)
      {
        imHassy.Gain = getreal(type);
      }
    }

    else if (!strncmp(make, "PENTAX", 6) ||
             !strncmp(make, "RICOH", 5) ||
             !strncmp(model, "PENTAX", 6))
    {
      if (!strncmp(model, "GR", 2) ||
          !strncmp(model, "GXR", 3))
      {
        parseRicohMakernotes(base, tag, type, len, CameraDNG);
      }
      else
      {
        parsePentaxMakernotes(base, tag, type, len, nonDNG);
      }
    }

    else if (!strncmp(make, "SAMSUNG", 7))
    {
      if (!dng_version)
        parseSamsungMakernotes(base, tag, type, len, nonDNG);
      else
        parsePentaxMakernotes(base, tag, type, len, CameraDNG);
    }

    else if (is_Sony)
    {
      if ((tag == 0xb028) && (len == 1) && tagtypeIs(LIBRAW_EXIFTAG_TYPE_LONG))
      { // DSLR-A100
        if ((c = get4()))
        {
          fseek(ifp, c, SEEK_SET);
          parse_makernote(base, tag);
        }
      }
      else
      {
        parseSonyMakernotes(
            base, tag, type, len, nonDNG, table_buf_0x0116,
            table_buf_0x0116_len, table_buf_0x2010, table_buf_0x2010_len,
            table_buf_0x9050, table_buf_0x9050_len, table_buf_0x9400,
            table_buf_0x9400_len, table_buf_0x9402, table_buf_0x9402_len,
            table_buf_0x9403, table_buf_0x9403_len, table_buf_0x9406,
            table_buf_0x9406_len, table_buf_0x940c, table_buf_0x940c_len,
            table_buf_0x940e, table_buf_0x940e_len);
      }
    }
    fseek(ifp, _pos, SEEK_SET);

    if (!strncasecmp(make, "Hasselblad", 10) && !is_Sony) {
      if (tag == 0x0011)
        imHassy.SensorCode = getint(type);
      else if (tag == 0x0016)
        imHassy.CoatingCode = getint(type);
      else if ((tag == 0x002a) &&
               tagtypeIs(LIBRAW_EXIFTAG_TYPE_SRATIONAL) &&
               (len == 12)) {
        FORC4 for (int i = 0; i < 3; i++)
                imHassy.mnColorMatrix[c][i] = getreal(type);

      } else if (tag == 0x0031) {
        imHassy.RecommendedCrop[0] = getint(type);
        imHassy.RecommendedCrop[1] = getint(type);
      }
    }

    if ((tag == 0x0004 || tag == 0x0114) && !strncmp(make, "KONICA", 6))
    {
      fseek(ifp, tag == 0x0004 ? 140 : 160, SEEK_CUR);
      switch (get2())
      {
      case 72:
        flip = 0;
        break;
      case 76:
        flip = 6;
        break;
      case 82:
        flip = 5;
        break;
      }
    }

    _pos2 = ftell(ifp);
    if (!strncasecmp(make, "Olympus", 7) ||
        (!strncasecmp(make, "CLAUSS", 6) && !strncasecmp(model, "piX 5oo", 7)))
    {
      if ((tag == 0x2010) || (tag == 0x2020) || (tag == 0x2030) ||
          (tag == 0x2031) || (tag == 0x2040) || (tag == 0x2050) ||
          (tag == 0x3000))
      {
        if (tagtypeIs(LIBRAW_EXIFTOOLTAGTYPE_binary))
        {
          parse_makernote(base, tag);
        }
        else if (tagtypeIs(LIBRAW_EXIFTAG_TYPE_IFD))
        {
          fseek(ifp, base + get4(), SEEK_SET);
          parse_makernote(base, tag);
        }
      }
      else if (tag == 0x0207)
      {
        getOlympus_CameraType2();
      }
      else if ((tag == 0x0404) || (tag == 0x101a))
      {
        if (!imgdata.shootinginfo.BodySerial[0])
          stmread(imgdata.shootinginfo.BodySerial, len, ifp);
      }
      else if (tag == 0x1002)
      {
        ilm.CurAp = libraw_powf64l(2.0f, getreal(type) / 2);
      }
      else if (tag == 0x1007)
      {
        imCommon.SensorTemperature = (float)get2();
      }
      else if (tag == 0x1008)
      {
        imCommon.LensTemperature = (float)get2();
      }
      else if ((tag == 0x1011) && strcmp(software, "v757-71"))
      {
        for (i = 0; i < 3; i++)
        {
          if (!imOly.ColorSpace)
          {
            FORC3 cmatrix[i][c] = ((short)get2()) / 256.0;
          }
          else
          {
            FORC3 imgdata.color.ccm[i][c] = ((short)get2()) / 256.0;
          }
        }
      }
      else if (tag == 0x1012)
      {
        FORC4 cblack[RGGB_2_RGBG(c)] = get2();
      }
      else if (tag == 0x1017)
      {
        cam_mul[0] = get2() / 256.0;
      }
      else if (tag == 0x1018)
      {
        cam_mul[2] = get2() / 256.0;
      }
      else if ((tag >= 0x20100000) && (tag <= 0x2010ffff))
      {
        parseOlympus_Equipment((tag & 0x0000ffff), type, len, nonDNG);
      }
      else if ((tag >= 0x20200000) && (tag <= 0x2020ffff))
      {
        parseOlympus_CameraSettings(base, (tag & 0x0000ffff), type, len,
                                    nonDNG);
      }
      else if ((tag == 0x20300108) || (tag == 0x20310109))
      {
        imOly.ColorSpace = get2();
        switch (imOly.ColorSpace) {
        case 0:
          imCommon.ColorSpace = LIBRAW_COLORSPACE_sRGB;
          break;
        case 1:
          imCommon.ColorSpace = LIBRAW_COLORSPACE_AdobeRGB;
          break;
        case 2:
          imCommon.ColorSpace = LIBRAW_COLORSPACE_ProPhotoRGB;
          break;
        default:
          imCommon.ColorSpace = LIBRAW_COLORSPACE_Unknown;
          break;
        }
      }
      else if ((tag >= 0x20400000) && (tag <= 0x2040ffff))
      {
        parseOlympus_ImageProcessing((tag & 0x0000ffff), type, len, nonDNG);
      }
      else if (tag == 0x20501500)
      {
        getOlympus_SensorTemperature(len);
      }
      else if ((tag >= 0x30000000) && (tag <= 0x3000ffff))
      {
        parseOlympus_RawInfo((tag & 0x0000ffff), type, len, nonDNG);
      }
    }
    fseek(ifp, _pos2, SEEK_SET);

    if ((tag == 0x0015) &&
        tagtypeIs(LIBRAW_EXIFTAG_TYPE_ASCII) &&
        is_raw)
    { // Hasselblad
      stmread (imHassy.SensorUnitConnector, len, ifp);
    }

    if (tagtypeIs(LIBRAW_EXIFTAG_TYPE_UNDEFINED) &&
        ((tag == 0x0081) || // Minolta
         (tag == 0x0100)))  // Olympus
    {
      thumb_offset = ftell(ifp);
      thumb_length = len;
    }
    if ((tag == 0x0088) && // Minolta, possibly Olympus too
        tagtypeIs(LIBRAW_EXIFTAG_TYPE_LONG) &&
        (thumb_offset = get4()))
      thumb_offset += base;

    if ((tag == 0x0089) && // Minolta, possibly Olympus too
        tagtypeIs(LIBRAW_EXIFTAG_TYPE_LONG))
      thumb_length = get4();

    if (tagtypeIs(LIBRAW_EXIFTAG_TYPE_UNDEFINED) &&  // Nikon
        ((tag == 0x008c) ||
         (tag == 0x0096))) {
      meta_offset = ftell(ifp);
    }

    if ((tag == 0x00a1) &&
        tagtypeIs(LIBRAW_EXIFTAG_TYPE_UNDEFINED) &&
        strncasecmp(make, "Samsung", 7))
    {
      order = 0x4949;
      fseek(ifp, 140, SEEK_CUR);
      FORC3 cam_mul[c] = get4();
    }

    if (tag == 0xb001 && tagtypeIs(LIBRAW_EXIFTAG_TYPE_SHORT)) // Sony ModelID
    {
      unique_id = get2();
    }
    if (tag == 0x0200 && len == 3) // Olympus
      shot_order = (get4(), get4());

    if (tag == 0x0f00 && tagtypeIs(LIBRAW_EXIFTAG_TYPE_UNDEFINED))
    {
      if (len == 614)
        fseek(ifp, 176, SEEK_CUR);
      else if (len == 734 || len == 1502) // Kodak, Minolta, Olympus
        fseek(ifp, 148, SEEK_CUR);
      else
        goto next;
      goto get2_256;
    }

    if (tag == 0x2011 && len == 2) // Casio
    {
    get2_256:
      order = 0x4d4d;
      cam_mul[0] = get2() / 256.0;
      cam_mul[2] = get2() / 256.0;
    }

  next:
    fseek(ifp, save, SEEK_SET);
  }
quit:
  order = sorder;
}
