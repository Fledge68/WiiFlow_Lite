﻿.PHONY = all clean run
CURDIR_TMP := $(CURDIR)

all:
	@echo Make App Booter
	@$(MAKE) --no-print-directory -C $(CURDIR_TMP)/resources/app_booter \
			-f $(CURDIR_TMP)/resources/app_booter/Makefile
	@mv -u $(CURDIR_TMP)/resources/app_booter/app_booter.bin \
		$(CURDIR_TMP)/out/bins/app_booter.bin

	@echo Make WiiFlow Loader
	@$(MAKE) --no-print-directory -C $(CURDIR_TMP)/resources/extldr \
			-f $(CURDIR_TMP)/resources/extldr/Makefile

	@echo Make WiiFlow Booter
	@$(MAKE) --no-print-directory -C $(CURDIR_TMP)/resources/wiiflow_game_booter \
		-f $(CURDIR_TMP)/resources/wiiflow_game_booter/Makefile
	@mv -u $(CURDIR_TMP)/resources/wiiflow_game_booter/booter.bin \
		$(CURDIR_TMP)/out/bins/ext_booter.bin

	@echo Make WiiFlow Main
	@$(MAKE) --no-print-directory -C $(CURDIR_TMP) -f $(CURDIR_TMP)/Makefile.main

clean:
	@echo Cleanup App Booter
	@$(MAKE) --no-print-directory -C $(CURDIR_TMP)/resources/app_booter \
			-f $(CURDIR_TMP)/resources/app_booter/Makefile clean

	@echo Cleanup WiiFlow Loader
	@$(MAKE) --no-print-directory -C $(CURDIR_TMP)/resources/extldr \
			-f $(CURDIR_TMP)/resources/extldr/Makefile clean

	@echo Cleanup WiiFlow Booter
	@$(MAKE) --no-print-directory -C $(CURDIR_TMP)/resources/wiiflow_game_booter \
			-f $(CURDIR_TMP)/resources/wiiflow_game_booter/Makefile clean

	@echo Cleanup WiiFlow Main
	@$(MAKE) --no-print-directory -C $(CURDIR_TMP) -f $(CURDIR_TMP)/Makefile.main clean
run:
	@$(MAKE) --no-print-directory -C $(CURDIR_TMP) -f $(CURDIR_TMP)/Makefile.main run
