#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glad/glad.h>

#ifdef __APPLE__
#include <dlfcn.h>
#endif

struct gladGLversionStruct GLVersion = { 0, 0 };

// OpenGL 函數指標定義
PFNGLCLEARPROC glad_glClear = NULL;
PFNGLCLEARCOLORPROC glad_glClearColor = NULL;
PFNGLENABLEPROC glad_glEnable = NULL;
PFNGLDISABLEPROC glad_glDisable = NULL;
PFNGLBLENDFUNCPROC glad_glBlendFunc = NULL;
PFNGLDRAWELEMENTSPROC glad_glDrawElements = NULL;
PFNGLDRAWARRAYSPROC glad_glDrawArrays = NULL;
PFNGLCREATESHADERPROC glad_glCreateShader = NULL;
PFNGLSHADERSOURCEPROC glad_glShaderSource = NULL;
PFNGLCOMPILESHADERPROC glad_glCompileShader = NULL;
PFNGLCREATEPROGRAMPROC glad_glCreateProgram = NULL;
PFNGLATTACHSHADERPROC glad_glAttachShader = NULL;
PFNGLLINKPROGRAMPROC glad_glLinkProgram = NULL;
PFNGLUSEPROGRAMPROC glad_glUseProgram = NULL;
PFNGLGENBUFFERSPROC glad_glGenBuffers = NULL;
PFNGLBINDBUFFERPROC glad_glBindBuffer = NULL;
PFNGLBUFFERDATAPROC glad_glBufferData = NULL;
PFNGLGENVERTEXARRAYSPROC glad_glGenVertexArrays = NULL;
PFNGLBINDVERTEXARRAYPROC glad_glBindVertexArray = NULL;
PFNGLENABLEVERTEXATTRIBARRAYPROC glad_glEnableVertexAttribArray = NULL;
PFNGLVERTEXATTRIBPOINTERPROC glad_glVertexAttribPointer = NULL;
PFNGLGETUNIFORMLOCATIONPROC glad_glGetUniformLocation = NULL;
PFNGLUNIFORMMATRIX4FVPROC glad_glUniformMatrix4fv = NULL;
PFNGLUNIFORM3FVPROC glad_glUniform3fv = NULL;
PFNGLUNIFORM1FPROC glad_glUniform1f = NULL;

static void* get_proc(const char *namez) {
#ifdef __APPLE__
    static void* libGL = NULL;
    if (!libGL) {
        libGL = dlopen("/System/Library/Frameworks/OpenGL.framework/OpenGL", RTLD_LAZY | RTLD_GLOBAL);
    }
    return libGL ? dlsym(libGL, namez) : NULL;
#else
    return NULL;
#endif
}

static int find_extensionsGL(void) {
    return 1;
}

static void find_coreGL(void) {
    // 載入核心 OpenGL 函數
    glad_glClear = (PFNGLCLEARPROC)get_proc("glClear");
    glad_glClearColor = (PFNGLCLEARCOLORPROC)get_proc("glClearColor");
    glad_glEnable = (PFNGLENABLEPROC)get_proc("glEnable");
    glad_glDisable = (PFNGLDISABLEPROC)get_proc("glDisable");
    glad_glBlendFunc = (PFNGLBLENDFUNCPROC)get_proc("glBlendFunc");
    glad_glDrawElements = (PFNGLDRAWELEMENTSPROC)get_proc("glDrawElements");
    glad_glDrawArrays = (PFNGLDRAWARRAYSPROC)get_proc("glDrawArrays");
    glad_glCreateShader = (PFNGLCREATESHADERPROC)get_proc("glCreateShader");
    glad_glShaderSource = (PFNGLSHADERSOURCEPROC)get_proc("glShaderSource");
    glad_glCompileShader = (PFNGLCOMPILESHADERPROC)get_proc("glCompileShader");
    glad_glCreateProgram = (PFNGLCREATEPROGRAMPROC)get_proc("glCreateProgram");
    glad_glAttachShader = (PFNGLATTACHSHADERPROC)get_proc("glAttachShader");
    glad_glLinkProgram = (PFNGLLINKPROGRAMPROC)get_proc("glLinkProgram");
    glad_glUseProgram = (PFNGLUSEPROGRAMPROC)get_proc("glUseProgram");
    glad_glGenBuffers = (PFNGLGENBUFFERSPROC)get_proc("glGenBuffers");
    glad_glBindBuffer = (PFNGLBINDBUFFERPROC)get_proc("glBindBuffer");
    glad_glBufferData = (PFNGLBUFFERDATAPROC)get_proc("glBufferData");
    glad_glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)get_proc("glGenVertexArrays");
    glad_glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)get_proc("glBindVertexArray");
    glad_glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)get_proc("glEnableVertexAttribArray");
    glad_glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)get_proc("glVertexAttribPointer");
    glad_glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)get_proc("glGetUniformLocation");
    glad_glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)get_proc("glUniformMatrix4fv");
    glad_glUniform3fv = (PFNGLUNIFORM3FVPROC)get_proc("glUniform3fv");
    glad_glUniform1f = (PFNGLUNIFORM1FPROC)get_proc("glUniform1f");
}

int gladLoadGLLoader(GLADloadproc load) {
    GLVersion.major = 3;
    GLVersion.minor = 3;
    
    find_coreGL();
    
    if (!find_extensionsGL()) return 0;
    return GLVersion.major != 0 || GLVersion.minor != 0;
}

int gladLoadGL(void) {
    return gladLoadGLLoader((GLADloadproc)get_proc);
}
