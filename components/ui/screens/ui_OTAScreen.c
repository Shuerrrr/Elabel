// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.5.0
// LVGL version: 8.3.11
// Project name: SquareLine_Project

#include "../ui.h"

void ui_OTAScreen_screen_init(void)
{
    ui_OTAScreen = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_OTAScreen, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

    ui_textlabel = lv_label_create(ui_OTAScreen);
    lv_obj_set_width(ui_textlabel, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_textlabel, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_textlabel, 1);
    lv_obj_set_y(ui_textlabel, -38);
    lv_obj_set_align(ui_textlabel, LV_ALIGN_CENTER);
    lv_label_set_text(ui_textlabel, "New Firmware\n      Available");
    lv_obj_set_style_text_color(ui_textlabel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_textlabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_textlabel, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_textlabel1 = lv_label_create(ui_OTAScreen);
    lv_obj_set_width(ui_textlabel1, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_textlabel1, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_textlabel1, 2);
    lv_obj_set_y(ui_textlabel1, 0);
    lv_obj_set_align(ui_textlabel1, LV_ALIGN_CENTER);
    lv_label_set_text(ui_textlabel1, "Click - Apply");
    lv_obj_set_style_text_color(ui_textlabel1, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_textlabel1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_textlabel1, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_textlabel2 = lv_label_create(ui_OTAScreen);
    lv_obj_set_width(ui_textlabel2, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_textlabel2, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_textlabel2, 3);
    lv_obj_set_y(ui_textlabel2, 30);
    lv_obj_set_align(ui_textlabel2, LV_ALIGN_CENTER);
    lv_label_set_text(ui_textlabel2, "Rotate - Skip");
    lv_obj_set_style_text_color(ui_textlabel2, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_textlabel2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_textlabel2, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);

    uic_textlabel = ui_textlabel;
    uic_textlabel1 = ui_textlabel1;
    uic_textlabel2 = ui_textlabel2;

}
