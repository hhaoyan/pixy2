RM := rm -rf

SRC_C := \
	src/cr_startup_lpc43xx-m0app.c \
	src/equeue.c \
	src/main_m0.c

COMMON_SRC_C := \
	../common/src/lpc43xx_uart.c \
	../common/src/debug_frmwrk.c

OBJS := \
	$(SRC_C:%.c=build/%.o) \
	$(subst ../,build/,$(COMMON_SRC_C:%.c=%.o))

LIBS := -L../libpixy_m0/build -llibpixy_m0 \
	-lc_nano\
    -lnosys\
    -lm\
    -lstdc++_nano

CFLAGS := \
	-DIPC_SLAVE -DPIXY -D__NEWLIB__ -D__MULTICORE_M0APP -DNDEBUG \
	-D__CODE_RED -DCORE_M0 -D__LPC43XX__ -DCORE_M0APP \
	-I../libpixy_m0/inc -I../common/inc -I../../common/inc \
	-Os -g -Wall \
	-fmessage-length=0 \
	-fno-builtin -ffunction-sections -fdata-sections \
	-mcpu=cortex-m0 -mfloat-abi=soft -mthumb \

LDFLAGS := 

# All Target
all: UPDATE_LIBM0 build build/main_m0.axf

build:
	@echo "Creating ./build/"
	@mkdir ./build

build/main_m0.axf: $(OBJS) ../libpixy_m0/build/liblibpixy_m0.a makefile
	@echo 'MCU Linker: building target: $@'
	@arm-none-eabi-gcc $(LDFLAGS) \
		-Xlinker -Map="build/main_m0.map" -Xlinker --gc-sections \
		-mcpu=cortex-m0 -mthumb -T "linkscripts/main_m0_Release.ld" \
		-o "build/main_m0.axf" $(OBJS) $(LIBS)
	@arm-none-eabi-objcopy --target elf32-littlearm --verbose \
		--strip-all --redefine-sym __vectors_start__=__vectors_start___core_m0app \
		--keep-symbol __vectors_start___core_m0app \
		--rename-section .text=".core_m0app" \
		--rename-section .data=".core_m0app.data" \
		--rename-section .bss=".core_m0app.bss" \
		"build/main_m0.axf" "build/main_m0.axf.o"
	@echo 'Performing post-build steps'
	@arm-none-eabi-size "build/main_m0.axf"

UPDATE_LIBM0:
	@echo 'Updating: libpixy_m0 codes'
	@make -C ../libpixy_m0

build/%.o: %.c makefile
	@echo 'MCU C Compiler: building file: $<'
	@mkdir -p $(dir $@)
	@arm-none-eabi-gcc $(CFLAGS) \
		-MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" \
		-o "$@" -c "$<"

build/common/%.o: ../common/%.c makefile
	@echo 'MCU C Compiler: building file: $<'
	@mkdir -p $(dir $@)
	@arm-none-eabi-gcc $(CFLAGS) \
		-MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" \
		-o "$@" -c "$<"

# Other Targets
clean:
	@echo "Cleaning up"
	@$(RM) $(OBJS) ./build/main_m0.axf
