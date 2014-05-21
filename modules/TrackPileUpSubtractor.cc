
/** \class TrackPileUpSubtractor
 *
 *  Subtract pile-up contribution from tracks.
 *
 *  $Date: 2014-01-17 14:29:22 +0100 (Fri, 17 Jan 2014) $
 *  $Revision: 1349 $
 *
 *
 *  \author P. Demin - UCL, Louvain-la-Neuve
 *
 */

#include "modules/TrackPileUpSubtractor.h"

#include "classes/DelphesClasses.h"
#include "classes/DelphesFactory.h"
#include "classes/DelphesFormula.h"

#include "ExRootAnalysis/ExRootResult.h"
#include "ExRootAnalysis/ExRootFilter.h"
#include "ExRootAnalysis/ExRootClassifier.h"

#include "TMath.h"
#include "TString.h"
#include "TFormula.h"
#include "TRandom3.h"
#include "TObjArray.h"
#include "TDatabasePDG.h"
#include "TLorentzVector.h"

#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <sstream>

using namespace std;

//------------------------------------------------------------------------------

TrackPileUpSubtractor::TrackPileUpSubtractor()
{
}

//------------------------------------------------------------------------------

TrackPileUpSubtractor::~TrackPileUpSubtractor()
{
}

//------------------------------------------------------------------------------

void TrackPileUpSubtractor::Init()
{
// import input array

  fVertexInputArray = ImportArray(GetString("VertexInputArray", "PileUpMerger/vertices"));
  fItVertexInputArray = fVertexInputArray->MakeIterator();
  
  fZVertexResolution  = GetDouble("ZVertexResolution", 0.005)*1.0E3;

  // import arrays with output from other modules

  ExRootConfParam param = GetParam("InputArray");
  Long_t i, size;
  const TObjArray *array;
  TIterator *iterator;

  size = param.GetSize();
  for(i = 0; i < size/2; ++i)
  {
    array = ImportArray(param[i*2].GetString());
    iterator = array->MakeIterator();

    fInputMap[iterator] = ExportArray(param[i*2 + 1].GetString());
  }
}

//------------------------------------------------------------------------------

void TrackPileUpSubtractor::Finish()
{
  map< TIterator *, TObjArray * >::iterator itInputMap;
  TIterator *iterator;

  for(itInputMap = fInputMap.begin(); itInputMap != fInputMap.end(); ++itInputMap)
  {
    iterator = itInputMap->first;

    if(iterator) delete iterator;
  }

  if(fItVertexInputArray) delete fItVertexInputArray;
}

//------------------------------------------------------------------------------

void TrackPileUpSubtractor::Process()
{
  Candidate *candidate, *particle;
  map< TIterator *, TObjArray * >::iterator itInputMap;
  TIterator *iterator;
  TObjArray *array;
  Double_t z, zvtx=0;

  
  // find z position of primary vertex
  
  fItVertexInputArray->Reset();
  while((candidate = static_cast<Candidate*>(fItVertexInputArray->Next())))
  {
    if(!candidate->IsPU)
    {
    zvtx = candidate->Position.Z();
    // break;
    }
  }

  // loop over all input arrays
  for(itInputMap = fInputMap.begin(); itInputMap != fInputMap.end(); ++itInputMap)
  {
    iterator = itInputMap->first;
    array = itInputMap->second;

    // loop over all candidates
    iterator->Reset();
    while((candidate = static_cast<Candidate*>(iterator->Next())))
    {
      particle = static_cast<Candidate*>(candidate->GetCandidates()->At(0));
      z = particle->Position.Z();
      
      // apply pile-up subtraction
      // assume perfect pile-up subtraction for tracks outside fZVertexResolution
      
      if(candidate->IsPU && TMath::Abs(z-zvtx) > fZVertexResolution) continue;

      array->Add(candidate);
    }
  }
}

//------------------------------------------------------------------------------
