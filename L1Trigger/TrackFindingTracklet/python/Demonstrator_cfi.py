# configuration of Demonstrator. This is used to compare FW with SW for the subset fo the chain between LabelIn & LabelOut. FW must be wrapped by EMP & compiled with IPBB.
import FWCore.ParameterSet.Config as cms

TrackTriggerDemonstrator_params = cms.PSet (

  LabelIn  = cms.string( "TrackFindingTrackletProducerKF"  ), #
  LabelOut = cms.string( "TrackFindingTrackletProducerKFout" ), 
  DirIPBB  = cms.string( "/vols/cms/cb719/emp_inputfiles/tq/" ), # path to ipbb proj area
  RunTime  = cms.double( 8.0 )                                  # runtime in us

)
