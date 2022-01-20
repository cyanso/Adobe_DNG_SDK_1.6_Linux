set(PROJECT_NAME dng_validate)

################################################################################
# Source groups
################################################################################
set(Property_Sheets
    "../settings/dngsdk32.props"
    "../settings/dngsdk64.props"
    "../settings/dngsdkBuild.props"
    "../settings/dngsdkDebug.props"
    "../settings/dngsdkFlags.props"
    "../settings/dngsdkFullLib.props"
    "../settings/dngsdkLib.props"
    "../settings/dngsdkRelease.props"
    "../settings/dngsdkRoot.props"
    "../settings/dngsdkValidateFlags.props"
    "../settings/dngsdkValidateTarget.props"
)
source_group("Property Sheets" FILES ${Property_Sheets})

set(dng_sdk
    "../../../source/dng_1d_function.cpp"
    "../../../source/dng_1d_function.h"
    "../../../source/dng_1d_table.cpp"
    "../../../source/dng_1d_table.h"
    "../../../source/dng_abort_sniffer.cpp"
    "../../../source/dng_abort_sniffer.h"
    "../../../source/dng_area_task.cpp"
    "../../../source/dng_area_task.h"
    "../../../source/dng_assertions.h"
    "../../../source/dng_auto_ptr.h"
    "../../../source/dng_bad_pixels.cpp"
    "../../../source/dng_bad_pixels.h"
    "../../../source/dng_big_table.cpp"
    "../../../source/dng_big_table.h"
    "../../../source/dng_bottlenecks.cpp"
    "../../../source/dng_bottlenecks.h"
    "../../../source/dng_camera_profile.cpp"
    "../../../source/dng_camera_profile.h"
    "../../../source/dng_classes.h"
    "../../../source/dng_color_space.cpp"
    "../../../source/dng_color_space.h"
    "../../../source/dng_color_spec.cpp"
    "../../../source/dng_color_spec.h"
    "../../../source/dng_date_time.cpp"
    "../../../source/dng_date_time.h"
    "../../../source/dng_errors.h"
    "../../../source/dng_exceptions.cpp"
    "../../../source/dng_exceptions.h"
    "../../../source/dng_exif.cpp"
    "../../../source/dng_exif.h"
    "../../../source/dng_fast_module.h"
    "../../../source/dng_file_stream.cpp"
    "../../../source/dng_file_stream.h"
    "../../../source/dng_filter_task.cpp"
    "../../../source/dng_filter_task.h"
    "../../../source/dng_fingerprint.cpp"
    "../../../source/dng_fingerprint.h"
    "../../../source/dng_flags.h"
    "../../../source/dng_gain_map.cpp"
    "../../../source/dng_gain_map.h"
    "../../../source/dng_globals.cpp"
    "../../../source/dng_globals.h"
    "../../../source/dng_host.cpp"
    "../../../source/dng_host.h"
    "../../../source/dng_hue_sat_map.cpp"
    "../../../source/dng_hue_sat_map.h"
    "../../../source/dng_ifd.cpp"
    "../../../source/dng_ifd.h"
    "../../../source/dng_image.cpp"
    "../../../source/dng_image.h"
    "../../../source/dng_image_writer.cpp"
    "../../../source/dng_image_writer.h"
    "../../../source/dng_info.cpp"
    "../../../source/dng_info.h"
    "../../../source/dng_iptc.cpp"
    "../../../source/dng_iptc.h"
    "../../../source/dng_jpeg_image.cpp"
    "../../../source/dng_jpeg_image.h"
    "../../../source/dng_lens_correction.cpp"
    "../../../source/dng_lens_correction.h"
    "../../../source/dng_linearization_info.cpp"
    "../../../source/dng_linearization_info.h"
    "../../../source/dng_local_string.cpp"
    "../../../source/dng_local_string.h"
    "../../../source/dng_lossless_jpeg.cpp"
    "../../../source/dng_lossless_jpeg.h"
    "../../../source/dng_matrix.cpp"
    "../../../source/dng_matrix.h"
    "../../../source/dng_memory.cpp"
    "../../../source/dng_memory.h"
    "../../../source/dng_memory_stream.cpp"
    "../../../source/dng_memory_stream.h"
    "../../../source/dng_misc_opcodes.cpp"
    "../../../source/dng_misc_opcodes.h"
    "../../../source/dng_mosaic_info.cpp"
    "../../../source/dng_mosaic_info.h"
    "../../../source/dng_mutex.cpp"
    "../../../source/dng_mutex.h"
    "../../../source/dng_negative.cpp"
    "../../../source/dng_negative.h"
    "../../../source/dng_opcode_list.cpp"
    "../../../source/dng_opcode_list.h"
    "../../../source/dng_opcodes.cpp"
    "../../../source/dng_opcodes.h"
    "../../../source/dng_orientation.cpp"
    "../../../source/dng_orientation.h"
    "../../../source/dng_parse_utils.cpp"
    "../../../source/dng_parse_utils.h"
    "../../../source/dng_pixel_buffer.cpp"
    "../../../source/dng_pixel_buffer.h"
    "../../../source/dng_point.cpp"
    "../../../source/dng_point.h"
    "../../../source/dng_preview.cpp"
    "../../../source/dng_preview.h"
    "../../../source/dng_pthread.cpp"
    "../../../source/dng_pthread.h"
    "../../../source/dng_rational.cpp"
    "../../../source/dng_rational.h"
    "../../../source/dng_read_image.cpp"
    "../../../source/dng_read_image.h"
    "../../../source/dng_rect.cpp"
    "../../../source/dng_rect.h"
    "../../../source/dng_ref_counted_block.cpp"
    "../../../source/dng_ref_counted_block.h"
    "../../../source/dng_reference.cpp"
    "../../../source/dng_reference.h"
    "../../../source/dng_render.cpp"
    "../../../source/dng_render.h"
    "../../../source/dng_resample.cpp"
    "../../../source/dng_resample.h"
    "../../../source/dng_safe_arithmetic.cpp"
    "../../../source/dng_safe_arithmetic.h"
    "../../../source/dng_sdk_limits.h"
    "../../../source/dng_shared.cpp"
    "../../../source/dng_shared.h"
    "../../../source/dng_simd_type.h"
    "../../../source/dng_simple_image.cpp"
    "../../../source/dng_simple_image.h"
    "../../../source/dng_spline.cpp"
    "../../../source/dng_spline.h"
    "../../../source/dng_stream.cpp"
    "../../../source/dng_stream.h"
    "../../../source/dng_string.cpp"
    "../../../source/dng_string.h"
    "../../../source/dng_string_list.cpp"
    "../../../source/dng_string_list.h"
    "../../../source/dng_tag_codes.h"
    "../../../source/dng_tag_types.cpp"
    "../../../source/dng_tag_types.h"
    "../../../source/dng_tag_values.h"
    "../../../source/dng_temperature.cpp"
    "../../../source/dng_temperature.h"
    "../../../source/dng_tile_iterator.cpp"
    "../../../source/dng_tile_iterator.h"
    "../../../source/dng_tone_curve.cpp"
    "../../../source/dng_tone_curve.h"
    "../../../source/dng_types.h"
    "../../../source/dng_uncopyable.h"
    "../../../source/dng_utils.cpp"
    "../../../source/dng_utils.h"
    "../../../source/dng_validate.cpp"
    "../../../source/dng_xmp.cpp"
    "../../../source/dng_xmp.h"
    "../../../source/dng_xmp_sdk.cpp"
    "../../../source/dng_xmp_sdk.h"
    "../../../source/dng_xy_coord.cpp"
    "../../../source/dng_xy_coord.h"
)
source_group("dng_sdk" FILES ${dng_sdk})

set(libjpeg
    "../../../../libjpeg/jaricom.c"
    "../../../../libjpeg/jcapimin.c"
    "../../../../libjpeg/jcapistd.c"
    "../../../../libjpeg/jcarith.c"
    "../../../../libjpeg/jccoefct.c"
    "../../../../libjpeg/jccolor.c"
    "../../../../libjpeg/jcdctmgr.c"
    "../../../../libjpeg/jchuff.c"
    "../../../../libjpeg/jcinit.c"
    "../../../../libjpeg/jcmainct.c"
    "../../../../libjpeg/jcmarker.c"
    "../../../../libjpeg/jcmaster.c"
    "../../../../libjpeg/jcomapi.c"
    "../../../../libjpeg/jconfig.h"
    "../../../../libjpeg/jcparam.c"
    "../../../../libjpeg/jcprepct.c"
    "../../../../libjpeg/jcsample.c"
    "../../../../libjpeg/jctrans.c"
    "../../../../libjpeg/jdapimin.c"
    "../../../../libjpeg/jdapistd.c"
    "../../../../libjpeg/jdarith.c"
    "../../../../libjpeg/jdatadst.c"
    "../../../../libjpeg/jdatasrc.c"
    "../../../../libjpeg/jdcoefct.c"
    "../../../../libjpeg/jdcolor.c"
    "../../../../libjpeg/jdct.h"
    "../../../../libjpeg/jddctmgr.c"
    "../../../../libjpeg/jdhuff.c"
    "../../../../libjpeg/jdinput.c"
    "../../../../libjpeg/jdmainct.c"
    "../../../../libjpeg/jdmarker.c"
    "../../../../libjpeg/jdmaster.c"
    "../../../../libjpeg/jdmerge.c"
    "../../../../libjpeg/jdpostct.c"
    "../../../../libjpeg/jdsample.c"
    "../../../../libjpeg/jdtrans.c"
    "../../../../libjpeg/jerror.c"
    "../../../../libjpeg/jerror.h"
    "../../../../libjpeg/jfdctflt.c"
    "../../../../libjpeg/jfdctfst.c"
    "../../../../libjpeg/jfdctint.c"
    "../../../../libjpeg/jidctflt.c"
    "../../../../libjpeg/jidctfst.c"
    "../../../../libjpeg/jidctint.c"
    "../../../../libjpeg/jmemmgr.c"
    "../../../../libjpeg/jmemnobs.c"
    "../../../../libjpeg/jpeglib.h"
    "../../../../libjpeg/jquant1.c"
    "../../../../libjpeg/jquant2.c"
    "../../../../libjpeg/jutils.c"
)
source_group("libjpeg" FILES ${libjpeg})

set(ALL_FILES
    ${Property_Sheets}
    ${dng_sdk}
    ${libjpeg}
)

################################################################################
# Target
################################################################################
add_executable(${PROJECT_NAME} ${ALL_FILES})

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
################################################################################
# Includes for CMake from *.props
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "ARM64")
    use_props(${PROJECT_NAME} ValidateDebug   "../settings/dngsdkRoot.cmake")
    use_props(${PROJECT_NAME} ValidateDebug   "../settings/dngsdkBuild.cmake")
    use_props(${PROJECT_NAME} ValidateDebug   "../settings/dngsdkFlags.cmake")
    use_props(${PROJECT_NAME} ValidateDebug   "../settings/dngsdkDebug.cmake")
    use_props(${PROJECT_NAME} ValidateDebug   "../settings/dngsdkArm64.cmake")
    use_props(${PROJECT_NAME} ValidateDebug   "../settings/dngsdkValidateFlags.cmake")
    use_props(${PROJECT_NAME} ValidateDebug   "../settings/dngsdkValidateTarget.cmake")
    use_props(${PROJECT_NAME} ValidateRelease "../settings/dngsdkRoot.cmake")
    use_props(${PROJECT_NAME} ValidateRelease "../settings/dngsdkBuild.cmake")
    use_props(${PROJECT_NAME} ValidateRelease "../settings/dngsdkFlags.cmake")
    use_props(${PROJECT_NAME} ValidateRelease "../settings/dngsdkRelease.cmake")
    use_props(${PROJECT_NAME} ValidateRelease "../settings/dngsdkArm64.cmake")
    use_props(${PROJECT_NAME} ValidateRelease "../settings/dngsdkValidateFlags.cmake")
    use_props(${PROJECT_NAME} ValidateRelease "../settings/dngsdkValidateTarget.cmake")
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    use_props(${PROJECT_NAME} ValidateDebug   "../settings/dngsdkRoot.cmake")
    use_props(${PROJECT_NAME} ValidateDebug   "../settings/dngsdkBuild.cmake")
    use_props(${PROJECT_NAME} ValidateDebug   "../settings/dngsdkFlags.cmake")
    use_props(${PROJECT_NAME} ValidateDebug   "../settings/dngsdkDebug.cmake")
    use_props(${PROJECT_NAME} ValidateDebug   "../settings/dngsdk64.cmake")
    use_props(${PROJECT_NAME} ValidateDebug   "../settings/dngsdkValidateFlags.cmake")
    use_props(${PROJECT_NAME} ValidateDebug   "../settings/dngsdkValidateTarget.cmake")
    use_props(${PROJECT_NAME} ValidateRelease "../settings/dngsdkRoot.cmake")
    use_props(${PROJECT_NAME} ValidateRelease "../settings/dngsdkBuild.cmake")
    use_props(${PROJECT_NAME} ValidateRelease "../settings/dngsdkFlags.cmake")
    use_props(${PROJECT_NAME} ValidateRelease "../settings/dngsdkRelease.cmake")
    use_props(${PROJECT_NAME} ValidateRelease "../settings/dngsdk64.cmake")
    use_props(${PROJECT_NAME} ValidateRelease "../settings/dngsdkValidateFlags.cmake")
    use_props(${PROJECT_NAME} ValidateRelease "../settings/dngsdkValidateTarget.cmake")
endif()

set(ROOT_NAMESPACE dng_validate)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_GLOBAL_KEYWORD "Win32Proj"
)
################################################################################
# Output directory
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "ARM64")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_DIRECTORY_VALIDATEDEBUG   "$<CONFIG>/${CMAKE_VS_PLATFORM_NAME}/"
        OUTPUT_DIRECTORY_VALIDATERELEASE "$<CONFIG>/${CMAKE_VS_PLATFORM_NAME}/"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_DIRECTORY_VALIDATEDEBUG   "$<CONFIG>/${CMAKE_VS_PLATFORM_NAME}/"
        OUTPUT_DIRECTORY_VALIDATERELEASE "$<CONFIG>/${CMAKE_VS_PLATFORM_NAME}/"
    )
endif()
################################################################################
# Compile definitions
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "ARM64")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "UNICODE;"
        "_UNICODE"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "UNICODE;"
        "_UNICODE"
    )
endif()

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "ARM64")
        target_compile_options(${PROJECT_NAME} PRIVATE
            ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
            ${DEFAULT_CXX_EXCEPTION_HANDLING};
            /Y-
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        target_compile_options(${PROJECT_NAME} PRIVATE
            ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
            ${DEFAULT_CXX_EXCEPTION_HANDLING};
            /Y-
        )
    endif()
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "ARM64")
        target_link_options(${PROJECT_NAME} PRIVATE
            /DEBUG:FULL
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        target_link_options(${PROJECT_NAME} PRIVATE
            /DEBUG:FULL
        )
    endif()
endif()

