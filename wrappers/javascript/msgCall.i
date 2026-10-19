/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2026 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Messages dispatcher and builder classes
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2026 Erez Geva
 */

%runtime %{ /* TODO-msgCall runtime */ %}
%header  %{ /* TODO-msgCall header  */ %}

/*
MessageDispatcher;
>>  DOMAIN_h(Message msg, DOMAIN_t tlv, char *idStr)
>>  void noTlv(const Message &msg)
>>  void noTlvCallBack(const Message &msg, const char *idStr)
--  void callHadler(Message msg)
--  void callHadler(Message msg, mng_vals_e tlv_id, BaseMngTlv *tlv)
MessageBuilder;
>>  bool DOMAIN_b(Message msg, DOMAIN_t tlv)
--  MessageBuilder(Message msg)
--  Message getMsg()
--  void clear()
--  bool buildTlv(actionField_e actionField, mng_vals_e tlv_id)
*/

/* Apple JavaScriptCore from WebKit
 * https://developer.apple.com/documentation/javascriptcore/c-javascriptcore-api
 */
#ifdef SWIG_JAVASCRIPT_JSC

/*

JSStringRef JSStringCreateWithUTF8CString(char *)
JSStringRelease(jstr)
JSValueRef JSEvaluateScript(context, _ script: JSStringRef!, _ thisObject: JSObjectRef!, _ sourceURL: JSStringRef!, _ startingLineNumber: Int32, _ exception: UnsafeMutablePointer<JSValueRef?>!)
if(jsResult == nullptr && exception != nullptr) {
     printErr(exception);

#define SWIG_Error(code, msg)     SWIG_JSC_exception(context, exception, code, msg)
#define SWIG_exception(code, msg) do { SWIG_JSC_exception(context, exception, code, msg); SWIG_fail; } while (0)
#define SWIG_exception_fail(code, msg) do { SWIG_Error(code, msg); SWIG_fail; } while(0) 
SWIG_exception_fail(SWIG_ERROR, "")

_exports_ConfigFile_classDefinition.staticFunctions = _exports_ConfigFile_staticFunctions;
_exports_ConfigFile_classDefinition.staticValues = _exports_ConfigFile_staticValues;
_exports_ConfigFile_classDefinition.callAsConstructor = _wrap_new_ConfigFile;
_exports_ConfigFile_objectDefinition.finalize = _wrap_delete_ConfigFile;
_exports_ConfigFile_objectDefinition.staticValues = _exports_ConfigFile_values;
_exports_ConfigFile_objectDefinition.staticFunctions = _exports_ConfigFile_functions;
_exports_ConfigFile_objectDefinition.parentClass = _SwigObject_classRef;

_exports_ConfigFile_classRef = JSClassCreate(&_exports_ConfigFile_objectDefinition);
SWIGTYPE_p_ConfigFile->clientdata = _exports_ConfigFile_classRef;

JS_registerClass(context, exports_object, "ConfigFile", &_exports_ConfigFile_classDefinition);
SWIGINTERN bool JS_registerClass(JSGlobalContextRef context, JSObjectRef parentObject,
                             const char* className, JSClassDefinition* definition) {

https://developer.apple.com/documentation/javascriptcore/jsclassdefinition





// 1. Evaluate the script to get the result value
JSValueRef scriptResult = JSEvaluateScript(context, scriptString, NULL, NULL, 1, NULL);

// 2. Create an instance of your custom JavaScriptCore class
JSClassDefinition def = kJSClassDefinitionEmpty;
def.className = "MyCustomClass";
JSClassRef myClass = JSClassCreate(&def);
JSObjectRef myObject = JSObjectMake(context, myClass, NULL);

// 3. Attach the script result to your object as a property
JSStringRef propertyName = JSStringCreateWithUTF8CString("scriptData");
JSObjectSetProperty(context, myObject, propertyName, scriptResult, kJSPropertyAttributeNone, NULL);

// Clean up
JSStringRelease(propertyName);
JSClassRelease(myClass);



// 1. Define the class and its constructor/callbacks
JSClassDefinition def = kJSClassDefinitionEmpty;
def.className = "MyClass";
JSClassRef classRef = JSClassCreate(&def);

// 2. Make the class constructor object
JSObjectRef constructor = JSObjectMakeConstructor(context, classRef, MyClassConstructorCallback);

// 3. Set it on the global object so the script can see it
JSObjectRef globalObject = JSContextGetGlobalObject(context);
JSStringRef name = JSStringCreateWithUTF8CString("MyClass");
JSObjectSetProperty(context, globalObject, name, constructor, kJSPropertyAttributeNone, NULL);

// 4. Now evaluate a script that uses "MyClass"
JSStringRef script = JSStringCreateWithUTF8CString("const instance = new MyClass(); instance;");
JSValueRef result = JSEvaluateScript(context, script, NULL, NULL, 1, NULL);

// Clean up
JSStringRelease(name);
JSStringRelease(script);
JSClassRelease(classRef);


*/


%wrapper %{ /* TODO-msgCall wrapper */ %}
%init    %{ /* TODO-msgCall init    */ %}
#endif

/* Google Chromium V8, used by Node.js
 * https://v8.dev/
 */
#ifdef SWIG_JAVASCRIPT_V8
// void example_initialize(v8::Handle<v8::Object> exports)

/*

    // Create a string containing the JavaScript source code.
    Local<String> source = String::NewFromUtf8(isolate, "'Hello' + ', World!'");

    // Compile the source code.
    Local<Script> script = Script::Compile(source);

    // Run the script to get the result.
    Local<Value> result = script->Run();

*/


%wrapper %{ /* TODO-msgCall wrapper */ %}
%init    %{ /* TODO-msgCall init    */ %}
#endif

/* Node.js Node-API
 * https://nodejs.org/api/n-api.html#node-api
 */
#ifdef SWIG_JAVASCRIPT_NAPI


/*
Napi::Value ExecuteJavaScript(Napi::Env env) {
    std::string source_code = "const a = 5; const b = 10; a + b;";

    // Evaluate the script directly
    Napi::Value result = env.RunScript(source_code);

    return result; // Returns a Napi::Value wrapping the number 15
}
*/


%wrapper %{ /* TODO-msgCall wrapper */ %}
%init    %{ /* TODO-msgCall init    */ %}
#endif

/* vim: set filetype=swig: */
