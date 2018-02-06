

RM := rm -rf

# Every subdirectory with source files must be described here
SUBDIRS := \
./ \

LIBS := -lIlmImf-2_1 -lIex-2_1 -lboost_program_options

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
./exrMatrix.cpp \
./imageData.cpp 

OBJS += \
./exrMatrix.o \
./imageData.o 

CPP_DEPS += \
./exrMatrix.d \
./imageData.d 


# Each subdirectory must supply rules for building sources it contributes
./%.o: ./%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/usr/local/include/OpenEXR -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(VERT_DEPS)),)
-include $(VERT_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(FRAG_DEPS)),)
-include $(FRAG_DEPS)
endif
endif


# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: exrMatrix

# Tool invocations
exrMatrix: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C++ Linker'
	g++ -L/usr/lib/ -L/usr/local/lib -L/opt/lib/openexr/lib -o"exrMatrix" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C++_DEPS)$(C_DEPS)$(CC_DEPS)$(VERT_DEPS)$(CPP_DEPS)$(EXECUTABLES)$(CXX_DEPS)$(C_UPPER_DEPS)$(FRAG_DEPS) exrMatrix
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

