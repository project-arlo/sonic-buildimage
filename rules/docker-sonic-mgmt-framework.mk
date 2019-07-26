# docker image for mgmt-framework

DOCKER_MGMT-FRAMEWORK_STEM = docker-sonic-mgmt-framework
DOCKER_MGMT-FRAMEWORK = $(DOCKER_MGMT-FRAMEWORK_STEM).gz
DOCKER_MGMT-FRAMEWORK_DBG = $(DOCKER_MGMT-FRAMEWORK_STEM)-$(DBG_IMAGE_MARK).gz

$(DOCKER_MGMT-FRAMEWORK)_PATH = $(DOCKERS_PATH)/$(DOCKER_MGMT-FRAMEWORK_STEM)

$(DOCKER_MGMT-FRAMEWORK)_DEPENDS += $(REDIS_TOOLS) $(SONIC_MGMT-FRAMEWORK)
#$(DOCKER_MGMT-FRAMEWORK)_DBG_DEPENDS = $($(DOCKER_CONFIG_ENGINE_STRETCH)_DBG_DEPENDS)

SONIC_DOCKER_IMAGES += $(DOCKER_MGMT-FRAMEWORK)
$(DOCKER_MGMT-FRAMEWORK)_LOAD_DOCKERS += $(DOCKER_CONFIG_ENGINE_STRETCH)
#$(DOCKER_MGMT-FRAMEWORK)_DBG_IMAGE_PACKAGES = $($(DOCKER_CONFIG_ENGINE_STRETCH)_DBG_IMAGE_PACKAGES)

SONIC_INSTALL_DOCKER_IMAGES += $(DOCKER_MGMT-FRAMEWORK)
SONIC_STRETCH_DOCKERS += $(DOCKER_MGMT-FRAMEWORK)

$(DOCKER_MGMT-FRAMEWORK)_CONTAINER_NAME = mgmt-framework
$(DOCKER_MGMT-FRAMEWORK)_RUN_OPT += --net=host --privileged -t
$(DOCKER_MGMT-FRAMEWORK)_RUN_OPT += -v /etc/sonic:/etc/sonic:ro
$(DOCKER_MGMT-FRAMEWORK)_RUN_OPT += --mount type=bind,source="/var/platform/",target="/mnt/platform/"
