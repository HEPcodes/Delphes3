
/** \class FastJetFinder
 *
 *  Finds jets using FastJet library.
 *
 *  $Date: 2014-04-17 12:28:09 +0200 (Thu, 17 Apr 2014) $
 *  $Revision: 1383 $
 *
 *
 *  \author P. Demin - UCL, Louvain-la-Neuve
 *
 */

#include "modules/FastJetFinder.h"

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
#include <vector>

#include "fastjet/PseudoJet.hh"
#include "fastjet/JetDefinition.hh"
#include "fastjet/ClusterSequence.hh"
#include "fastjet/Selector.hh"
#include "fastjet/ClusterSequenceArea.hh"
#include "fastjet/tools/JetMedianBackgroundEstimator.hh"

#include "fastjet/plugins/SISCone/fastjet/SISConePlugin.hh"
#include "fastjet/plugins/CDFCones/fastjet/CDFMidPointPlugin.hh"
#include "fastjet/plugins/CDFCones/fastjet/CDFJetCluPlugin.hh"

#include "fastjet/contribs/Nsubjettiness/Nsubjettiness.hh"
#include "fastjet/contribs/Nsubjettiness/Njettiness.hh"
#include "fastjet/contribs/Nsubjettiness/NjettinessPlugin.hh"
#include "fastjet/contribs/Nsubjettiness/WinnerTakeAllRecombiner.hh"

using namespace std;
using namespace fastjet;
using namespace fastjet::contrib;


//------------------------------------------------------------------------------

FastJetFinder::FastJetFinder() :
  fPlugin(0), fRecomb(0), fNjettinessPlugin(0), fDefinition(0), fAreaDefinition(0), fItInputArray(0)
{

}

//------------------------------------------------------------------------------

FastJetFinder::~FastJetFinder()
{

}

//------------------------------------------------------------------------------

void FastJetFinder::Init()
{
  JetDefinition::Plugin *plugin = 0;
  JetDefinition::Recombiner *recomb = 0;
  NjettinessPlugin *njetPlugin = 0;

  // read eta ranges

  ExRootConfParam param = GetParam("RhoEtaRange");
  Long_t i, size;

  fEtaRangeMap.clear();
  size = param.GetSize();
  for(i = 0; i < size/2; ++i)
  {
    fEtaRangeMap[param[i*2].GetDouble()] = param[i*2 + 1].GetDouble();
  }

  // define algorithm

  fJetAlgorithm = GetInt("JetAlgorithm", 6);
  fParameterR = GetDouble("ParameterR", 0.5);

  fConeRadius = GetDouble("ConeRadius", 0.5);
  fSeedThreshold = GetDouble("SeedThreshold", 1.0);
  fConeAreaFraction = GetDouble("ConeAreaFraction", 1.0);
  fMaxIterations = GetInt("MaxIterations", 100);
  fMaxPairSize = GetInt("MaxPairSize", 2);
  fIratch = GetInt("Iratch", 1);
  fAdjacencyCut = GetInt("AdjacencyCut", 2);
  fOverlapThreshold = GetDouble("OverlapThreshold", 0.75);

  fJetPTMin = GetDouble("JetPTMin", 10.0);

  //-- N(sub)jettiness parameters --

  fComputeNsubjettiness = GetBool("ComputeNsubjettiness", false);
  fBeta = GetDouble("Beta", 1.0);
  fAxisMode = GetInt("AxisMode", 1);
  fRcutOff = GetDouble("RcutOff", 0.8); // used only if Njettiness is used as jet clustering algo (case 8)
  fN = GetInt("N", 2);                  // used only if Njettiness is used as jet clustering algo (case 8)

  // ---  Jet Area Parameters ---
  fAreaAlgorithm = GetInt("AreaAlgorithm", 0);
  fComputeRho = GetBool("ComputeRho", false);

  // - ghost based areas -
  fGhostEtaMax = GetDouble("GhostEtaMax", 5.0);
  fRepeat = GetInt("Repeat", 1);
  fGhostArea = GetDouble("GhostArea", 0.01);
  fGridScatter = GetDouble("GridScatter", 1.0);
  fPtScatter = GetDouble("PtScatter", 0.1);
  fMeanGhostPt = GetDouble("MeanGhostPt", 1.0E-100);

  // - voronoi based areas -
  fEffectiveRfact = GetDouble("EffectiveRfact", 1.0);

  switch(fAreaAlgorithm)
  {
    case 1:
      fAreaDefinition = new AreaDefinition(active_area_explicit_ghosts, GhostedAreaSpec(fGhostEtaMax, fRepeat, fGhostArea, fGridScatter, fPtScatter, fMeanGhostPt));
      break;
    case 2:
      fAreaDefinition = new AreaDefinition(one_ghost_passive_area, GhostedAreaSpec(fGhostEtaMax, fRepeat, fGhostArea, fGridScatter, fPtScatter, fMeanGhostPt));
      break;
    case 3:
      fAreaDefinition = new AreaDefinition(passive_area, GhostedAreaSpec(fGhostEtaMax, fRepeat, fGhostArea, fGridScatter, fPtScatter, fMeanGhostPt));
      break;
    case 4:
      fAreaDefinition = new AreaDefinition(VoronoiAreaSpec(fEffectiveRfact));
      break;
    case 5:
      fAreaDefinition = new AreaDefinition(active_area, GhostedAreaSpec(fGhostEtaMax, fRepeat, fGhostArea, fGridScatter, fPtScatter, fMeanGhostPt));
      break;
    default:
    case 0:
      fAreaDefinition = 0;
      break;
  }

  switch(fJetAlgorithm)
  {
    case 1:
      plugin = new CDFJetCluPlugin(fSeedThreshold, fConeRadius, fAdjacencyCut, fMaxIterations, fIratch, fOverlapThreshold);
      fDefinition = new JetDefinition(plugin);
      break;
    case 2:
      plugin = new CDFMidPointPlugin(fSeedThreshold, fConeRadius, fConeAreaFraction, fMaxPairSize, fMaxIterations, fOverlapThreshold);
      fDefinition = new JetDefinition(plugin);
      break;
    case 3:
      plugin = new SISConePlugin(fConeRadius, fOverlapThreshold, fMaxIterations, fJetPTMin);
      fDefinition = new JetDefinition(plugin);
      break;
    case 4:
      fDefinition = new JetDefinition(kt_algorithm, fParameterR);
      break;
    case 5:
      fDefinition = new JetDefinition(cambridge_algorithm, fParameterR);
      break;
    default:
    case 6:
      fDefinition = new JetDefinition(antikt_algorithm, fParameterR);
      break;
    case 7:
      recomb = new WinnerTakeAllRecombiner();
      fDefinition = new JetDefinition(antikt_algorithm, fParameterR, recomb, Best);
      break;
    case 8:
      njetPlugin = new NjettinessPlugin(fN, Njettiness::wta_kt_axes, Njettiness::unnormalized_cutoff_measure, fBeta, fRcutOff);
      fDefinition = new JetDefinition(njetPlugin);
      break;
  }

  fPlugin = plugin;
  fRecomb = recomb;
  fNjettinessPlugin = njetPlugin;

  ClusterSequence::print_banner();

  // import input array

  fInputArray = ImportArray(GetString("InputArray", "Calorimeter/towers"));
  fItInputArray = fInputArray->MakeIterator();

  // create output arrays

  fOutputArray = ExportArray(GetString("OutputArray", "jets"));
  fRhoOutputArray = ExportArray(GetString("RhoOutputArray", "rho"));
}

//------------------------------------------------------------------------------

void FastJetFinder::Finish()
{
  if(fItInputArray) delete fItInputArray;
  if(fDefinition) delete fDefinition;
  if(fAreaDefinition) delete fAreaDefinition;
  if(fPlugin) delete static_cast<JetDefinition::Plugin*>(fPlugin);
  if(fRecomb) delete static_cast<JetDefinition::Recombiner*>(fRecomb);
  if(fNjettinessPlugin) delete static_cast<JetDefinition::Plugin*>(fNjettinessPlugin);
}

//------------------------------------------------------------------------------

void FastJetFinder::Process()
{
  Candidate *candidate, *constituent;
  TLorentzVector momentum;
  Double_t deta, dphi, detaMax, dphiMax;
  Double_t time, weightTime, avTime;
  Int_t number;
  Double_t rho = 0;
  PseudoJet jet, area;
  vector<PseudoJet> inputList, outputList;
  ClusterSequence *sequence;
  map< Double_t, Double_t >::iterator itEtaRangeMap;

  DelphesFactory *factory = GetFactory();

  inputList.clear();

  // loop over input objects
  fItInputArray->Reset();
  number = 0;
  while((candidate = static_cast<Candidate*>(fItInputArray->Next())))
  {
    momentum = candidate->Momentum;
    jet = PseudoJet(momentum.Px(), momentum.Py(), momentum.Pz(), momentum.E());
    jet.set_user_index(number);
    inputList.push_back(jet);
    ++number;
  }

  // construct jets
  if(fAreaDefinition)
  {
    sequence = new ClusterSequenceArea(inputList, *fDefinition, *fAreaDefinition);
  }
  else
  {
    sequence = new ClusterSequence(inputList, *fDefinition);
  }

  // compute rho and store it
  if(fComputeRho && fAreaDefinition)
  {
    for(itEtaRangeMap = fEtaRangeMap.begin(); itEtaRangeMap != fEtaRangeMap.end(); ++itEtaRangeMap)
    {
      Selector select_rapidity = SelectorAbsRapRange(itEtaRangeMap->first, itEtaRangeMap->second);
      JetMedianBackgroundEstimator estimator(select_rapidity, *fDefinition, *fAreaDefinition);
      estimator.set_particles(inputList);
      rho = estimator.rho();

      candidate = factory->NewCandidate();
      candidate->Momentum.SetPtEtaPhiE(rho, 0.0, 0.0, rho);
      candidate->Edges[0] = itEtaRangeMap->first;
      candidate->Edges[1] = itEtaRangeMap->second;
      fRhoOutputArray->Add(candidate);
    }
  }

  outputList.clear();
  outputList = sorted_by_pt(sequence->inclusive_jets(fJetPTMin));


  // loop over all jets and export them
  detaMax = 0.0;
  dphiMax = 0.0;
  vector<PseudoJet>::iterator itInputList, itOutputList;
  for(itOutputList = outputList.begin(); itOutputList != outputList.end(); ++itOutputList)
  {
    momentum.SetPxPyPzE(itOutputList->px(), itOutputList->py(), itOutputList->pz(), itOutputList->E());
    area.reset(0.0, 0.0, 0.0, 0.0);
    if(fAreaDefinition) area = itOutputList->area_4vector();

    candidate = factory->NewCandidate();

    time=0;
    weightTime=0;

    inputList.clear();
    inputList = sequence->constituents(*itOutputList);

    for(itInputList = inputList.begin(); itInputList != inputList.end(); ++itInputList)
    {
      constituent = static_cast<Candidate*>(fInputArray->At(itInputList->user_index()));

      deta = TMath::Abs(momentum.Eta() - constituent->Momentum.Eta());
      dphi = TMath::Abs(momentum.DeltaPhi(constituent->Momentum));
      if(deta > detaMax) detaMax = deta;
      if(dphi > dphiMax) dphiMax = dphi;

      time += TMath::Sqrt(constituent->Momentum.E())*(constituent->Position.T());
      weightTime += TMath::Sqrt(constituent->Momentum.E());

      candidate->AddCandidate(constituent);
    }

    avTime = time/weightTime;

    candidate->Momentum = momentum;
    candidate->Position.SetT(avTime);
    candidate->Area.SetPxPyPzE(area.px(), area.py(), area.pz(), area.E());

    candidate->DeltaEta = detaMax;
    candidate->DeltaPhi = dphiMax;

    // --- compute N-subjettiness with N = 1,2,3,4,5 ----

    if(fComputeNsubjettiness)
    {
      Njettiness::AxesMode axisMode;

      switch(fAxisMode)
      {
        default:
        case 1:
          axisMode = Njettiness::wta_kt_axes;
          break;
        case 2:
          axisMode = Njettiness::onepass_wta_kt_axes;
          break;
        case 3:
          axisMode = Njettiness::kt_axes;
          break;
        case 4:
          axisMode = Njettiness::onepass_kt_axes;
          break;
      }

      Njettiness::MeasureMode measureMode = Njettiness::unnormalized_measure;

      Nsubjettiness nSub1(1, axisMode, measureMode, fBeta);
      Nsubjettiness nSub2(2, axisMode, measureMode, fBeta);
      Nsubjettiness nSub3(3, axisMode, measureMode, fBeta);
      Nsubjettiness nSub4(4, axisMode, measureMode, fBeta);
      Nsubjettiness nSub5(5, axisMode, measureMode, fBeta);

      candidate->Tau[0] = nSub1(*itOutputList);
      candidate->Tau[1] = nSub2(*itOutputList);
      candidate->Tau[2] = nSub3(*itOutputList);
      candidate->Tau[3] = nSub4(*itOutputList);
      candidate->Tau[4] = nSub5(*itOutputList);
    }


    fOutputArray->Add(candidate);
  }
  delete sequence;
}
