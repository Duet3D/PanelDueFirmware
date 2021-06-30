cmake_minimum_required( VERSION 3.10 )

#configure_file(src/config.h.in src/config.h)

# skip compiler build tests
set ( CMAKE_C_COMPILER_FORCED True )
set ( CMAKE_CXX_COMPILER_FORCED True )

set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__SAM4S4B__ -DBOARD=USER_BOARD -DARM_MATH_CM4=true -DSCREEN_70E" )

set( CMAKE_TOOLCHAIN_FILE toolchains/arm-none-eabi.cmake )


project( paneldue VERSION 3.3 )
set( CMAKE_PROJECT_DESCRIPTION "This is PanelDue Firmware" )
set( CMAKE_PROJECT_HOMEPAGE_URL "https://github.com/Duet3D/PanelDueFirmware" )

add_subdirectory( lib/librrf )
add_executable( paneldue.elf
	src/ASF/common/boards/user_board/init.c
	src/ASF/common/services/clock/sam4s/sysclk.c
	src/ASF/common/services/delay/sam/cycle_counter.c
	src/ASF/common/utils/interrupt/interrupt_sam_nvic.c
	src/ASF/sam/drivers/chipid/chipid.c
	src/ASF/sam/drivers/efc/efc.c
	src/ASF/sam/drivers/matrix/matrix.c
	src/ASF/sam/drivers/pio/pio.c
	src/ASF/sam/drivers/pio/pio_handler.c
	src/ASF/sam/drivers/pmc/pmc.c
	src/ASF/sam/drivers/pwm/pwm.c
	src/ASF/sam/drivers/rstc/rstc.c
	src/ASF/sam/drivers/uart/uart.c
	src/ASF/sam/drivers/wdt/wdt.c
	src/ASF/sam/services/flash_efc/flash_efc.c
	src/ASF/sam/utils/cmsis/sam4s/source/templates/gcc/startup_sam4s.c
	src/ASF/sam/utils/cmsis/sam4s/source/templates/system_sam4s.c
	src/ASF/sam/utils/syscalls/gcc/syscalls.c
	src/FileManager.cpp
	src/FlashData.cpp
	src/Fonts/glcd19x21.cpp
	src/Fonts/glcd28x32.cpp
	src/Hardware/Backlight.cpp
	src/Hardware/Buzzer.cpp
	src/Hardware/FlashStorage.cpp
	src/Hardware/Mem.cpp
	src/Hardware/OneBitPort.cpp
	src/Hardware/Reset.cpp
	src/Hardware/RotaryEncoder.cpp
	src/Hardware/SerialIo.cpp
	src/Hardware/SysTick.cpp
	src/Hardware/UTFT.cpp
	src/Hardware/UTouch.cpp
	src/Icons/HomeIcons.cpp
	src/Icons/KeyIcons.cpp
	src/Icons/MiscIcons.cpp
	src/Icons/NozzleIcons.cpp
	src/Library/Misc.cpp
	src/ObjectModel/Axis.cpp
	src/ObjectModel/BedOrChamber.cpp
	src/ObjectModel/Spindle.cpp
	src/ObjectModel/Tool.cpp
	src/ObjectModel/Utils.cpp
	src/PanelDue.cpp
	src/RequestTimer.cpp
	src/UI/ColourSchemes.cpp
	src/UI/Display.cpp
	src/UI/MessageLog.cpp
	src/UI/UserInterface.cpp
)


target_include_directories( paneldue.elf PUBLIC
	src
	src/ASF/common/boards
	src/ASF/common/boards/user_board
	src/ASF/common/services/clock
	src/ASF/common/services/delay
	src/ASF/common/utils
	src/ASF/sam/drivers
	src/ASF/sam/drivers/chipid
	src/ASF/sam/drivers/efc
	src/ASF/sam/drivers/efc
	src/ASF/sam/drivers/matrix
	src/ASF/sam/drivers/pio
	src/ASF/sam/drivers/pmc
	src/ASF/sam/drivers/pwm
	src/ASF/sam/drivers/rstc
	src/ASF/sam/drivers/uart
	src/ASF/sam/drivers/wdt
	src/ASF/sam/services/flash_efc
	src/ASF/sam/utils
	src/ASF/sam/utils/cmsis/sam3s/source/templates
	src/ASF/sam/utils/cmsis/sam4s/include
	src/ASF/sam/utils/header_files
	src/ASF/sam/utils/preprocessor
	src/ASF/thirdparty/CMSIS/Include
	src/UI
	src/config
)

target_link_libraries( paneldue.elf rrf )

add_custom_target( paneldue.bin ALL
	DEPENDS paneldue.elf
	COMMAND arm-none-eabi-objcopy -O binary paneldue.elf paneldue.bin
)
add_custom_target( paneldue-logo.bin ALL
	DEPENDS paneldue.bin ${PROJECT_SOURCE_DIR}/SplashScreens/SplashScreen-Duet3D-800x480.bin
	COMMAND cat paneldue.bin ${PROJECT_SOURCE_DIR}/SplashScreens/SplashScreen-Duet3D-800x480.bin > paneldue-logo.bin
)

