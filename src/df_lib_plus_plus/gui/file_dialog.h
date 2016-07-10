#pragma once

#include "andy_string.h"
#include "containers/darray.h"

#include <stdlib.h>


DArray<String> FileDialogOpen(char const *initialFolder=NULL);
String FileDialogSave(char const *initialFolder, char const *initialFilename);
