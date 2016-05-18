#include "inputMethod.h"
#include <stdarg.h>
#include "X11/keysym.h"

// http://www.x.org/archive/X11R7.6/doc/man/man3/XOpenIM.3.xhtml
// http://www.x.org/archive/X11R7.6/doc/man/man3/XCreateIC.3.xhtml

static char* currLocaleModifierList = defaultLocaleModifierList;
char* pendingText = NULL;

void inputMethodSetCurrentText(char* text) {
    pendingText = text;
}

KeySym getKeySymForChar(char c) {
    const static struct {
        char character;
        KeySym keySym;
    } charMapping[] = {
        { ' ', XK_space },
        { '\n', XK_Return },
        { '\r', XK_Linefeed },
        { '\t', XK_Tab },
        { '\b', XK_BackSpace },
        { '-', XK_minus },
        { '+', XK_plus },
        { '#', XK_numbersign },
        { '*', XK_multiply },
        { '~', XK_asciitilde },
        { '\'', XK_quoteright },
        { '"', XK_quotedbl },
        { '!', XK_exclam },
        { '@', XK_at },
        { '%', XK_percent },
        { '&', XK_ampersand },
        { '$', XK_dollar },
        { '/', XK_slash },
        { '(', XK_parenleft },
        { ')', XK_parenright },
        { '[', XK_bracketleft },
        { ']', XK_bracketright },
        { '{', XK_braceleft },
        { '}', XK_braceright },
        { '?', XK_question },
        { '\\', XK_backslash },
        { '<', XK_less },
        { '>', XK_greater },
        { '|', XK_bar },
        { '_', XK_underscore },
        { '^', XK_asciicircum },
        { ',', XK_comma },
        { ';', XK_semicolon },
        { '.', XK_period },
        { ':', XK_colon },
        { '=', XK_equal },
        { '`', XK_quoteleft },
    };
    if (c >= 'a' && c <= 'z') {
        return XK_a + (c - 'a');
    } else if (c >= 'A' && c <= 'Z') {
        return XK_A + (c - 'A');
    } else if (c >= '0' && c <= '9') {
        return XK_0 + (c - '0');
    }
    int i;
    for (i = 0; i < sizeof(charMapping) / sizeof(charMapping[0]); i++) {
        if (charMapping[i].character == c) {
            return charMapping[i].keySym;
        }
    }
    fprintf(stderr, "Did not Found mapping for char '%c' (%d)\n", c, c); 
    return NoSymbol;
}

Status XCloseIM(XIM inputMethod) {
    return 0;
}

char* XSetLocaleModifiers(char* modifier_list) {
    // http://www.x.org/archive/X11R7.6/doc/man/man3/XSetLocaleModifiers.3.xhtml
    Bool valid = False;
    if (modifier_list == NULL || modifier_list[0] == '\0') {
        return currLocaleModifierList;
    } else if (modifier_list[0] == '@' && modifier_list[1] == 'i' &&
        modifier_list[2] == 'm' && modifier_list[3] == '=') { /* @im= */
        // TODO: What to do with that?
        valid = True;
    } else if (strcmp(currLocaleModifierList, defaultLocaleModifierList) == 0) {
        valid = True;
    }
    if (valid) {
        char* curr = currLocaleModifierList;
        currLocaleModifierList = strdup(modifier_list);
        if (currLocaleModifierList == NULL) {
            currLocaleModifierList = curr;
            return NULL;
        }
        return curr;
    }
    return NULL;
}

XIM XOpenIM(Display* display, XrmDatabase db, char* res_name, char* res_class) {
    char* envXModifiers = getenv("XMODIFIERS");
    if (envXModifiers != NULL) {        
        XSetLocaleModifiers(envXModifiers);
    }
    return (XIM) display;
}

void XDestroyIC(XIC inputConnection) {
    if (GET_XIC_STRUCT(inputConnection)->inputRect != NULL) {
        free(GET_XIC_STRUCT(inputConnection)->inputRect);
    }
    SDL_StopTextInput();
}

Bool parsePreEditAttributes(XIC inputConnection, XVaNestedList attributes) {
    if (attributes == NULL) return False;
    void** attrs = (void**) attributes;
    int i = 0;
    char* key;
    while ((key = attrs[i++]) != NULL) {
        if (strcmp(key, XNArea) == 0) {
            XRectangle* rect = attrs[i++];
            if (XIMPreeditArea & GET_XIC_STRUCT(inputConnection)->style != 0) {
                SDL_Rect* inputRect;
                if ((inputRect = GET_XIC_STRUCT(inputConnection)->inputRect) == NULL) {
                    inputRect = malloc(sizeof(SDL_Rect));
                    if (inputRect == NULL) return False;
                    GET_XIC_STRUCT(inputConnection)->inputRect = inputRect;
                }
                inputRect->x = rect->x;
                inputRect->y = rect->y;
                inputRect->w = rect->width;
                inputRect->h = rect->height;
                SDL_SetTextInputRect(inputRect);
            }
        /*} else if (strcmp(key, XNAreaNeeded) == 0) {
            */
        } else if (strcmp(key, XNSpotLocation) == 0) {
            XPoint* point = attrs[i++];
            if (XIMPreeditPosition & GET_XIC_STRUCT(inputConnection)->style != 0) {
                SDL_Rect* inputRect;
                if ((inputRect = GET_XIC_STRUCT(inputConnection)->inputRect) == NULL) {
                    inputRect = malloc(sizeof(SDL_Rect));
                    if (inputRect == NULL) return False;
                    GET_XIC_STRUCT(inputConnection)->inputRect = inputRect;
                }
                inputRect->x = point->x;
                inputRect->y = point->y;
                inputRect->w = 0;
                inputRect->h = 0;
                SDL_SetTextInputRect(inputRect);
            }
        /*} else if (strcmp(key, XNColormap) == 0 || strcmp(key, XNStdColormap) == 0) {
            
        } else if (strcmp(key, XNForeground) == 0 || strcmp(key, XNBackground) == 0) {
            
        } else if (strcmp(key, XNBackgroundPixmap) == 0) {
            
        */} else if (strcmp(key, XNFontSet) == 0) {
            XFontSet* fontset = attrs[i++]; // TODO
            
        /*} else if (strcmp(key, XNLineSpace) == 0) {
            
        } else if (strcmp(key, XNCursor) == 0) {
            
        } else if (strcmp(key, XNPreeditStartCallback) == 0) {
            
        } else if (strcmp(key, XNPreeditDoneCallback) == 0) {
            
        } else if (strcmp(key, XNPreeditDrawCallback) == 0) {
            
        } else if (strcmp(key, XNPreeditCaretCallback) == 0) {
            
        } else if (strcmp(key, XNStatusStartCallback) == 0) {
            
        } else if (strcmp(key, XNStatusDoneCallback) == 0) {
            
        } else if (strcmp(key, XNStatusDrawCallback) == 0) {
            */
        } else {
            return False;
        }
    }
    return True;
}

Bool fillPreEditAttributes(XIC inputConnection, XVaNestedList returnArgs) {
    if (returnArgs == NULL) return False;
    void** attrs = (void**) returnArgs;
    int i = 0;
    char* key;
    while ((key = attrs[i++]) != NULL) {
        if (strcmp(key, XNArea) == 0) {
            if (GET_XIC_STRUCT(inputConnection)->inputRect == NULL) { return False; }
            XRectangle* rect = attrs[i++];
            rect->x = GET_XIC_STRUCT(inputConnection)->inputRect->x;
            rect->y = GET_XIC_STRUCT(inputConnection)->inputRect->y;
            rect->width = GET_XIC_STRUCT(inputConnection)->inputRect->w;
            rect->height = GET_XIC_STRUCT(inputConnection)->inputRect->h;
        /*} else if (strcmp(key, XNAreaNeeded) == 0) {
            */
        } else if (strcmp(key, XNSpotLocation) == 0) {
            if (GET_XIC_STRUCT(inputConnection)->inputRect == NULL) { return False; }
            XPoint* point = attrs[i++];
            point->x = GET_XIC_STRUCT(inputConnection)->inputRect->x;
            point->y = GET_XIC_STRUCT(inputConnection)->inputRect->y;
        /*} else if (strcmp(key, XNColormap) == 0 || strcmp(key, XNStdColormap) == 0) {
            
        } else if (strcmp(key, XNForeground) == 0 || strcmp(key, XNBackground) == 0) {
            
        } else if (strcmp(key, XNBackgroundPixmap) == 0) {
            
        } else if (strcmp(key, XNFontSet) == 0) {
            
        } else if (strcmp(key, XNLineSpace) == 0) {
            
        } else if (strcmp(key, XNCursor) == 0) {
            
        } else if (strcmp(key, XNPreeditStartCallback) == 0) {
            
        } else if (strcmp(key, XNPreeditDoneCallback) == 0) {
            
        } else if (strcmp(key, XNPreeditDrawCallback) == 0) {
            
        } else if (strcmp(key, XNPreeditCaretCallback) == 0) {
            
        } else if (strcmp(key, XNStatusStartCallback) == 0) {
            
        } else if (strcmp(key, XNStatusDoneCallback) == 0) {
            
        } else if (strcmp(key, XNStatusDrawCallback) == 0) {
            */
        } else {
            return False;
        }
    }
    return True;
}

char* setICValues(XIC inputConnection, va_list arguments, Bool allowSetReadOnly) {
    char* key = NULL;
    while ((key = va_arg(arguments, char*)) != NULL) {
        if (strcmp(key, XNInputStyle) == 0) {
            if (!allowSetReadOnly) {
                break;
            }
            GET_XIC_STRUCT(inputConnection)->style = va_arg(arguments, XIMStyle);
            int i = 0;
            Bool found = False;
            for (i = 0; i < supportedStyles.count_styles; i++) {
                if (supportedStyles.supported_styles[i] == GET_XIC_STRUCT(inputConnection)->style) {
                    found = True;
                    break;
                }
            }
            if (!found) break;
        } else if (strcmp(key, XNClientWindow) == 0) {
            Window focusWindow = va_arg(arguments, Window);
            if (!IS_TYPE(focusWindow, WINDOW) || focusWindow == SCREEN_WINDOW) {
                break;
            }
            while ((focusWindow = GET_PARENT(focusWindow)) != SCREEN_WINDOW) {}
            if (IS_MAPPED_TOP_LEVEL_WINDOW(focusWindow)) {
                SDL_Window* sdlWindow = GET_WINDOW_STRUCT(focusWindow)->sdlWindow;
                SDL_SetKeyboardFocus(sdlWindow);
                if (GET_XIC_STRUCT(inputConnection)->inputRect != NULL) {
                    SDL_SetTextInputRect(GET_XIC_STRUCT(inputConnection)->inputRect);
                }
                SDL_StartTextInput();
            } else {
                // TODO
            }
            GET_XIC_STRUCT(inputConnection)->focus = focusWindow;
        } else if (strcmp(key, XNFocusWindow) == 0) {
            Window focusWindow = va_arg(arguments, Window);    
        } else if (strcmp(key, XNPreeditAttributes) == 0) {
            if (!parsePreEditAttributes(inputConnection, va_arg(arguments, XVaNestedList))) break;
        /*} else if (strcmp(key, XNStatusAttributes) == 0) {
                
        } else if (strcmp(key, XNGeometryCallback) == 0) {
                
        } else if (strcmp(key, XNResourceName) == 0 || strcmp(key, XNResourceClass)) {
             */
        } else {
            break;
        }
    }
    return key;
}

XIC XCreateIC(XIM inputMethod, ...) {
    XIC inputConnection = malloc(sizeof(_XIC));
    if (inputConnection == NULL) {
        // TODO
        return NULL;
    }
    GET_XIC_STRUCT(inputConnection)->display = (Display*) inputMethod;
    GET_XIC_STRUCT(inputConnection)->style = XIMUndefined;
    GET_XIC_STRUCT(inputConnection)->focus = None;
    GET_XIC_STRUCT(inputConnection)->inputRect = NULL;
    GET_XIC_STRUCT(inputConnection)->eventFilter = KeyPressMask | KeyReleaseMask;
    va_list argumentList;
    va_start(argumentList, inputMethod);
    char* key;
    if ((key = setICValues(inputConnection, argumentList, True)) != NULL) {
        // TODO
        fprintf(stderr, "setICValues failed in %s because of key %s!\n", __func__, key);
        free(inputConnection);
        va_end(argumentList);
        return NULL;
    }
    va_end(argumentList);
    if (GET_XIC_STRUCT(inputConnection)->style == XIMUndefined) {
        free(inputConnection);
        return NULL;
    }
    return inputConnection;
}

char* XSetICValues(XIC inputConnection, ...) {
    // http://www.x.org/archive/X11R7.6/doc/man/man3/XGetICValues.3.xhtml
    va_list argumentList;
    va_start(argumentList, inputConnection);
    char* res = setICValues(inputConnection, argumentList, False);
    va_end(argumentList);
    return res;
}

XVaNestedList XVaCreateNestedList(int dummy, ...) {
    // http://www.x.org/archive/X11R7.6/doc/man/man3/XVaCreateNestedList.3.xhtml
    va_list argumentList, argCount;
    int i, nArgs = 0;
    void* item;
    va_start(argumentList, dummy);
    va_copy(argCount, argumentList);
    while (va_arg(argCount, void*) != NULL) { nArgs++; }
    va_end(argCount);
    void** list = malloc(sizeof(void*) * nArgs);
    if (list == NULL) {
        return NULL;
    }
    va_end(argumentList);
    while ((item = va_arg(argumentList, void*)) != NULL) {
        list[i++] = item;
    }
    list[i++] = NULL;
    return list;
}

char* XGetICValues(XIC inputConnection, ...) {
    // http://www.x.org/archive/X11R7.6/doc/man/man3/XGetICValues.3.xhtml
    va_list argumentList;
    va_start(argumentList, inputConnection);
    char* key = NULL;
    while ((key = va_arg(argumentList, char*)) != NULL) {
        if (strcmp(key, XNInputStyle) == 0) {
            XIMStyle* styleReturn = va_arg(argumentList, XIMStyle*);
            *styleReturn = GET_XIC_STRUCT(inputConnection)->style;
        /*} else if (strcmp(key, XNClientWindow) == 0) {
            // TODO
        } else if (strcmp(key, XNFocusWindow) == 0) {
            
        } else if (strcmp(key, XNPreeditAttributes) == 0) {
            if (!fillPreEditAttributes(inputConnection, va_arg(arguments, XVaNestedList))) break;
        } else if (strcmp(key, XNStatusAttributes) == 0) {
                
        } else if (strcmp(key, XNGeometryCallback) == 0) {
                
        } else if (strcmp(key, XNResourceName) == 0 || strcmp(key, XNResourceClass)) {
            */    
        } else {
            break;
        }
    }
    va_end(argumentList);
    return key;
}

void XSetICFocus(XIC inputConnection) {
    // http://www.x.org/archive/X11R7.6/doc/man/man3/XSetICFocus.3.
    // Nothing to do, focus management handled by SDL.
}
	
char* XGetIMValues(XIM inputMethod, ...) {
    va_list argumentList;
    va_start(argumentList, inputMethod);
    char* key;
    while ((key = va_arg(argumentList, char*)) != NULL) {
        if (strcmp(key, XNQueryInputStyle) == 0) {
            XIMStyles** styles = va_arg(argumentList, XIMStyles**);
            if (styles == NULL) {
                break;
            }
            *styles = (XIMStyles*) &supportedStyles;
        } else {
            break;
        }
    }
    va_end(argumentList);
    return key;
}

void XFreeFontSet(Display *display, XFontSet font_set) {
    // http://www.x.org/archive/X11R7.6/doc/man/man3/XCreateFontSet.3.xhtml
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
}

void XFreeStringList(char **list) {
    // http://www.x.org/archive/X11R7.6/doc/man/man3/XFreeStringList.3.xhtml
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
}

XFontSet XCreateFontSet(Display *display, char *base_font_name_list, char ***missing_charset_list_return, int *missing_charset_count_return, char **def_string_return) {
    // http://www.x.org/archive/X11R7.6/doc/man/man3/XCreateFontSet.3.xhtml
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
}

int Xutf8LookupString(XIC inputConnection, XKeyPressedEvent* event, char* buffer_return,
                      int bytes_buffer, KeySym* keysym_return, Status* status_return) {
    // http://www.x.org/archive/X11R7.6/doc/man/man3/Xutf8LookupString.3.xhtml
    if (event->keycode == 0) {
        fprintf(stderr, "InputMethod Event! text = '%s'.\n", pendingText);
        if (pendingText == NULL) {
            *status_return = XLookupNone;
            return 0;
        }
        int textLen = strlen(pendingText) + 1;
        if (textLen > bytes_buffer) {
            *status_return = XBufferOverflow;
            return textLen;
        }
        if (textLen > 0) {
            *status_return = XLookupBoth;
            *keysym_return = getKeySymForChar(pendingText[textLen - 2]);
        } else {
            *status_return = XLookupChars;
        }
        memcpy(buffer_return, pendingText, textLen);
        pendingText = NULL;
        return textLen;
    } else {
        fprintf(stderr, "Normal Event, Keycode = %d, '%c'\n", event->keycode, event->keycode);
        if (event->keycode <= 127) {
            *status_return = XLookupBoth;
            *buffer_return = event->keycode;
        } else {
            *status_return = XLookupKeySym;
        }
        *keysym_return = XKeycodeToKeysym(event->display, event->keycode, 0);
        return 1;
    }
}
