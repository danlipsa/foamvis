set (SOURCES Application.cpp ApproximationEdge.cpp
  Attribute.cpp AttributeCreator.cpp AttributeInfo.cpp
  AttributeAverages.cpp AttributeAverages2D.cpp AttributeAverages3D.cpp
  AttributeHistogram.cpp Average.cpp AverageShaders.cpp
  AdjacentBody.cpp PipelineAverage3D.cpp
  Base.cpp Body.cpp BodyAlongTime.cpp
  BodySelector.cpp BrowseSimulations.cpp
  ConstraintEdge.cpp ColorBarModel.cpp Comparisons.cpp
  DataProperties.cpp
  Debug.cpp Disk.cpp DisplayBodyFunctors.cpp DisplayElement.cpp
  ImageBasedAverage.cpp DisplayFaceFunctors.cpp
  DisplayEdgeFunctors.cpp Edge.cpp
  HistogramStatistics.cpp
  EditColorMap.cpp Element.cpp ExpressionTree.cpp
  Enums.cpp Foam.cpp FoamvisInteractorStyle.cpp
  Face.cpp ForceOneObject.cpp
  ForceAverage.cpp
  WidgetBase.cpp WidgetGl.cpp WidgetHistogram.cpp
  WidgetSave.cpp WidgetVtk.cpp
  Histogram.cpp HistogramItem.cpp
  HistogramSettings.cpp main.cpp MainWindow.cpp
  NameSemanticValue.cpp
  OOBox.cpp ObjectPosition.cpp OpenGLUtils.cpp
  OrientedElement.cpp Options.cpp
  OrientedEdge.cpp OrientedFace.cpp AdjacentOrientedFace.cpp
  ScalarAverage.cpp Settings.cpp SelectBodiesById.cpp
  ParsingData.cpp PipelineBase.cpp ParsingDriver.cpp
  ParsingEnums.cpp ProcessBodyTorus.cpp
  PropertySetter.cpp ShaderProgram.cpp
  QuadraticEdge.cpp RegularGridAverage.cpp
  RestrictedRangeSlider.cpp Simulation.cpp
  StripIterator.cpp ScalarDisplay.cpp T1KDE2D.cpp TimeStepsSlider.cpp
  T1.cpp TransferFunctionHistogram.cpp TensorAverage.cpp
  Utils.cpp VectorAverage.cpp Vertex.cpp
  ViewSettings.cpp VectorOperation.cpp)

find_package(Qt4)
if (NOT QT4_FOUND)
  message(SEND_ERROR "Qt ${QT_MIN_VERSION} or greater not found.  "
    "Please check the QT_QMAKE_EXECUTABLE variable.")
endif ()



add_executable (foam $(SOURCES))
