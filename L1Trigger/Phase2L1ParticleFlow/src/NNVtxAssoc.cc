// Package:      L1Trigger/Phase2L1ParticleFlow
// Class:        NNVtxAssoc
// Description:  Designed to run the track to vertex associations created by the E2E NNVtx.
//               TTTrackNetworkSelector either accepts or rejects that a PF object's (t) track is associated to a vertex (v).
// Authors:      Kai Hong Law and Benjamin Radburn-Smith
// Created:      February 2025

#include "L1Trigger/Phase2L1ParticleFlow/interface/NNVtxAssoc.h"
#include "DataFormats/L1TParticleFlow/interface/PFTrack.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include <cmath>
#include <iomanip>

NNVtxAssoc::NNVtxAssoc(const std::shared_ptr<hls4mlEmulator::Model> model,
                       const double AssociationThreshold,
                       const std::vector<double>& AssociationNetworkZ0binning,
                       const std::vector<double>& AssociationNetworkEtaBounds,
                       const std::vector<double>& AssociationNetworkZ0ResBins,
                       bool debug
                       )
    : associationThreshold_(AssociationThreshold),
      z0_binning_(AssociationNetworkZ0binning),
      eta_bins_(AssociationNetworkEtaBounds),
      res_bins_(AssociationNetworkZ0ResBins),
      modelRef_(model),
      isDebugEnabled_(debug)  {
  NNvectorVar_.clear();

  fPt_ = 0;
  fMVA_ = 0;
  fResBin_ = 0;
  fDz_ = 0;
  log_.setf(std::ios::fixed, std::ios::floatfield);
  log_.precision(3);
}

void NNVtxAssoc::setNNVectorVar() {
  NNvectorVar_.clear();
  if (isDebugEnabled_) {
    LogDebug("NNVtxAssoc") << "\n ===== Input Vector =====" << std::endl;
  }

  NNvectorVar_.push_back(fPt_);                              // pt
  NNvectorVar_.push_back(fMVA_);                          //pT as a fraction of jet pT
  NNvectorVar_.push_back(fResBin_);                          // pt log
  NNvectorVar_.push_back(fDz_);                            //dEta from jet axis

    if (isDebugEnabled_) {
      LogDebug("NNVtxAssoc") 
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

bool NNVtxAssoc::EvaluateNNFixed(float& score) {
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
    LogDebug("NNVtxAssoc") << "\n ===== Vertex Association Output Score =====" << std::endl;
  }
    modelResult_ = classresult.to_float();
    if (isDebugEnabled_) {
      LogDebug("NNVtxAssoc") << "Score" << " : " << modelResult_
                                << std::endl;
    }
  float NNOutput_exp = 1.0 / (1.0 + exp(-1.0 * (modelResult_)));
  score = NNOutput_exp;
  return NNOutput_exp >= associationThreshold_;
}  //end EvaluateNNFixed

template <typename T>
bool NNVtxAssoc::TTTrackNetworkSelector(const PFRegionEmu& region, const T& t, const l1ct::PVObjEmu& v, float& score) {
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

    // Deal with this template class using 2 different objects (t) which have different calls to their PFTracks:
    const l1t::PFTrack* srcTrack = NULL;
    if constexpr (std::is_same_v<T, const l1ct::TkObjEmu>)
      srcTrack = t.src;
    else if constexpr (std::is_same_v<T, const l1ct::PFChargedObjEmu>)
      srcTrack = t.srcTrack;
    if (srcTrack)
      fMVA_ = srcTrack->trackWord().getMVAQualityBits();

    setNNVectorVar();
    return EvaluateNNFixed(score);
}

#ifdef CMSSW_GIT_HASH
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"

edm::ParameterSetDescription NNVtxAssoc::getParameterSetDescription() {
  edm::ParameterSetDescription description;
  description.add<double>("associationThreshold");
  description.add<std::string>("associationNetworkPath");
  description.add<std::vector<double>>("associationNetworkZ0binning");
  description.add<std::vector<double>>("associationNetworkEtaBounds");
  description.add<std::vector<double>>("associationNetworkZ0ResBins");
  return description;
}
#endif

void NNVtxAssoc::NNVtxAssocDebug() {
  log_ << "-- NNVtxAssocDebug --\n";
  log_ << "AssociationThreshold: " << this->associationThreshold_ << "\n";
  log_ << "z0_binning: ";
  for (auto i : this->z0_binning_)
    log_ << i << " ";
  log_ << "\n";
  log_ << "eta_bins: ";
  for (auto i : this->eta_bins_)
    log_ << i << " ";
  log_ << "\n";
  log_ << "res_bins: ";
  for (auto i : this->res_bins_)
    log_ << i << " ";
  log_ << "\n";
  edm::LogPrint("NNVtxAssoc") << log_.str();
}

template bool NNVtxAssoc::TTTrackNetworkSelector<const l1ct::TkObjEmu>(const PFRegionEmu&,
                                                                       const l1ct::TkObjEmu&,
                                                                       const l1ct::PVObjEmu&,
                                                                       float& score);
template bool NNVtxAssoc::TTTrackNetworkSelector<const l1ct::PFChargedObjEmu>(const PFRegionEmu&,
                                                                              const l1ct::PFChargedObjEmu&,
                                                                              const l1ct::PVObjEmu&,
                                                                              float& score);
