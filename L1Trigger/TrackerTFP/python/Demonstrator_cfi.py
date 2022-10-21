# configuration of Demonstrator.
import FWCore.ParameterSet.Config as cms

TrackTriggerDemonstrator_params = cms.PSet (

  LabelIn  = cms.string( "TrackerTFPProducerTBout"          ), #
  LabelOut = cms.string( "TrackerTFPProducerKFout"            ), #
  DirIPBB  = cms.string( "/vols/cms/cb719/emp_inputfiles/skinny_chain/" ), # path to ipbb proj area
  RunTime  = cms.double( 6.0 )                                # runtime in us

)
