#include "atoms.h"
#include "atomList.h"
#include "errors.h"

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
    char* atomName = getAtomName(atom);
    if (atomName == NULL) {
        handleError(0, display, NULL, 0, BadAtom, XCB_GET_ATOM_NAME, 0);
        return NULL;
    }
    return atomName;
}

Atom internalInternAtom(char* atomName) {
    fprintf(stderr, "Intern Atom %s.\n", atomName);
    AtomStruct* atomStruct = getAtomStructByName(atomName);
    if (atomStruct != NULL) {
        fprintf(stderr, "Atom allreaddy existed %lu.\n", atomStruct->atom);
        free(atomName);
        return atomStruct->atom;
    } else {
        fprintf(stderr, "Creating new Atom %lu.\n", lastUsedAtom + 1);
        AtomStruct *atomStruct = malloc(sizeof(AtomStruct));
        if (atomStruct == NULL) {
            return None;
        }
        atomStruct->atom = ++lastUsedAtom;
        atomStruct->name = atomName;
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
    fprintf(stderr, "Intern Atom %s.\n", atom_name);
    display->request++;
    if (0 /* atom_name in predefined names */) {
        fprintf(stderr, "Atom allreaddy existed %lu.\n", lastUsedAtom);
        // TODO: Do this
    }
    AtomStruct* atomStruct = getAtomStructByName(atom_name);
    if (atomStruct != NULL) {
        fprintf(stderr, "Atom allreaddy existed %lu.\n", atomStruct->atom);
        return atomStruct->atom;
    } else if (!only_if_exists) {
        fprintf(stderr, "Creating new Atom %lu.\n", lastUsedAtom + 1);
        AtomStruct *atomStruct = malloc(sizeof(AtomStruct));
        if (atomStruct == NULL) {
            handleError(0, display, NULL, 0, BadAlloc, XCB_INTERN_ATOM, 0);
            return None;
        }
        atomStruct->atom = ++lastUsedAtom;
        atomStruct->name = atom_name;
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
