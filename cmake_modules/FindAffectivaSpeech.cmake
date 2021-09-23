include(FindPackageHandleStandardArgs)
unset(AffectivaSpeech_FOUND)

find_path(AffectivaSpeech_INCLUDE_DIR
    NAMES SpeechDetector.h
    HINTS ${AFFECTIVA_SDK_DIR}/include)

find_library(AffectivaSpeech_LIBRARY
    NAMES affectiva-speech
    HINTS ${AFFECTIVA_SDK_DIR}/lib)


find_package_handle_standard_args(AffectivaSpeech DEFAULT_MSG AffectivaSpeech_INCLUDE_DIR AffectivaSpeech_LIBRARY)

mark_as_advanced(AffectivaSpeech_INCLUDE_DIR AffectivaSpeech_LIBRARY)

set(AffectivaSpeech_INCLUDE_DIRS ${AffectivaSpeech_INCLUDE_DIR})
set(AffectivaSpeech_LIBRARIES ${AffectivaSpeech_LIBRARY})
