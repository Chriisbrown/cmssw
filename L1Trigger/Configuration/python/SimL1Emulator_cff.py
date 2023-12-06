import FWCore.ParameterSet.Config as cms

# Defines the L1 Emulator sequence for simulation use-case subsystem emulators
# run on the results of previous (in the hardware chain) subsystem emulator:
#
# SimL1Emulator = cms.Sequence(...)
#
# properly configured for the current Era (e.g. Run1, 2015, or 2016). Also
# configures event setup producers appropriate to the current Era, to handle
# conditions which are not yet available in the GT.
#
# Author List
# Jim Brooke, 24 April 2008
# Vasile Mihai Ghete, 2009
# Jim Brooke, Michael Mulhearn, 2015
# Vladimir Rekovic 2016,2017

# Notes on Inputs:

# ECAL TPG emulator and HCAL TPG run in the simulation sequence in order to be able
# to use unsuppressed digis produced by ECAL and HCAL simulation, respectively
# in Configuration/StandardSequences/python/Digi_cff.py
# SimCalorimetry.Configuration.SimCalorimetry_cff
# which calls
# SimCalorimetry.Configuration.ecalDigiSequence_cff
# SimCalorimetry.Configuration.hcalDigiSequence_cff

#
# At the moment, there is no emulator available for upgrade HF Trigger Primitives,
# so these missing (required!) inputs are presently ignored by downstream modules.
#

from L1Trigger.Configuration.SimL1TechnicalTriggers_cff import *

from L1Trigger.L1TCalorimeter.simDigis_cff import *
from L1Trigger.L1TMuon.simDigis_cff import *
from L1Trigger.L1TGlobal.simDigis_cff import *

# define a core which can be extented in customizations:
SimL1EmulatorCoreTask = cms.Task(
SimL1TCalorimeterTask,
SimL1TMuonTask,
SimL1TechnicalTriggersTask,
SimL1TGlobalTask
)
SimL1EmulatorCore = cms.Sequence(SimL1EmulatorCoreTask)

SimL1EmulatorTask = cms.Task(SimL1EmulatorCoreTask)
SimL1Emulator = cms.Sequence( SimL1EmulatorTask )

#
# Emulators are configured from DB (GlobalTags)
#

from L1Trigger.L1TGlobal.GlobalParameters_cff import *

# 2017 EMTF and TwinMux emulator use payloads from DB, not yet in GT,
# soon to be removed when availble in GTs
from L1Trigger.L1TTwinMux.fakeTwinMuxParams_cff import *

_phase2_siml1emulator = SimL1EmulatorTask.copy()

# ########################################################################
# ########################################################################
#
# Phase-2
#
# ########################################################################
# ########################################################################

# ########################################################################
# Phase-2 Trigger Primitives
# ########################################################################
from L1Trigger.DTTriggerPhase2.CalibratedDigis_cfi import *
CalibratedDigis.dtDigiTag = "simMuonDTDigis"
_phase2_siml1emulator.add(CalibratedDigis)
from L1Trigger.DTTriggerPhase2.dtTriggerPhase2PrimitiveDigis_cfi import *
_phase2_siml1emulator.add(dtTriggerPhase2PrimitiveDigis)

# HGCAL TP
# ########################################################################
from L1Trigger.L1THGCal.hgcalTriggerPrimitives_cff import *
_phase2_siml1emulator.add(L1THGCalTriggerPrimitivesTask)

# ########################################################################
# Phase 2 L1T
# ########################################################################

# Barrel and EndCap EGamma
# ########################################################################
from L1Trigger.L1CaloTrigger.l1tEGammaCrystalsEmulatorProducer_cfi import *
_phase2_siml1emulator.add(l1tEGammaClusterEmuProducer)

from L1Trigger.L1CaloTrigger.l1tPhase2L1CaloEGammaEmulator_cfi import *
_phase2_siml1emulator.add(l1tPhase2L1CaloEGammaEmulator)

# Barrel and EndCap CaloJet/HT
# ########################################################################
# ---- Produce the calibrated tower collection combining Barrel, HGCal, HF
from L1Trigger.L1CaloTrigger.l1tTowerCalibrationProducer_cfi import *
l1tTowerCalibration = l1tTowerCalibrationProducer.clone(
L1HgcalTowersInputTag = ("l1tHGCalTowerProducer","HGCalTowerProcessor",""),
l1CaloTowers = ("l1tEGammaClusterEmuProducer","L1CaloTowerCollection","")
)
# ---- Produce the L1CaloJets
from L1Trigger.L1CaloTrigger.l1tCaloJetProducer_cfi import *
l1tCaloJet = l1tCaloJetProducer.clone (
l1CaloTowers = ("l1tTowerCalibration","L1CaloTowerCalibratedCollection",""),
L1CrystalClustersInputTag = ("l1tEGammaClusterEmuProducer", "","")
)
# ---- Produce the CaloJet HTT Sums
from L1Trigger.L1CaloTrigger.l1tCaloJetHTTProducer_cfi import *
l1tCaloJetHTT = l1tCaloJetHTTProducer.clone(
BXVCaloJetsInputTag = ("L1CaloJet", "CaloJets")
)

cut = "NN"

_phase2_siml1emulator.add(l1tTowerCalibration)
_phase2_siml1emulator.add(l1tCaloJet)
_phase2_siml1emulator.add(l1tCaloJetHTT)

# ########################################################################
# Phase-2 L1T - TrackTrigger dependent modules
# ########################################################################
from L1Trigger.L1TTrackMatch.l1tGTTInputProducer_cfi import *
from L1Trigger.VertexFinder.l1tVertexProducer_cfi import *
from L1Trigger.TrackFindingTracklet.Producer_cfi import *
# L1TRK_NAME  = TrackFindingTrackletProducer_params.LabelTT.value()
# L1TRK_LABEL = TrackFindingTrackletProducer_params.BranchAcceptedTracks.value()
L1TRK_NAME  = "l1tTTTracksFromTrackletEmulation"
L1TRK_LABEL = "Level1TTTracks"
l1tGTTInputProducer.l1TracksInputTag = cms.InputTag(L1TRK_NAME,L1TRK_LABEL)
l1tVertexFinder = l1tVertexProducer.clone()
l1tVertexFinderEmulator = l1tVertexProducer.clone()
if cut == "FH":
    l1tVertexFinderEmulator.VertexReconstruction.Algorithm = "fastHistoEmulation"
if cut == "Truth":
    l1tVertexFinderEmulator.VertexReconstruction.Algorithm = "fastHistoEmulation"
if cut == "NN":
    l1tVertexFinderEmulator.VertexReconstruction.Algorithm = "NNEmulation"#"NNEmulation"

_phase2_siml1emulator.add(l1tGTTInputProducer)
_phase2_siml1emulator.add(l1tGTTInputProducerExtended)
_phase2_siml1emulator.add(l1tVertexFinderEmulator)

# Emulated GMT Muons (Tk + Stub, Tk + MuonTFT, StandaloneMuon)
# ########################################################################
from L1Trigger.Phase2L1GMT.gmt_cfi import *
l1tTkStubsGmt = l1tGMTStubs.clone()
l1tTkMuonsGmt = l1tGMTMuons.clone(
srcStubs = 'l1tTkStubsGmt'
)
l1tSAMuonsGmt = l1tStandaloneMuons.clone()
_phase2_siml1emulator.add( l1tTkStubsGmt )
_phase2_siml1emulator.add( l1tTkMuonsGmt )
_phase2_siml1emulator.add( l1tSAMuonsGmt )

## fix for low-pt muons, this collection is a copy of the l1tTkMuonsGmt collection
## in which we only keep those low pt muons with an SA muon associated to it. Threshold
## for this cutoff is configurable.
l1tTkMuonsGmtLowPtFix = l1tGMTFilteredMuons.clone()
_phase2_siml1emulator.add( l1tTkMuonsGmtLowPtFix )

# PF Candidates
# ########################################################################

from L1Trigger.Phase2L1ParticleFlow.l1ctLayer1_cff import *
from L1Trigger.Phase2L1ParticleFlow.l1ctLayer2EG_cff import *
_phase2_siml1emulator.add(L1TLayer1TaskInputsTask, L1TLayer1Task, L1TLayer2EGTask)

# PF MET
# ########################################################################
from L1Trigger.L1TTrackMatch.l1tTrackSelectionProducer_cfi import *
from L1Trigger.Phase2L1ParticleFlow.l1tPFTracksFromL1Tracks_cfi import * 
# Track selection from no cut,  FH dZ cuts or NN association
l1tTrackSelectionProducer.cutSet = cms.PSet(ptMin = cms.double(2.0), # pt must be greater than this value, [GeV]
                                             absEtaMax = cms.double(2.4), # absolute value of eta must be less than this value
                                             absZ0Max = cms.double(15.0), # z0 must be less than this value, [cm]
                                             nStubsMin = cms.int32(4), # number of stubs must be greater than or equal to this value
                                             nPSStubsMin = cms.int32(0), # the number of stubs in the PS Modules must be greater than or equal to this value

                                             reducedBendChi2Max = cms.double(10000), # bend chi2 must be less than this value
                                             reducedChi2RZMax = cms.double(10000), # chi2rz/dof must be less than this value
                                             reducedChi2RPhiMax = cms.double(10000), # chi2rphi/dof must be less than this value

                                             deltaZMaxEtaBounds = cms.vdouble(0.0, 0.7, 1.0, 1.2, 1.6, 2.0, 2.4), # these values define the bin boundaries in |eta|
                                             deltaZMax = cms.vdouble(30, 30, 30, 30, 30, 30), # delta z must be less than these values, there will be one less value here than in deltaZMaxEtaBounds, [cm]
                                             )

if cut == "FH":
    l1tTrackSelectionProducer.l1TracksInputTag = cms.InputTag(L1TRK_NAME, L1TRK_LABEL)
    l1tTrackSelectionProducer.processSimulatedTracks = cms.bool(True)
    l1tTrackSelectionProducer.processEmulatedTracks = cms.bool(False)
if cut == "Truth":
    l1tTrackSelectionProducer.l1TracksInputTag = cms.InputTag(L1TRK_NAME, L1TRK_LABEL)
    l1tTrackSelectionProducer.processSimulatedTracks = cms.bool(False)
    l1tTrackSelectionProducer.processEmulatedTracks = cms.bool(True)
    l1tTrackSelectionProducer.useTruth = cms.bool(True)
if cut == "NN":
    l1tTrackSelectionProducer.l1TracksInputTag = cms.InputTag("l1tGTTInputProducer", "Level1TTTracksConverted")
    l1tTrackSelectionProducer.processSimulatedTracks = cms.bool(False)
    l1tTrackSelectionProducer.processEmulatedTracks = cms.bool(True)
    l1tTrackSelectionProducer.useAssociationNetwork = cms.bool(True) #Enable Association Network
    l1tTrackSelectionProducer.AssociationThreshold = cms.double(0.1) #Association Network threshold for PV tracks

# Standard PUPPI Quality Cuts
if cut == "NN":
    l1tPFTracksFromL1Tracks.L1TrackTag = cms.InputTag("l1tTrackSelectionProducer", "Level1TTTracksSelectedAssociatedEmulation")
    l1tPFTracksFromL1TracksExtended.L1TrackTag = cms.InputTag("l1tTrackSelectionProducerExtended", "Level1TTTracksExtendedSelectedAssociatedEmulation")
    l1tPFTracksFromL1Tracks.qualityBits = cms.vstring(
        "momentum.perp > 2 && getStubRefs.size >= 4",
        "momentum.perp > 2 && getStubRefs.size >= 4", # historical reasons
        "momentum.perp > 2 && getStubRefs.size >= 4"
    )

else:
    l1tPFTracksFromL1Tracks.L1TrackTag = cms.InputTag("l1tTrackSelectionProducer", "Level1TTTracksSelectedAssociatedEmulation")
    l1tPFTracksFromL1TracksExtended.L1TrackTag = cms.InputTag("l1tTrackSelectionProducerExtended", "Level1TTTracksExtendedSelectedAssociatedEmulation")

_phase2_siml1emulator.add(l1tTrackSelectionProducer)
_phase2_siml1emulator.add(l1tTrackSelectionProducerExtended)
_phase2_siml1emulator.add(l1tPFTracksFromL1Tracks)

from L1Trigger.Phase2L1ParticleFlow.l1tMETPFProducer_cfi import *
_phase2_siml1emulator.add(l1tMETPFProducer)

# --> add modules
from Configuration.Eras.Modifier_phase2_trigger_cff import phase2_trigger
phase2_trigger.toReplaceWith( SimL1EmulatorTask , _phase2_siml1emulator)
