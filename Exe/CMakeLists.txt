SET( CONFOREG_EXE_SRC
  register
  )


FOREACH( var ${CONFOREG_EXE_SRC} )
  ADD_EXECUTABLE( ${var} ${var} )
  TARGET_LINK_LIBRARIES( ${var}
  ITKIO
  ITKQuadEdgeMesh
  ITKCommon
  ITKBasicFilters
  ITKAlgorithms
  ITKStatistics
  ITKNumerics
  )
ENDFOREACH( var ${CONFOREG_EXE_SRC} )
