#ifndef COLOR_H
#define COLOR_H

#define MAYBE_UNUSED __attribute__((unused))

MAYBE_UNUSED static const char* color_reset = "\x1b[0m";
MAYBE_UNUSED static const char* color_bold = "\x1b[1m";

MAYBE_UNUSED static const char* color_black = "\x1b[30m";
MAYBE_UNUSED static const char* color_red = "\x1b[31m";
MAYBE_UNUSED static const char* color_green = "\x1b[32m";
MAYBE_UNUSED static const char* color_yellow = "\x1b[33m";
MAYBE_UNUSED static const char* color_blue = "\x1b[34m";
MAYBE_UNUSED static const char* color_magenta = "\x1b[35m";
MAYBE_UNUSED static const char* color_cyan = "\x1b[36m";
MAYBE_UNUSED static const char* color_bright_gray = "\x1b[37m";

MAYBE_UNUSED static const char* color_gray = "\x1b[90m";
MAYBE_UNUSED static const char* color_bright_red = "\x1b[91m";
MAYBE_UNUSED static const char* color_bright_green = "\x1b[92m";
MAYBE_UNUSED static const char* color_bright_yellow = "\x1b[93m";
MAYBE_UNUSED static const char* color_bright_blue = "\x1b[94m";
MAYBE_UNUSED static const char* color_bright_magenta = "\x1b[95m";
MAYBE_UNUSED static const char* color_bright_cyan = "\x1b[96m";
MAYBE_UNUSED static const char* color_white = "\x1b[97m";

#endif
