#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "4coder_default_include.cpp"

#include "4coder_fleury_ubiquitous.h"
#include "4coder_fleury_lang.h"
#include "4coder_fleury_index.h"
#include "4coder_fleury_colors.h"
#include "4coder_fleury_render_helpers.h"
#include "4coder_fleury_brace.h"
#include "4coder_fleury_error_annotations.h"
#include "4coder_fleury_divider_comments.h"
#include "4coder_fleury_power_mode.h"
#include "4coder_fleury_cursor.h"
#include "4coder_fleury_plot.h"
#include "4coder_fleury_lego.h"
#include "4coder_fleury_pos_context_tooltips.h"
#include "4coder_fleury_code_peek.h"
#include "4coder_fleury_recent_files.h"
#include "4coder_fleury_base_commands.h"
#include "4coder_fleury_hooks.h"

static Arena global_frame_arena;
global Arena permanent_arena = {};

#include "4coder_fleury_ubiquitous.cpp"
#include "4coder_fleury_lang.cpp"
#include "4coder_fleury_index.cpp"
#include "4coder_fleury_colors.cpp"
#include "4coder_fleury_render_helpers.cpp"
#include "4coder_fleury_brace.cpp"
#include "4coder_fleury_error_annotations.cpp"
#include "4coder_fleury_divider_comments.cpp"
#include "4coder_fleury_power_mode.cpp"
#include "4coder_fleury_cursor.cpp"
#include "4coder_fleury_plot.cpp"
#include "4coder_fleury_lego.cpp"
#include "4coder_fleury_pos_context_tooltips.cpp"
#include "4coder_fleury_code_peek.cpp"
#include "4coder_fleury_recent_files.cpp"
#include "4coder_fleury_base_commands.cpp"
#include "4coder_fleury_hooks.cpp"

#include "4coder_clearfield_rect.cpp"

#include "4coder_nix_reload_changed_buffers.cpp"

#include "4coder_lida_bindings.cpp"

//~ NOTE(rjf): 4coder Stuff
#include "generated/managed_id_metadata.cpp"

void custom_layer_init(Application_Links *app)
{
    default_framework_init(app);
    global_frame_arena = make_arena(get_base_allocator_system());
    permanent_arena = make_arena(get_base_allocator_system());

    // NOTE(rjf): Set up hooks.
    {
        set_all_default_hooks(app);
        set_custom_hook(app, HookID_Tick,                    F4_Tick);
        set_custom_hook(app, HookID_RenderCaller,            F4_Render);
        set_custom_hook(app, HookID_BeginBuffer,             F4_BeginBuffer);
        set_custom_hook(app, HookID_Layout,                  F4_Layout);
        set_custom_hook(app, HookID_WholeScreenRenderCaller, F4_WholeScreenRender);
        set_custom_hook(app, HookID_DeltaRule,               F4_DeltaRule);
        set_custom_hook(app, HookID_BufferEditRange,         F4_BufferEditRange);
        set_custom_hook_memory_size(app, HookID_DeltaRule, delta_ctx_size(sizeof(Vec2_f32)));
    }

    // NOTE(rjf): Set up mapping.
    {
        Thread_Context *tctx = get_thread_context(app);
        mapping_init(tctx, &framework_mapping);
        lida_SetAbsolutelyNecessaryBindings(&framework_mapping);
        lida_SetDefaultBindings(&framework_mapping);
    }

    // NOTE(rjf): Set up custom code index.
    {
        F4_Index_Initialize();
    }

    // NOTE(rjf): Register languages.
    {
        F4_RegisterLanguages();
    }
}

// TODO(rjf): This is only being used to check if a font file exists because
// there's a bug in try_create_new_face that crashes the program if a font is
// not found. This function is only necessary until that is fixed.
function b32
IsFileReadable(String_Const_u8 path)
{
    b32 result = 0;
    FILE *file = fopen((char *)path.str, "r");
    if(file)
    {
        result = 1;
        fclose(file);
    }
    return result;
}

CUSTOM_COMMAND_SIG(lida_startup)
CUSTOM_DOC("the main startup function")
{
    ProfileScope(app, "default startup");

    User_Input input = get_current_input(app);
    if(!match_core_code(&input, CoreCode_Startup))
    {
        return;
    }

    //~ NOTE(rjf): Default 4coder initialization.
    String_Const_u8_Array file_names = input.event.core.file_names;
    load_themes_default_folder(app);
    default_4coder_initialize(app, file_names);

    //~ NOTE(rjf): Open special buffers.
    {
        // NOTE(rjf): Open compilation buffer.
        {
            Buffer_ID buffer = create_buffer(app, string_u8_litexpr("*compilation*"),
                                             BufferCreate_NeverAttachToFile |
                                             BufferCreate_AlwaysNew);
            buffer_set_setting(app, buffer, BufferSetting_Unimportant, true);
            buffer_set_setting(app, buffer, BufferSetting_ReadOnly, true);
        }

        // NOTE(rjf): Open peek buffer.
        {
            Buffer_ID buffer = create_buffer(app, string_u8_litexpr("*peek*"),
                                             BufferCreate_NeverAttachToFile |
                                             BufferCreate_AlwaysNew);
            buffer_set_setting(app, buffer, BufferSetting_Unimportant, true);
        }

        // NOTE(rjf): Open LOC buffer.
        {
            Buffer_ID buffer = create_buffer(app, string_u8_litexpr("*loc*"),
                                             BufferCreate_NeverAttachToFile |
                                             BufferCreate_AlwaysNew);
            buffer_set_setting(app, buffer, BufferSetting_Unimportant, true);
        }
    }

    //~ NOTE(rjf): Initialize panels
    {
        Buffer_Identifier left = buffer_identifier(string_u8_litexpr("*scratch*"));
        Buffer_Identifier right = buffer_identifier(string_u8_litexpr("*messages*"));
        Buffer_ID left_id = buffer_identifier_to_id(app, left);
        Buffer_ID right_id = buffer_identifier_to_id(app, right);

        // NOTE(rjf): Left Panel
        View_ID view = get_active_view(app, Access_Always);
        new_view_settings(app, view);
        view_set_buffer(app, view, left_id, 0);

        view_set_active(app, view);

        // NOTE(rjf): Right Panel
        open_panel_vsplit(app);

        View_ID right_view = get_active_view(app, Access_Always);
        view_set_buffer(app, right_view, right_id, 0);

        // NOTE(rjf): Restore Active to Left
        view_set_active(app, view);
    }

    //~ NOTE(rjf): Auto-Load Project.
    {
        b32 auto_load = def_get_config_b32(vars_save_string_lit("automatically_load_project"));
        if (auto_load)
        {
            load_project(app);
        }
    }

    //~ NOTE(rjf): Set misc options.
    {
        global_battery_saver = def_get_config_b32(vars_save_string_lit("f4_battery_saver"));
    }

    //~ NOTE(rjf): Initialize audio.
    {
        def_audio_init();
    }

    //~ NOTE(rjf): Initialize stylish fonts.
    {
        Scratch_Block scratch(app);
        String_Const_u8 bin_path = system_get_path(scratch, SystemPath_Binary);

        // NOTE(rjf): Fallback font.
        Face_ID face_that_should_totally_be_there = get_face_id(app, 0);

        // NOTE(rjf): Title font.
        {
            Face_Description desc = {0};
            {
                desc.font.file_name =  push_u8_stringf(scratch, "%.*sfonts/RobotoCondensed-Regular.ttf", string_expand(bin_path));
                desc.parameters.pt_size = 18;
                desc.parameters.bold = 0;
                desc.parameters.italic = 0;
                desc.parameters.hinting = 0;
            }

            if(IsFileReadable(desc.font.file_name))
            {
                global_styled_title_face = try_create_new_face(app, &desc);
            }
            else
            {
                global_styled_title_face = face_that_should_totally_be_there;
            }
        }

        // NOTE(rjf): Label font.
        {
            Face_Description desc = {0};
            {
                desc.font.file_name =  push_u8_stringf(scratch, "%.*sfonts/RobotoCondensed-Regular.ttf", string_expand(bin_path));
                desc.parameters.pt_size = 10;
                desc.parameters.bold = 1;
                desc.parameters.italic = 1;
                desc.parameters.hinting = 0;
            }

            if(IsFileReadable(desc.font.file_name))
            {
                global_styled_label_face = try_create_new_face(app, &desc);
            }
            else
            {
                global_styled_label_face = face_that_should_totally_be_there;
            }
        }

        // NOTE(rjf): Small code font.
        {
            Face_Description normal_code_desc = get_face_description(app, get_face_id(app, 0));

            Face_Description desc = {0};
            {
                desc.font.file_name =  push_u8_stringf(scratch, "%.*sfonts/Inconsolata-Regular.ttf", string_expand(bin_path));
                desc.parameters.pt_size = normal_code_desc.parameters.pt_size - 1;
                desc.parameters.bold = 1;
                desc.parameters.italic = 1;
                desc.parameters.hinting = 0;
            }

            if(IsFileReadable(desc.font.file_name))
            {
                global_small_code_face = try_create_new_face(app, &desc);
            }
            else
            {
                global_small_code_face = face_that_should_totally_be_there;
            }
        }
    }

    //~ NOTE(rjf): Prep virtual whitespace.
    {
        def_enable_virtual_whitespace = def_get_config_b32(vars_save_string_lit("enable_virtual_whitespace"));
        clear_all_layouts(app);
    }
}
