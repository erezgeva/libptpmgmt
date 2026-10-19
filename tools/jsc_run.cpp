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
#include <cstring>
#include <list>
#include <string>
#include <unistd.h>
#include <dlfcn.h>
#include <memory>
#include <JavaScriptCore/JavaScript.h>

typedef bool (*JSCinit)(JSGlobalContextRef, JSObjectRef *);
/* From JavaScriptCore/JSObjectRef.h
 * typedef JSValueRef (*JSObjectCallAsFunctionCallback)
 *  (JSContextRef context, JSObjectRef function, JSObjectRef thisObject,
 *   size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);
 */

/* JSC String */
class JSCString
{
  private:
    std::string cstr;
    JSStringRef jstr = nullptr;
  public:
    JSCString(const std::string &_string) : cstr(_string),
        jstr(JSStringCreateWithUTF8CString(_string.c_str())) { }
    JSCString(JSContextRef context, JSValueRef ref) {
        jstr = JSValueToStringCopy(context, ref, nullptr);
        size_t numChars = JSStringGetMaximumUTF8CStringSize(jstr);
        char chars[numChars];
        JSStringGetUTF8CString(jstr, chars, numChars);
        cstr = chars;
    }
    ~JSCString() {
        if(jstr != nullptr)
            JSStringRelease(jstr);
    }
    JSStringRef JSString() { return jstr; }
    const std::string &cppString() const { return cstr; }
    const char *cString() const { return cstr.c_str(); }
};
/* Store shared library handler */
class Module
{
  private:
    void *handle = nullptr;
    std::string intName;
  public:
    bool open(const std::string &name0) {
        std::string name = name0;
        size_t node = name.rfind(".node");
        if(node != name.npos)
            name = name.substr(0, node);
        std::string libName = name + ".so";
        handle = dlopen(libName.c_str(), RTLD_LAZY);
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
/* Class to release resources */
class JSCSclean
{
  public:
    ~JSCSclean();
};

/* JSC global context */
JSGlobalContextRef globalContext = nullptr;
/* Store shared library handlers */
std::list<std::unique_ptr<Module>> storeModules;

static bool load(const JSCString &modulePath, JSObjectRef &_module)
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
    init(globalContext, &_module);
    return true;
}
/* Load shared library from JavaScript using 'require'. */
static JSValueRef require(JSContextRef context, JSObjectRef, JSObjectRef,
    size_t argumentCount, const JSValueRef arguments[], JSValueRef *)
{
    if(argumentCount > 0) {
        /* We use only the first argument! */
        JSCString modulePath(context, arguments[0]);
        JSObjectRef _module;
        if(load(modulePath, _module))
            return _module;
    }
    fprintf(stderr, "Ooops.\n");
    return JSValueMakeUndefined(context);
}
/* JavaScript 'console.log()' */
static JSValueRef print(JSContextRef context, JSObjectRef, JSObjectRef,
    size_t argumentCount, const JSValueRef arguments[], JSValueRef *)
{
    if(argumentCount > 0) {
        /* We print just the first argument, which is sufficient for testing! */
        JSCString messega(context, arguments[0]);
        printf("%s\n", messega.cString());
    }
    return JSValueMakeUndefined(context);
}
static void regFunc(JSObjectRef &object, const std::string &name,
    JSObjectCallAsFunctionCallback callback)
{
    JSCString fName(name);
    JSObjectRef cb = JSObjectMakeFunctionWithCallback(globalContext,
            fName.JSString(), callback);
    JSObjectSetProperty(globalContext, object, fName.JSString(), cb,
        kJSPropertyAttributeNone, nullptr);
}
static void printErr(JSValueRef &exception)
{
    JSCString errMsg(globalContext, exception);
    JSObjectRef errObj = JSValueToObject(globalContext, exception, nullptr);
    if(errObj == nullptr) {
        fprintf(stderr, "%s\n", errMsg.cString());
        return;
    }
    JSCString srcKey("sourceURL");
    JSCString srcURL(globalContext, JSObjectGetProperty(globalContext, errObj,
            srcKey.JSString(), nullptr));
    JSCString linkKey("line");
    JSValueRef jsLine = JSObjectGetProperty(globalContext, errObj,
            linkKey.JSString(), nullptr);
    int line = JSValueToNumber(globalContext, jsLine, nullptr);
    fprintf(stderr, "%s:%d:%s\n", srcURL.cString(), line, errMsg.cString());
}
static bool execute(const std::string &src, const std::string &path)
{
    JSValueRef exception;
    JSCString jsSrc(src);
    JSCString jsPath(path);
    JSValueRef jsResult = JSEvaluateScript(globalContext, jsSrc.JSString(), nullptr,
            jsPath.JSString(), 0, &exception);
    if(jsResult == nullptr && exception != nullptr) {
        printErr(exception);
        return false;
    }
    return true;
}
static bool run(int argc, char *argv[])
{
    const char *path = argv[1];
    globalContext = JSGlobalContextCreate(nullptr);
    if(globalContext == nullptr)
        return false;
    JSObjectRef globalObject = JSContextGetGlobalObject(globalContext);
    regFunc(globalObject, "require", require);
    regFunc(globalObject, "print", print);
    std::string args = "const args = [";
    for(int i = 1; i < argc; i++) {
        if(i > 1)
            args += ",";
        args += '"';
        args += argv[i];
        args += '"';
    }
    if(!execute(args + "];const ENGINE = \"jsc\";" +
            "var console = {};console.log = print;", "<console>"))
        return false;
    std::string source;
    if(access(path, R_OK) == 0) {
        FILE *f = fopen(path, "r");
        if(f != nullptr) {
            const int size = 1000;
            char b[size];
            while(fgets(b, size, f) != nullptr)
                source += b;
            fclose(f);
        } else {
            fprintf(stderr, "Unable to open file %s.\n", path);
            return false;
        }
    } else {
        fprintf(stderr, "File %s does not exist.\n", path);
        return false;
    }
    return execute(source, path);
}
JSCSclean::~JSCSclean()
{
    if(globalContext != nullptr)
        JSGlobalContextRelease(globalContext);
    storeModules.clear();
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
