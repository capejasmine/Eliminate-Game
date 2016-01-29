define all-files-under
$(patsubst ./%,%, \
  $(shell cd $(LOCAL_PATH) ; \
          find $(1) -name "$(2)" -and -not -name ".*") \
 )
endef

define all-cpp-files-under
$(call all-files-under,$(1),*.cpp)
endef

define all-c-files-under
$(call all-files-under,$(1),*.c)
endef

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

$(call import-add-path,$(LOCAL_PATH)/../../cocos2dx/cocos2d)
$(call import-add-path,$(LOCAL_PATH)/../../cocos2dx/cocos2d/external)
$(call import-add-path,$(LOCAL_PATH)/../../cocos2dx/cocos2d/cocos)

FILE_INCLUDES := $(shell find $(LOCAL_PATH)/../../Classes -type d)

LOCAL_MODULE := cocos2dcpp_shared

LOCAL_MODULE_FILENAME := libcocos2dcpp

LOCAL_SRC_FILES := $(call all-cpp-files-under,../../Classes) \
    $(call all-c-files-under,../../Classes) \
    $(call all-cpp-files-under,.) \	

LOCAL_C_INCLUDES := $(shell find $(LOCAL_PATH)/../../Classes -type d)\
#$(LOCAL_PATH)/../../cocos2d/cocos/editor-support/cocostudio\
#$(LOCAL_PATH)/../../cocos2d/extensions\

LOCAL_WHOLE_STATIC_LIBRARIES := cocos2dx_static
LOCAL_WHOLE_STATIC_LIBRARIES += cocosdenshion_static

# LOCAL_WHOLE_STATIC_LIBRARIES += box2d_static
# LOCAL_WHOLE_STATIC_LIBRARIES += cocosbuilder_static
# LOCAL_WHOLE_STATIC_LIBRARIES += spine_static
# LOCAL_WHOLE_STATIC_LIBRARIES += cocostudio_static
# LOCAL_WHOLE_STATIC_LIBRARIES += cocos_network_static
# LOCAL_WHOLE_STATIC_LIBRARIES += cocos_extension_static


include $(BUILD_SHARED_LIBRARY)

$(call import-module,.)
$(call import-module,audio/android)

# $(call import-module,Box2D)
# $(call import-module,editor-support/cocosbuilder)
# $(call import-module,editor-support/spine)
# $(call import-module,editor-support/cocostudio)
# $(call import-module,network)
# $(call import-module,extensions)
