#ifndef ALLOCATION_DEF_H
#define ALLOCATION_DEF_H

#define memnew(klass) new klass()
#define memdelete(pointer) pointer ? delete pointer : WARN_PRINT("GitAPI: Tried to delete a NULL object");

#endif // !ALLOCATION_DEF_H
