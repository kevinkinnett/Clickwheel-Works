#include <assert.h>
#include <stdio.h>

#define POCKETSTEP_TEXT_IMPLEMENTATION
#include "../pocketstep_text.h"

static int fixed_measure(const char *text, int length, void *context)
{
    int width = *(const int *)context;
    (void)text;
    return length * width;
}

static void test_measured_wrapping_and_pages(void)
{
    const char *text = "one two three four five six";
    struct ps_text_span lines[2];
    int glyph_width = 2;
    int next;

    assert(ps_text_layout_page(text, 0, 14, lines, 2,
                               fixed_measure, &glyph_width, &next) == 2);
    assert(lines[0].start == 0 && lines[0].length == 7);
    assert(lines[1].start == 8 && lines[1].length == 5);
    assert(next == 14);
    assert(ps_text_page_count(text, 14, lines, 2,
                              fixed_measure, &glyph_width) == 3);
}

static void test_explicit_break_and_long_word(void)
{
    const char *text = "first\nextraordinary";
    struct ps_text_span lines[2];
    int glyph_width = 1;
    int next;

    assert(ps_text_layout_page(text, 0, 5, lines, 2,
                               fixed_measure, &glyph_width, &next) == 2);
    assert(lines[0].length == 5);
    assert(lines[1].start == 6 && lines[1].length == 5);
    assert(next == 11);
    assert(ps_text_page_count(text, 5, lines, 2,
                              fixed_measure, &glyph_width) == 2);
}

static void test_invalid_and_empty_input(void)
{
    struct ps_text_span line;
    int glyph_width = 1;
    int next;

    assert(ps_text_page_count("", 10, &line, 1,
                              fixed_measure, &glyph_width) == 0);
    assert(ps_text_page_count("text   ", 10, &line, 1,
                              fixed_measure, &glyph_width) == 1);
    assert(ps_text_layout_page(0, 0, 10, &line, 1,
                               fixed_measure, &glyph_width, &next) ==
           PS_TEXT_INVALID);
    assert(ps_text_layout_page("text", 0, 0, &line, 1,
                               fixed_measure, &glyph_width, &next) ==
           PS_TEXT_INVALID);
    assert(ps_text_layout_page("text", 0, 10, &line, 0,
                               fixed_measure, &glyph_width, &next) ==
           PS_TEXT_INVALID);
}

int main(void)
{
    test_measured_wrapping_and_pages();
    test_explicit_break_and_long_word();
    test_invalid_and_empty_input();
    puts("PocketStep text tests passed");
    return 0;
}
