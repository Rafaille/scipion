/***************************************************************************
 *
 * Authors:    Amaya Jimenez      ajimenez@cnb.csic.es (2002)
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
 *  e-mail address 'xmipp@cnb.csic.es'
 ***************************************************************************/

#include "gpu_rotate_image.h"

#include <reconstruction_cuda/cuda_gpu_rotate_image.h>
#include <data/args.h>


// Read arguments ==========================================================
void ProgGpuRotateImage::readParams()
{
	ang = getParam("--ang");

}

// Show ====================================================================
void ProgGpuRotateImage::show()
{
    std::cout
    << "Rotate an image by an angle: " << ang       << std::endl
    ;
}

// usage ===================================================================
void ProgGpuRotateImage::defineParams()
{
    addUsageLine("Computes the rotation of an image with CUDA in GPU");
    addParamsLine("   --ang <Metadata1>        : Rotation angle");

}

//#define DEBUG
// Compute distance --------------------------------------------------------
void ProgGpuRotateImage::run()
{

    int ang2 = ang.getNumber();
    std::cout << "Inside run with ang " << ang2 << std::endl;
    MultidimArray<float> original_image(2,2);
    MultidimArray<float> rotated_image(2,2);

    A2D_ELEM(original_image,0,0) = 38.;
    A2D_ELEM(original_image,1,0) = 39.;
    A2D_ELEM(original_image,0,1) = 118.;
    A2D_ELEM(original_image,1,1) = 13.;

    float *original_image_gpu = MULTIDIM_ARRAY(original_image);
    float *rotated_image_gpu = MULTIDIM_ARRAY(rotated_image);

    cuda_rotate_image(original_image_gpu, rotated_image_gpu, ang2);

	std::cout << "original_image" << original_image << std::endl;
	std::cout << "rotated_image" << rotated_image << std::endl;


}

