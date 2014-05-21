#ifndef EnergyScale_h
#define EnergyScale_h

/** \class MomentumSmearing
 *
 *  Performs transverse momentum resolution smearing.
 *
 *  $Date: 2013-06-25 14:42:33 +0200 (Tue, 25 Jun 2013) $
 *  $Revision: 1141 $
 *
 *
 *  \author P. Demin - UCL, Louvain-la-Neuve
 *
 */

#include "classes/DelphesModule.h"

class TIterator;
class TObjArray;
class DelphesFormula;

class EnergyScale: public DelphesModule
{
public:

  EnergyScale();
  ~EnergyScale();

  void Init();
  void Process();
  void Finish();

private:

  DelphesFormula *fFormula; //!

  TIterator *fItInputArray; //!

  const TObjArray *fInputArray; //!
  
  TObjArray *fOutputArray; //!

  ClassDef(EnergyScale, 1)
};

#endif
