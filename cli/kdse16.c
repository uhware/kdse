/*
 * Copyright (C) 2026 Mark Karaman
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * This file is part of KDSE.
 */

#include "kdse/kdse16_checked.h"
#include "kdse/kdse16_compute.h"

#include <errno.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void usage(FILE *stream)
{
    (void)fprintf(stream,
        "Usage:\n"
        "  kdse16 validate VALUE\n"
        "  kdse16 canonicalize VALUE\n"
        "  kdse16 profile VALUE\n"
        "  kdse16 compute VALUE --input N --threshold T --loss-percent P\n"
        "\n"
        "VALUE accepts decimal, 0x hexadecimal, or 0b binary notation.\n"
        "KDSE-16 bit 15 is unassigned and ignored.\n");
}

static bool parse_binary(const char *text, kdse16_t *value)
{
    uint32_t result = 0u;
    size_t digits = 0u;

    text += 2;
    while (*text != '\0') {
        if (*text != '0' && *text != '1') {
            return false;
        }
        if (++digits > 16u) {
            return false;
        }
        result = (result << 1u) | (uint32_t)(*text - '0');
        ++text;
    }
    if (digits == 0u) {
        return false;
    }
    *value = (kdse16_t)result;
    return true;
}

static bool parse_value(const char *text, kdse16_t *value)
{
    char *end;
    unsigned long result;

    if (text[0] == '0' && (text[1] == 'b' || text[1] == 'B')) {
        return parse_binary(text, value);
    }
    if (text[0] == '-') {
        return false;
    }
    errno = 0;
    end = NULL;
    result = strtoul(text, &end, 0);
    if (errno != 0 || end == text || *end != '\0' || result > UINT16_MAX) {
        return false;
    }
    *value = (kdse16_t)result;
    return true;
}

static bool parse_number(const char *text, double *value)
{
    char *end;
    double result;

    errno = 0;
    end = NULL;
    result = strtod(text, &end);
    if (errno != 0 || end == text || *end != '\0' || !isfinite(result)) {
        return false;
    }
    *value = result;
    return true;
}

static void print_bits(uint16_t payload)
{
    const uint8_t length = kdse16_payload_length(payload);
    uint8_t position;

    if (payload == 0u) {
        (void)putchar('0');
        return;
    }
    for (position = 0u; position < length; ++position) {
        const uint8_t shift = (uint8_t)((unsigned)length -
            (unsigned)position - 1u);
        (void)putchar(((payload >> shift) & UINT16_C(1)) != 0u ? '1' : '0');
    }
}

static void print_identity(kdse16_t value)
{
    const uint16_t payload = kdse16_payload(value);

    (void)printf("container: %" PRIu16 "\n", value);
    (void)printf("unassigned-bit: %u (ignored)\n",
                 (unsigned)((value >> 15u) & UINT16_C(1)));
    (void)printf("payload-decimal: %" PRIu16 "\n", payload);
    (void)fputs("payload-bits: ", stdout);
    print_bits(payload);
    (void)putchar('\n');
}

static int command_validate(kdse16_t value)
{
    const kdse_status_t status = kdse16_validate(value);

    (void)printf("valid: %s\n",
                 status == KDSE_STATUS_OK ? "yes" : "no");
    print_identity(value);
    (void)printf("status: %s\n", kdse_status_string(status));
    return status == KDSE_STATUS_OK ? EXIT_SUCCESS : EXIT_FAILURE;
}

static int command_canonicalize(kdse16_t value)
{
    kdse16_t ordered;
    const kdse_status_t status = kdse16_canonicalize(value, &ordered);

    if (status != KDSE_STATUS_OK) {
        (void)fprintf(stderr, "invalid KDSE-16: %s\n",
                      kdse_status_string(status));
        return EXIT_FAILURE;
    }
    (void)fputs("input-payload: ", stdout);
    print_bits(kdse16_payload(value));
    (void)fputs("\nordered-payload: ", stdout);
    print_bits(ordered);
    (void)printf("\nordered-decimal: %" PRIu16 "\n", ordered);
    return EXIT_SUCCESS;
}

static int command_profile(kdse16_t value)
{
    kdse16_profile_t profile;
    const kdse_status_t status = kdse16_decode_profile(value, &profile);
    uint8_t depth;

    if (status != KDSE_STATUS_OK) {
        (void)fprintf(stderr, "invalid KDSE-16: %s\n",
                      kdse_status_string(status));
        return EXIT_FAILURE;
    }
    print_identity(value);
    (void)printf("payload-length: %" PRIu8 "\n", profile.payload_length);
    (void)printf("branches: %" PRIu8 "\n", profile.branch_count);
    (void)printf("terminals: %" PRIu8 "\n", profile.terminal_count);
    (void)printf("max-depth: %" PRIu8 "\n", profile.max_depth);
    (void)fputs("terminal-depth-profile: [", stdout);
    for (depth = 0u; depth <= profile.max_depth; ++depth) {
        if (depth != 0u) {
            (void)fputs(", ", stdout);
        }
        (void)printf("%" PRIu8, profile.terminals[depth]);
    }
    (void)puts("]");
    return EXIT_SUCCESS;
}

static int command_compute(kdse16_t value, int argc, char **argv)
{
    double input = 0.0;
    double threshold = 0.0;
    double loss_percent = 0.0;
    bool have_input = false;
    bool have_threshold = false;
    bool have_loss = false;
    int index;
    kdse_status_t status;
    double output;

    for (index = 0; index < argc; index += 2) {
        if (index + 1 >= argc) {
            usage(stderr);
            return 2;
        }
        if (strcmp(argv[index], "--input") == 0) {
            have_input = parse_number(argv[index + 1], &input);
            if (!have_input) {
                (void)fputs("invalid --input value\n", stderr);
                return 2;
            }
        } else if (strcmp(argv[index], "--threshold") == 0) {
            have_threshold = parse_number(argv[index + 1], &threshold);
            if (!have_threshold) {
                (void)fputs("invalid --threshold value\n", stderr);
                return 2;
            }
        } else if (strcmp(argv[index], "--loss-percent") == 0) {
            have_loss = parse_number(argv[index + 1], &loss_percent);
            if (!have_loss) {
                (void)fputs("invalid --loss-percent value\n", stderr);
                return 2;
            }
        } else {
            (void)fprintf(stderr, "unknown option: %s\n", argv[index]);
            return 2;
        }
    }

    if (!have_input || !have_threshold || !have_loss) {
        usage(stderr);
        return 2;
    }
    if (input < 0.0 || threshold <= 0.0 ||
        loss_percent < 0.0 || loss_percent >= 100.0) {
        (void)fputs("required ranges: input >= 0, threshold > 0, "
                    "0 <= loss-percent < 100\n", stderr);
        return 2;
    }

    status = kdse16_validate(value);
    if (status != KDSE_STATUS_OK) {
        (void)fprintf(stderr, "invalid KDSE-16: %s\n",
                      kdse_status_string(status));
        return EXIT_FAILURE;
    }

    output = kdse16_compute(value, input, threshold,
                                 loss_percent / 100.0);
    (void)fputs("payload: ", stdout);
    print_bits(kdse16_payload(value));
    (void)printf("\ninput: %.15g\n", input);
    (void)printf("threshold: %.15g\n", threshold);
    (void)printf("branch-loss-percent: %.15g\n", loss_percent);
    (void)printf("output: %.15g\n", output);
    return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
    kdse16_t value;

    if (argc < 2 || strcmp(argv[1], "--help") == 0 ||
        strcmp(argv[1], "help") == 0) {
        usage(argc < 2 ? stderr : stdout);
        return argc < 2 ? 2 : EXIT_SUCCESS;
    }
    if (argc < 3 || !parse_value(argv[2], &value)) {
        (void)fputs("VALUE must be an unsigned 16-bit integer "
                    "(0 through 65535)\n", stderr);
        return 2;
    }

    if (strcmp(argv[1], "validate") == 0 && argc == 3) {
        return command_validate(value);
    }
    if (strcmp(argv[1], "canonicalize") == 0 && argc == 3) {
        return command_canonicalize(value);
    }
    if (strcmp(argv[1], "profile") == 0 && argc == 3) {
        return command_profile(value);
    }
    if (strcmp(argv[1], "compute") == 0) {
        return command_compute(value, argc - 3, &argv[3]);
    }

    usage(stderr);
    return 2;
}
