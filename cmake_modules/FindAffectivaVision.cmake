include(FindPackageHandleStandardArgs)
unset(AffectivaVision_FOUND)

find_path(AffectivaVision_INCLUDE_DIR
    NAMES FrameDetector.h
    HINTS ${AFFECTIVA_SDK_DIR}/include)

find_library(AffectivaVision_LIBRARY
    NAMES affectiva-vision
    HINTS ${AFFECTIVA_SDK_DIR}/lib
    HINTS ${AFFECTIVA_SDK_DIR}/lib/Release)

find_package_handle_standard_args(AffectivaVision DEFAULT_MSG AffectivaVision_INCLUDE_DIR AffectivaVision_LIBRARY)

mark_as_advanced(AffectivaVision_INCLUDE_DIR AffectivaVision_LIBRARY)

set(AffectivaVision_INCLUDE_DIRS ${AffectivaVision_INCLUDE_DIR})
set(AffectivaVision_LIBRARIES ${AffectivaVision_LIBRARY})
