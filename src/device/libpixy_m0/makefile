RM := rm -rf

SRC_C := \
	src/chirp.c \
	src/debug.c \
	src/exec_m0.c \
	src/frame_m0.c \
	src/qqueue.c \
	src/rls_m0.c \
	src/smlink.c

SRC_CPP :=  

OBJS := \
	$(subst ..,.,$(SRC_C:%.c=build/%.o)) \
	$(subst ..,.,$(SRC_CPP:%.cpp=build/%.o))

LIBS := 

CFLAGS := \
	-DIPC_SLAVE -DPIXY -D__NEWLIB__ -D__MULTICORE_NONE -DNDEBUG \
	-D__CODE_RED -DCORE_M0 -DCR_PRINTF_CHAR -D__LPC43XX__ \
	-Iinc -I../common/inc -I../../common/inc \
	-Os -g -Wall -fmessage-length=0 \
	-fno-builtin -ffunction-sections -fdata-sections \
	-mcpu=cortex-m0 -mfloat-abi=soft -mthumb

# All Target
all: src build/liblibpixy_m0.a

src:
	@echo "Creating ./build/"
	@mkdir ./build

build/liblibpixy_m0.a: $(OBJS) makefile
	@echo 'MCU Archiver: building target: $@'
	@arm-none-eabi-ar -r $@ $(OBJS) $(LIBS)
	@echo 'Performing post-build steps'
	@arm-none-eabi-size $@

build/%.o: %.c makefile
	@echo 'MCU C Compiler: building file: $<'
	@mkdir -p $(dir $@)
	@arm-none-eabi-gcc $(CFLAGS) \
		-MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" \
		-o "$@" -c "$<"

# Other Targets
clean:
	@echo "Cleaning up"
	@$(RM) $(OBJS) ./build/liblibpixy_m0.a
