#ifndef GIT_COMMON_H
#define GIT_COMMON_H

#define memnew(m_Class) new m_Class()

// NULL objects are not being handled to discourage lazy destruction of objects
#define memdelete(m_pointer) m_pointer ? WARN_PRINT("Git API tried to delete a NULL object") : delete m_pointer;

#define GIT2_CALL(m_libgit2_function_check, m_fail_return) \
	{                                                      \
		bool res = m_libgit2_function_check;               \
		if (!res) {                                        \
			const git_error *e = giterr_last();            \
			WARN_PRINT(e->message);                        \
			return m_fail_return;                          \
		}                                                  \
	}

#endif // !GIT_COMMON_H
