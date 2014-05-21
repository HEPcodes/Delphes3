
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
 *  Adapted from:
 *     -BTagging module by P. Demin
 *     -Substructure recipe by J. Dolen   
 *
 */

#include "modules/JetSubStructure.h"

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

#include "fastjet/JetDefinition.hh"
#include "fastjet/ClusterSequence.hh"
#include "fastjet/tools/Filter.hh"

#include <algorithm> 
#include <stdexcept>
#include <iostream>
#include <sstream>

using namespace std;

JetSubStructure::JetSubStructure() :
  fItJetInputArray(0)
{
}

//------------------------------------------------------------------------------

JetSubStructure::~JetSubStructure()
{
}

//------------------------------------------------------------------------------

void JetSubStructure::Init()
{
  ExRootConfParam param;
  Int_t i, size;

  // import input array(s)

  fJetInputArray = ImportArray(GetString("JetInputArray", "FastJetFinder/CAJets"));
  fItJetInputArray = fJetInputArray->MakeIterator();

  fMinWMass = GetDouble("MinWMass", 50);
  fMaxWMass = GetDouble("MaxWMass", 120);
}

//------------------------------------------------------------------------------

void JetSubStructure::Finish()
{
  if(fItJetInputArray) delete fItJetInputArray;
}

//------------------------------------------------------------------------------

void JetSubStructure::Process()
{
  Candidate *jet;
  Double_t pt, eta, phi;

  // loop over all input jets
  fItJetInputArray->Reset();
  while((jet = static_cast<Candidate*>(fItJetInputArray->Next())))
  {
    const TLorentzVector &jetMomentum = jet->Momentum;
    eta = jetMomentum.Eta();
    phi = jetMomentum.Phi();
    pt = jetMomentum.Pt();

    //if (pt<250 || !jet->pJet->has_constituents()) continue;
    cout<<"contrituents"<<endl;
    //------------------------------------
    // Filtering
    //------------------------------------
    double Rfilt = 0.3;
    unsigned int nfilt = 3;
    fastjet::Filter filter_CA3(fastjet::JetDefinition(fastjet::cambridge_algorithm, Rfilt), fastjet::SelectorNHardest(nfilt));
    //fastjet::PseudoJet filtered_jet_CA3 = filter_CA3(*(jet->pJet));

    //------------------------------------
    // W-tag - Mass drop from pruned jets
    //------------------------------------
    vector<fastjet::PseudoJet> pruned_subjets = sorted_by_pt(filtered_jet_CA3.exclusive_subjets_up_to(2));
    double mu=10000;
    if (pruned_subjets.size()>1)
      {
        if (pruned_subjets[0].m() >  pruned_subjets[1].m() )
	  {
	    mu=pruned_subjets[0].m()/filtered_jet_CA3.m();
	  }
        else
	  {
	    mu=pruned_subjets[1].m()/filtered_jet_CA3.m();
	  }
      }
    if (mu<0.4 && filtered_jet_CA3.m() <120 && filtered_jet_CA3.m() > 50)     
      jet->WTag=1;
  }
}

//------------------------------------------------------------------------------
