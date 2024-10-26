CUSTOM_COMMAND_SIG(keyboard_macro_toggle_recording)
CUSTOM_DOC("Start recording kmacro if not recording. Stop recording if recording.")
{
    if (global_keyboard_macro_is_recording) {
        Buffer_ID buffer = get_keyboard_log_buffer(app);
        global_keyboard_macro_is_recording = false;
        i64 end = buffer_get_size(app, buffer);
        Buffer_Cursor cursor = buffer_compute_cursor(app, buffer, seek_pos(end));
        Buffer_Cursor back_cursor = buffer_compute_cursor(app, buffer, seek_line_col(cursor.line - 1, 1));
        global_keyboard_macro_range.one_past_last = back_cursor.pos;
    } else {
        Buffer_ID buffer = get_keyboard_log_buffer(app);
        global_keyboard_macro_is_recording = true;
        global_keyboard_macro_range.first = buffer_get_size(app, buffer);
    }
}