#ifndef _ATOMS_H_
#define _ATOMS_H_

#include "X11/Xlib.h"

typedef struct AtomStruct AtomStruct;
struct AtomStruct {
    Atom atom;
    const char* name;
    AtomStruct* next;
};

Bool isValidAtom(Atom atom);
Atom internalInternAtom(char* atomName);

#endif /* _ATOMS_H_ */
