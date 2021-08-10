# Automatically generated build file. Do not edit.
COMPONENT_INCLUDES += $(PROJECT_PATH)/components/music/include $(PROJECT_PATH)/components/music/migu_sdk/include
COMPONENT_LDFLAGS += -L$(BUILD_DIR_BASE)/music -lmusic -L$(PROJECT_PATH)/components/music/migu_sdk/lib -lmigu -lmigu_music_service -lmigu_sdk_helper -lmigu_https
COMPONENT_LINKER_DEPS += 
COMPONENT_SUBMODULES += 
COMPONENT_LIBRARIES += music
COMPONENT_LDFRAGMENTS += 
component-music-build: 
