#include "line_container.h"
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <limits.h>

static inline
Line __get_line(FILE* file) {
    Line line = NULL;
    int c;

    while ((c = getc(file)) != EOF) {
        if (c == '\n') break;
        cvector_push_back(line, (char)c);
    }

    return line;
}

LineContainer line_container_from_file(FILE* file) {
    if (!file) return NULL;

    LineContainer container = NULL;

    Line line = NULL;

    while (line = __get_line(file)) {
        cvector_push_back(container, line);
    }

    return container;
}

void line_container_destroy(LineContainer container) {
    Line* it;

    for (it = cvector_begin(container); it != cvector_end(container); ++it) {
        cvector_free(*it);
    }

    cvector_free(container);
}
