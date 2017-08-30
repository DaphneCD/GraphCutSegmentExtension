cmake_minimum_required(VERSION 3.5)

project(TumorSegm)

find_package(VTK)
include(${VTK_USE_FILE})

set(tumor_segm_SRCS
  AdaptiveSegment2D.cpp
  AdaptiveSegment2D.h
  AdaptiveSegment3D.cpp
  AdaptiveSegment3D.h
  AppData.cpp
  AppData.h
  common.h
  Data2D.h
  Data2Mat.h
  Data3D.h
  Graph2D.cpp
  Graph2D.h
  Graph3D.cpp
  Graph3D.h
  GraphLinkCost.h
  HardConstraint.cpp
  HardConstraint.h
  Image2D.h
  Image3D.h
  ImageProcessing.cpp
  ImageProcessing.h
  MyBasic.h
  MyMath.cpp
  MyMath.h
  smart_assert.h
  Smoothterm.cpp
  Smoothterm.h
  StarShape.cpp
  StarShape.h
  VectOperation.h
  BK/block.h
  BK/graph.cpp
  BK/graph.h
  BK/maxflow.cpp
  )

add_library(TumorSegm STATIC ${tumor_segm_SRCS})
target_link_libraries(TumorSegm PUBLIC ${VTK_LIBRARIES})

