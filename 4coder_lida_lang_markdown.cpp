internal F4_LANGUAGE_INDEXFILE(F4_MD_IndexFile)
{
    int scope_nest = 0;
    for(b32 handled = 0; !ctx->done;)
    {
        handled = 0;
        
        Token *name = 0;
        Token *base_type = 0;
        F4_Index_Note *containing_struct = 0;
        F4_Index_Note *note = 0;
        
        if (F4_Index_ParsePattern(ctx, "%t", "#") ||
            F4_Index_ParsePattern(ctx, "%t", "##") ||
            F4_Index_ParsePattern(ctx, "%t", "###")) {
            handled = 1;
            
        }
        
        if(handled == 0)
        {
            F4_Index_ParseCtx_Inc(ctx, 0);
        }
    }
}

internal F4_LANGUAGE_POSCONTEXT(F4_MD_PosContext)
{
    int count = 0;
    F4_Language_PosContextData *first = 0;
    F4_Language_PosContextData *last = 0;
    
    Token_Array tokens = get_token_array_from_buffer(app, buffer);
    Token_Iterator_Array it = token_iterator_pos(0, &tokens, pos);
    
    return first;
}

internal F4_LANGUAGE_HIGHLIGHT(F4_MD_Highlight)
{
}