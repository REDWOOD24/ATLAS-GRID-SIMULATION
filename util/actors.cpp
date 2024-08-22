#include "actors.h"

Actors::Actors(sg4::Engine* _e){e = _e;}


void Actors::get_disk_info(const std::string host)
{
 sg4::Actor::create("", e->host_by_name(host), Storage().get_disk_info);
}

void Actors::send_data(const std::string sender, const std::string receiver, int num_of_data_messages, size_t size_of_data_message){

  std::unique_ptr<Data> data = std::make_unique<Data>();
  sg4::Actor::create("sender", e->host_by_name(sender), data->sender, num_of_data_messages, size_of_data_message);
  sg4::Actor::create("receiver", e->host_by_name(receiver), data->receiver);   
}
