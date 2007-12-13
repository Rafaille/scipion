/***************************************************************************
 *
 * Authors: Sjors Scheres (scheres@cnb.uam.es)   
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

/* INCLUDES ---------------------------------------------------------------- */
#include <reconstruction/ml_align3d.h> 

/* MAIN -------------------------------------------------------------------- */
int main(int argc, char **argv) {

  int c,nn,imgno,opt_refno;
  double LL,sumw_allrefs,sumcorr;
  double aux,wsum_sigma_offset,wsum_sigma_noise2;
  vector<Matrix3D<double > > wsum_Mref;
  vector<Matrix3D<double > > wsum_Mwedge;
  vector<double> sumw;
  Matrix3D<double> Maux;
  vector<int> count_defocus;
  FileName fn_img,fn_tmp;
  Matrix1D<double> oneline(0);
  DocFile DFo,DFf;
  SelFile SFo,SFa;

  Prog_MLalign3D_prm prm;

  // Get input parameters
  try {
    prm.read(argc,argv);
    prm.produce_Side_info();
    prm.show();

  } catch (Xmipp_error XE) {cout << XE; prm.usage(); exit(0);}
    
  try {
    Maux.resize(prm.dim,prm.dim,prm.dim);
    Maux.setXmippOrigin();
    DFo.reserve(2*prm.SF.ImgNo()+1);
    DFf.reserve(2*prm.SFr.ImgNo()+4);
    SFa.reserve(prm.Niter*prm.nr_ref);
    SFa.clear();

  // Loop over all iterations
    for (int iter=prm.istart; iter<=prm.Niter; iter++) {

      if (prm.verb>0) cerr << "  multi-reference refinement:  iteration " << iter <<" of "<< prm.Niter<<endl;

      DFo.clear();
      DFo.append_comment("Headerinfo columns: rot (1), tilt (2), psi (3), Xoff (4), Yoff (5), Zoff (6), WedNo (7) Ref (8), Pmax/sumP (9)");

      // Integrate over all images
      prm.ML_sum_over_all_images(prm.SF,prm.Iref, LL,sumcorr,DFo, 
				 wsum_Mref,wsum_Mwedge,
				 wsum_sigma_noise2,wsum_sigma_offset,sumw); 

      // Update model parameters
      prm.update_parameters(wsum_Mref,wsum_Mwedge,
			    wsum_sigma_noise2,wsum_sigma_offset, 
			    sumw,sumcorr,sumw_allrefs,iter);
 
      prm.write_output_files(iter,SFa,DFf,sumw_allrefs,sumw,LL,sumcorr);
      
      // Write out docfile with optimal transformation & references
      fn_tmp=prm.fn_root+"_it";
      fn_tmp.compose(fn_tmp,iter,"doc");
      DFo.write(fn_tmp);

    } // end loop iterations

    // Write out converged structures
    prm.write_output_files(-1,SFa,DFf,sumw_allrefs,sumw,LL,sumcorr);
      
    // Write out docfile with optimal transformation & references
    fn_img=prm.fn_root+".doc";
    DFo.write(fn_img);

  } catch (Xmipp_error XE) {cout << XE; prm.usage(); exit(0);}
}




