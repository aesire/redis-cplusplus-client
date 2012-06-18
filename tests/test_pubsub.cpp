#include <boost/assign/list_of.hpp>
#include "functions.h"
#include "../redisclient.h"

using namespace boost::assign;


int subscribe_counter = 0;
int unsubscribe_counter = 0;

void test_pubsub(redis::client& c)
{
  struct my_subscriber : redis::client::subscriber
  {
    void subscribe(redis::client& client,
                   const std::string& channel,
                   int subscriptions)
    {
      subscribe_counter++;
      std::cout << "Subscribed to #"<< channel << " ("
                << subscriptions << " subscriptions)" << std::endl;
      
      std::vector<std::string> channels;
      int remaining = client.unsubscribe(channels);
    }

    void message(redis::client& client,
                 const std::string& channel,
                 const std::string& msg)
    {
      std::cout << channel << " :: " << msg << std::endl;
      if (msg == "exit")
      {
        std::vector<std::string> channels = list_of(channel);
        int remaining = client.unsubscribe(channels);
      }
    }

    void unsubscribe(redis::client& client,
                     const std::string& channel,
                     int subscriptions)
    {
      unsubscribe_counter++;
      std::cout << "Unsubscribed from #" << channel << " ("
                << subscriptions << ") subscriptions" << std::endl;
    }
  };
  
  struct my_invalid_subscriber : redis::client::subscriber
  {
    void subscribe(redis::client& client,
                   const std::string& channel,
                   int subscriptions)
    {
      // this command should fail
      client.set("tomorrow", "land");
    }
  };


  my_subscriber subscriber;
  my_invalid_subscriber invalid_subscriber;
  
  std::vector<std::string> channels = list_of("one")("two");


  test("subscribe/unsubscribe");
  {
    c.subscribe(channels, subscriber);
    
    ASSERT_EQUAL(subscribe_counter, 2);
    ASSERT_EQUAL(unsubscribe_counter, 2);
  }
  
  test("invalid command in subscribe");
  {
    try
    {
      c.subscribe(channels, invalid_subscriber);
      FAIL();
    }
    catch (redis::protocol_error& e)
    {
      c.unsubscribe(channels);
    }
    catch (...)
    {
      FAIL();
    }
  }
}
