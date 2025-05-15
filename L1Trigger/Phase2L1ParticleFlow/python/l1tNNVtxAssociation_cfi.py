import FWCore.ParameterSet.Config as cms
import os
# NNVertex Association Variables
L1TNNVtxAssociationPSet = cms.PSet(
    associationThreshold = cms.double(0.1), #Association Network threshold for PV tracks
    associationModelPath = cms.string(os.environ["CMSSW_BASE"]+"/src/L1TNNVtx_Assoc_Model/L1TNNVtx_Assoc_Model_v0")
)