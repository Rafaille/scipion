/***************************************************************************
 *
 * Authors:     Carlos Oscar S. Sorzano (coss@cnb.uam.es)
 *
 * Unidad de  Bioinformatica of Centro Nacional de Biotecnologia , CSIC
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307  USA
 *
 *  All comments concerning this program package may be sent to the
 *  e-mail address 'xmipp@cnb.uam.es'
 ***************************************************************************/

#include "normalize.h"
#include "args.h"
#include "micrograph.h"
#include "mask.h"

#include <string>
#include <iostream>

void Normalize_parameters::read(int argc, char** argv)
{
    Prog_parameters::read(argc, argv);

    // Get normalizing method
    std::string aux;
    aux = getParameter(argc, argv, "-method", "NewXmipp");

    if (aux == "OldXmipp")
        method = OLDXMIPP;
    else if (aux == "Near_OldXmipp")
        method = NEAR_OLDXMIPP;
    else if (aux == "NewXmipp")
        method = NEWXMIPP;
    else if (aux == "NewXmipp2")
        method = NEWXMIPP2;
    else if (aux == "Michael")
        method = MICHAEL;
    else if (aux == "Random")
        method = RANDOM;
    else if (aux == "None")
        method = NONE;
    else if (aux == "Ramp")
        method = RAMP;
    else if (aux == "Neighbour")
        method = NEIGHBOUR;
    else
        REPORT_ERROR(1, "Normalize: Unknown normalizing method");

    // Normalizing a volume
    volume = checkParameter(argc, argv, "-vol");

    // Invert contrast?
    invert_contrast = checkParameter(argc, argv, "-invert");

    // Remove dust particles?
    remove_black_dust = checkParameter(argc, argv, "-remove_black_dust");
    remove_white_dust = checkParameter(argc, argv, "-remove_white_dust");
    thresh_black_dust = textToFloat(getParameter(argc, argv, "-thr_black_dust", "-3.5"));
    thresh_white_dust = textToFloat(getParameter(argc, argv, "-thr_white_dust", "3.5"));
    thresh_neigh      = textToFloat(getParameter(argc, argv, "-thr_neigh", "1.2"));

    apply_geo = false;

    // Get background mask
    if (!volume)
    {
        if (method == NEWXMIPP || method == NEWXMIPP2 || method == MICHAEL ||
            method == NEAR_OLDXMIPP || method == RAMP || method == NEIGHBOUR)
        {
            enable_mask = checkParameter(argc, argv, "-mask");
            if (enable_mask)
            {
                mask_prm.allowed_data_types = INT_MASK;
                mask_prm.read(argc, argv);
            }
            else
            {
                enable_mask = false;
                int i = paremeterPosition(argc, argv, "-background");
                if (i + 2 >= argc)
                    REPORT_ERROR(1,
                                 "Normalize: Not enough parameters after -background");

                aux = argv[i + 1];
                r  = textToInteger(argv[i + 2]);

                if (aux == "frame")
                    background_mode = FRAME;
                else if (aux == "circle")
                    background_mode = CIRCLE;
                else
                    REPORT_ERROR(1, "Normalize: Unknown background mode");
            }

            produce_side_info();

            // Default is NOT to apply inverse transformation from image header to the mask
            apply_geo = checkParameter(argc, argv, "-apply_geo");
        }
        else
            background_mode = NONE;

        if (method == RANDOM)
        {
            int i = paremeterPosition(argc, argv, "-prm");
            if (i + 4 >= argc)
                REPORT_ERROR(1,
                             "Normalize_parameters::read: Not enough parameters after -prm");

            a0 = textToFloat(argv[i + 1]);
            aF = textToFloat(argv[i + 2]);
            b0 = textToFloat(argv[i + 3]);
            bF = textToFloat(argv[i + 4]);
        }
    }
}

void Normalize_parameters::produce_side_info()
{
    int Zdim, Ydim, Xdim;
    get_input_size(Zdim, Ydim, Xdim);
    if (!volume)
    {
        if (Zdim != 1)
            REPORT_ERROR(1, "Normalize: This program works only with images");

        if (!enable_mask)
        {
            bg_mask.resize(Ydim, Xdim);
            bg_mask.setXmippOrigin();

            switch (background_mode)
            {
            case FRAME:
                BinaryFrameMask(bg_mask, Xdim - 2 * r, Ydim - 2 * r,
                                OUTSIDE_MASK);
                break;
            case CIRCLE:
                BinaryCircularMask(bg_mask, r, OUTSIDE_MASK);
                break;
            }
        }
        else
        {
            mask_prm.generate_2Dmask(Ydim, Xdim);
            bg_mask = mask_prm.imask2D;
	    // backup a copy of the mask for apply_geo mode
	    bg_mask_bck = bg_mask;
        }
    }
}

void Normalize_parameters::show()
{
    Prog_parameters::show();
    std::cout << "Normalizing volumes: " << volume << std::endl;
    if (!volume)
    {
        std::cout << "Normalizing method: ";
        switch (method)
        {
        case OLDXMIPP:
            std::cout << "OldXmipp\n";
            break;
        case NEAR_OLDXMIPP:
            std::cout << "Near_OldXmipp\n";
            break;
        case NEWXMIPP:
            std::cout << "NewXmipp\n";
            break;
        case NEWXMIPP2:
            std::cout << "NewXmipp2\n";
            break;
        case MICHAEL:
            std::cout << "Michael\n";
            break;
        case NONE:
            std::cout << "None\n";
            break;
        case RAMP:
            std::cout << "Ramp\n";
            break;
        case NEIGHBOUR:
            std::cout << "Neighbour\n";
            break;
        case RANDOM:
            std::cout << "Random a=[" << a0 << "," << aF << "], " << "b=[" <<
            b0 << "," << bF << "]\n";
            break;
        }

        if (method == NEWXMIPP || method == NEWXMIPP2 ||
            method == NEAR_OLDXMIPP || method == MICHAEL || 
	    method == RAMP || method == NEIGHBOUR)
        {
            std::cout << "Background mode: ";
            switch (background_mode)
            {
            case NONE :
                std::cout << "None\n";
                break;
            case FRAME:
                std::cout << "Frame, width=" << r << std::endl;
                std::cout << "Apply transformation to mask: " << apply_geo <<
                std::endl;
                break;
            case CIRCLE:
                std::cout << "Circle, radius=" << r << std::endl;
                std::cout << "Apply transformation to mask: " << apply_geo <<
                std::endl;
                break;
            }
        }

        if (invert_contrast)
            std::cout << "Invert contrast "<< std::endl;

        if (remove_black_dust)
            std::cout << "Remove black dust particles, using threshold " <<
            floatToString(thresh_black_dust) << std::endl;

        if (remove_white_dust)
            std::cout << "Remove white dust particles, using threshold " <<
            floatToString(thresh_white_dust) << std::endl;
    }

    if (method == NEWXMIPP && enable_mask)
        mask_prm.show();
}

void Normalize_parameters::usage()
{
    Prog_parameters::usage();

    std::cerr << "NORMALIZATION OF VOLUMES\n"
    << "  [-vol]                    : Activate this mode\n";

    std::cerr << "NORMALIZATION OF IMAGES\n"
    << "  [-method <mth=NewXmipp>   : Normalizing method. Valid ones are:\n"
    << "                              OldXmipp, Near_OldXmipp, NewXmipp\n"
    << "                              NewXmipp2, Michael, None, Random, Ramp, Neighbour\n"
    << "                              Methods NewXmipp, Michael, Near_OldXmipp\n"
    << "                              and Ramp need a background mask:\n"
    << "  [-background frame <r>  | : Rectangular background of r pixels\n"
    << "   -background circle <r> | : Circular background outside radius=r\n"
    << "   -mask <options>]           Use an alternative type of background mask\n"
    << "                               (see xmipp_mask for options) \n"
    << "  [-invert]                 : Invert contrast \n"
    << "  [-remove_black_dust]      : Remove black dust particles \n"
    << "  [-remove_white_dust]      : Remove white dust particles \n"
    << "  [-thr_black_dust=-3.5]    : Sigma threshold for black dust particles \n"
    << "  [-thr_white_dust=3.5]     : Sigma threshold for white dust particles \n"
    << "  [-thr_neigh]              : Sigma threshold for neighbour removal \n"
    << "  [-prm a0 aF b0 bF]        : Only in random mode. y=ax+b\n";
}

void Normalize_parameters::apply_geo_mask(ImageXmipp& img)
{
    Matrix2D< double > tmp;
    // get copy of the mask 
    tmp.resize(bg_mask_bck);
    type_cast(bg_mask_bck, tmp);

    double outside = DIRECT_MAT_ELEM(tmp, 0, 0);

    // Instead of IS_INV for images use IS_NOT_INV for masks!
    tmp.selfApplyGeometryBSpline(img.get_transformation_matrix(), 3, IS_NOT_INV,
                                DONT_WRAP, outside);
    // The type cast gives strange results here, using round instead
    //type_cast(tmp, bg_mask);
    FOR_ALL_DIRECT_ELEMENTS_IN_MATRIX2D(bg_mask) {
      dMij(bg_mask,i,j)=ROUND(dMij(tmp,i,j));
    }
}

void Normalize_parameters::apply(Image* img)
{
    double a, b;
    if (invert_contrast)
	(*img)() *= -1.;
    
    if (remove_black_dust || remove_white_dust)
    {
        double avg, stddev, min, max, zz;
        (*img)().computeStats(avg, stddev, min, max);

        if ((min - avg) / stddev < thresh_black_dust && remove_black_dust)
        {
            FOR_ALL_DIRECT_ELEMENTS_IN_MATRIX2D((*img)())
            {
                zz = (dMij((*img)(), i, j) - avg) / stddev;
                if (zz < thresh_black_dust)
                    dMij((*img)(), i, j) = rnd_gaus(avg,stddev);
            }
        }

        if ((max - avg) / stddev > thresh_white_dust && remove_white_dust)
        {
            FOR_ALL_DIRECT_ELEMENTS_IN_MATRIX2D((*img)())
            {
                zz = (dMij((*img)(), i, j) - avg) / stddev;
                if (zz > thresh_white_dust)
                    dMij((*img)(), i, j) = rnd_gaus(avg,stddev);
            }
        }
    }

    switch (method)
    {
    case OLDXMIPP:
        normalize_OldXmipp(img);
        break;
    case NEAR_OLDXMIPP:
        normalize_Near_OldXmipp(img, bg_mask);
        break;
    case NEWXMIPP:
        normalize_NewXmipp(img, bg_mask);
        break;
    case NEWXMIPP2:
        normalize_NewXmipp2(img, bg_mask);
        break;
    case RAMP:
        normalize_ramp(img, bg_mask);
        break;
    case NEIGHBOUR:
        normalize_remove_neighbours(img, bg_mask, thresh_neigh);
        break;
    case MICHAEL:
        normalize_Michael(img, bg_mask);
        break;
    case RANDOM:
        a = rnd_unif(a0, aF);
        b = rnd_unif(b0, bF);

        FOR_ALL_ELEMENTS_IN_MATRIX2D((*img)())
        (*img)(i, j) = a * (*img)(i, j) + b;

        break;
    }
}
