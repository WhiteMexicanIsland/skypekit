#include "skypekit.h"

using namespace Sid;

typedef VALUE (* ruby_method_vararg)(...);

void Delay(int Sec)
{
  #ifdef _WIN32
    Sleep(Sec * 1000);
  #else
    sleep(Sec);
  #endif
};
/** helper for ssh get key **/
char* getKeyPair (char* keyFileName)
{
  char* keyBuf  = 0; 
  uint  keyLen  = 0;
  FILE* f = 0;
  size_t fsize = 0;
  f = fopen(keyFileName, "r");
  if (f)
  {
    fseek(f, 0, SEEK_END);
    fsize = ftell(f);
    rewind(f);
    keyLen = fsize + 1;
    keyBuf = new char[keyLen];
    size_t read = fread(keyBuf,1,fsize,f);
    if (read != fsize) 
    { 
      printf("Error reading %s\n", keyFileName);
      return NULL;
    };
    keyBuf[fsize]=0; //cert should be null terminated
    fclose(f);
    return keyBuf;    
  };
  printf("Error opening app token file: %s\n", keyFileName);
  return NULL;
};

/** ACCOUNT METHODS **/
MAccount::MAccount(unsigned int oid, SERootObject* root) : Account(oid, root) {};

Account::STATUS MAccount::getStatus()
{
  return (Account::STATUS)GetUintProp(Account::P_STATUS);
}

/** CONVERSATION METHODS **/
MConversation::MConversation(unsigned int oid, SERootObject* rootobj) : Conversation(oid, rootobj){};

/** SKYPE METHODS **/
bool MSkype::loggedIn()
{
  return loginAccount && loginAccount->getStatus() == Account::LOGGED_IN;
}

bool MSkype::loggedOut()
{
  return loginAccount && loginAccount->getStatus() <= Account::LOGGED_OUT_AND_PWD_SAVED;
}

MConversation::Ref MSkype::get_current_conversation()
{
  if (!loggedIn() || !currentConversation) {
    return MConversation::Ref();
  }
  return currentConversation;
} 

void MSkype::set_current_conversation(const Conversation::Ref& c)
{
  currentConversation = c;
}

void MSkype::OnMessage(const Message::Ref& message)
{
        int MessageType = message->GetProp(Message::P_TYPE).toInt();
        SEIntList Props;
        SEIntDict Values;
        Props.append(Message::P_AUTHOR);
        Props.append(Message::P_BODY_XML);
        Values = message->GetProps(Props);
        /*
        if (rb_SkypeObject && rb_SkypeMessagesListener && rb_class_of(rb_SkypeMessagesListener) == rb_cSymbol){
	  rb_funcall(rb_class_of(rb_SkypeMessagesListener), rb_to_id(rb_SkypeMessagesListener), 0);
        } 
        */
};


/** VARIABLES **/
static VALUE rb_Skype;
static VALUE rb_SkypeError;
static VALUE rb_Conversation;

/** WRAPPERS **/
static MSkype *get_skype( VALUE );
static MSkype* 
get_skype(VALUE self)
{
  MSkype *skype;
  Data_Get_Struct(self, MSkype, skype);
  if (!skype) rb_raise(rb_SkypeError, "not connected");
  return skype;
}

static void
free_skype(MSkype *skype)
{
  skype->stop();
  delete skype;
}

static VALUE
skype_alloc(VALUE klass)
{
  return Data_Wrap_Struct(klass, NULL, free_skype, NULL);
}




/** FUNCTIONS **/
static VALUE
skype_initialize(VALUE self, VALUE keypath, VALUE host, VALUE port, VALUE logfile)
{
  MSkype* skype = new MSkype();
  Check_Type(keypath, T_STRING);
  Check_Type(host, T_STRING);
  Check_Type(port, T_FIXNUM);
  Check_Type(logfile, T_STRING);
  char* keyBody = getKeyPair(RSTRING_PTR(keypath));
  if (!keyBody) rb_raise(rb_SkypeError, "not valid ssl key");
  skype->init(keyBody, RSTRING_PTR(host), NUM2INT(port), RSTRING_PTR(logfile));
  skype->start();
  skype->rb_SkypeObject = self;  
  DATA_PTR(self) = skype;
  return self;
}

static VALUE
skype_login(VALUE self, VALUE login, VALUE password)
{
 Check_Type(login, T_STRING);
 Check_Type(password, T_STRING);
 MSkype *skype = get_skype(self);
 if (skype->GetAccount(RSTRING_PTR(login), skype->loginAccount)){
  skype->loginAccount->LoginWithPassword(RSTRING_PTR(password), false, false);
  while (!skype->loggedIn()) { Delay(1); };
 }
 return self;
}

static VALUE
skype_logout(VALUE self)
{
 
 MSkype *skype = get_skype(self); 
 skype->loginAccount->Logout(false);
 while (!skype->loggedOut()) { Delay(1); };
 return self;
}



static VALUE make_conversation_obj(ConversationRef C)
{
    SEIntList Props;
    Props.append(Conversation::P_TYPE);
    Props.append(Conversation::P_DISPLAYNAME);
    Props.append(Conversation::P_IDENTITY);
    
    SEIntDict Values = C->GetProps(Props);
    
    VALUE obj = rb_obj_alloc(rb_Conversation);
    SEString tmp_string = ""; 
    rb_iv_set(obj, "oid", INT2NUM(C->getOID()));
    tmp_string = Values.find(Conversation::P_TYPE);
    rb_iv_set(obj, "type", tmp_string? rb_str_freeze(rb_tainted_str_new2(tmp_string)): Qnil);
    tmp_string = Values.find(Conversation::P_DISPLAYNAME);
    rb_iv_set(obj, "name", tmp_string? rb_str_freeze(rb_tainted_str_new2(tmp_string)): Qnil);
    tmp_string = Values.find(Conversation::P_IDENTITY);
    rb_iv_set(obj, "ident", tmp_string? rb_str_freeze(rb_tainted_str_new2(tmp_string)): Qnil);
    return obj;
}

static VALUE make_conversation_hash(VALUE obj)
{
    VALUE h = rb_hash_new();
    rb_hash_aset(h, rb_str_new2("oid"), rb_iv_get(obj, "oid"));
    rb_hash_aset(h, rb_str_new2("type"), rb_iv_get(obj, "type"));
    rb_hash_aset(h, rb_str_new2("name"), rb_iv_get(obj, "name"));
    rb_hash_aset(h, rb_str_new2("ident"), rb_iv_get(obj, "ident"));
    return h;
}

static VALUE 
get_conversation_by_oid(VALUE self, VALUE oid){
 ConversationRef C(NUM2INT(oid));
 if (C){
  return make_conversation_obj(C);
 } else {
  return Qnil;
 }
}

#define DefineConversationMemberMethods(m)\
static VALUE get_conversation_##m(VALUE obj)\
{return rb_iv_get(obj, #m);}

DefineConversationMemberMethods(oid)
DefineConversationMemberMethods(type)
DefineConversationMemberMethods(name)
DefineConversationMemberMethods(ident)

/*  inspect */
static VALUE conversation_inspect(VALUE obj)
{
    VALUE n = rb_iv_get(obj, "name");
    VALUE s = rb_str_new(0, RSTRING_LEN(n) + 26);
    sprintf(RSTRING_PTR(s), "#<Skypekit::Conversation:%s>", RSTRING_PTR(n));
    return s;
}


static VALUE
skype_get_conversations(VALUE self)
{
 VALUE ret;
 ConversationRef C;

 MSkype *skype = get_skype(self); 
 
 ConversationRefs Inbox = ConversationRefs();
 skype->GetConversationList(Inbox, Conversation::INBOX_CONVERSATIONS);
 int InboxSize = Inbox.size();
 
 ret = rb_ary_new2(InboxSize);
 for (int I = 0; I < InboxSize; I++)
 {
   C = (ConversationRef)Inbox[I];
   rb_ary_store(ret, I, make_conversation_obj(C));
 };
 
 return ret;
}

static VALUE
skype_current_conversation(VALUE self, VALUE conversation)
{
 Check_Type(conversation, T_OBJECT);
 VALUE oid = rb_iv_get(conversation, "oid");
 if (oid) {
   ConversationRef C(NUM2INT(oid));
   if (C){
     MSkype *skype = get_skype(self);
     skype->set_current_conversation(C);
   }
 }
 return self;
}

static bool send_message(ConversationRef C, char* message){
  Message::Ref Reply;
  return C->PostText(message, Reply, false);
}

static VALUE
skype_send_message(VALUE self, VALUE message)
{
  MSkype *skype = get_skype(self);
  ConversationRef C = skype->get_current_conversation();
  VALUE ret = Qfalse;
  if (C){
    if (send_message(C, RSTRING_PTR(message))) ret = Qtrue;
  } else {
    rb_raise(rb_SkypeError, "before use this you need select current conversation");
  }
  return ret;
}

static VALUE
skype_send_message_by_conversation(VALUE self, VALUE conversation, VALUE message)
{
  Check_Type(conversation, T_OBJECT);
  VALUE oid = rb_iv_get(conversation, "oid");
  VALUE ret = Qfalse;
  if (oid) {
    ConversationRef C(NUM2INT(oid));
    if (C){
      if (send_message(C, RSTRING_PTR(message))) ret = Qtrue;
    }
  }
  return ret;
}

static VALUE
skype_messages_listener(VALUE self, VALUE cb) {
  if (rb_class_of(cb) != rb_cSymbol) rb_raise(rb_eTypeError, "Expected Symbol callback");
  
  return Qnil;
}

static VALUE hello_world(VALUE klass)
{
  return rb_str_new2("hello world");
}

extern "C"

void 
Init_skypekit() 
{
  rb_Skype = rb_define_class("Skypekit", rb_cObject);
  rb_SkypeError = rb_define_class_under(rb_Skype, "Error", rb_eStandardError);
  rb_Conversation = rb_define_class_under(rb_Skype, "Conversation", rb_cObject);
  /******     Skype CLASS METHODS     ******/
  rb_define_method(rb_Skype, "hello_world", (ruby_method_vararg)hello_world, 0);
  rb_define_alloc_func(rb_Skype, skype_alloc);
  rb_define_method(rb_Skype, "initialize", (ruby_method_vararg)skype_initialize, 4);
  rb_define_method(rb_Skype, "login", (ruby_method_vararg)skype_login, 2);
  rb_define_method(rb_Skype, "logout", (ruby_method_vararg)skype_logout, 0);
  rb_define_method(rb_Skype, "conversations", (ruby_method_vararg)skype_get_conversations, 0);
  rb_define_method(rb_Skype, "set_conversation", (ruby_method_vararg)skype_current_conversation, 1);
  rb_define_method(rb_Skype, "send_message", (ruby_method_vararg)skype_send_message, 1);
  rb_define_method(rb_Skype, "send_message_by_conversation", (ruby_method_vararg)skype_send_message_by_conversation, 2);
  rb_define_method(rb_Skype, "messages_listener", (ruby_method_vararg)skype_messages_listener, 1);
  /******     Skype Conversation CLASS METHODS     ******/
  rb_define_singleton_method(rb_Conversation, "oid", (ruby_method_vararg)get_conversation_by_oid, 1);
  rb_define_method(rb_Conversation, "oid", (ruby_method_vararg)get_conversation_oid, 0);
  rb_define_method(rb_Conversation, "type", (ruby_method_vararg)get_conversation_type, 0);
  rb_define_method(rb_Conversation, "name", (ruby_method_vararg)get_conversation_name, 0);
  rb_define_method(rb_Conversation, "ident", (ruby_method_vararg)get_conversation_ident, 0);
  rb_define_method(rb_Conversation, "hash", (ruby_method_vararg)make_conversation_hash, 0);
  rb_define_method(rb_Conversation, "inspect", (ruby_method_vararg)conversation_inspect, 0);
}
