require "test/unit"
require "skypekit"


def cb
  puts "A1"
  return 0
end

class TestSkypekit < Test::Unit::TestCase
  
  def test_hello_world
    puts "initialize"
    skype = Skypekit.new('/home/leo/skype/sslkeys/skype.pem', '127.0.0.1', 8963, '/tmp/skype.log')
    puts "login"
    skype.login("test4monkey", "test4monkey")
    
    #skype.messages_listener(:cb)
    
    conversations = skype.conversations
    puts conversations.inspect
    conversations.each do |c|
      puts c.name
      puts c.oid
      puts c.hash
    end
    puts "Select conversation"
    skype.set_conversation(conversations[0])
    conversation = Skypekit::Conversation.oid(conversations[0].oid)
    puts conversation.hash
    puts "Send message"
    skype.send_message("I am Message!!!")
    skype.send_message_by_conversation(conversation, "Hello from RW!!!")
    sleep 3
    puts "logout"
    skype.logout
    assert_equal 'hello world', skype.hello_world
  end
end
