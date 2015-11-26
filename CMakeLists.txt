
project(GraphCutSegment)
cmake_minimum_required(VERSION 2.8.9)

#-----------------------------------------------------------------------------
# Extension meta-information
set(EXTENSION_HOMEPAGE "http://www.example.com/Slicer/Extensions/GraphCutSegment")
set(EXTENSION_CATEGORY "Segmentation")
set(EXTENSION_CONTRIBUTORS "Daphne Chen (Western University)")
set(EXTENSION_DESCRIPTION "This is a segment extension using graph cut algorithm.")
set(EXTENSION_ICONURL "http://www.example.com/Slicer/Extensions/GraphCutSegment.png")
set(EXTENSION_SCREENSHOTURLS "http://www.example.com/Slicer/Extensions/GraphCutSegment/Screenshots/1.png")

#-----------------------------------------------------------------------------
# Extension dependencies
find_package(Slicer REQUIRED)
include(${Slicer_USE_FILE})

#-----------------------------------------------------------------------------
# Extension modules
add_subdirectory(GraphCutInteractiveSegmenter)
add_subdirectory(GraphCutSegmentSelfTest)
## NEXT_MODULE

#-----------------------------------------------------------------------------
include(${Slicer_EXTENSION_CPACK})
