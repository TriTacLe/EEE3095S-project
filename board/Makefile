.DEFAULT_GOAL := all

TARGET := GameServer
CONFIG ?= Debug
PREFIX ?= arm-none-eabi-

ifeq ($(CONFIG),Debug)
OPT ?= -O0
DEBUG_FLAGS := -g3
else ifeq ($(CONFIG),Release)
OPT ?= -Os
DEBUG_FLAGS :=
else
$(error CONFIG must be Debug or Release)
endif

BUILD_DIR ?= build/$(CONFIG)

# Use the compiler on PATH, an explicitly supplied GCC_PATH, or the usual
# CubeIDE toolchain location. No toolchain is downloaded by make.
ifndef GCC_PATH
ifeq ($(shell command -v $(PREFIX)gcc 2>/dev/null),)
GCC_PATH := $(firstword $(wildcard \
  /opt/st/stm32cubeide*/plugins/com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.*/tools/bin \
  $(HOME)/st/stm32cubeide*/plugins/com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.*/tools/bin \
  /Applications/STM32CubeIDE.app/Contents/Eclipse/plugins/com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.*/tools/bin))
endif
endif

TOOL_PREFIX := $(if $(GCC_PATH),$(GCC_PATH)/,)$(PREFIX)
CC      := "$(TOOL_PREFIX)gcc"
OBJCOPY := "$(TOOL_PREFIX)objcopy"
SIZE    := "$(TOOL_PREFIX)size"

# The game core and the board application live outside this directory so the
# same sources feed the host simulator in ../sim.
CORE_DIR  := ../server/core
STM32_DIR := ../server/stm32

C_SOURCES := $(sort $(wildcard Core/Src/*.c) \
                    $(wildcard Drivers/STM32F0xx_HAL_Driver/Src/*.c) \
                    $(wildcard $(CORE_DIR)/*.c) \
                    $(STM32_DIR)/app.c \
                    $(STM32_DIR)/port_stm32.c)
ASM_SOURCES := Core/Startup/startup_stm32f051c6tx.s

vpath %.c $(sort $(dir $(C_SOURCES)))
vpath %.s $(dir $(ASM_SOURCES))

OBJECTS := $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o))) \
           $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
DEPS := $(OBJECTS:.o=.d)
LDSCRIPT := STM32F051C6TX_FLASH.ld

MCU_FLAGS := -mcpu=cortex-m0 -mthumb -mfloat-abi=soft
CPPFLAGS := -DSTM32F051x8 -DUSE_HAL_DRIVER \
            -ICore/Inc \
            -I$(CORE_DIR) \
            -I$(STM32_DIR) \
            -IDrivers/STM32F0xx_HAL_Driver/Inc \
            -IDrivers/STM32F0xx_HAL_Driver/Inc/Legacy \
            -IDrivers/CMSIS/Device/ST/STM32F0xx/Include \
            -IDrivers/CMSIS/Include
CFLAGS := $(MCU_FLAGS) $(OPT) $(DEBUG_FLAGS) -std=gnu11 -Wall -Wextra \
          -fdata-sections -ffunction-sections -fno-common -MMD -MP
ASFLAGS := $(MCU_FLAGS) $(DEBUG_FLAGS) -x assembler-with-cpp -MMD -MP
LDFLAGS := $(MCU_FLAGS) --specs=nano.specs --specs=nosys.specs \
           -T$(LDSCRIPT) -Wl,--gc-sections -Wl,-Map=$(BUILD_DIR)/$(TARGET).map

.PHONY: all clean size
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).bin size

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.s | $(BUILD_DIR)
	$(CC) $(ASFLAGS) -c $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) $(LDSCRIPT)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf
	$(OBJCOPY) -O binary $< $@

size: $(BUILD_DIR)/$(TARGET).elf
	$(SIZE) $<

clean:
	rm -rf build

-include $(DEPS)
