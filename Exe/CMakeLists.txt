SET( CONFOREG_EXE_ITK_SRC
  register
# register3D
  plot
# plot3D
  getpart
  )

SET( CONFOREG_EXE_OTB_SRC
  OTBTest
  )

FOREACH( var ${CONFOREG_EXE_ITK_SRC} )
  ADD_EXECUTABLE( ${var} ${var} )
  TARGET_LINK_LIBRARIES( ${var}
  ITKIO
  ITKCommon
  ITKBasicFilters
  ITKAlgorithms
  ITKStatistics
  ITKNumerics
  )
ENDFOREACH( var ${CONFOREG_EXE_ITK_SRC} )

FOREACH( var ${CONFOREG_EXE_OTB_SRC} )
  ADD_EXECUTABLE( ${var} ${var} )
  TARGET_LINK_LIBRARIES( ${var}
  OTBCommon 
  OTBIO
  OTBMultiScale
  )
ENDFOREACH( var ${CONFOREG_EXE_OTB_SRC} )