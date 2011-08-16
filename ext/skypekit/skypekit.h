#ifndef SKYPEKIT_RUBY_H
#define SKYPEKIT_RUBY_H

#include <ruby.h>
#include "skype-embedded_2.h"
#include <unistd.h>


char* getKeyPair (char* keyFileName);
//---------------------------------------------------------------------------------------
// Subclassing Account

class MAccount : public Account
{
public:
    typedef DRef<MAccount, Account> Ref;
    typedef DRefs<MAccount, Account> Refs;

    MAccount(unsigned int oid, SERootObject* root);
    ~MAccount() {};

    //void OnChange(int prop);
    Account::STATUS getStatus();
};

//---------------------------------------------------------------------------------------
// Subclassing Conversation
class MConversation : public Conversation
{
public:
    typedef DRef<MConversation, Conversation> Ref;
    typedef DRefs<MConversation, Conversation> Refs;

    MConversation(unsigned int oid, SERootObject* root);
    ~MConversation() {}
    
    void OnMessage(const Message::Ref& message);
};


//---------------------------------------------------------------------------------------
// Subclassing Skype

class MSkype : public Skype
{
public:
    MSkype() : Skype() {}
    ~MSkype() {}

    // Every time an account object is created, we will return instance of MAccount
    Account* newAccount(int oid) {return new MAccount(oid, this);}
    Conversation* newConversation(int oid)  {return new MConversation(oid, this);}
   
    bool loggedIn();
    bool loggedOut();

    MConversation::Ref get_current_conversation();
    void set_current_conversation(const Conversation::Ref& c);

    MConversation::Refs inbox;
    void OnConversationListChange(const ConversationRef &conversation, const Conversation::LIST_TYPE &type, const bool &added);

public:    
    VALUE rb_SkypeObject;
    MAccount::Ref loginAccount;
    MConversation::Ref currentConversation;
};

#endif
