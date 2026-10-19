/* SPDX-License-Identifier: GPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2026 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief JSC tool to run script with loading library
 *
 * Tool to run JavaScript with JavaScriptCore
 * Use the 'require' function to load SWIG shared library.
 * Similar to Node.js way to load SWIG shared library.
 *
 * The tool is used for testing only.
 * This is NOT part of production!
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2026 Erez Geva
 *
 * @note Apple JavaScriptCore Framework
 *       The JavaScript engine of WebKit and Bun
 *       https://developer.apple.com/documentation/javascriptcore
 */
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <climits>
#include <csignal>
#include <list>
#include <stack>
#include <string>
#include <memory>
#include <unistd.h>
#include <dlfcn.h>
#include <libgen.h>
#include <JavaScriptCore/JavaScript.h>

typedef bool (*JSCinit)(JSGlobalContextRef, JSObjectRef *);
/* From JavaScriptCore/JSObjectRef.h
 * typedef JSValueRef (*JSObjectCallAsFunctionCallback)
 *  (JSContextRef context, JSObjectRef function, JSObjectRef thisObject,
 *   size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);
 */

// Keep script path
std::stack<std::string> scriptPaths;
char *startPath = nullptr; // Starting working directory

static bool isNull(JSContextRef context, JSValueRef ref)
{
    return ref == nullptr || JSValueIsUndefined(context, ref) ||
        JSValueIsNull(context, ref);
}

// JSC String
class JSCString
{
  private:
    std::string cstr;
    JSStringRef jstr = nullptr;
  public:
    JSCString(const std::string &_string) : cstr(_string),
        jstr(JSStringCreateWithUTF8CString(_string.c_str())) { }
    JSCString(JSContextRef context, JSValueRef ref) {
        if(!isNull(context, ref)) {
            jstr = JSValueToStringCopy(context, ref, nullptr);
            size_t numChars = JSStringGetMaximumUTF8CStringSize(jstr);
            if(numChars > 0) {
                char chars[numChars];
                JSStringGetUTF8CString(jstr, chars, numChars);
                cstr = chars;
            }
        } else
            // Create empty JSC String
            jstr = JSStringCreateWithUTF8CString("");
    }
    ~JSCString() {
        if(jstr != nullptr)
            JSStringRelease(jstr);
    }
    JSStringRef JSString() { return jstr; }
    const std::string &cppString() const { return cstr; }
    const char *cString() const { return cstr.c_str(); }
    bool empty() const { return cstr.empty() || jstr == nullptr; }
};
// Store shared library handler
class Module
{
  private:
    void *handle = nullptr;
    std::string intName;
  public:
    bool open(const std::string &name) {
        std::string libName = name + ".so";
        // Load Shared library relative to current running script directory location
        chdir(scriptPaths.top().c_str());
        handle = dlopen(libName.c_str(), RTLD_LAZY);
        // Return to script executing with starting working directory
        chdir(startPath);
        if(handle != nullptr) {
            const char *p2, *p = name.c_str();
            p2 = strrchr(p, '/');
            intName = p2 == nullptr ? p : p2 + 1;
            intName += "_initialize";
            return true;
        }
        return false;
    }
    JSCinit initFunc() {
        return handle == nullptr ? nullptr :
            reinterpret_cast<JSCinit>(dlsym(handle, intName.c_str()));
    }
    ~Module() {
        if(handle != nullptr)
            dlclose(handle);
    }
};
// Class to release resources
class JSCSclean
{
  public:
    ~JSCSclean();
};
// Store shared library handlers
std::list<std::unique_ptr<Module>> storeModules;
// JSC global context
JSGlobalContextRef globalContext = nullptr;
// signal callback
std::string signal_callback;

static bool pushDir(const char *path)
{
    char *p1 = strdup(path); // Allocate for dirname()
    if(p1 != nullptr) {
        // We store absolute path, to avoid depends on current work directory.
        char buf[PATH_MAX];
        char *p2 = realpath(dirname(p1), buf);
        free(p1);
        if(p2 != nullptr) {
            scriptPaths.push(p2);
            return true;
        }
    }
    return false;
}
static void printErr(JSContextRef context, JSValueRef &exception)
{
    JSCString errMsg(context, exception);
    JSObjectRef errObj = JSValueToObject(context, exception, nullptr);
    if(isNull(context, errObj)) {
        fprintf(stderr, "%s\n", errMsg.cString());
        return;
    }
    JSCString srcKey("sourceURL");
    JSCString srcURL(context, JSObjectGetProperty(context, errObj,
            srcKey.JSString(), nullptr));
    JSCString linkKey("line");
    JSValueRef jsLine = JSObjectGetProperty(context, errObj,
            linkKey.JSString(), nullptr);
    int line = JSValueToNumber(context, jsLine, nullptr);
    fprintf(stderr, "%s:%d:%s\n", srcURL.cString(), line, errMsg.cString());
}
static bool execute(JSContextRef context, const std::string &src,
    const std::string &path)
{
    JSValueRef exception = nullptr;
    JSCString jsSrc(src);
    JSCString jsPath(path);
    JSValueRef jsResult = JSEvaluateScript(context, jsSrc.JSString(),
            nullptr, jsPath.JSString(), 0, &exception);
    if(exception != nullptr) {
        printErr(context, exception);
        return false;
    }
    return true;
}
static void signal_handler(int)
{
    execute(globalContext, signal_callback + "();", "<signal_handler>");
}
static JSObjectRef getObj(JSContextRef context, JSObjectRef parentObj,
    const std::string &name)
{
    JSValueRef exception = nullptr;
    JSCString jname(name);
    JSValueRef jsVal = JSObjectGetProperty(context, parentObj, jname.JSString(),
            &exception);
    if(exception != nullptr) {
        printErr(context, exception);
        return nullptr;
    }
    return JSValueIsObject(context, jsVal) ? (JSObjectRef)jsVal : nullptr;
}
// Feth 'module.exports' object
static inline JSObjectRef getExports(JSContextRef context)
{
    JSObjectRef globalObject = JSContextGetGlobalObject(context);
    JSObjectRef mod = getObj(context, globalObject, "module");
    return mod == nullptr ? nullptr : getObj(context, mod, "exports");
}
static bool load(JSContextRef context, const JSCString &modulePath,
    JSObjectRef &_module)
{
    Module *m = new Module;
    if(m == nullptr) {
        fprintf(stderr, "Fail allocation of a Module\n");
        return false;
    }
    std::unique_ptr<Module> m2(m);
    if(!m->open(modulePath.cppString())) {
        fprintf(stderr, "Could not find module %s: %s\n",
            modulePath.cString(), dlerror());
        return false;
    }
    JSCinit init = m->initFunc();
    if(init == nullptr) {
        printf("Could not find module's initializer function.");
        return false;
    }
    storeModules.push_back(std::move(m2));
    init((JSGlobalContextRef)context, &_module);
    return true;
}
bool readFile(const std::string &path, std::string &source)
{
    FILE *f = fopen(path.c_str(), "r");
    if(f == nullptr) {
        fprintf(stderr, "Unable to open file %s.\n", path.c_str());
        return false;
    }
    const int size = 2048;
    char buf[size];
    while(fgets(buf, size, f) != nullptr)
        source += buf;
    fclose(f);
    return true;
}
static bool loadJs(JSContextRef context, const JSCString &modulePath,
    JSObjectRef &_module)
{
    std::string path = modulePath.cppString() + ".js";
    std::string source;
    // Load new script relative to current running script directory location
    chdir(scriptPaths.top().c_str());
    if(access(path.c_str(), R_OK) != 0 || !readFile(path, source)) {
        // Return to script executing with starting working directory
        chdir(startPath);
        return false;
    }
    // Store new loaded script relative to current running script directory location
    bool usePop = pushDir(modulePath.cString());
    // Return to script executing with starting working directory
    chdir(startPath);
    bool ret = execute(context, source, path);
    if(usePop)
        scriptPaths.pop();
    if(!ret)
        return false;
    // Feth 'module.exports' object
    JSObjectRef exports = getExports(context);
    if(exports == nullptr)
        return false;
    _module = exports;
    return true;
}
// Load shared library or JavaScript module from JavaScript using 'require'.
static JSValueRef require(JSContextRef context, JSObjectRef, JSObjectRef,
    size_t argumentCount, const JSValueRef arguments[], JSValueRef *)
{
    if(argumentCount > 0) {
        // We use only the first argument!
        JSCString modulePath(context, arguments[0]);
        JSObjectRef _module;
        if(loadJs(context, modulePath, _module) ||
            load(context, modulePath, _module))
            return _module;
    }
    fprintf(stderr, "Ooops.\n");
    return JSValueMakeUndefined(context);
}
// JavaScript 'console.log()'
static JSValueRef print(JSContextRef context, JSObjectRef, JSObjectRef,
    size_t argumentCount, const JSValueRef arguments[], JSValueRef *)
{
    if(argumentCount > 0) {
        // We print just the first argument, which is sufficient for testing!
        JSCString messega(context, arguments[0]);
        printf("%s\n", messega.cString());
    }
    return JSValueMakeUndefined(context);
}
// JavaScript 'console.error()'
static JSValueRef printe(JSContextRef context, JSObjectRef, JSObjectRef,
    size_t argumentCount, const JSValueRef arguments[], JSValueRef *)
{
    if(argumentCount > 0) {
        // We print just the first argument, which is sufficient for testing!
        JSCString messega(context, arguments[0]);
        fprintf(stderr, "%s\n", messega.cString());
    }
    return JSValueMakeUndefined(context);
}
static JSValueRef read_(JSContextRef context, JSObjectRef, JSObjectRef,
    size_t argumentCount, const JSValueRef arguments[], JSValueRef *)
{
    if(argumentCount > 0) {
        JSCString messega(context, arguments[0]);
        printf("%s", messega.cString());
    }
    char ret[512];
    fgets(ret, sizeof ret, stdin);
    JSCString jret(ret);
    return JSValueMakeString(context, jret.JSString());
}
static JSValueRef sleep_(JSContextRef context, JSObjectRef, JSObjectRef,
    size_t argumentCount, const JSValueRef arguments[], JSValueRef *)
{
    double seconds = 0;
    if(argumentCount > 0)
        seconds = JSValueToNumber(context, arguments[0], nullptr);
    if(seconds > 0)
        usleep(seconds * 1000000); // Seconds to microseconds
    return JSValueMakeUndefined(context);
}
static JSValueRef signal_(JSContextRef context, JSObjectRef, JSObjectRef,
    size_t argumentCount, const JSValueRef arguments[], JSValueRef *)
{
    if(argumentCount == 2) {
        JSCString name(context, arguments[0]);
        JSCString callback(context, arguments[1]);
        int signum;
        if(name.cppString() == "SIGINT")
            signum = SIGINT;
        else if(name.cppString() == "SIGTERM")
            signum = SIGTERM;
        else if(name.cppString() == "SIGHUP")
            signum = SIGHUP;
        else
            return JSValueMakeUndefined(context);
        // We keep only the last callback!
        signal_callback = callback.cppString();
        signal(signum, signal_handler);
    }
    return JSValueMakeUndefined(context);
}
// Return process identification
static JSValueRef getpid_(JSContextRef context, JSObjectRef, JSObjectRef,
    size_t, const JSValueRef [], JSValueRef *)
{
    return JSValueMakeNumber(context, getpid());
}
static void regFunc(JSContextRef context, JSObjectRef &object,
    const std::string &name, JSObjectCallAsFunctionCallback callback)
{
    JSCString fName(name);
    JSObjectRef cb = JSObjectMakeFunctionWithCallback(context,
            fName.JSString(), callback);
    JSObjectSetProperty(context, object, fName.JSString(), cb,
        kJSPropertyAttributeNone, nullptr);
}
static bool run(int argc, char *argv[])
{
    const char *path = argv[1];
    globalContext = JSGlobalContextCreate(nullptr);
    if(globalContext == nullptr)
        return false;
    JSObjectRef globalObject = JSContextGetGlobalObject(globalContext);
    regFunc(globalContext, globalObject, "require", require);
    regFunc(globalContext, globalObject, "print", print);
    regFunc(globalContext, globalObject, "printe", printe);
    regFunc(globalContext, globalObject, "read", read_);
    regFunc(globalContext, globalObject, "sleep", sleep_);
    regFunc(globalContext, globalObject, "fsignal", signal_);
    regFunc(globalContext, globalObject, "getpid", getpid_);
    // Pass command line arguments to script
    std::string args = "const args = [";
    for(int i = 1; i < argc; i++) {
        if(i > 1)
            args += ",";
        args += '"';
        args += argv[i];
        args += '"';
    }
    args += "];";
    /*
     * Pass the ADD_PATH envirounment to script.
     * The script may use it for require of modules, script or shared library.
     */
    const char *add_path = getenv("ADD_PATH");
    if(add_path != nullptr && strlen(add_path) > 0) {
        args += "ADD_PATH='";
        args += add_path;
        args += "';";
    }
    /*
     * Assign 'console.log()' and 'console.error()'.
     * Add the 'module' object, so we can assign 'module.exports'.
     * Bridge 'signal()' so we get the callback name, we can later call it!
     */
    if(!execute(globalContext,
            args + "var console={};console.log=print;console.error=printe;" +
            "var module={};const signal=function(n,c){fsignal(n,c.name);};",
            "<console>"))
        return false;
    // Read main script
    std::string source;
    if(access(path, R_OK) == 0) {
        if(!readFile(path, source))
            return false;
    } else {
        fprintf(stderr, "File %s does not exist.\n", path);
        return false;
    }
    // Memory release on ~JSCSclean()
    startPath = get_current_dir_name();
    if(startPath == nullptr || !pushDir(path)) {
        fprintf(stderr, "Fail keep main script path.\n");
        return false;
    }
    return execute(globalContext, source, path);
}
JSCSclean::~JSCSclean()
{
    if(globalContext != nullptr)
        JSGlobalContextRelease(globalContext);
    storeModules.clear();
    free(startPath);
}
int main(int argc, char *argv[])
{
    if(argc > 1) {
        JSCSclean resources;
        if(!run(argc, argv)) {
            fprintf(stderr, "FAIL: Error during execution of script.\n");
            return -1;
        }
    }
    return 0;
}
