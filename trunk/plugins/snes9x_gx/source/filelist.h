/****************************************************************************
 * Snes9x Nintendo Wii/Gamecube Port
 *
 * Tantric 2009-2010
 *
 * filelist.h
 *
 * Contains a list of all of the files stored in the images/, fonts/, and
 * sounds/ folders
 ***************************************************************************/

#ifndef _FILELIST_H_
#define _FILELIST_H_

#include <gccore.h>

// Fonts
extern const u8		font_ttf[];
extern const u32	font_ttf_size;

// Languages
extern const u8		jp_lang[];
extern const u32	jp_lang_size;
extern const u8		en_lang[];
extern const u32	en_lang_size;
extern const u8		de_lang[];
extern const u32	de_lang_size;
extern const u8		fr_lang[];
extern const u32	fr_lang_size;
extern const u8		es_lang[];
extern const u32	es_lang_size;
extern const u8		it_lang[];
extern const u32	it_lang_size;
extern const u8		nl_lang[];
extern const u32	nl_lang_size;
extern const u8		zh_lang[];
extern const u32	zh_lang_size;
extern const u8		ko_lang[];
extern const u32	ko_lang_size;
extern const u8		pt_lang[];
extern const u32	pt_lang_size;
extern const u8		pt_br_lang[];
extern const u32	pt_br_lang_size;
extern const u8		ca_lang[];
extern const u32	ca_lang_size;
extern const u8		tr_lang[];
extern const u32	tr_lang_size;

// Sounds

extern const u8		bg_music_ogg[];
extern const u32	bg_music_ogg_size;

extern const u8		enter_ogg[];
extern const u32	enter_ogg_size;

extern const u8		exit_ogg[];
extern const u32	exit_ogg_size;

extern const u8		button_over_pcm[];
extern const u32	button_over_pcm_size;

extern const u8		button_click_pcm[];
extern const u32	button_click_pcm_size;

// Graphics

extern const u8		logo_png[];
extern const u32	logo_png_size;

extern const u8		logo_over_png[];
extern const u32	logo_over_png_size;

extern const u8		bg_top_png[];
extern const u32	bg_top_png_size;

extern const u8		bg_bottom_png[];
extern const u32	bg_bottom_png_size;

extern const u8		icon_settings_png[];
extern const u32	icon_settings_png_size;

extern const u8		icon_home_png[];
extern const u32	icon_home_png_size;

extern const u8		icon_game_settings_png[];
extern const u32	icon_game_settings_png_size;
extern const u8		icon_game_cheats_png[];
extern const u32	icon_game_cheats_png_size;
extern const u8		icon_game_controllers_png[];
extern const u32	icon_game_controllers_png_size;
extern const u8		icon_game_load_png[];
extern const u32	icon_game_load_png_size;
extern const u8		icon_game_save_png[];
extern const u32	icon_game_save_png_size;
extern const u8		icon_game_reset_png[];
extern const u32	icon_game_reset_png_size;

extern const u8		icon_settings_wiimote_png[];
extern const u32	icon_settings_wiimote_png_size;
extern const u8		icon_settings_classic_png[];
extern const u32	icon_settings_classic_png_size;
extern const u8		icon_settings_gamecube_png[];
extern const u32	icon_settings_gamecube_png_size;
extern const u8		icon_settings_nunchuk_png[];
extern const u32	icon_settings_nunchuk_png_size;
extern const u8		icon_settings_wiiupro_png[];
extern const u32	icon_settings_wiiupro_png_size;

extern const u8		icon_settings_snescontroller_png[];
extern const u32	icon_settings_snescontroller_png_size;
extern const u8		icon_settings_superscope_png[];
extern const u32	icon_settings_superscope_png_size;
extern const u8		icon_settings_justifier_png[];
extern const u32	icon_settings_justifier_png_size;
extern const u8		icon_settings_mouse_png[];
extern const u32	icon_settings_mouse_png_size;

extern const u8		icon_settings_file_png[];
extern const u32	icon_settings_file_png_size;
extern const u8		icon_settings_mappings_png[];
extern const u32	icon_settings_mappings_png_size;
extern const u8		icon_settings_menu_png[];
extern const u32	icon_settings_menu_png_size;
extern const u8		icon_settings_network_png[];
extern const u32	icon_settings_network_png_size;
extern const u8		icon_settings_video_png[];
extern const u32	icon_settings_video_png_size;
extern const u8		icon_settings_screenshot_png[];
extern const u32	icon_settings_screenshot_png_size;

extern const u8		button_png[];
extern const u32	button_png_size;

extern const u8		button_over_png[];
extern const u32	button_over_png_size;

extern const u8		button_prompt_png[];
extern const u32	button_prompt_png_size;

extern const u8		button_prompt_over_png[];
extern const u32	button_prompt_over_png_size;

extern const u8		button_long_png[];
extern const u32	button_long_png_size;

extern const u8		button_long_over_png[];
extern const u32	button_long_over_png_size;

extern const u8		button_short_png[];
extern const u32	button_short_png_size;

extern const u8		button_short_over_png[];
extern const u32	button_short_over_png_size;

extern const u8		button_small_png[];
extern const u32	button_small_png_size;

extern const u8		button_small_over_png[];
extern const u32	button_small_over_png_size;

extern const u8		button_large_png[];
extern const u32	button_large_png_size;

extern const u8		button_large_over_png[];
extern const u32	button_large_over_png_size;

extern const u8		button_arrow_left_png[];
extern const u32	button_arrow_left_png_size;

extern const u8		button_arrow_right_png[];
extern const u32	button_arrow_right_png_size;

extern const u8		button_arrow_up_png[];
extern const u32	button_arrow_up_png_size;

extern const u8		button_arrow_down_png[];
extern const u32	button_arrow_down_png_size;

extern const u8		button_arrow_left_over_png[];
extern const u32	button_arrow_left_over_png_size;

extern const u8		button_arrow_right_over_png[];
extern const u32	button_arrow_right_over_png_size;

extern const u8		button_arrow_up_over_png[];
extern const u32	button_arrow_up_over_png_size;

extern const u8		button_arrow_down_over_png[];
extern const u32	button_arrow_down_over_png_size;

extern const u8		button_gamesave_png[];
extern const u32	button_gamesave_png_size;

extern const u8		button_gamesave_over_png[];
extern const u32	button_gamesave_over_png_size;

extern const u8		button_gamesave_blank_png[];
extern const u32	button_gamesave_blank_png_size;

extern const u8		screen_position_png[];
extern const u32	screen_position_png_size;

extern const u8		dialogue_box_png[];
extern const u32	dialogue_box_png_size;

extern const u8		credits_box_png[];
extern const u32	credits_box_png_size;

extern const u8		progressbar_png[];
extern const u32	progressbar_png_size;

extern const u8		progressbar_empty_png[];
extern const u32	progressbar_empty_png_size;

extern const u8		progressbar_outline_png[];
extern const u32	progressbar_outline_png_size;

extern const u8		throbber_png[];
extern const u32	throbber_png_size;

extern const u8		icon_folder_png[];
extern const u32	icon_folder_png_size;

extern const u8		icon_sd_png[];
extern const u32	icon_sd_png_size;

extern const u8		icon_usb_png[];
extern const u32	icon_usb_png_size;

extern const u8		icon_dvd_png[];
extern const u32	icon_dvd_png_size;

extern const u8		icon_smb_png[];
extern const u32	icon_smb_png_size;

extern const u8		battery_png[];
extern const u32	battery_png_size;

extern const u8		battery_red_png[];
extern const u32	battery_red_png_size;

extern const u8		battery_bar_png[];
extern const u32	battery_bar_png_size;

extern const u8		bg_options_png[];
extern const u32	bg_options_png_size;

extern const u8		bg_options_entry_png[];
extern const u32	bg_options_entry_png_size;

extern const u8		bg_game_selection_png[];
extern const u32	bg_game_selection_png_size;

extern const u8		bg_game_selection_entry_png[];
extern const u32	bg_game_selection_entry_png_size;

extern const u8		bg_preview_png[];
extern const u32	bg_preview_png_size;

extern const u8		scrollbar_png[];
extern const u32	scrollbar_png_size;

extern const u8		scrollbar_arrowup_png[];
extern const u32	scrollbar_arrowup_png_size;

extern const u8		scrollbar_arrowup_over_png[];
extern const u32	scrollbar_arrowup_over_png_size;

extern const u8		scrollbar_arrowdown_png[];
extern const u32	scrollbar_arrowdown_png_size;

extern const u8		scrollbar_arrowdown_over_png[];
extern const u32	scrollbar_arrowdown_over_png_size;

extern const u8		scrollbar_box_png[];
extern const u32	scrollbar_box_png_size;

extern const u8		scrollbar_box_over_png[];
extern const u32	scrollbar_box_over_png_size;

extern const u8		keyboard_textbox_png[];
extern const u32	keyboard_textbox_png_size;

extern const u8		keyboard_key_png[];
extern const u32	keyboard_key_png_size;

extern const u8		keyboard_key_over_png[];
extern const u32	keyboard_key_over_png_size;

extern const u8		keyboard_mediumkey_png[];
extern const u32	keyboard_mediumkey_png_size;

extern const u8		keyboard_mediumkey_over_png[];
extern const u32	keyboard_mediumkey_over_png_size;

extern const u8		keyboard_largekey_png[];
extern const u32	keyboard_largekey_png_size;

extern const u8		keyboard_largekey_over_png[];
extern const u32	keyboard_largekey_over_png_size;

extern const u8		player1_point_png[];
extern const u32	player1_point_png_size;

extern const u8		player2_point_png[];
extern const u32	player2_point_png_size;

extern const u8		player3_point_png[];
extern const u32	player3_point_png_size;

extern const u8		player4_point_png[];
extern const u32	player4_point_png_size;

extern const u8		player1_grab_png[];
extern const u32	player1_grab_png_size;

extern const u8		player2_grab_png[];
extern const u32	player2_grab_png_size;

extern const u8		player3_grab_png[];
extern const u32	player3_grab_png_size;

extern const u8		player4_grab_png[];
extern const u32	player4_grab_png_size;

#endif
