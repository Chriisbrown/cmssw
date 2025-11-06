#ifndef L1TRIGGER_PHASE2L1PARTICLEFLOWS_NNVtxAssoc_H
#define L1TRIGGER_PHASE2L1PARTICLEFLOWS_NNVtxAssoc_H

// Package:      L1Trigger/Phase2L1ParticleFlow
// Class:        NNVtxAssoc
// Description:  Designed to run the track to vertex associations created by the E2E NNVtx.
//               TTTrackNetworkSelector either accepts or rejects that a PF object's (t) track is associated to a vertex (v).
// Authors:      Kai Hong Law and Benjamin Radburn-Smith
// Created:      February 2025

#include <string>
#include <iomanip>
#include "DataFormats/L1TParticleFlow/interface/layer1_emulator.h"

//HLS4ML compiled emulator modeling
#include "ap_fixed.h"
#include "hls4ml/emulator.h"

namespace edm {
  class ParameterSet;
  class ParameterSetDescription;
}  // namespace edm
using namespace l1ct;

class NNVtxAssoc {
public:
  NNVtxAssoc(const std::shared_ptr<hls4mlEmulator::Model> model, 
             const double AssociationThreshold,
             const std::vector<double>& AssociationNetworkZ0binning,
             const std::vector<double>& AssociationNetworkEtaBounds,
             const std::vector<double>& AssociationNetworkZ0ResBins,
             bool debug);
  void NNVtxAssocDebug();

  typedef ap_ufixed<22,9> inputtype;
  typedef ap_fixed<22,9> classtype;

  void setNNVectorVar();
  bool EvaluateNNFixed(float& score);
  static edm::ParameterSetDescription getParameterSetDescription();

  template <typename T>
  bool TTTrackNetworkSelector(const PFRegionEmu& region, const T& t, const l1ct::PVObjEmu& v,float& score);

private:
  std::vector<inputtype> NNvectorVar_;
  float fPt_;
  float fMVA_;
  float fResBin_;
  float fDz_;

  double associationThreshold_;

  std::vector<double> z0_binning_ ;
  std::vector<double> eta_bins_;
  std::vector<double> res_bins_;

  std::shared_ptr<hls4mlEmulator::Model> modelRef_;

  std::stringstream log_;

  bool isDebugEnabled_;

};
#endif
