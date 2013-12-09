// @HEADER
// ************************************************************************
//
// Questions? Contact Denis Ridzal (dridzal@sandia.gov), or
//                    Drew Kouri   (dpkouri@sandia.gov)
//
// ************************************************************************
// @HEADER

/** \file
    \brief  Contains definitions of custom data types in ROL.
    \author Created by D. Ridzal and D. Kouri.
 */

#ifndef ROL_TYPES_HPP
#define ROL_TYPES_HPP

#ifdef  HAVE_ROL_DEBUG
#define ROL_VALIDATE( A )  A
#else
#define ROL_VALIDATE( A ) /* empty */
#endif

#include <Teuchos_ScalarTraits.hpp>
#include <Teuchos_TestForException.hpp>

/** \def    ROL_NUM_CHECKDERIV_STEPS
    \brief  Number of steps for derivative checks.
 */
#define ROL_NUM_CHECKDERIV_STEPS 13

namespace ROL {
  
  /** \brief  Platform-dependent machine epsilon. 
   */
  static const double ROL_EPSILON   = std::abs(Teuchos::ScalarTraits<double>::eps());
    
  /** \brief  Tolerance for various equality tests.
   */
  static const double ROL_THRESHOLD = 10.0 * ROL_EPSILON;

  /** \enum   ROL::EDescent
      \brief  Enumeration of descent direction types.

      \arg    STEEPEST        describe
      \arg    NONLINEARCG     describe
      \arg    SECANT          describe
      \arg    NEWTON          describe 
      \arg    NEWTONKRYLOV    describe
      \arg    SECANTPRECOND   describe
   */
  enum EDescent{
    DESCENT_STEEPEST = 0,
    DESCENT_NONLINEARCG,
    DESCENT_SECANT,
    DESCENT_NEWTON,
    DESCENT_NEWTONKRYLOV,
    DESCENT_SECANTPRECOND,
    DESCENT_LAST
  };

  inline std::string EDescentToString(EDescent tr) {
    std::string retString;
    switch(tr) {
      case DESCENT_STEEPEST:      retString = "Steepest Descent";                          break;
      case DESCENT_NONLINEARCG:   retString = "Nonlinear CG";                              break;
      case DESCENT_SECANT:        retString = "Quasi-Newton Method";                       break;
      case DESCENT_NEWTON:        retString = "Newton's Method";                           break;
      case DESCENT_NEWTONKRYLOV:  retString = "Newton-Krylov";                             break;
      case DESCENT_SECANTPRECOND: retString = "Newton-Krylov with Secant Preconditioning"; break;
      case DESCENT_LAST:          retString = "Last Type (Dummy)";                         break;
      default:                    retString = "INVALID ESecant";
    }
    return retString;
  }
  
  /** \brief  Verifies validity of a Secant enum.
    
      \param  tr  [in]  - enum of the Secant
      \return 1 if the argument is a valid Secant; 0 otherwise.
    */
  inline int isValidDescent(EDescent d){
    return( (d == DESCENT_STEEPEST)      ||
            (d == DESCENT_NONLINEARCG)   ||
            (d == DESCENT_SECANT)        ||
            (d == DESCENT_NEWTON)        ||
            (d == DESCENT_NEWTONKRYLOV)  ||
            (d == DESCENT_SECANTPRECOND)
          );
  }

  inline EDescent & operator++(EDescent &type) {
    return type = static_cast<EDescent>(type+1);
  }

  inline EDescent operator++(EDescent &type, int) {
    EDescent oldval = type;
    ++type;
    return oldval;
  }

  inline EDescent & operator--(EDescent &type) {
    return type = static_cast<EDescent>(type-1);
  }

  inline EDescent operator--(EDescent &type, int) {
    EDescent oldval = type;
    --type;
    return oldval;
  }

  /** \enum   ROL::ESecant
      \brief  Enumeration of secant update algorithms.

      \arg    LBFGS           describe
      \arg    LDFP            describe
      \arg    LSR1            describe 
      \arg    BARZILAIBORWEIN describe
   */
  enum ESecant{
    SECANT_LBFGS = 0,
    SECANT_LDFP,
    SECANT_LSR1,
    SECANT_BARZILAIBORWEIN,
    SECANT_USERDEFINED,
    SECANT_LAST
  };

  inline std::string ESecantToString(ESecant tr) {
    std::string retString;
    switch(tr) {
      case SECANT_LBFGS:           retString = "Limited-Memory BFGS"; break;
      case SECANT_LDFP:            retString = "Limited-Memory DFP";  break;
      case SECANT_LSR1:            retString = "Limited-Memory SR1";  break;
      case SECANT_BARZILAIBORWEIN: retString = "Barzilai-Borwein";    break;
      case SECANT_USERDEFINED:     retString = "User-Defined";        break;
      case SECANT_LAST:            retString = "Last Type (Dummy)";   break;
      default:                     retString = "INVALID ESecant";
    }
    return retString;
  }
  
  /** \brief  Verifies validity of a Secant enum.
    
      \param  tr  [in]  - enum of the Secant
      \return 1 if the argument is a valid Secant; 0 otherwise.
    */
  inline int isValidSecant(ESecant s) {
    return( (s == SECANT_LBFGS)           ||
            (s == SECANT_LDFP)            ||
            (s == SECANT_LSR1)            ||
            (s == SECANT_BARZILAIBORWEIN) ||
            (s == SECANT_USERDEFINED)
          );
  }

  inline ESecant & operator++(ESecant &type) {
    return type = static_cast<ESecant>(type+1);
  }

  inline ESecant operator++(ESecant &type, int) {
    ESecant oldval = type;
    ++type;
    return oldval;
  }

  inline ESecant & operator--(ESecant &type) {
    return type = static_cast<ESecant>(type-1);
  }

  inline ESecant operator--(ESecant &type, int) {
    ESecant oldval = type;
    --type;
    return oldval;
  }

  /** \enum   ROL::ENonlinearCG
      \brief  Enumeration of nonlinear CG algorithms.

      \arg    HESTENES_STIEFEL   describe
      \arg    FLETCHER_REEVES    describe
      \arg    DANIEL             describe 
      \arg    POLAK_RIBIERE      describe
      \arg    FLETCHER_CONJDESC  describe
      \arg    LIU_STOREY         describe
      \arg    DAI_YUAN           describe
      \arg    HAGAR_ZHANG        describe
   */
  enum ENonlinearCG{
    NONLINEARCG_HESTENES_STIEFEL = 0,
    NONLINEARCG_FLETCHER_REEVES,
    NONLINEARCG_DANIEL,
    NONLINEARCG_POLAK_RIBIERE,
    NONLINEARCG_FLETCHER_CONJDESC,
    NONLINEARCG_LIU_STOREY,
    NONLINEARCG_DAI_YUAN,
    NONLINEARCG_HAGAR_ZHANG,
    NONLINEARCG_LAST
  };

  inline std::string ENonlinearCGToString(ENonlinearCG tr) {
    std::string retString;
    switch(tr) {
      case NONLINEARCG_HESTENES_STIEFEL:      retString = "Hestenes-Stiefel";            break;
      case NONLINEARCG_FLETCHER_REEVES:       retString = "Fletcher-Reeves";             break;
      case NONLINEARCG_DANIEL:                retString = "Daniel (uses Hessian)";       break;
      case NONLINEARCG_POLAK_RIBIERE:         retString = "Polak-Ribiere";               break;
      case NONLINEARCG_FLETCHER_CONJDESC:     retString = "Fletcher Conjugate Descent";  break;
      case NONLINEARCG_LIU_STOREY:            retString = "Liu-Storey";                  break;
      case NONLINEARCG_DAI_YUAN:              retString = "Dai-Yuan";                    break;
      case NONLINEARCG_HAGAR_ZHANG:           retString = "Hagar-Zhang";                 break;
      case NONLINEARCG_LAST:                  retString = "Last Type (Dummy)";           break;
      default:                                retString = "INVALID ENonlinearCG";
    }
    return retString;
  }
  
  /** \brief  Verifies validity of a NonlinearCG enum.
    
      \param  tr  [in]  - enum of the NonlinearCG
      \return 1 if the argument is a valid NonlinearCG; 0 otherwise.
    */
  inline int isValidNonlinearCG(ENonlinearCG s) {
    return( (s == NONLINEARCG_HESTENES_STIEFEL)  ||
            (s == NONLINEARCG_FLETCHER_REEVES)   ||
            (s == NONLINEARCG_DANIEL)            ||
            (s == NONLINEARCG_POLAK_RIBIERE)     ||
            (s == NONLINEARCG_FLETCHER_CONJDESC) ||
            (s == NONLINEARCG_LIU_STOREY)        ||
            (s == NONLINEARCG_DAI_YUAN)          ||
            (s == NONLINEARCG_HAGAR_ZHANG)
          );
  }

  inline ENonlinearCG & operator++(ENonlinearCG &type) {
    return type = static_cast<ENonlinearCG>(type+1);
  }

  inline ENonlinearCG operator++(ENonlinearCG &type, int) {
    ENonlinearCG oldval = type;
    ++type;
    return oldval;
  }

  inline ENonlinearCG & operator--(ENonlinearCG &type) {
    return type = static_cast<ENonlinearCG>(type-1);
  }

  inline ENonlinearCG operator--(ENonlinearCG &type, int) {
    ENonlinearCG oldval = type;
    --type;
    return oldval;
  }
  
  /** \enum   ROL::ELineSearch
      \brief  Enumeration of line-search types.

      \arg    BACKTRACKING    describe
      \arg    BISECTION       describe
      \arg    GOLDENSECTION   describe
      \arg    CUBICINTERP     describe
      \arg    BRENTS          describe
   */
  enum ELineSearch{
    LINESEARCH_BACKTRACKING = 0,
    LINESEARCH_BISECTION,
    LINESEARCH_GOLDENSECTION,
    LINESEARCH_CUBICINTERP,
    LINESEARCH_BRENTS,
    LINESEARCH_LAST
  };

  inline std::string ELineSearchToString(ELineSearch ls) {
    std::string retString;
    switch(ls) {
      case LINESEARCH_BACKTRACKING:   retString = "Backtracking";        break;
      case LINESEARCH_BISECTION:      retString = "Bisection";           break;
      case LINESEARCH_GOLDENSECTION:  retString = "Golden Section";      break;
      case LINESEARCH_CUBICINTERP:    retString = "Cubic Interpolation"; break;
      case LINESEARCH_BRENTS:         retString = "Brents";              break;
      case LINESEARCH_LAST:           retString = "Last Type (Dummy)";   break;
      default:                        retString = "INVALID ELineSearch";
    }
    return retString;
  }
  
  /** \brief  Verifies validity of a LineSearch enum.
    
      \param  ls  [in]  - enum of the linesearch
      \return 1 if the argument is a valid linesearch; 0 otherwise.
    */
  inline int isValidLineSearch(ELineSearch ls){
    return( (ls == LINESEARCH_BACKTRACKING)  ||
            (ls == LINESEARCH_BISECTION)     ||
            (ls == LINESEARCH_GOLDENSECTION) ||
            (ls == LINESEARCH_CUBICINTERP)   ||
            (ls == LINESEARCH_BRENTS)
          );
  }

  inline ELineSearch & operator++(ELineSearch &type) {
    return type = static_cast<ELineSearch>(type+1);
  }

  inline ELineSearch operator++(ELineSearch &type, int) {
    ELineSearch oldval = type;
    ++type;
    return oldval;
  }

  inline ELineSearch & operator--(ELineSearch &type) {
    return type = static_cast<ELineSearch>(type-1);
  }

  inline ELineSearch operator--(ELineSearch &type, int) {
    ELineSearch oldval = type;
    --type;
    return oldval;
  }

  /** \enum   ROL::ECurvatureCondition
      \brief  Enumeration of line-search curvature conditions.

      \arg    WOLFE           describe
      \arg    STRONGWOLFE     describe
      \arg    GOLDSTEIN       describe
   */
  enum ECurvatureCondition{
    CURVATURECONDITION_WOLFE = 0,
    CURVATURECONDITION_STRONGWOLFE,
    CURVATURECONDITION_GOLDSTEIN,
    CURVATURECONDITION_LAST
  };

  inline std::string ECurvatureConditionToString(ECurvatureCondition ls) {
    std::string retString;
    switch(ls) {
      case CURVATURECONDITION_WOLFE:       retString = "Wolfe Conditions";            break;
      case CURVATURECONDITION_STRONGWOLFE: retString = "Strong Wolfe Conditions";     break;
      case CURVATURECONDITION_GOLDSTEIN:   retString = "Goldstein Conditions";        break;
      case CURVATURECONDITION_LAST:        retString = "Last Type (Dummy)";           break;
      default:                             retString = "INVALID ECurvatureCondition";
    }
    return retString;
  }
  
  /** \brief  Verifies validity of a CurvatureCondition enum.
    
      \param  ls  [in]  - enum of the Curvature Conditions
      \return 1 if the argument is a valid curvature condition; 0 otherwise.
    */
  inline int isValidCurvatureCondition(ECurvatureCondition ls){
    return( (ls == CURVATURECONDITION_WOLFE)       ||
            (ls == CURVATURECONDITION_STRONGWOLFE) ||
            (ls == CURVATURECONDITION_GOLDSTEIN)
          );
  }

  inline ECurvatureCondition & operator++(ECurvatureCondition &type) {
    return type = static_cast<ECurvatureCondition>(type+1);
  }

  inline ECurvatureCondition operator++(ECurvatureCondition &type, int) {
    ECurvatureCondition oldval = type;
    ++type;
    return oldval;
  }

  inline ECurvatureCondition & operator--(ECurvatureCondition &type) {
    return type = static_cast<ECurvatureCondition>(type-1);
  }

  inline ECurvatureCondition operator--(ECurvatureCondition &type, int) {
    ECurvatureCondition oldval = type;
    --type;
    return oldval;
  }

  /** \enum   ROL::ETrustRegion
      \brief  Enumeration of trust-region solver types.

      \arg    CAUCHYPOINT     describe
      \arg    TRUNCATEDCG     describe
      \arg    DOGLEG          describe
      \arg    DOUBLEDOGLEG    describe
   */
  enum ETrustRegion{
    TRUSTREGION_CAUCHYPOINT = 0,
    TRUSTREGION_TRUNCATEDCG,
    TRUSTREGION_DOGLEG,
    TRUSTREGION_DOUBLEDOGLEG,
    TRUSTREGION_LAST
  };

  inline std::string ETrustRegionToString(ETrustRegion tr) {
    std::string retString;
    switch(tr) {
      case TRUSTREGION_CAUCHYPOINT:   retString = "Cauchy Point";        break;
      case TRUSTREGION_TRUNCATEDCG:   retString = "Truncated CG";        break;
      case TRUSTREGION_DOGLEG:        retString = "Dogleg";              break;
      case TRUSTREGION_DOUBLEDOGLEG:  retString = "Double Dogleg";       break;
      case TRUSTREGION_LAST:          retString = "Last Type (Dummy)";   break;
      default:                        retString = "INVALID ETrustRegion";
    }
    return retString;
  }
  
  /** \brief  Verifies validity of a TrustRegion enum.
    
      \param  tr  [in]  - enum of the TrustRegion
      \return 1 if the argument is a valid TrustRegion; 0 otherwise.
    */
  inline int isValidTrustRegion(ETrustRegion ls){
    return( (ls == TRUSTREGION_CAUCHYPOINT)  ||
            (ls == TRUSTREGION_TRUNCATEDCG)  ||
            (ls == TRUSTREGION_DOGLEG)       ||
            (ls == TRUSTREGION_DOUBLEDOGLEG)
          );
  }

  inline ETrustRegion & operator++(ETrustRegion &type) {
    return type = static_cast<ETrustRegion>(type+1);
  }

  inline ETrustRegion operator++(ETrustRegion &type, int) {
    ETrustRegion oldval = type;
    ++type;
    return oldval;
  }

  inline ETrustRegion & operator--(ETrustRegion &type) {
    return type = static_cast<ETrustRegion>(type-1);
  }

  inline ETrustRegion operator--(ETrustRegion &type, int) {
    ETrustRegion oldval = type;
    --type;
    return oldval;
  }

  /** \enum   ROL::ETestObjectives
      \brief  Enumeration of test objective functions.

      \arg    ROSENBROCK           describe
      \arg    FREUDENSTEINANDROTH  describe
      \arg    POWELL               describe
      \arg    SUMOFSQUARES         describe
      \arg    LEASTSQUARES         describe
   */
  enum ETestObjectives {
    TESTOBJECTIVES_ROSENBROCK = 0,
    TESTOBJECTIVES_FREUDENSTEINANDROTH,
    TESTOBJECTIVES_BEALE,
    TESTOBJECTIVES_POWELL,
    TESTOBJECTIVES_SUMOFSQUARES,
    TESTOBJECTIVES_LEASTSQUARES,
    TESTOBJECTIVES_POISSONCONTROL,
    TESTOBJECTIVES_POISSONINVERSION,
    TESTOBJECTIVES_LAST
  };

  inline std::string ETestObjectivesToString(ETestObjectives to) {
    std::string retString;
    switch(to) {
      case TESTOBJECTIVES_ROSENBROCK:          retString = "Rosenbrock's Function";            break;
      case TESTOBJECTIVES_FREUDENSTEINANDROTH: retString = "Freudenstein and Roth's Function"; break;
      case TESTOBJECTIVES_BEALE:               retString = "Beale's Function";                 break;
      case TESTOBJECTIVES_POWELL:              retString = "Powell's Badly Scaled Function";   break;
      case TESTOBJECTIVES_SUMOFSQUARES:        retString = "Sum of Squares Function";          break;
      case TESTOBJECTIVES_LEASTSQUARES:        retString = "Least Squares Function";           break;
      case TESTOBJECTIVES_POISSONCONTROL:      retString = "Poisson Optimal Control";          break;
      case TESTOBJECTIVES_POISSONINVERSION:    retString = "Poisson Inversion Problem";        break;
      case TESTOBJECTIVES_LAST:                retString = "Last Type (Dummy)";                break;
      default:                                 retString = "INVALID ETestObjectives";
    }
    return retString;
  }
  
  /** \brief  Verifies validity of a TestObjectives enum.
    
      \param  ls  [in]  - enum of the TestObjectives
      \return 1 if the argument is a valid TestObjectives; 0 otherwise.
    */
  inline int isValidTestObjectives(ETestObjectives to){
    return( (to == TESTOBJECTIVES_ROSENBROCK)          ||
            (to == TESTOBJECTIVES_FREUDENSTEINANDROTH) ||
            (to == TESTOBJECTIVES_BEALE)               ||
            (to == TESTOBJECTIVES_POWELL)              ||
            (to == TESTOBJECTIVES_SUMOFSQUARES)        ||
            (to == TESTOBJECTIVES_LEASTSQUARES)        ||
            (to == TESTOBJECTIVES_POISSONCONTROL)      ||
            (to == TESTOBJECTIVES_POISSONINVERSION)
          );
  }

  inline ETestObjectives & operator++(ETestObjectives &type) {
    return type = static_cast<ETestObjectives>(type+1);
  }

  inline ETestObjectives operator++(ETestObjectives &type, int) {
    ETestObjectives oldval = type;
    ++type;
    return oldval;
  }

  inline ETestObjectives & operator--(ETestObjectives &type) {
    return type = static_cast<ETestObjectives>(type-1);
  }

  inline ETestObjectives operator--(ETestObjectives &type, int) {
    ETestObjectives oldval = type;
    --type;
    return oldval;
  }

} // namespace ROL


/*! \mainpage ROL Documentation (Development Version)
 
  \image html rol.png
  \image latex rol.jpg "Rapid Optimization Library" width=1in

  \section intro_sec Introduction

  %ROL, the Rapid Optimization Library, is a Trilinos package for matrix-free
  optimization.
 
  \section overview_sec Overview

  Current release of %ROL includes the following features:
  \li Unconstrained.

  \section quickstart_sec Quick Start

  The following example demonstrates, in 2 steps, the use of %ROL.

  \subsection vector_qs_sec Step 1: Define linear algebra / vector interface.
  
  \code
      ROL::Vector
  \endcode

  We additionally set the number of computational cells \c numCells.


  \subsection objective_qs_sec Step 2: Define objective function.

  \code
      ROL::Objective
  \endcode


  \subsection done_qs_sec Done!


  \section devplans_sec Development Plans

  The next release of %ROL is expected to ...
*/
  

#endif
