#include <assert.h>
#include "atoms.h"
#include "atomList.h"
#include "errors.h"
#include "display.h"

AtomStruct* atomStorageStart = NULL;
AtomStruct* atomStorageLast = NULL;
static Atom lastUsedAtom = _NET_LAST_PREDEFINED;
AtomStruct preDefAtomStructResult;

AtomStruct* getAtomStruct(Atom atom) {
    AtomStruct* atomStruct = atomStorageStart;
    while (atomStruct != NULL) {
        if (atomStruct->atom == atom) { return atomStruct; }
        atomStruct = atomStruct->next;
    }
    return NULL;
}

AtomStruct* getAtomStructByName(const char* name) {
    int i;
    for (i = 0; i < PREDEFINED_ATOM_LIST_SIZE; i++) {
        if (strcmp(PredefinedAtomList[i].name, name) == 0) {
            preDefAtomStructResult.atom = PredefinedAtomList[i].atom;
            preDefAtomStructResult.name = PredefinedAtomList[i].name;
            return &preDefAtomStructResult;
        }
    }
    AtomStruct* atomStruct = atomStorageStart;
    while (atomStruct != NULL) {
        if (strcmp(atomStruct->name, name) == 0) { return atomStruct; }
        atomStruct = atomStruct->next;
    }
    return NULL;
}

Bool isValidAtom(Atom atom) {
    if (atom <= _NET_LAST_PREDEFINED) { return True; }
    return getAtomStruct(atom) != NULL;
}

char* getAtomName(Atom atom) {
    if (atom <= _NET_LAST_PREDEFINED) {
        int i;
        for (i = 0; i < PREDEFINED_ATOM_LIST_SIZE; i++) {
            if (PredefinedAtomList[i].atom == atom) {
                return (char*) PredefinedAtomList[i].name;
            }
        }
    }
    AtomStruct* atomStruct = getAtomStruct(atom);
    if (atomStruct == NULL) {
        return NULL;
    }
    return (char *) atomStruct->name;
}

char* XGetAtomName(Display* display, Atom atom) {
    // https://tronche.com/gui/x/xlib/window-information/XGetAtomName.html
    SET_X_SERVER_REQUEST(display, XCB_GET_ATOM_NAME);
    char* atomName = getAtomName(atom);
    if (atomName == NULL) {
        handleError(0, display, NULL, 0, BadAtom, 0);
        return NULL;
    }
    return atomName;
}

Atom internalInternAtom(char* atomName) {
    fprintf(stderr, "Intern Atom %s.\n", atomName);
    AtomStruct* atomStruct = getAtomStructByName(atomName);
    if (atomStruct != NULL) {
        fprintf(stderr, "Atom already existed %lu.\n", atomStruct->atom);
        return atomStruct->atom;
    } else {
        fprintf(stderr, "Creating new Atom %s (%lu).\n", atomName, lastUsedAtom + 1);
        atomStruct = malloc(sizeof(AtomStruct));
        if (atomStruct == NULL) {
            return None;
        }
        atomStruct->name = strdup(atomName);
        if (atomStruct->name == NULL) {
            free(atomStruct);
            return None;
        }
        atomStruct->atom = ++lastUsedAtom;
        atomStruct->next = NULL;
        if (atomStorageLast == NULL) {
            atomStorageStart = atomStruct;
        } else {
            atomStorageLast->next = atomStruct;
        }
        atomStorageLast = atomStruct;
        return atomStruct->atom;
    }
}

Atom XInternAtom(Display* display, _Xconst char* atom_name, Bool only_if_exists) {
    // https://tronche.com/gui/x/xlib/window-information/XInternAtom.html
    SET_X_SERVER_REQUEST(display, XCB_INTERN_ATOM);
    fprintf(stderr, "Intern Atom %s.\n", atom_name);
    int preExistingIndex = -1;
    if (strncmp(atom_name, "XA_", 3) == 0) {
        int i = 0;
        do {
            if (strcmp(&PredefinedAtomList[i].name[4], &atom_name[4]) == 0) {
                preExistingIndex = i;
                break;
            }
        } while (PredefinedAtomList[i++].atom != XA_WM_TRANSIENT_FOR);
    } else if (strncmp(atom_name, "NET_", 4) == 0) {
        int i = 78;
        assert(PredefinedAtomList[i].atom == _NET_WM_NAME);
        do {
            if (strcmp(&PredefinedAtomList[i].name[5], &atom_name[5]) == 0) {
                preExistingIndex = i;
                break;
            }
        } while (PredefinedAtomList[i++].atom != _NET_FRAME_EXTENTS);
    }
    if (preExistingIndex >= 0) {
        fprintf(stderr, "Atom already existed %lu.\n", lastUsedAtom);
        return PredefinedAtomList[preExistingIndex].atom;
    }
    AtomStruct* atomStruct = getAtomStructByName(atom_name);
    if (atomStruct != NULL) {
        fprintf(stderr, "Atom already existed %lu.\n", atomStruct->atom);
        return atomStruct->atom;
    } else if (!only_if_exists) {
        fprintf(stderr, "Creating new Atom %lu.\n", lastUsedAtom + 1);
        atomStruct = malloc(sizeof(AtomStruct));
        if (atomStruct == NULL) {
            handleError(0, display, NULL, 0, BadAlloc, 0);
            return None;
        }
        atomStruct->name = strdup(atom_name);
        if (atomStruct->name == NULL) {
            free(atomStruct);
            handleError(0, display, NULL, 0, BadAlloc, 0);
            return None;
        }
        atomStruct->atom = ++lastUsedAtom;
        atomStruct->next = NULL;
        if (atomStorageLast == NULL) {
            atomStorageStart = atomStruct;
        } else {
            atomStorageLast->next = atomStruct;
        }
        atomStorageLast = atomStruct;
        return atomStruct->atom;
    }
    return None;
}
