
#include <smart_ptr.h>
#include <entity.h>

#ifndef _libint2_src_bin_libint_prefactors_h_
#define _libint2_src_bin_libint_prefactors_h_

namespace libint2 {
  
  /** 
     Prefactors is a collection of common quantities which appear
     as prefactors in recurrence relations for Gaussian integrals.
     See Obara-Saika paper for description of the most common ones.
  */
  
  class Prefactors {
    
    public:
    Prefactors();
    ~Prefactors();
    
    typedef RTimeEntity<double> rdouble;
    typedef CTimeEntity<double> cdouble;

    static const unsigned int NMAX = 200;
    static const unsigned int np = 2;
    static const unsigned int nfunc_per_part = 1;

    /**
       X-Y vectors, where X and Y are for the same particle:
       X_Y[0] = AB, X_Y[1] = CD, etc.
     */
    SafePtr<rdouble> vX_Y[np];
    /// Cartesian components of X-Y vectors
    SafePtr<rdouble> X_Y[np][3];

    /**
       XY-X vectors:
       XY is either P or Q,
       X is either (A or B) or (C or D).
       Hence, vXY_X[0][0] = P-A, vXY_X[1][0] = Q-C,
       vXY_X[0][1] = P-B, and vXY_X[1][1] = Q-D.
    */
    SafePtr<rdouble> vXY_X[np][2];
    /// cartesian components of vXY_X vector
    SafePtr<rdouble> XY_X[np][2][3];
    /**
       W-XY vectors:
       vW_XY[0] = W-P, vW_XY[1] = W-Q.
    */
    SafePtr<rdouble> vW_XY[np];
    /// cartesian components of W_XY vector
    SafePtr<rdouble> W_XY[np][3];

    /**
       orbital exponents
    */
    SafePtr<rdouble> zeta[np][2];
    /**
       squared orbital exponents
    */
    SafePtr<rdouble> zeta2[np][2];

    /**
       alpha12[p] is the sum of exponents for particle p:
       alpha12[0] = zeta,
       alpha12[1] = eta.
    */
    SafePtr<rdouble> alpha12[np];
    /// rho = zeta*eta/(zeta+eta)
    SafePtr<rdouble> rho;
    /// 1/(2*alpha12)
    SafePtr<rdouble> one_o_2alpha12[np];
    /// rho/alpha12
    SafePtr<rdouble> rho_o_alpha12[np];
    /// 1/(2*(zeta+eta))
    SafePtr<rdouble> one_o_2alphasum;
    
    /**
       Prefactors for the VRR relation for R12_k_G12 integrals (k>=0):
    */
    /// prefactor in front of (a0|c0)
    SafePtr<rdouble> R12kG12VRR_vpfac0[np];
    /// cartesian components of pfac0 vector
    SafePtr<rdouble> R12kG12VRR_pfac0[np][3];
    /// prefactor in front of (a-1 0|c0)
    SafePtr<rdouble> R12kG12VRR_pfac1[np];
    /// prefactor in front of (a0|c-1 0)
    SafePtr<rdouble> R12kG12VRR_pfac2;
    /// prefactor in front of (|k-2|)
    SafePtr<rdouble> R12kG12VRR_pfac3[np];
    /// prefactor in front of (a0|k-2|c0)
    SafePtr<rdouble> R12kG12VRR_vpfac4[np];
    /// cartesian components of pfac4 vector
    SafePtr<rdouble> R12kG12VRR_pfac4[np][3];

    /// integers represented as doubles
    SafePtr<cdouble> N_i[NMAX];

    SafePtr<cdouble> Cdouble(double a);
  };
  
  extern Prefactors prefactors;
  
};

#endif

