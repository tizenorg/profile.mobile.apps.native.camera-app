CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(camera C CXX)

message(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")
message("furcate arm or i686")
EXEC_PROGRAM(uname ARGS -m OUTPUT_VARIABLE CMAKE_SYSTEM_VERSION)
message("This package is building by ${CMAKE_SYSTEM_VERSION}")
message(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")

SET(SRCS_BASE
	src/animation/cam_animation.c
	src/animation/cam_ui_effect_utils.c
	src/animation/cam_ui_rotate_utils.c
	src/app-control/cam_app.c
	src/app-control/cam_common_edc_callback.c
	src/capacity/cam_device_capacity.cpp
	src/capacity/CamDeviceCapacity.cpp
	src/capacity/cam_typeconverter.cpp
	src/capacity/CamTypeConverter.cpp
	src/core/cam.c
	src/dev-control/cam_mm.c
	src/menu/cam_menu_composer.c
	src/menu/cam_menu_item.c
	src/record/cam_rec.c
	src/shot/cam_shot.c
	src/shot/cam_selfie_alarm_shot.c
	src/system-mgr/cam_exif_info.c
	src/system-mgr/cam_file.c
	src/system-mgr/cam_sound.c
	src/system-mgr/cam_sound_session_manager.c
	src/system-mgr/cam_sr_control.c
	src/system-mgr/cam_lbs.c
	src/util/cam_config.c
	src/util/cam_face_detection_focus.c
	src/util/cam_long_press.c
	src/util/cam_utils.c
	src/util/cam_mode.c
	src/util/cam_mem_check.c
	src/view/main-view/cam_recording_view.c
	src/view/main-view/cam_setting_view.c
	src/view/main-view/cam_shot_processing_view.c
	src/view/main-view/cam_standby_view.c
	src/view/main-view/cam_mode_view.c
	src/view/main-view/cam_mode_view_grid_mode.c
	src/view/main-view/cam_mode_view_utils.c
	src/view/main-view/cam_selfie_alarm_layout.c
	src/view/widget/cam_popup.c
	src/view/widget/cam_storage_popup.c
	src/view/widget/cam_ev_edc_callback.c
	src/view/widget/cam_indicator.c
	src/view/widget/cam_zoom_edc_callback.c
	src/view/widget/cam_edit_box.c
	src/view/widget/cam_genlist_popup.c
	src/view/widget/cam_gengrid_popup.c
	src/view/widget/cam_help_popup.c
)

if(CAMERA_MODE_BEST_PHOTO)
	SET(SRCS_BEST_PHOTO
		src/engine/cam_best_photo_shot_engine.c
	)
endif()

SET(SRCS ${SRCS_BASE}
	#${SRCS_PANORAMA}
	#${SRCS_BEAUTY_FACE}
	${SRCS_BEST_PHOTO}
)

INCLUDE_DIRECTORIES(
	${CMAKE_CURRENT_SOURCE_DIR}/inc
	src/
	src/animation
	src/app-control
	src/capacity
	src/core
	src/dev-control
	src/menu
	src/record
	src/shot
	src/engine
	src/system-mgr
	src/util
	src/view/main-view
	src/view/widget
)

INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs REQUIRED
	bundle
	cairo
	capi-appfw-application
	capi-appfw-preference
	capi-content-media-content
	capi-media-audio-io
	capi-media-camera
	capi-media-image-util
	capi-media-recorder
	capi-media-sound-manager
	capi-media-wav-player
	capi-system-device
	capi-location-manager
	capi-system-runtime-info
	dlog
	ecore
	#ecore-x
	efl-extension
	eina
	elementary
        ui-gadget-1
	feedback
	glib-2.0
	gobject-2.0
	libexif
	storage
	vconf
	appcore-common
	capi-system-system-settings
	capi-telephony
	notification
	libtzplatform-config
)

FOREACH(flag ${pkgs_CFLAGS})
	SET(EXTRA_CXXFLAGS "${EXTRA_CXXFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CXXFLAGS} -Wall -fPIC -fvisibility=hidden")
SET(CMAKE_C_FLAGS ${CMAKE_CXX_FLAGS})

SET(PREFIX ${CMAKE_INSTALL_PREFIX})

SET(BINDIR "${PREFIX}/bin")
SET(LIBDIR "${PREFIX}/lib")
SET(RESDIR "${PREFIX}/res")
SET(SHAREDDIR "${PREFIX}/shared")
SET(EDJDIR "${RESDIR}/edje")
SET(LOCALEDIR "${RESDIR}/locale")

ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")
ADD_DEFINITIONS("-DFACTORYFS=\"$ENV{PREFIX}\"")
ADD_DEFINITIONS("-DSLP_DEBUG")

if(NOT "${CMAKE_SYSTEM_VERSION}" MATCHES "^arm.*")
	ADD_DEFINITIONS(-DCAMERA_MACHINE_I686)
endif()

########################## set custom-defined lib path  begin ########################
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
SET(CMAKE_INSTALL_RPATH "${LIBDIR}")
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
########################## set custom-defined lib path  end ##########################

MESSAGE("LDFLAGS= ${pkgs_LDFLAGS}")
MESSAGE("CMAKE_C_FLAGS= ${CMAKE_C_FLAGS}")
MESSAGE("prefix=${CMAKE_INSTALL_PREFIX}")

ADD_EXECUTABLE(${PROJECT_NAME} ${SRCS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${pkgs_LDFLAGS} -lm -ldl -pie)

###################################### EDC, Begin ################################
SET(EDC_DIR		${CMAKE_SOURCE_DIR}/edc)
SET(IMAGE_DIR	${CMAKE_SOURCE_DIR}/images)
SET(SOUND_DIR	${CMAKE_SOURCE_DIR}/sounds)

SET(EDC_FILES
	cam_main_layout
	cam_recording_view
	cam_setting_view
	cam_shot_processing_view
	cam_standby_view
	style_ctxpopup_camera_with_radio
	cam_utils
	cam_ev_layout
	cam_zoom_layout
	cam_images
	style_gengrid_camera
	cam_shooting_mode_layout
	cam_quicksetting_help
	style_button_camera
	cam_shot_guide_text
	cam_selfie_layout
)

FOREACH(file_name ${EDC_FILES})
	ADD_CUSTOM_TARGET(${file_name}.edj
		COMMAND edje_cc -no-save -id ${IMAGE_DIR}
		-sd ${SOUND_DIR}
		${EDC_DIR}/${file_name}.edc ${EDC_DIR}/${file_name}.edj
		DEPENDS ${EDC_DIR}/${file_name}.edc
	)
	ADD_DEPENDENCIES(${PROJECT_NAME} ${file_name}.edj)
	INSTALL(FILES ${EDC_DIR}/${file_name}.edj DESTINATION ${EDJDIR})
ENDFOREACH(file_name)
###################################### EDC, end ################################

#################################### install 3rd party libs begin ##################################
#if("${CMAKE_SYSTEM_VERSION}" MATCHES "^arm.*")
#	INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/library/arcsoft/common/libarcplatform_arm.so DESTINATION ${LIBDIR})
#
#	if(CAMERA_MODE_PANORAMA)
#		INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/library/arcsoft/panorama/libarcsoft_panorama_burstcapture.so DESTINATION ${LIBDIR})
#	endif()
#
#	if(CAMERA_MODE_BEAUTY_FACE)
#		INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/library/arcsoft/beauty-face/libarcsoft_flawlessface.so DESTINATION ${LIBDIR})
#		INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/library/arcsoft/beauty-face/libarcsoft_skin_soften.so DESTINATION ${LIBDIR})
#	endif()
#
#	if(CAMERA_MODE_BEST_PHOTO)
#		INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/library/dmc/best-photo/libarcpicbest_arm.so DESTINATION ${LIBDIR})
#	endif()
#endif()
#################################### install 3rd party libs end ####################################

# install bin
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION bin)

# install manifest file & icon

INSTALL(FILES ${CMAKE_SOURCE_DIR}/org.tizen.camera-app.xml DESTINATION ${CMAKE_APP_SHARE_PACKAGES_DIR})
INSTALL(FILES ${CMAKE_SOURCE_DIR}/org.tizen.camera-app.png DESTINATION ${CMAKE_APP_ICON_DIR})
INSTALL(FILES ${CMAKE_SOURCE_DIR}/fake_portrait.png DESTINATION ${SHAREDDIR}/res/)

# install sound & image files
INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/sounds DESTINATION ${RESDIR})

# install .camera.ini file
INSTALL(FILES ${CMAKE_SOURCE_DIR}/.camera.ini DESTINATION ${RESDIR})

# po files
ADD_SUBDIRECTORY(po)

