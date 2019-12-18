RM := rm -rf

SRC_CPP := src/analogdig.cpp \
	src/button.cpp \
	src/conncomp.cpp \
	src/exec.cpp \
	src/i2c.cpp \
	src/progblobs.cpp \
	src/progvideo.cpp \
	src/serial.cpp \
	src/spi.cpp \
	src/uart.cpp \
	src/main_m4.cpp \
	src/progpt.cpp \
	src/progline.cpp \
	src/line.cpp \
	src/equeue.cpp \
	src/spi2.cpp \
	src/cr_startup_lpc43xx.cpp 

SRC_C := src/cr_start_m0.c

COMMON_SRC_CPP := \
	../../common/src/chirp.cpp \
	../../common/src/calc.cpp \
	../../common/src/blob.cpp \
	../../common/src/blobs.cpp \
	../../common/src/colorlut.cpp \
	../../common/src/qqueue.cpp

COMMON_SRC_C := \
	../common/src/lpc43xx_scu.c \
	../common/src/lpc43xx_cgu.c \
	../common/src/lpc43xx_timer.c \
	../common/src/system_LPC43xx.c \
	../common/src/lpc43xx_adc.c \
	../common/src/lpc43xx_ssp.c \
	../common/src/lpc43xx_i2c.c \
	../common/src/debug_frmwrk.c \
	../common/src/lpc43xx_uart.c

OBJS := $(SRC_C:%.c=build/%.o) \
	$(SRC_CPP:%.cpp=build/%.o) \
	$(subst ../../common,build/common0,$(COMMON_SRC_CPP:%.cpp=%.o)) \
	$(subst ../common,build/common,$(COMMON_SRC_C:%.c=%.o))

LIBS := -L../libpixy_m4/build -llibpixy_m4 \
	../main_m0/build/main_m0.axf.o \
	-lc_nano\
    -lnosys\
    -lm\
    -lstdc++_nano\
    -specs=/usr/local/Cellar/arm-gcc-bin/8-2019-q3-update/arm-none-eabi/lib/thumb/v7e-m+fp/softfp/nosys.specs

CFLAGS := \
	-D__NEWLIB__ -DIPC_MASTER -DPIXY -DNDEBUG -DCORE_M4 -DCPP_USE_HEAP -D__LPC43XX__ \
	-Iinc -I../libpixy_m4/inc -I../common/inc -I../../common/inc \
	-Os -g -Wall \
	-fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections \
	-fsingle-precision-constant -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 \
	-mfloat-abi=softfp -mthumb

CPPFLAGS := \
	-D__MULTICORE_MASTER -DIPC_MASTER -DPIXY -D__NEWLIB__ -DNDEBUG -D__CODE_RED \
	-DCORE_M4 -DCPP_USE_HEAP -D__LPC43XX__ -D__NEWLIB__ \
	-Iinc -I../libpixy_m4/inc -I../common/inc -I../../common/inc \
	-O1 -Os -g -Wall \
	-fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections \
	-fno-rtti -fno-exceptions -fsingle-precision-constant -fno-unwind-tables \
	-mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -mthumb

LDFLAGS := 

# All Target
all: UPDATE_LIBM4 UPDATE_MAINM0 build build/main_m4_spi_flash.hex

UPDATE_LIBM4:
	@echo 'Updating: libpixy_m4 codes'
	@make -C ../libpixy_m4

UPDATE_MAINM0:
	@echo 'Updating: main_m0 codes'
	@make -C ../main_m0

build:
	@echo "Creating ./build/"
	@mkdir ./build

build/main_m4_spi_flash.hex: build/main_m4.hex
	@echo "Prepending version info"
	@printf 'general,3,0,20,0,2,0,2,0,255'| cat - "build/main_m4.hex" > "build/main_m4_spi_flash.hex"

build/main_m4.hex: build/main_m4.bin
	@echo "Dumping hex object $<"
	@arm-none-eabi-objcopy -v -O ihex "build/main_m4.axf" "build/main_m4.hex"

build/main_m4.bin: build/main_m4.axf
	@arm-none-eabi-size "$<"
	@echo "Dumping object $<"
	@arm-none-eabi-objcopy -v -O binary "$<" "build/main_m4.bin"

build/main_m4.axf: $(OBJS) ../libpixy_m4/build/liblibpixy_m4.a ../main_m0/build/main_m0.axf.o makefile linkscripts/RAM_link_template.ld
	@echo 'MCU Linker: building target: $@'
	@arm-none-eabi-gcc $(LDFLAGS) \
		-Xlinker -Map="build/main_m4.map" -Xlinker --gc-sections \
		-mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp \
		-mthumb -T "RAM_link_template.ld" -L linkscripts \
		-o "build/main_m4.axf" $(OBJS) $(LIBS)
	@echo "Stripping: $@"
	#@arm-none-eabi-strip "build/main_m4.axf"
	@echo 'Performing post-build steps'
	@arm-none-eabi-size "build/main_m4.axf"

build/%.o: %.c makefile
	@echo 'MCU C Compiler: building file: $<'
	@mkdir -p $(dir $@)
	@arm-none-eabi-gcc $(CFLAGS) \
		-MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" \
		-o "$@" -c "$<"

build/%.o: %.cpp makefile
	@echo 'MCU CPP Compiler: building file: $<'
	@mkdir -p $(dir $@)
	@arm-none-eabi-c++ $(CPPFLAGS) \
		-MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" \
		-o "$@" -c "$<"

build/common/%.o: ../common/%.c makefile
	@echo 'MCU C Compiler: building common file: $<'
	@mkdir -p $(dir $@)
	@arm-none-eabi-gcc $(CFLAGS) \
		-MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" \
		-o "$@" -c "$<"

build/common0/%.o: ../../common/%.cpp makefile
	@echo 'MCU CPP Compiler: building common0 cpp file: $<'
	@mkdir -p $(dir $@)
	@arm-none-eabi-c++ $(CPPFLAGS) \
		-MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" \
		-o "$@" -c "$<"

# Other Targets
clean:
	@echo "Cleaning up"
	@$(RM) $(OBJS) ./build/main_m4.axf ./build/main_m4.bin ./build/main_m4.hex
