#ifndef Calorimeter_h
#define Calorimeter_h

/** \class Calorimeter
 *
 *  Fills calorimeter towers, performs calorimeter resolution smearing,
 *  preselects towers hit by photons and creates energy flow objects.
 *
 *  $Date: 2013-09-03 17:54:56 +0200 (Tue, 03 Sep 2013) $
 *  $Revision: 1273 $
 *
 *
 *  \author P. Demin - UCL, Louvain-la-Neuve
 *
 */

#include "classes/DelphesModule.h"

#include <map>
#include <set>
#include <vector>

class TObjArray;
class DelphesFormula;
class Candidate;

class Calorimeter: public DelphesModule
{
public:

  Calorimeter();
  ~Calorimeter();

  void Init();
  void Process();
  void Finish();

private:

  typedef std::map< Long64_t, std::pair< Double_t, Double_t > > TFractionMap; //!
  typedef std::map< Double_t, std::set< Double_t > > TBinMap; //!

  Candidate *fTower;
  Double_t fTowerEta, fTowerPhi, fTowerEdges[4];
  Double_t fTowerECalEnergy, fTowerHCalEnergy;
  Double_t fTrackECalEnergy, fTrackHCalEnergy;
  Int_t fTowerTrackHits, fTowerPhotonHits;

  TFractionMap fFractionMap; //!
  TBinMap fBinMap; //!

  std::vector < Double_t > fEtaBins;
  std::vector < std::vector < Double_t >* > fPhiBins;

  std::vector < Long64_t > fTowerHits;

  std::vector < Double_t > fTowerECalFractions;
  std::vector < Double_t > fTowerHCalFractions;

  std::vector < Double_t > fTrackECalFractions;
  std::vector < Double_t > fTrackHCalFractions;

  DelphesFormula *fECalResolutionFormula; //!
  DelphesFormula *fHCalResolutionFormula; //!

  TIterator *fItParticleInputArray; //!
  TIterator *fItTrackInputArray; //!

  const TObjArray *fParticleInputArray; //!
  const TObjArray *fTrackInputArray; //!

  TObjArray *fTowerOutputArray; //!
  TObjArray *fPhotonOutputArray; //!

  TObjArray *fEFlowTrackOutputArray; //!
  TObjArray *fEFlowTowerOutputArray; //!

  TObjArray *fTowerTrackArray; //!
  TIterator *fItTowerTrackArray; //!

  void FinalizeTower();
  Double_t LogNormal(Double_t mean, Double_t sigma);

  ClassDef(Calorimeter, 1)
};

#endif
