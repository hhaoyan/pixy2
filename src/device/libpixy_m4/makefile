RM := rm -rf

SRC_C := \
	src/fpu_init.c \
	src/spifilib_fam_standard_cmd.c \
	src/usbhw.c \
	src/platform_config.c \
	src/usbcore.c \
	src/usbuser.c \
	src/spifilib_dev_common.c \
	src/usbdesc.c

SRC_CPP :=  \
	src/camera.cpp \
	src/led.cpp \
   	src/pixy_init.cpp \
   	src/sccb.cpp \
	src/exec.cpp \
  	src/misc.cpp \
  	src/power.cpp \
 	src/smlink.cpp \
	src/flash.cpp \
 	src/param.cpp \
 	src/rcservo.cpp \
 	src/usblink.cpp

OBJS := \
	$(subst ..,.,$(SRC_C:%.c=build/%.o)) \
	$(subst ..,.,$(SRC_CPP:%.cpp=build/%.o))

LIBS :=

CFLAGS := \
	-D__NEWLIB__ -DIPC_MASTER -DPIXY -DNDEBUG -DCORE_M4 -D__LPC43XX__ \
	-Iinc -I../common/inc -I../../common/inc \
	-Os -g -Wall \
	-fmessage-length=0 -fno-builtin -ffunction-sections \
	-fdata-sections -fsingle-precision-constant \
	-fno-exceptions -fno-unwind-tables \
	-mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -mthumb

CPPFLAGS := -fno-rtti -fno-exceptions 

# All Target
all: src build/liblibpixy_m4.a

src:
	@echo "Creating ./build/"
	@mkdir ./build

build/liblibpixy_m4.a: $(OBJS) makefile
	@echo 'MCU Archiver: building target: $@'
	@arm-none-eabi-ar -r "$@" $(OBJS) $(LIBS)
	@echo 'Performing post-build steps'
	@arm-none-eabi-size "$@"

build/%.o: %.cpp makefile
	@echo 'MCU C++ Compiler: building file: $<'
	@mkdir -p $(dir $@)
	@arm-none-eabi-c++ $(CFLAGS) $(CPPFLAGS) \
		-MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" \
		-o "$@" -c "$<"	

build/%.o: %.c makefile
	@echo 'MCU C Compiler: building file: $<'
	@mkdir -p $(dir $@)
	@arm-none-eabi-gcc $(CFLAGS) \
		-MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" \
		-o "$@" -c "$<"

# Other Targets
clean:
	@echo "Cleaning up"
	@$(RM) $(OBJS) ./build/liblibpixy_m4.a
