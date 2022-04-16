.PHONY: all build build-container cmake format format-container shell image build-container clean clean-image clean-all flash
############################### Native Makefile ###############################

PROJECT_NAME := firmware
BUILD_DIR ?= build
BUILD_TYPE ?= Debug
PLATFORM = $(OS)
ifeq ($(PLATFORM),Windows_NT)
	BUILD_SYSTEM ?= MinGW Makefiles
else
	UNAME := $(shell uname -s)
	ifeq ($(UNAME),Linux)
		BUILD_SYSTEM ?= Unix Makefiles
	else
		@echo "Unsuported platform"
		exit 1
	endif
endif

all: build

build: cmake
	$(MAKE) -C ${BUILD_DIR} --no-print-directory

cmake: ${BUILD_DIR}/Makefile

${BUILD_DIR}/Makefile:
	cmake \
		-G "$(BUILD_SYSTEM)" \
		-B${BUILD_DIR} \
		-DPROJECT_NAME=$(PROJECT_NAME) \
		-DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
		-DCMAKE_TOOLCHAIN_FILE=gcc-arm-none-eabi.cmake \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
		-DDUMP_ASM=OFF

SRCS := $(shell find . -name '*.[ch]' -or -name '*.[ch]pp')
format: $(addsuffix .format,${SRCS})
%.format: %
	clang-format -i $<

clean:
	rm -rf ${BUILD_DIR}

flash: $(BUILD_DIR)/jflash
	JLinkExe -commanderscript $<
# Change to yours
TARGET := $(BUILD_DIR)/flash-crc32.bin
device = STM32F407VG
# First create a file with all commands
$(BUILD_DIR)/jflash:
	@touch $@
	@echo device $(device) > $@
	@echo -e si 1'\n'speed 4000 >> $@
	@echo loadfile $(TARGET),0x8000000 >> $@
	@echo -e r'\n'g'\n'qc >> $@

################################## Container ##################################

UID ?= $(shell id -u)
GID ?= $(shell id -g)
USER ?= $(shell id -un)
GROUP ?= $(shell id -gn)
WORKDIR := ${PWD}

CONTAINER_TOOL ?= docker
CONTAINER_FILE := Dockerfile
IMAGE_NAME := fedora-arm-embedded-dev
CONTAINER_NAME := fedora-arm-embedded-dev

NEED_IMAGE = $(shell $(CONTAINER_TOOL) image inspect ${IMAGE_NAME} 2> /dev/null > /dev/null || echo image)
NEED_CONTAINER = $(shell $(CONTAINER_TOOL) container inspect ${CONTAINER_NAME} 2> /dev/null > /dev/null || echo container)
PODMAN_ARG = $(if $(filter $(CONTAINER_TOOL), podman),--userns=keep-id,)
CONTAINER_RUN = $(CONTAINER_TOOL) run \
				--name ${CONTAINER_NAME} \
				--rm \
				-it \
				$(PODMAN_ARG) \
				-v ${PWD}:/workdir \
				--workdir /workdir \
				--security-opt label=disable \
				--hostname ${CONTAINER_NAME} \
				${IMAGE_NAME}

build-container: ${NEED_IMAGE}
	${CONTAINER_RUN} bash -lc 'make -j$(shell nproc)'

format-container: ${NEED_CONTAINER}
	${CONTAINER_RUN} bash -lc 'make format -j$(shell nproc)'

shell: ${NEED_CONTAINER}
	${CONTAINER_RUN} bash -l

image: ${CONTAINER_FILE}
	$(CONTAINER_TOOL) build \
		-t ${IMAGE_NAME} \
		-f=${CONTAINER_FILE} \
		--build-arg UID=$(UID) \
		--build-arg GID=$(GID) \
		--build-arg USERNAME=$(USER) \
		--build-arg GROUPNAME=$(GROUP) \
		.

clean-image:
	$(CONTAINER_TOOL) container rm -f ${CONTAINER_NAME} 2> /dev/null > /dev/null || true
	$(CONTAINER_TOOL) image rmi -f ${IMAGE_NAME} 2> /dev/null > /dev/null || true

clean-all: clean clean-image
