#ifndef ALLOCATION_DEF_H
#define ALLOCATION_DEF_H

#define memnew(m_Class) new m_Class()

// NULL objects are not being handled to discourage lazy destruction of objects
#define memdelete(m_pointer) m_pointer ? WARN_PRINT("Git API tried to delete a NULL object") : delete m_pointer;

#endif // !ALLOCATION_DEF_H
