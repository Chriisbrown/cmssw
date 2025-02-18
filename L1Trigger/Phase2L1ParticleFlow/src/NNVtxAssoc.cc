// Package:      L1Trigger/Phase2L1ParticleFlow
// Class:        NNVtxAssoc
// Description:  Designed to run the track to vertex associations created by the E2E NNVtx.
//               TTTrackNetworkSelector either accepts or rejects that a PF object's (t) track is associated to a vertex (v).
// Authors:      Kai Hong Law and Benjamin Radburn-Smith
// Created:      February 2025

#include "L1Trigger/Phase2L1ParticleFlow/interface/NNVtxAssoc.h"
#include "DataFormats/L1TParticleFlow/interface/PFTrack.h"
#include <iomanip>

NNVtxAssoc::NNVtxAssoc(tensorflow::Session* AssociationSesh,
                       const double AssociationThreshold,
                       const std::vector<double>& AssociationNetworkZ0binning,
                       const std::vector<double>& AssociationNetworkEtaBounds,
                       const std::vector<double>& AssociationNetworkZ0ResBins)
    : AssociationSesh_(AssociationSesh),
      AssociationThreshold_(AssociationThreshold),
      z0_binning_(AssociationNetworkZ0binning),
      eta_bins_(AssociationNetworkEtaBounds),
      res_bins_(AssociationNetworkZ0ResBins) {}

template <typename T>
bool NNVtxAssoc::TTTrackNetworkSelector(const PFRegionEmu& region, T& t, const l1ct::PVObjEmu& v) {
  tensorflow::Tensor inputAssoc(tensorflow::DT_FLOAT, {1, 4});
  std::vector<tensorflow::Tensor> outputAssoc;

  auto lower = std::lower_bound(eta_bins_.begin(), eta_bins_.end(), region.floatGlbEta(t.hwVtxEta()));

  int resbin = std::distance(eta_bins_.begin(), lower);
  float binWidth = z0_binning_[2];
  // Calculate integer dZ from track z0 and vertex z0 (use floating point version and convert internally allowing use of both emulator and simulator vertex and track)
  float dZ =
      abs(floor(((t.floatZ0() + z0_binning_[1]) / (binWidth))) - floor(((v.floatZ0() + z0_binning_[1]) / (binWidth))));

  // The following constants <22, 9> are defined by the quantisation of the Neural Network
  ap_ufixed<22, 9> ptEmulation_rescale = t.hwPt;
  ap_ufixed<22, 9> resBinEmulation_rescale = res_bins_[resbin];
  ap_ufixed<22, 9> MVAEmulation_rescale = 0;
  ap_ufixed<22, 9> dZEmulation_rescale = dZ;

  // Deal with this template class using 2 different objects (t) which have different calls to their PFTracks:
  const l1t::PFTrack* srcTrack = NULL;
  if constexpr (std::is_same_v<T, const l1ct::TkObjEmu>)
    srcTrack = t.src;
  else if constexpr (std::is_same_v<T, const l1ct::PFChargedObjEmu>)
    srcTrack = t.srcTrack;
  if (srcTrack)
    MVAEmulation_rescale = srcTrack->trackWord().getMVAQualityBits();

  inputAssoc.tensor<float, 2>()(0, 0) = ptEmulation_rescale.to_double();
  inputAssoc.tensor<float, 2>()(0, 1) = MVAEmulation_rescale.to_double();
  inputAssoc.tensor<float, 2>()(0, 2) = resBinEmulation_rescale.to_double() / 16.0;
  inputAssoc.tensor<float, 2>()(0, 3) = dZEmulation_rescale.to_double();

  // Run Association Network:
  tensorflow::run(AssociationSesh_,
                  {{"assoc:0", inputAssoc}},
                  {"Identity:0"},
                  &outputAssoc);  //BRS: Need to update with new training

  double NNOutput = (double)outputAssoc[0].tensor<float, 2>()(0, 0);
  double NNOutput_exp = 1.0 / (1.0 + exp(-1.0 * (NNOutput)));

  return NNOutput_exp >= AssociationThreshold_;
}

#ifdef CMSSW_GIT_HASH
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"

edm::ParameterSetDescription NNVtxAssoc::getParameterSetDescription() {
  edm::ParameterSetDescription description;
  description.add<double>("associationThreshold");
  description.add<std::string>("associationGraph");
  description.add<std::vector<double>>("associationNetworkZ0binning");
  description.add<std::vector<double>>("associationNetworkEtaBounds");
  description.add<std::vector<double>>("associationNetworkZ0ResBins");
  return description;
}
#endif

void NNVtxAssoc::NNVtxAssocDebug() {
  // ToDo: Can switch this to use logs
  std::cout << std::setprecision(3) << std::fixed;
  std::cout << "-- NNVtxAssocDebug --\n";
  std::cout << "AssociationThreshold: " << this->AssociationThreshold_ << "\n";
  std::cout << "z0_binning: ";
  for (auto i : this->z0_binning_)
    std::cout << i << " ";
  std::cout << "\n";
  std::cout << "eta_bins: ";
  for (auto i : this->eta_bins_)
    std::cout << i << " ";
  std::cout << "\n";
  std::cout << "res_bins: ";
  for (auto i : this->res_bins_)
    std::cout << i << " ";
  std::cout << "\n";
}

template bool NNVtxAssoc::TTTrackNetworkSelector<const l1ct::TkObjEmu>(const PFRegionEmu&,
                                                                       const l1ct::TkObjEmu&,
                                                                       const l1ct::PVObjEmu&);
template bool NNVtxAssoc::TTTrackNetworkSelector<const l1ct::PFChargedObjEmu>(const PFRegionEmu&,
                                                                              const l1ct::PFChargedObjEmu&,
                                                                              const l1ct::PVObjEmu&);
