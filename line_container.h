#ifndef _LINE_H_
#define _LINE_H_

#include <stdio.h>
#include <stddef.h>
#include "cvector.h"

typedef cvector_vector_type(char) Line;
typedef cvector_vector_type(Line) LineContainer;

LineContainer line_container_from_file(FILE* file);
void line_container_destroy(LineContainer container);


#endif /* _LINE_H_ */