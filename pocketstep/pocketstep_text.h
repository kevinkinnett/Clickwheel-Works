/*
 * PocketStep Text
 * Measured word wrapping and pagination for constrained C programs.
 *
 * Copyright (C) 2026 Kevin Kinnett
 * SPDX-License-Identifier: MIT
 *
 * Define POCKETSTEP_TEXT_IMPLEMENTATION in one translation unit before
 * including this file.
 */

#ifndef POCKETSTEP_TEXT_H
#define POCKETSTEP_TEXT_H

#define PS_TEXT_INVALID -1

struct ps_text_span
{
    int start;
    int length;
};

typedef int (*ps_text_measure_handler)(const char *text, int length,
                                       void *context);

int ps_text_layout_page(const char *text, int start, int max_width,
                        struct ps_text_span *lines, int line_capacity,
                        ps_text_measure_handler measure, void *context,
                        int *next);
int ps_text_page_count(const char *text, int max_width,
                       struct ps_text_span *scratch, int line_capacity,
                       ps_text_measure_handler measure, void *context);

#endif

#ifdef POCKETSTEP_TEXT_IMPLEMENTATION
#ifndef POCKETSTEP_TEXT_IMPLEMENTATION_ONCE
#define POCKETSTEP_TEXT_IMPLEMENTATION_ONCE

static int ps_text_horizontal_space(char value)
{
    return value == ' ' || value == '\t' || value == '\r';
}

static int ps_text_fits(const char *text, int start, int end,
                        int max_width, ps_text_measure_handler measure,
                        void *context)
{
    return end > start &&
           measure(text + start, end - start, context) <= max_width;
}

int ps_text_layout_page(const char *text, int start, int max_width,
                        struct ps_text_span *lines, int line_capacity,
                        ps_text_measure_handler measure, void *context,
                        int *next)
{
    int position;
    int line_count = 0;

    if (text == 0 || start < 0 || max_width <= 0 || lines == 0 ||
        line_capacity <= 0 || measure == 0 || next == 0)
        return PS_TEXT_INVALID;
    position = start;
    while (ps_text_horizontal_space(text[position]))
        position++;

    while (line_count < line_capacity && text[position] != '\0')
    {
        int line_start = position;
        int best = position;
        int scan = position;

        if (text[position] == '\n')
        {
            lines[line_count].start = position;
            lines[line_count].length = 0;
            line_count++;
            position++;
            continue;
        }

        while (text[scan] != '\0' && text[scan] != '\n')
        {
            int word_end;

            while (ps_text_horizontal_space(text[scan]))
                scan++;
            if (text[scan] == '\0' || text[scan] == '\n')
                break;
            word_end = scan;
            while (text[word_end] != '\0' && text[word_end] != '\n' &&
                   !ps_text_horizontal_space(text[word_end]))
                word_end++;
            if (!ps_text_fits(text, line_start, word_end, max_width,
                              measure, context))
                break;
            best = word_end;
            scan = word_end;
        }

        if (best == line_start)
        {
            best = line_start + 1;
            while (text[best] != '\0' && text[best] != '\n' &&
                   !ps_text_horizontal_space(text[best]) &&
                   ps_text_fits(text, line_start, best + 1, max_width,
                                measure, context))
                best++;
        }
        lines[line_count].start = line_start;
        lines[line_count].length = best - line_start;
        line_count++;
        position = best;
        while (ps_text_horizontal_space(text[position]))
            position++;
        if (text[position] == '\n')
            position++;
    }
    *next = position;
    return line_count;
}

int ps_text_page_count(const char *text, int max_width,
                       struct ps_text_span *scratch, int line_capacity,
                       ps_text_measure_handler measure, void *context)
{
    int pages = 0;
    int position = 0;

    if (text == 0 || text[0] == '\0')
        return 0;
    while (ps_text_horizontal_space(text[position]))
        position++;
    while (text[position] != '\0')
    {
        int next;
        int result = ps_text_layout_page(text, position, max_width,
                                         scratch, line_capacity,
                                         measure, context, &next);

        if (result <= 0 || next <= position)
            return PS_TEXT_INVALID;
        pages++;
        position = next;
    }
    return pages;
}

#endif
#endif
