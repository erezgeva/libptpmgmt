%typemap(in, noblock=1) callHadler1 {
  return MessageDispatcher_callHadler(self, $input, Qnil, Qnil);}
/* we need all 3 argument, so we "steal" 2 more :-) */
%typemap(in, noblock=1) callHadler2 {
  return MessageDispatcher_callHadler(self, $input, argv[1], argv[2]);}
/* we need all 2 argument, so we "steal" 1 more :-) */
%typemap(in, noblock=1) buildMessage {
  return MessageBulder_buildTlv(self, arg1, $input, argv[1]);}
#define APPLY_SWIG_MessageDispatcher1 \
%apply callHadler1 { const Message &msg };
#define APPLY_SWIG_MessageDispatcher2 \
%clear const Message &msg;\
%apply callHadler2 { const Message &msg };
#define CLEAR_SWIG_MessageDispatcher \
%clear const Message &msg;
#define APPLY_SWIG_MessageBulder \
%apply buildMessage { actionField_e actionField };
#define CLEAR_SWIG_MessageBulder \
%clear actionField_e actionField;
%include "msgCall.h"

%header %{
SWIGINTERN VALUE MessageDispatcher_callHadler(VALUE self, VALUE msgVal,
    VALUE argv1, VALUE tlvVal) {
  void *voidMsg = nullptr;
  int ret = SWIG_ConvertPtr(msgVal, &voidMsg, SWIGTYPE_p_Message, 0);
  if(!SWIG_IsOK(ret)) {
    SWIG_Error(SWIG_ArgError(ret), Ruby_Format_TypeError("",
               "Message const &","MessageDispatcher_callHadler", 2, msgVal ));
    return Qnil;
  }
  if(voidMsg == nullptr) {
    SWIG_Error(SWIG_ValueError, Ruby_Format_TypeError("invalid null reference",
               "Message const &", "MessageDispatcher_callHadler", 2, msgVal));
    return Qnil;
  }
  ptpmgmt::Message *msg = (ptpmgmt::Message *)voidMsg;
  if(msg == nullptr) {
    SWIG_Error(SWIG_ValueError, Ruby_Format_TypeError("invalid null reference",
               "Message const &", "MessageDispatcher_callHadler", 2, msgVal));
    return Qnil;
  }
  ptpmgmt::mng_vals_e tlv_id;
  bool newTlvVal = false;
  if (NIL_P(argv1)) {
    tlv_id = msg->getTlvId();
    const ptpmgmt::BaseMngTlv *tlv = msg->getData();
    if(tlv != nullptr) {
      tlvVal = SWIG_NewPointerObj(SWIG_as_voidptr(tlv), SWIGTYPE_p_BaseMngTlv, 0);
      newTlvVal = true;
    } else {
      tlvVal = Qnil;
    }
  } else {
    int intVal;
    ret = SWIG_AsVal_int(argv1, &intVal);
    if(!SWIG_IsOK(ret)) {
      SWIG_Error(SWIG_ArgError(ret), Ruby_Format_TypeError("",
                 "mng_vals_e","MessageDispatcher_callHadler", 3, argv1));
      return Qnil;
    }
    tlv_id = static_cast<ptpmgmt::mng_vals_e>(intVal);
    void *voidptr;
    ret = SWIG_ConvertPtr(tlvVal, &voidptr, SWIGTYPE_p_BaseMngTlv, 0);
    if(!SWIG_IsOK(ret)) {
      SWIG_Error(SWIG_ArgError(ret), Ruby_Format_TypeError("",
                 "BaseMngTlv const *","MessageDispatcher_callHadler", 4, tlvVal));
      return Qnil;
    }
  }
  if(!NIL_P(tlvVal) && msg->isValidId(tlv_id)) {
    std::string idstr = ptpmgmt::Message::mng2str_c(tlv_id);
    std::string callback_name = idstr;
    callback_name += "_h";
    std::string convFunc = "conv_";
    convFunc += idstr;
    /* As we have a TLV, it must have a valid ID and a real conversion
     * function. In case something is wrong, we want Ruby to exit with error! */
    /* Call conv_XXX(tlv) */
    VALUE data = rb_funcall(mPtpmgmt, rb_intern(convFunc.c_str()), 1, tlvVal);
    VALUE idstrVal = SWIG_From_std_string(idstr);
    VALUE argv[3];
    argv[0] = msgVal;
    argv[1] = data;
    argv[2] = idstrVal;
    /* User might not be interesting in a message for this ID.
     * No reason for error, we call noTlvCallBack if it is exist.
     * Check if function exist, call it or return Qundef. */
    /* Call callback_name(msg, tlv, idStr) */
    VALUE r = rb_check_funcall(self, rb_intern(callback_name.c_str()), 3, argv);
    if(r == Qundef) { /* No call back */
      /* argv[0] = msgVal; aleady */
      argv[1] = idstrVal;
      /* Call noTlvCallBack(msg, idStr) */
      rb_check_funcall(self, rb_intern("noTlvCallBack"), 2, argv);
    }
    rb_gc_mark(idstrVal);
    rb_gc_mark(data);
  } else {
    VALUE argv[1];
    argv[0] = msgVal;
    /* noTlv method is optional.
     * So, Check if function exist, call it or return Qundef. */
    /* Call noTlv(msg) */
    rb_check_funcall(self, rb_intern("noTlv"), 1, argv);
  }
  if(newTlvVal)
    rb_gc_mark(tlvVal);
  return Qnil;
}

SWIGINTERN VALUE MessageBulder_buildTlv(VALUE self, MessageBulder *_this,
    VALUE actionVal, VALUE tlv_idVal) {
  int intVal;
  int ret = SWIG_AsVal_int(actionVal, &intVal);
  if (!SWIG_IsOK(ret)) {
    SWIG_Error(SWIG_ArgError(ret), Ruby_Format_TypeError("",
               "actionField_e","MessageBulder_buildTlv", 2, actionVal));
    return Qfalse;
  }
  ptpmgmt::actionField_e actionField = static_cast<ptpmgmt::actionField_e>(intVal);
  ret = SWIG_AsVal_int(tlv_idVal, &intVal);
  if(!SWIG_IsOK(ret)) {
    SWIG_Error(SWIG_ArgError(ret), Ruby_Format_TypeError("",
               "mng_vals_e","MessageBulder_buildTlv", 3, tlv_idVal));
    return Qfalse;
  }
  ptpmgmt::mng_vals_e tlv_id = static_cast<ptpmgmt::mng_vals_e>(intVal);
  ptpmgmt::Message &msg = _this->getMsg();
  bool result = false;
  if(actionField == ptpmgmt::GET || msg.isEmpty(tlv_id)) {
    result = msg.setAction(actionField, tlv_id);
  } else {
    std::string idstr = msg.mng2str_c(tlv_id);
    std::string klass = "Ptpmgmt::";
    klass += idstr;
    klass += "_t";
    std::string callback_name = idstr;
    callback_name += "_b";
    /* Call new XXX_t */
    VALUE tlvVal = rb_class_new_instance(0, nullptr, rb_path2class(klass.c_str()));
    if(TYPE(tlvVal) == T_DATA) {
      VALUE msgVal = SWIG_NewPointerObj(SWIG_as_voidptr(&msg), SWIGTYPE_p_Message, 0);
      VALUE argv[2];
      argv[0] = msgVal;
      argv[1] = tlvVal;
      /* Function return boolean, or Qundef if no such function.
       * Nothing for garbage collector.
       * If user did not define the callback method,
       * this function will return false. User can check :-)
       * We do not consider it as error.  */
      /* Call callback_name(msg, tlv) */
      VALUE r = rb_check_funcall(self, rb_intern(callback_name.c_str()), 2, argv);
      if(r == Qtrue) {
        /* Cast TLV to C++ object, so we can call C++ setAction() */
        void *voidptr;
        ret = SWIG_ConvertPtr(tlvVal, &voidptr, SWIGTYPE_p_BaseMngTlv, 0);
        if (!SWIG_IsOK(ret)) {
          SWIG_Error(SWIG_ArgError(ret), Ruby_Format_TypeError("",
                      "BaseMngTlv *","MessageBulder_buildTlv", 4, tlvVal));
          return Qfalse;
        }
        ptpmgmt::BaseMngTlv *tlv = reinterpret_cast<BaseMngTlv*>(voidptr);
        result = msg.setAction(actionField, tlv_id, tlv);
      }
      rb_gc_mark(msgVal);
    }
    if(result) {
      /* We need to keep it as the Message use it in build().
       * Destructor of this class will remove it from the Message object. */
      rb_iv_set(self, "@usedTlv", tlvVal);
    } else {
      rb_gc_mark(tlvVal);
    }
  }
  return SWIG_From_bool(result);
}
%}
