#ifndef JetSubStructure_h
#define JetSubStructure_h

/** \class JetSubStructure
 *
 *  Performs analysis of jet substructure
 *
 *  $Date: 2013-06-10 16:000 (Mon, 10June 2013) $
 *  $Revision: 1099 $
 *
 *
 *  \author J. Stupak - Purdue Calumet
 *
 */

#include "classes/DelphesModule.h"

#include <map>

class TObjArray;

class ExRootFilter;

class JetSubStructure: public DelphesModule
{
public:

  JetSubStructure();
  ~JetSubStructure();

  void Init();
  void Process();
  void Finish();

private:

  Int_t fBitNumber;

  Double_t fMinWMass;
  Double_t fMaxWMass;

  TIterator *fItJetInputArray; //!
  const TObjArray *fJetInputArray; //!
  
  ClassDef(JetSubStructure, 1)
};

#endif
