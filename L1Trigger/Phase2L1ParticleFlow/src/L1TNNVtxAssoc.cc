// Package:      L1Trigger/Phase2L1ParticleFlow
// Class:        L1TNNVtxAssoc
// Description:  Designed to run the track to vertex associations created by the E2E NNVtx.
//               TTTrackNetworkSelector either accepts or rejects that a PF object's (t) track is associated to a vertex (v).
// Authors:      Kai Hong Law and Benjamin Radburn-Smith
// Created:      February 2025                                                                             #include "L1Trigger/Phase2L1ParticleFlow/interface/L1TNNVtxAssoc.h"
#include "L1Trigger/Phase2L1ParticleFlow/interface/L1TNNVtxAssoc.h"
#include "DataFormats/L1TParticleFlow/interface/PFTrack.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include <cmath>

L1TNNVtxAssoc::L1TNNVtxAssoc(const std::shared_ptr<hls4mlEmulator::Model> model, bool debug)
    : modelRef_(model) {
  NNvectorVar_.clear();
  isDebugEnabled_ = debug;

  fPt_ = 0;
  fMVA_ = 0;
  fResBin_ = 0;
  fDz_ = 0;
}

void L1TNNVtxAssoc::setNNVectorVar() {
  NNvectorVar_.clear();
  if (isDebugEnabled_) {
    LogDebug("L1TNNVtxAssoc") << "\n ===== Input Vector =====" << std::endl;
  }

  NNvectorVar_.push_back(fPt_);                              // pt
  NNvectorVar_.push_back(fMVA_);                          //pT as a fraction of jet pT
  NNvectorVar_.push_back(fResBin_);                          // pt log
  NNvectorVar_.push_back(fDz_);                            //dEta from jet axis

  std::cout << fPt_ << " | " << fResBin_ << " | " << fMVA_ << " | " << fDz_ << std::endl;

    if (isDebugEnabled_) {
      LogDebug("L1TNNVtxAssoc") 
                                << "pT: " << NNvectorVar_[0 ]
                                << " | "
                                   "Track Quality MVA: "
                                << NNvectorVar_[1]
                                << " | "
                                   "Resolution Bin: "
                                << NNvectorVar_[2]
                                << " | "
                                   "dZ: "
                                << NNvectorVar_[3]
                                << " | "
                                << "===========" << std::endl;
    }
}

bool L1TNNVtxAssoc::EvaluateNNFixed( double threshold) {
  const int NInputs = 4;
  classtype classresult;

  inputtype fillzero = 0.0;

  inputtype modelInput[NInputs] = {};  // Do something
  std::fill(modelInput, modelInput + NInputs, fillzero);
  for (unsigned int i = 0; i < NNvectorVar_.size(); i++) {
    modelInput[i] = NNvectorVar_[i];
  }
  modelRef_->prepare_input(modelInput);
  modelRef_->predict();
  modelRef_->read_result(&classresult);

  float modelResult_;
  if (isDebugEnabled_) {
    LogDebug("L1TNNVtxAssoc") << "\n ===== Vertex Association Output Score =====" << std::endl;
  }
    modelResult_ = classresult.to_float();
    if (isDebugEnabled_) {
      LogDebug("L1TNNVtxAssoc") << "Score" << " : " << modelResult_
                                << std::endl;
    }
  float NNOutput_exp = 1.0 / (1.0 + exp(-1.0 * (modelResult_)));
  return NNOutput_exp >= threshold;
}  //end EvaluateNNFixed

template <typename T>
bool L1TNNVtxAssoc::computeFixed(const PFRegionEmu& region, T& t, const l1ct::PVObjEmu& v, const double threshold) {
  auto lower = std::lower_bound(eta_bins_.begin(), eta_bins_.end(), region.floatGlbEta(t.hwVtxEta()));

  int resbin = std::distance(eta_bins_.begin(), lower);
  float binWidth = z0_binning_[2];
  // Calculate integer dZ from track z0 and vertex z0 (use floating point version and convert internally allowing use of both emulator and simulator vertex and track)
  float dZ =
      abs(floor(((t.floatZ0() + z0_binning_[1]) / (binWidth))) - floor(((v.floatZ0() + z0_binning_[1]) / (binWidth))));

  // The following constants <22, 9> are defined by the quantisation of the Neural Network
  fPt_ = t.hwPt;
  fResBin_=  res_bins_[resbin] / 16;
  fMVA_ = 0;
  fDz_ = dZ;

  std::cout << fPt_ << " | " << fResBin_ << " | " << fMVA_ << " | " << fDz_ << std::endl;

  // Deal with this template class using 2 different objects (t) which have different calls to their PFTracks:
  const l1t::PFTrack* srcTrack = NULL;
  if constexpr (std::is_same_v<T, const l1ct::TkObjEmu>)
    srcTrack = t.src;
  else if constexpr (std::is_same_v<T, const l1ct::PFChargedObjEmu>)
    srcTrack = t.srcTrack;
  if (srcTrack)
    fMVA_ = srcTrack->trackWord().getMVAQualityBits();

  setNNVectorVar();
  return EvaluateNNFixed(threshold);
}

edm::ParameterSetDescription L1TNNVtxAssoc::getParameterSetDescription() {
  edm::ParameterSetDescription description;
  description.add<double>("associationThreshold");
  description.add<std::string>("associationModelPath");
  return description;
}

template bool L1TNNVtxAssoc::computeFixed<const l1ct::TkObjEmu>(const PFRegionEmu&,
                                                             const l1ct::TkObjEmu&,
                                                             const l1ct::PVObjEmu&,
                                                             const double );
template bool L1TNNVtxAssoc::computeFixed<const l1ct::PFChargedObjEmu>(const PFRegionEmu&,
                                                                    const l1ct::PFChargedObjEmu&,
                                                                    const l1ct::PVObjEmu&,
                                                                    const double );