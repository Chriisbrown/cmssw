#ifndef RECOEGAMMA_EGAMMAISOLATIONALGOS_EGAMMAISOHCALDETIDCOLLECTIONPRODUCER_H
#define RECOEGAMMA_EGAMMAISOLATIONALGOS_EGAMMAISOHCALDETIDCOLLECTIONPRODUCER_H

// -*- C++ -*-
//
// Package:    EgammaIsoHcalDetIdCollectionProducer
// Class:      EgammaIsoHcalDetIdCollectionProducer
//
/**\class EgammaIsoHcalDetIdCollectionProducer 
Original author: Sam Harper (RAL)
 
Make a collection of detids to be kept tipically in a AOD rechit collection
Modified from the ECAL version "InterestingDetIdCollectionProducer" to be HCAL

*/

// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/HcalRecHit/interface/HcalRecHitCollections.h"
#include "DataFormats/EgammaReco/interface/SuperClusterFwd.h"
#include "DataFormats/EgammaReco/interface/SuperCluster.h"
#include "DataFormats/EgammaCandidates/interface/GsfElectronFwd.h"
#include "DataFormats/EgammaCandidates/interface/GsfElectron.h"
#include "DataFormats/EgammaCandidates/interface/PhotonFwd.h"
#include "DataFormats/EgammaCandidates/interface/Photon.h"

#include "RecoEgamma/EgammaIsolationAlgos/interface/EGHcalRecHitSelector.h"

class EgammaIsoHcalDetIdCollectionProducer : public edm::stream::EDProducer<> {
public:
  explicit EgammaIsoHcalDetIdCollectionProducer(const edm::ParameterSet&);
  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

  void beginRun(edm::Run const&, const edm::EventSetup&) final;
  void produce(edm::Event&, const edm::EventSetup&) override;

private:
  // ----------member data ---------------------------
  edm::EDGetTokenT<HBHERecHitCollection> recHitsToken_;
  edm::EDGetTokenT<reco::SuperClusterCollection> superClustersToken_;
  edm::EDGetTokenT<reco::GsfElectronCollection> elesToken_;
  edm::EDGetTokenT<reco::PhotonCollection> phosToken_;

  std::string interestingDetIdCollection_;

  float minSCEt_;
  float minEleEt_;
  float minPhoEt_;

  EGHcalRecHitSelector hcalHitSelector_;
};

#endif
